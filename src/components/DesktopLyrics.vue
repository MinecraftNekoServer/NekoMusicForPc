<template>
  <Teleport to="body">
    <Transition name="desktop-lyrics">
      <div v-if="show && currentLyric" class="desktop-lyrics-container" :style="lyricsStyle">
        <div class="lyrics-content">
          <div class="lyric-text">{{ currentLyric }}</div>
          <div v-if="currentLyricTranslation" class="lyric-translation">{{ currentLyricTranslation }}</div>
        </div>
      </div>
    </Transition>
  </Teleport>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted, watch } from 'vue'
import apiConfig from '../config/apiConfig'

const show = ref(false)
const lyrics = ref([])
const currentLyric = ref('')
const currentLyricTranslation = ref('')
const currentTime = ref(0)
const lyricsPosition = ref({ x: window.innerWidth / 2 - 300, y: window.innerHeight - 150 })
const isDragging = ref(false)
const dragOffset = ref({ x: 0, y: 0 })

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

// 监听时间更新，更新当前歌词
const handleTimeUpdate = (event) => {
  currentTime.value = event.detail.currentTime
  const lyric = findCurrentLyric(currentTime.value)
  if (lyric) {
    currentLyric.value = lyric.text
    currentLyricTranslation.value = lyric.translation
  }
}

// 监听音乐切换
const handleMusicChanged = async (event) => {
  const music = event.detail
  if (!music) return
  
  try {
    // 先尝试从内存缓存读取
    if (window.cachedLyrics && window.cachedLyrics[music.id]) {
      lyrics.value = parseLyrics(window.cachedLyrics[music.id])
      return
    }
    
    // 从 API 获取歌词
    const url = `${apiConfig.BASE_URL}${apiConfig.MUSIC_LYRICS(music.id)}?t=${Date.now()}`
    const response = await fetch(url)
    const result = await response.json()
    
    if (result.success && result.data) {
      lyrics.value = parseLyrics(result.data)
      // 缓存到内存
      window.cachedLyrics = window.cachedLyrics || {}
      window.cachedLyrics[music.id] = result.data
    } else {
      lyrics.value = []
    }
  } catch (error) {
    console.error('加载歌词失败:', error)
    lyrics.value = []
  }
}

// 监听桌面歌词切换
const handleDesktopLyricsToggle = (event) => {
  show.value = event.detail.enabled
}

// 拖拽功能
const handleMouseDown = (event) => {
  if (event.target.closest('.lyrics-content')) {
    isDragging.value = true
    dragOffset.value = {
      x: event.clientX - lyricsPosition.value.x,
      y: event.clientY - lyricsPosition.value.y
    }
  }
}

const handleMouseMove = (event) => {
  if (isDragging.value) {
    lyricsPosition.value = {
      x: event.clientX - dragOffset.value.x,
      y: event.clientY - dragOffset.value.y
    }
  }
}

const handleMouseUp = () => {
  isDragging.value = false
}

// 歌词样式
const lyricsStyle = computed(() => ({
  left: `${lyricsPosition.value.x}px`,
  top: `${lyricsPosition.value.y}px`
}))

onMounted(() => {
  // 监听音频时间更新
  window.addEventListener('audio-time-update', handleTimeUpdate)
  
  // 监听音乐切换
  window.addEventListener('music-changed', handleMusicChanged)
  
  // 监听桌面歌词切换
  window.addEventListener('desktop-lyrics-toggle', handleDesktopLyricsToggle)
  
  // 监听拖拽事件
  document.addEventListener('mousedown', handleMouseDown)
  document.addEventListener('mousemove', handleMouseMove)
  document.addEventListener('mouseup', handleMouseUp)
  
  // 从 localStorage 加载位置
  const savedPosition = localStorage.getItem('desktopLyricsPosition')
  if (savedPosition) {
    try {
      const position = JSON.parse(savedPosition)
      lyricsPosition.value = position
    } catch (e) {
      console.error('解析桌面歌词位置失败:', e)
    }
  }
})

onUnmounted(() => {
  window.removeEventListener('audio-time-update', handleTimeUpdate)
  window.removeEventListener('music-changed', handleMusicChanged)
  window.removeEventListener('desktop-lyrics-toggle', handleDesktopLyricsToggle)
  document.removeEventListener('mousedown', handleMouseDown)
  document.removeEventListener('mousemove', handleMouseMove)
  document.removeEventListener('mouseup', handleMouseUp)
})

// 监听位置变化，保存到 localStorage
watch(lyricsPosition, (newPosition) => {
  localStorage.setItem('desktopLyricsPosition', JSON.stringify(newPosition))
}, { deep: true })
</script>

<style scoped>
.desktop-lyrics-container {
  position: fixed;
  z-index: 9999;
  pointer-events: auto;
  user-select: none;
  cursor: move;
  transition: opacity 0.3s ease;
}

.lyrics-content {
  text-align: center;
  padding: 20px 40px;
  background: rgba(0, 0, 0, 0.6);
  border-radius: 12px;
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255, 255, 255, 0.1);
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
}

.lyric-text {
  font-size: 28px;
  font-weight: bold;
  color: #ffffff;
  text-shadow: 0 2px 4px rgba(0, 0, 0, 0.5);
  margin-bottom: 8px;
  line-height: 1.4;
}

.lyric-translation {
  font-size: 18px;
  color: rgba(255, 255, 255, 0.8);
  text-shadow: 0 1px 2px rgba(0, 0, 0, 0.5);
  line-height: 1.4;
}

/* 动画 */
.desktop-lyrics-enter-active,
.desktop-lyrics-leave-active {
  transition: opacity 0.3s ease, transform 0.3s ease;
}

.desktop-lyrics-enter-from,
.desktop-lyrics-leave-to {
  opacity: 0;
  transform: translateY(20px);
}

.desktop-lyrics-enter-to,
.desktop-lyrics-leave-from {
  opacity: 1;
  transform: translateY(0);
}
</style>