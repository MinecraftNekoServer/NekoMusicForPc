<template>
  <!-- 桌面歌词组件现在通过独立的Electron窗口显示 -->
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import apiConfig from '../config/apiConfig'

const lyrics = ref([])
const currentTime = ref(0)

// 解析歌词
const parseLyrics = (lyricsText) => {
  if (!lyricsText) return []

  const lines = lyricsText.split('\n')
  const parsed = []

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim()

    if (!line) continue

    // 匹配时间戳歌词行 [mm:ss.xx] 或 [mm:ss.xxx]
    const timeRegex = /\[(\d{1,2}):(\d{1,2})\.(\d{2,3})\]/
    const timeMatch = line.match(timeRegex)

    if (timeMatch) {
      const minutes = parseInt(timeMatch[1])
      const seconds = parseInt(timeMatch[2])
      const milliseconds = parseInt(timeMatch[3])

      let millisecondsDivisor
      if (milliseconds.toString().length === 2) {
        millisecondsDivisor = 100
      } else {
        millisecondsDivisor = 1000
      }
      const timeInSeconds = minutes * 60 + seconds + (milliseconds / millisecondsDivisor)
      const text = line.replace(timeRegex, '').trim()

      // 查找下一行是否有翻译
      let translation = ''
      if (i + 1 < lines.length) {
        const nextLine = lines[i + 1].trim()
        const jsonMatch = nextLine.match(/^\{["\'](.+)["\']\}$/)
        if (jsonMatch) {
          translation = jsonMatch[1]
        }
      }

      parsed.push({
        time: timeInSeconds,
        text: text,
        translation: translation
      })
    }
  }

  parsed.sort((a, b) => a.time - b.time)
  return parsed
}

// 根据当前时间找到对应的歌词
const findCurrentLyric = (time) => {
  if (!lyrics.value || lyrics.value.length === 0) {
    return null
  }

  for (let i = 0; i < lyrics.value.length; i++) {
    if (time < lyrics.value[i].time) {
      // 找到第一个大于当前时间的歌词，返回前一个
      if (i > 0) {
        return lyrics.value[i - 1]
      }
      return null
    }
  }

  // 如果时间大于所有歌词，返回最后一个
  return lyrics.value[lyrics.value.length - 1]
}

// 更新歌词窗口显示
const updateLyricsWindow = () => {
  try {
    if (!window.electronAPI) {
      console.log('DesktopLyrics: electronAPI 不存在，无法更新歌词')
      return
    }

    const lyric = findCurrentLyric(currentTime.value)
    console.log('DesktopLyrics: 当前时间', currentTime.value, '找到歌词:', lyric)
    
    if (lyric) {
      console.log('DesktopLyrics: 发送歌词更新', { lyric: lyric.text, translation: lyric.translation })
      window.electronAPI.updateLyrics({
        lyric: lyric.text,
        translation: lyric.translation
      })
    } else {
      console.log('DesktopLyrics: 没有找到对应歌词')
    }
  } catch (error) {
    console.error('DesktopLyrics: 更新歌词窗口失败:', error)
  }
}

// 监听时间更新，更新当前歌词
let lastUpdateTime = 0
const handleTimeUpdate = (event) => {
  currentTime.value = event.detail.currentTime
  
  // 节流：每100ms最多更新一次（即10fps）
  const now = Date.now()
  if (now - lastUpdateTime < 100) return
  lastUpdateTime = now
  
  updateLyricsWindow()
}

// 监听音乐切换
const handleMusicChanged = async (event) => {
  const music = event.detail
  if (!music) return

  console.log('DesktopLyrics: 音乐切换到', music.title)

  try {
    // 先尝试从内存缓存读取
    if (window.cachedLyrics && window.cachedLyrics[music.id]) {
      console.log('DesktopLyrics: 从缓存读取歌词')
      lyrics.value = parseLyrics(window.cachedLyrics[music.id])
      updateLyricsWindow()
      return
    }

    // 从 API 获取歌词
    const url = `${apiConfig.BASE_URL}${apiConfig.MUSIC_LYRICS(music.id)}?t=${Date.now()}`
    console.log('DesktopLyrics: 从API获取歌词', url)
    const response = await fetch(url)
    const result = await response.json()

    if (result.success && result.data) {
      lyrics.value = parseLyrics(result.data)
      // 缓存到内存
      window.cachedLyrics = window.cachedLyrics || {}
      window.cachedLyrics[music.id] = result.data
      updateLyricsWindow()
      console.log('DesktopLyrics: 歌词加载成功，共', lyrics.value.length, '行')
    } else {
      lyrics.value = []
      console.log('DesktopLyrics: 歌词加载失败')
    }
  } catch (error) {
    console.error('DesktopLyrics: 加载歌词失败:', error)
    lyrics.value = []
  }
}

// 监听桌面歌词切换
const handleDesktopLyricsToggle = (event) => {
  console.log('DesktopLyrics: 切换显示', event.detail.enabled)
  console.log('DesktopLyrics: window.electronAPI 是否存在:', !!window.electronAPI)
  console.log('DesktopLyrics: process.versions:', window.process?.versions)

  if (!window.electronAPI) {
    console.log('DesktopLyrics: electronAPI 不存在，无法控制歌词窗口')
    // 显示提示
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: {
        message: '桌面歌词功能仅在Electron桌面应用中可用',
        type: 'error'
      }
    }))
    return
  }

  try {
    if (event.detail.enabled) {
      console.log('DesktopLyrics: 调用 showLyricsWindow')
      window.electronAPI.showLyricsWindow()
    } else {
      console.log('DesktopLyrics: 调用 hideLyricsWindow')
      window.electronAPI.hideLyricsWindow()
    }
  } catch (error) {
    console.error('DesktopLyrics: 调用歌词窗口API失败:', error)
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: {
        message: '歌词窗口操作失败: ' + error.message,
        type: 'error'
      }
    }))
  }
}

// 监听歌词窗口隐藏事件
const handleLyricsWindowHidden = () => {
  console.log('DesktopLyrics: 歌词窗口已隐藏')
  // 通知PlayerBar更新状态
  window.dispatchEvent(new CustomEvent('toggle-desktop-lyrics'))
}

onMounted(() => {
  console.log('DesktopLyrics: 组件挂载')

  // 监听音频时间更新
  window.addEventListener('audio-time-update', handleTimeUpdate)

  // 监听音乐切换
  window.addEventListener('music-changed', handleMusicChanged)

  // 监听桌面歌词切换
  window.addEventListener('desktop-lyrics-toggle', handleDesktopLyricsToggle)

  // 监听歌词窗口隐藏事件
  window.addEventListener('lyrics-window-hidden', handleLyricsWindowHidden)

  // 尝试从当前正在播放的音乐加载歌词
  window.dispatchEvent(new CustomEvent('get-player-state'))
})

onUnmounted(() => {
  console.log('DesktopLyrics: 组件卸载')

  window.removeEventListener('audio-time-update', handleTimeUpdate)
  window.removeEventListener('music-changed', handleMusicChanged)
  window.removeEventListener('desktop-lyrics-toggle', handleDesktopLyricsToggle)
  window.removeEventListener('lyrics-window-hidden', handleLyricsWindowHidden)
})
</script>