<template>
  <Teleport to="body">
    <Transition name="desktop-lyrics">
      <div v-if="show" class="desktop-lyrics-container" :style="lyricsStyle" 
           @dblclick="toggleVisibility"
           @contextmenu.prevent="showContextMenu"
           @mousedown="handleMouseDown"
           @mouseenter="isHovered = true"
           @mouseleave="isHovered = false">
        
        <div class="lyrics-content" :class="{ 'minimized': isMinimized }">
          <div class="lyric-header" v-if="isHovered || isMinimized">
            <div class="lyric-controls">
              <button @click.stop="toggleMinimize" class="control-btn" :title="isMinimized ? t('key.maximize') : t('key.minimize')">
                <svg v-if="isMinimized" viewBox="0 0 24 24" width="16" height="16">
                  <path fill="currentColor" d="M7.41 15.41L12 10.83l4.59 4.58L18 14l-6-6-6 6z"/>
                </svg>
                <svg v-else viewBox="0 0 24 24" width="16" height="16">
                  <path fill="currentColor" d="M7.41 8.59L12 13.17l4.59-4.58L18 10l-6 6-6-6z"/>
                </svg>
              </button>
              <button @click.stop="resetPosition" class="control-btn" :title="t('key.refresh')">
                <svg viewBox="0 0 24 24" width="16" height="16">
                  <path fill="currentColor" d="M12 5V1L7 6l5 5V7c3.31 0 6 2.69 6 6s-2.69 6-6 6-6-2.69-6-6H4c0 4.42 3.58 8 8 8s8-3.58 8-8-3.58-8-8-8z"/>
                </svg>
              </button>
              <button @click.stop="decreaseFontSize" class="control-btn" title="减小字体">
                <svg viewBox="0 0 24 24" width="16" height="16">
                  <path fill="currentColor" d="M19 13H5v-2h14v2z"/>
                </svg>
              </button>
              <span class="font-size-display">{{ fontSize }}px</span>
              <button @click.stop="increaseFontSize" class="control-btn" title="增大字体">
                <svg viewBox="0 0 24 24" width="16" height="16">
                  <path fill="currentColor" d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
                </svg>
              </button>
              <button @click.stop="hideLyrics" class="control-btn" title="隐藏">
                <svg viewBox="0 0 24 24" width="16" height="16">
                  <path fill="currentColor" d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/>
                </svg>
              </button>
            </div>
          </div>
          
          <div class="lyric-body">
            <div v-if="!isMinimized">
              <div v-if="currentLyric" class="lyric-text" :style="{ fontSize: fontSize + 'px' }">
                {{ currentLyric }}
              </div>
              <div v-if="currentLyricTranslation && !isMinimized" class="lyric-translation" :style="{ fontSize: (fontSize * 0.7) + 'px' }">
                {{ currentLyricTranslation }}
              </div>
              <div v-if="!currentLyric && lyrics.length > 0" class="no-lyric-playing">
                等待播放...
              </div>
              <div v-if="lyrics.length === 0" class="no-lyrics">
                暂无歌词
              </div>
            </div>
            <div v-else class="minimized-content">
              <span class="minimized-icon">♪</span>
              <span v-if="currentLyric" class="minimized-text">{{ currentLyric.substring(0, 10) }}...</span>
              <span v-else class="minimized-text">歌词</span>
            </div>
          </div>
        </div>
      </div>
    </Transition>
  </Teleport>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted, watch, nextTick } from 'vue'
import { useI18n } from 'vue-i18n'
import apiConfig from '../config/apiConfig'

const { t } = useI18n()

const show = ref(false)
const lyrics = ref([])
const currentLyric = ref('')
const currentLyricTranslation = ref('')
const currentTime = ref(0)
const lyricsPosition = ref({ x: window.innerWidth / 2 - 200, y: window.innerHeight - 150 })
const isDragging = ref(false)
const dragOffset = ref({ x: 0, y: 0 })
const isHovered = ref(false)
const isMinimized = ref(false)
const fontSize = ref(28)

// 默认位置
const defaultPosition = { x: window.innerWidth / 2 - 200, y: window.innerHeight - 150 }

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
  
  console.log('DesktopLyrics: 音乐切换到', music.title)
  
  try {
    // 先尝试从内存缓存读取
    if (window.cachedLyrics && window.cachedLyrics[music.id]) {
      console.log('DesktopLyrics: 从缓存读取歌词')
      lyrics.value = parseLyrics(window.cachedLyrics[music.id])
      // 立即更新当前歌词
      const lyric = findCurrentLyric(currentTime.value)
      if (lyric) {
        currentLyric.value = lyric.text
        currentLyricTranslation.value = lyric.translation
      }
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
      // 立即更新当前歌词
      const lyric = findCurrentLyric(currentTime.value)
      if (lyric) {
        currentLyric.value = lyric.text
        currentLyricTranslation.value = lyric.translation
      }
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
  show.value = event.detail.enabled
  isMinimized.value = false
}

// 拖拽功能
const handleMouseDown = (event) => {
  if (event.target.closest('.lyrics-content') && !event.target.closest('.control-btn')) {
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

// 双击切换最小化
const toggleVisibility = () => {
  isMinimized.value = !isMinimized.value
}

// 最小化切换
const toggleMinimize = () => {
  isMinimized.value = !isMinimized.value
}

// 重置位置
const resetPosition = () => {
  lyricsPosition.value = { ...defaultPosition }
}

// 增大字体
const increaseFontSize = () => {
  if (fontSize.value < 48) {
    fontSize.value += 2
    saveSettings()
  }
}

// 减小字体
const decreaseFontSize = () => {
  if (fontSize.value > 16) {
    fontSize.value -= 2
    saveSettings()
  }
}

// 隐藏歌词
const hideLyrics = () => {
  show.value = false
  // 通知PlayerBar更新状态
  window.dispatchEvent(new CustomEvent('toggle-desktop-lyrics'))
}

// 显示右键菜单（暂时简化处理）
const showContextMenu = (event) => {
  event.preventDefault()
  // 可以在这里添加右键菜单功能
  console.log('右键菜单')
}

// 歌词样式
const lyricsStyle = computed(() => ({
  left: `${lyricsPosition.value.x}px`,
  top: `${lyricsPosition.value.y}px`
}))

// 保存设置
const saveSettings = () => {
  localStorage.setItem('desktopLyricsFontSize', fontSize.value.toString())
  localStorage.setItem('desktopLyricsMinimized', isMinimized.value.toString())
}

// 加载设置
const loadSettings = () => {
  const savedFontSize = localStorage.getItem('desktopLyricsFontSize')
  if (savedFontSize) {
    fontSize.value = parseInt(savedFontSize)
  }
  
  const savedMinimized = localStorage.getItem('desktopLyricsMinimized')
  if (savedMinimized) {
    isMinimized.value = savedMinimized === 'true'
  }
  
  const savedPosition = localStorage.getItem('desktopLyricsPosition')
  if (savedPosition) {
    try {
      const position = JSON.parse(savedPosition)
      lyricsPosition.value = position
    } catch (e) {
      console.error('解析桌面歌词位置失败:', e)
      lyricsPosition.value = { ...defaultPosition }
    }
  }
}

onMounted(() => {
  console.log('DesktopLyrics: 组件挂载')
  
  // 加载设置
  loadSettings()
  
  // 监听音频时间更新
  window.addEventListener('audio-time-update', handleTimeUpdate)
  
  // 监听音乐切换
  window.addEventListener('music-changed', handleMusicChanged)
  
  // 监听桌面歌词切换
  window.addEventListener('desktop-lyrics-toggle', handleDesktopLyricsToggle)
  
  // 监听拖拽事件
  document.addEventListener('mousemove', handleMouseMove)
  document.addEventListener('mouseup', handleMouseUp)
  
  // 尝试从当前正在播放的音乐加载歌词
  window.dispatchEvent(new CustomEvent('get-player-state'))
})

onUnmounted(() => {
  console.log('DesktopLyrics: 组件卸载')
  
  window.removeEventListener('audio-time-update', handleTimeUpdate)
  window.removeEventListener('music-changed', handleMusicChanged)
  window.removeEventListener('desktop-lyrics-toggle', handleDesktopLyricsToggle)
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
  max-width: 90vw;
}

.lyrics-content {
  text-align: center;
  padding: 20px 40px;
  background: rgba(0, 0, 0, 0.7);
  backdrop-filter: blur(12px);
  border-radius: 16px;
  border: 1px solid rgba(255, 255, 255, 0.15);
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
  transition: all 0.3s ease;
}

.lyrics-content.minimized {
  padding: 12px 20px;
  min-width: 150px;
}

.lyric-header {
  position: absolute;
  top: -50px;
  left: 0;
  right: 0;
  display: flex;
  justify-content: center;
  opacity: 0;
  transition: opacity 0.2s ease;
}

.desktop-lyrics-container:hover .lyric-header,
.lyrics-content.minimized .lyric-header {
  opacity: 1;
}

.lyric-controls {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 16px;
  background: rgba(0, 0, 0, 0.8);
  border-radius: 8px;
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255, 255, 255, 0.1);
}

.control-btn {
  width: 28px;
  height: 28px;
  border: none;
  background: transparent;
  border-radius: 4px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: rgba(255, 255, 255, 0.7);
  transition: all 0.2s ease;
}

.control-btn:hover {
  background: rgba(255, 255, 255, 0.1);
  color: white;
}

.control-btn svg {
  flex-shrink: 0;
}

.font-size-display {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.8);
  min-width: 40px;
  text-align: center;
}

.lyric-body {
  min-height: 60px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.lyric-text {
  font-weight: bold;
  color: #ffffff;
  text-shadow: 0 2px 8px rgba(0, 0, 0, 0.8), 0 0 20px rgba(102, 126, 234, 0.5);
  margin-bottom: 8px;
  line-height: 1.5;
  transition: all 0.3s ease;
  word-wrap: break-word;
  white-space: pre-wrap;
}

.lyric-translation {
  color: rgba(255, 255, 255, 0.85);
  text-shadow: 0 1px 4px rgba(0, 0, 0, 0.8);
  line-height: 1.5;
  transition: all 0.3s ease;
  word-wrap: break-word;
  white-space: pre-wrap;
}

.no-lyric-playing,
.no-lyrics {
  color: rgba(255, 255, 255, 0.5);
  font-size: 16px;
}

.minimized-content {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 14px;
  color: white;
}

.minimized-icon {
  font-size: 18px;
}

.minimized-text {
  color: rgba(255, 255, 255, 0.9);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
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

/* 响应式调整 */
@media screen and (max-width: 768px) {
  .lyrics-content {
    padding: 16px 24px;
  }
  
  .lyric-text {
    font-size: 20px !important;
  }
  
  .lyric-translation {
    font-size: 14px !important;
  }
}
</style>