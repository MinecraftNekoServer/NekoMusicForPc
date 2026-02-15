<template>
  <div class="player-view">
    <!-- 顶部控制栏 -->
    <div class="top-bar">
      <!-- 左侧关闭按钮 -->
      <button class="close-btn" @click="closePlayer" title="关闭">
        <svg viewBox="0 0 24 24" width="16" height="16" xmlns="http://www.w3.org/2000/svg">
          <g transform="scale(1,-1) translate(0,-24)">
            <path
                fill="#ffffff"
                d="M13.586 6.576a1.51 1.51 0 0 0-1.272-.572c-.446-.074-.866.272-1.216.57L2.45 15.21a1.4 1.4 0 1 0 1.98 1.98L12.37 9.08l7.3 8.26a1.4 1.4 0 0 0 1.98-1.98z"
            />
          </g>
        </svg>

      </button>

      <!-- 右侧窗口控制按钮 -->
      <div class="window-controls">
        <button class="window-control-btn" @click="minimizeWindow" title="最小化">
          <svg viewBox="0 0 24 24" width="16" height="16">
            <path fill="currentColor" d="M19 13H5v-2h14v2z"/>
          </svg>
        </button>
        <button class="window-control-btn" @click="maximizeWindow" title="全屏">
          <svg viewBox="0 0 24 24" width="16" height="16">
            <path fill="currentColor" d="M7 14H5v5h5v-2H7v-3zm-2-4h2V7h3V5H5v5zm12 7h-3v2h5v-5h-2v3zM14 5v2h3v3h2V5h-5z"/>
          </svg>
        </button>
        <button class="window-control-btn close" @click="closeWindow" title="关闭窗口">
          <svg viewBox="0 0 24 24" width="16" height="16">
            <path fill="currentColor" d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/>
          </svg>
        </button>
      </div>
    </div>

    <div v-if="currentMusic" class="player-content">
      <!-- 左侧封面 -->
      <div class="cover-section">
        <div class="cover-container">
          <img :src="getCoverUrl(currentMusic.id)" alt="封面" class="album-cover" @error="handleCoverError" />
        </div>
      </div>

      <!-- 右侧内容 -->
      <div class="content-section">
        <!-- 歌曲信息 -->
        <div class="music-info">
          <h1 class="music-title">{{ currentMusic.title }}</h1>
          <div class="music-meta">
            <span class="music-album">{{ currentMusic.album || '-' }}</span>
            <span class="separator">·</span>
            <span class="music-artist">{{ currentMusic.artist }}</span>
          </div>
        </div>

        <!-- 歌词区域 -->
        <div class="lyrics-section">
          <div class="lyrics-container" ref="lyricsContainer">
            <div v-if="lyrics && lyrics.length > 0" class="lyrics-content">
              <div 
                v-for="(line, index) in lyrics" 
                :key="index"
                :class="['lyric-line', { active: currentLyricIndex === index }]"
                @click="seekToLyric(line.time)"
              >
                <div class="lyric-text">{{ line.text }}</div>
                <div class="lyric-translation" v-if="line.translation">{{ line.translation }}</div>
              </div>
            </div>
            <div v-else class="no-lyrics">
              <svg viewBox="0 0 24 24" width="48" height="48">
                <path fill="currentColor" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-2 15l-5-5 1.41-1.41L10 14.17l7.59-7.59L19 8l-9 9z"/>
              </svg>
              <p>暂无歌词</p>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- 底部播放控制 -->
    <div v-if="currentMusic" class="player-controls glass">
      <!-- 左侧歌曲信息（不显示封面） -->
      <div class="player-info">
        <div class="player-details">
          <span class="player-title">{{ currentMusic.title }}</span>
          <span class="player-artist">{{ currentMusic.artist }}</span>
        </div>
      </div>
      
      <!-- 中间播放控制 -->
      <div class="player-controls-main">
        <div class="control-buttons">
          <button class="control-btn" @click="togglePlayMode" :title="playModeTitle" :class="{ active: playMode !== 'off' }">
            <img v-if="playMode === 'list'" src="/顺序播放.svg" alt="列表循环" width="20" height="20" class="play-mode-icon" />
            <img v-else-if="playMode === 'single'" src="/单曲循环.svg" alt="单曲循环" width="20" height="20" class="play-mode-icon single-mode" />
            <img v-else src="/随机播放.svg" alt="随机播放" width="20" height="20" class="play-mode-icon" />
          </button>
          <button class="control-btn" @click="previous" title="上一首">
            <svg viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M6 6h2v12H6zm3.5 6l8.5 6V6z"/>
            </svg>
          </button>
          <button class="control-btn play-btn" @click="togglePlay" :title="isPlaying ? '暂停' : '播放'">
            <svg v-if="!isPlaying" viewBox="0 0 24 24" width="24" height="24">
              <path fill="currentColor" d="M8 5v14l11-7z"/>
            </svg>
            <svg v-else viewBox="0 0 24 24" width="24" height="24">
              <path fill="currentColor" d="M6 19h4V5H6v14zm8-14v14h4V5h-4z"/>
            </svg>
          </button>
          <button class="control-btn" @click="next" title="下一首">
            <svg viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M6 18l8.5-6L6 6v12zM16 6v12h2V6h-2z"/>
            </svg>
          </button>
          <button class="control-btn favorite-btn" @click="toggleFavorite" :class="{ active: isFavorite }" title="收藏">
            <svg viewBox="0 0 24 24" width="20" height="20">
              <path :fill="isFavorite ? '#ff4545' : 'currentColor'" d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/>
            </svg>
          </button>
        </div>
        
        <div class="player-progress">
          <span class="time">{{ formatTime(currentTime) }}</span>
          <div class="progress-bar" 
             @mousedown="handleProgressMouseDown" 
             @mouseup="handleProgressMouseUp"
             @mouseleave="handleProgressMouseUp"
             @click="seekTo">
            <div class="progress-fill" :style="{ width: progress + '%' }">
              <div class="progress-glow"></div>
            </div>
            <div class="progress-thumb" :style="{ left: progress + '%' }">
              <div class="thumb-glow"></div>
            </div>
          </div>
          <span class="time">{{ formatTime(duration) }}</span>
        </div>
      </div>
      
      <!-- 右侧控制 -->
      <div class="player-controls-right">
        <button class="control-btn" @click="showAddToPlaylistModal" title="添加到歌单" :disabled="!currentMusic">
          <svg viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg" width="18" height="18">
            <path fill="currentColor" d="M384 640m-85.333333 0a85.333333 85.333333 0 1 0 170.666666 0 85.333333 85.333333 0 1 0-170.666666 0Z"/>
            <path fill="currentColor" d="M597.333333 597.333333m-85.333333 0a85.333333 85.333333 0 1 0 170.666667 0 85.333333 85.333333 0 1 0-170.666667 0Z"/>
            <path fill="currentColor" d="M631.637333 299.52a42.666667 42.666667 0 0 1 50.773334 36.864L682.666667 341.333333v256a42.666667 42.666667 0 0 1-85.034667 4.992L597.333333 597.333333V393.344l-128 25.6V640a42.666667 42.666667 0 0 1-37.674666 42.368L426.666667 682.666667a42.666667 42.666667 0 0 1-42.368-37.674667L384 640V384a42.666667 42.666667 0 0 1 29.44-40.533333l4.864-1.28 213.333333-42.666667z"/>
            <path fill="currentColor" d="M682.666667 128a213.333333 213.333333 0 0 1 213.12 204.074667L896 341.333333v256a42.666667 42.666667 0 0 1-85.034667 4.992L810.666667 597.333333V341.333333a128 128 0 0 0-120.490667-127.786666L682.666667 213.333333H341.333333a128 128 0 0 0-127.786666 120.490667L213.333333 341.333333v341.333334a128 128 0 0 0 120.490667 127.786666L341.333333 810.666667h256a42.666667 42.666667 0 0 1 4.992 85.034666L597.333333 896H341.333333a213.333333 213.333333 0 0 1-213.12-204.074667L128 682.666667V341.333333a213.333333 213.333333 0 0 1 204.074667-213.12L341.333333 128h341.333334z"/>
            <path fill="currentColor" d="M640 725.333333m42.666667 0l170.666666 0q42.666667 0 42.666667 42.666667l0 0q0 42.666667-42.666667 42.666667l-170.666666 0q-42.666667 0-42.666667-42.666667l0 0q0-42.666667 42.666667-42.666667Z"/>
            <path fill="currentColor" d="M810.666667 640m0 42.666667l0 170.666666q0 42.666667-42.666667 42.666667l0 0q-42.666667 0-42.666667-42.666667l0-170.666666q0-42.666667 42.666667-42.666667l0 0q42.666667 0 42.666667 42.666667Z"/>
          </svg>
        </button>

        <div class="volume-wrapper">
          <button class="control-btn" @click="toggleMute" :title="isMuted ? '取消静音' : '静音'">
            <svg v-if="!isMuted && volume > 50" viewBox="0 0 24 24" width="18" height="18">
              <path fill="currentColor" d="M3 9v6h4l5 5V4L7 9H3zm13.5 3c0-1.77-1.02-3.29-2.5-4.03v8.05c1.48-.73 2.5-2.25 2.5-4.02zM14 3.23v2.06c2.89.86 5 3.54 5 6.71s-2.11 5.85-5 6.71v2.06c4.01-.91 7-4.49 7-8.77s-2.99-7.86-7-8.77z"/>
            </svg>
            <svg v-else-if="!isMuted && volume > 0" viewBox="0 0 24 24" width="18" height="18">
              <path fill="currentColor" d="M3 9v6h4l5 5V4L7 9H3zm13.5 3c0-1.77-1.02-3.29-2.5-4.03v8.05c1.48-.73 2.5-2.25 2.5-4.02z"/>
            </svg>
            <svg v-else viewBox="0 0 24 24" width="18" height="18">
              <path fill="currentColor" d="M16.5 12c0-1.77-1.02-3.29-2.5-4.03v2.21l2.45 2.45c.03-.2.05-.41.05-.63zm2.5 0c0 .94-.2 1.82-.54 2.64l1.51 1.51C20.63 14.91 21 13.5 21 12c0-4.28-2.99-7.86-7-8.77v2.06c2.89.86 5 3.54 5 6.71zM4.27 3L3 4.27 7.73 9H3v6h4l5 5v-6.73l4.25 4.25c-.67.52-1.42.93-2.25 1.18v2.06c1.38-.31 2.63-.95 3.69-1.81L19.73 21 21 19.73l-9-9L4.27 3zM12 4L9.91 6.09 12 8.18V4z"/>
            </svg>
          </button>
          
          <div class="volume-panel">
            <div class="volume-slider-vertical" @mousedown="handleVolumeMouseDown">
              <div class="volume-track">
                <div class="volume-fill" :style="{ height: volume + '%' }"></div>
              </div>
              <div class="volume-thumb" :style="{ bottom: volume + '%' }"></div>
            </div>
            <div class="volume-value">{{ volume }}%</div>
          </div>
        </div>
        
        <button class="control-btn" @click="togglePlaylist" title="播放列表">
          <svg viewBox="0 0 24 24" width="18" height="18">
            <path fill="currentColor" d="M3 13h2v-2H3v2zm0 4h2v-2H3v2zm0-8h2V7H3v2zm4 4h14v-2H7v2zm0 4h14v-2H7v2zM7 7v2h14V7H7z"/>
          </svg>
        </button>
      </div>
    </div>

    <div v-else class="no-music">
      <svg viewBox="0 0 24 24" width="64" height="64">
        <path fill="currentColor" d="M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4V7h4V3h-6z"/>
      </svg>
      <p>暂无播放音乐</p>
      <button class="btn-back" @click="closePlayer">返回</button>
    </div>

    <!-- 添加到歌单模态框 -->
    <Teleport to="body">
      <Transition name="modal">
        <div v-if="showAddToPlaylistPanel" class="modal-overlay" @click="showAddToPlaylistPanel = false">
          <div class="modal-content modal-small" @click.stop>
            <div class="modal-header">
              <h3 class="modal-title">添加到歌单</h3>
              <button class="modal-close-btn" @click="showAddToPlaylistPanel = false">
                <svg viewBox="0 0 24 24" width="20" height="20">
                  <path fill="currentColor" d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/>
                </svg>
              </button>
            </div>

            <!-- 当前音乐信息 -->
            <div class="current-music-info">
              <img :src="getCoverUrl(currentMusic?.id)" alt="封面" class="current-music-cover" />
              <div class="current-music-details">
                <span class="current-music-title">{{ currentMusic?.title || '-' }}</span>
                <span class="current-music-artist">{{ currentMusic?.artist || '-' }}</span>
              </div>
            </div>

            <div class="playlist-section">
              <div class="section-header">
                <span class="section-title">选择歌单</span>
              </div>
              <div class="playlist-selector">
                <div
                  v-for="playlist in userPlaylists"
                  :key="playlist.id"
                  class="playlist-option"
                  @click="addToUserPlaylist(playlist.id)"
                >
                  <img :src="getPlaylistCover(playlist)" alt="封面" class="playlist-option-cover" />
                  <span class="playlist-option-name">{{ playlist.name }}</span>
                  <span class="playlist-option-count">{{ playlist.musicCount || 0 }}首</span>
                </div>
                <div v-if="userPlaylists.length === 0" class="playlists-empty">
                  <svg viewBox="0 0 24 24" width="48" height="48" fill="none" stroke="currentColor" stroke-width="1">
                    <path d="M9 18V5l12-2v13"/>
                    <circle cx="6" cy="18" r="3"/>
                    <circle cx="18" cy="16" r="3"/>
                  </svg>
                  <p>暂无歌单</p>
                </div>
              </div>
            </div>

            <!-- 新建歌单区域 -->
            <div class="create-playlist-section">
              <div class="section-header">
                <span class="section-title">或创建新歌单</span>
              </div>
              <div class="create-playlist-form">
                <input
                  v-model="newPlaylistName"
                  type="text"
                  placeholder="输入歌单名称"
                  class="playlist-name-input"
                  @keyup.enter="handleCreateNewPlaylist"
                />
                <button
                  class="create-playlist-btn"
                  @click="handleCreateNewPlaylist"
                  :disabled="!newPlaylistName.trim()"
                >
                  <svg viewBox="0 0 24 24" width="16" height="16">
                    <path fill="currentColor" d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
                  </svg>
                  <span>创建</span>
                </button>
              </div>
            </div>

            <div class="modal-buttons">
              <button class="modal-btn modal-btn-secondary" @click="showAddToPlaylistPanel = false">取消</button>
            </div>
          </div>
        </div>
      </Transition>
    </Teleport>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted, watch } from 'vue'
import { useRouter } from 'vue-router'
import apiConfig from '../config/apiConfig'

const router = useRouter()
const currentMusic = ref(null)
const isPlaying = ref(false)
const playMode = ref('list')
const currentTime = ref(0)
const duration = ref(0)
const lyrics = ref([])
const currentLyricIndex = ref(0)
const lyricsContainer = ref(null)
const volume = ref(100)
const isMuted = ref(false)
const isVolumeDragging = ref(false)
const isFavorite = ref(false)
const favorites = ref([])

// 添加到歌单相关
const showAddToPlaylistPanel = ref(false)
const userPlaylists = ref([])
const newPlaylistName = ref('')

const playModeTitle = computed(() => {
  const titles = {
    'list': '列表循环',
    'single': '单曲循环',
    'shuffle': '随机播放'
  }
  return titles[playMode.value] || '列表循环'
})

const progress = computed(() => {
  if (duration.value === 0) return 0
  return (currentTime.value / duration.value) * 100
})

const getCoverUrl = (id) => {
  return `https://music.cnmsb.xin/api/music/cover/${id}`
}

const formatTime = (seconds) => {
  if (!seconds) return '0:00'
  const mins = Math.floor(seconds / 60)
  const secs = Math.floor(seconds % 60)
  return `${mins}:${secs.toString().padStart(2, '0')}`
}

const closePlayer = () => {
  router.back()
}

const togglePlay = () => {
  if (!currentMusic.value) return
  // 直接通知 PlayerBar 切换播放状态
  window.dispatchEvent(new CustomEvent('toggle-play'))
}

const previous = () => {
  window.dispatchEvent(new CustomEvent('tray-previous'))
}

const next = () => {
  window.dispatchEvent(new CustomEvent('tray-next'))
}

const togglePlayMode = () => {
  window.dispatchEvent(new CustomEvent('tray-set-play-mode', { detail: playMode.value === 'list' ? 'single' : playMode.value === 'single' ? 'shuffle' : 'list' }))
}

const toggleFavorite = async () => {
  if (!currentMusic.value) return
  
  const token = localStorage.getItem('token')
  if (!token) {
    window.dispatchEvent(new CustomEvent('show-toast', { 
      detail: { message: '请先登录', type: 'error' } 
    }))
    return
  }
  
  try {
    if (isFavorite.value) {
      const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.USER_FAVORITES_DELETE(currentMusic.value.id)}`, {
        method: 'DELETE',
        headers: { 'Authorization': token }
      })
      
      if (response.ok) {
        favorites.value = favorites.value.filter(f => f.id !== currentMusic.value.id)
        isFavorite.value = false
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '已取消收藏', type: 'success' } 
        }))
      }
    } else {
      const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.USER_FAVORITES}`, {
        method: 'POST',
        headers: { 
          'Authorization': token,
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ musicId: currentMusic.value.id })
      })
      
      if (response.ok) {
        favorites.value.push(currentMusic.value)
        isFavorite.value = true
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '收藏成功', type: 'success' } 
        }))
      }
    }
  } catch (error) {
    console.error('收藏操作失败:', error)
  }
}

const seekTo = (event) => {
  if (!currentMusic.value || !duration.value) return
  
  const rect = event.currentTarget.getBoundingClientRect()
  const x = event.clientX - rect.left
  const percentage = Math.max(0, Math.min(1, x / rect.width))
  const seekTime = percentage * duration.value
  
  // 直接通知 PlayerBar 跳转进度
  window.dispatchEvent(new CustomEvent('seek-to', { detail: seekTime }))
}

const isProgressDragging = ref(false)

const handleProgressMouseDown = (event) => {
  isProgressDragging.value = true
  seekTo(event)
  event.preventDefault()
}

const handleProgressMouseUp = () => {
  isProgressDragging.value = false
}

const handleGlobalProgressMouseMove = (event) => {
  if (isProgressDragging.value) {
    const progressBar = document.querySelector('.progress-bar')
    if (progressBar) {
      const rect = progressBar.getBoundingClientRect()
      const x = event.clientX - rect.left
      const percentage = Math.max(0, Math.min(1, x / rect.width))
      const seekTime = percentage * duration.value
      // 直接通知 PlayerBar 跳转进度
      window.dispatchEvent(new CustomEvent('seek-to', { detail: seekTime }))
    }
  }
}

const handleGlobalMouseUp = () => {
  handleVolumeMouseUp()
  handleProgressMouseUp()
}

const handleGlobalMouseMove = (event) => {
  if (isVolumeDragging.value) {
    handleVolumeMouseMove(event)
  }
  if (isProgressDragging.value) {
    handleGlobalProgressMouseMove(event)
  }
}

const seekToLyric = (time) => {
  window.dispatchEvent(new CustomEvent('seek-to', { detail: time }))
}

const parseLyrics = (lyricsText) => {
  if (!lyricsText) return []
  
  const lines = lyricsText.split('\n')
  const parsed = []
  
  for (let i = 0; i < lines.length; i++) {
    const line = lines[i].trim()
    
    // 跳过空行
    if (!line) {
      continue
    }
    
    // 匹配时间戳歌词行 [mm:ss.xx] 或 [mm:ss.xxx] (支持1-3位数字)
    const timeRegex = /\[(\d{1,2}):(\d{1,2})\.(\d{2,3})\]/
    const timeMatch = line.match(timeRegex)
    
    if (timeMatch) {
      // 这是歌词行，提取时间和文本
      const minutes = parseInt(timeMatch[1])
      const seconds = parseInt(timeMatch[2])
      const milliseconds = parseInt(timeMatch[3])
      
      // 根据毫秒部分的位数正确计算秒数
      let millisecondsDivisor
      if (milliseconds.toString().length === 2) {
        millisecondsDivisor = 100 // 两位毫秒，如 .25
      } else {
        millisecondsDivisor = 1000 // 三位毫秒，如 .250
      }
      const timeInSeconds = minutes * 60 + seconds + (milliseconds / millisecondsDivisor)
      const text = line.replace(timeRegex, '').trim()
      
      // 查找下一行是否有翻译
      let translation = ''
      if (i + 1 < lines.length) {
        const nextLine = lines[i + 1].trim()
        // 检查是否是JSON格式的翻译行
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
  
  // 按时间排序
  parsed.sort((a, b) => a.time - b.time)
  return parsed
}

const loadLyrics = async () => {
  if (!currentMusic.value) return
  
  try {
    console.log('loadLyrics: 开始加载歌词，音乐ID:', currentMusic.value.id)
    
    // 先尝试从内存缓存读取
    if (window.cachedLyrics && window.cachedLyrics[currentMusic.value.id]) {
      console.log('loadLyrics: 从内存缓存读取歌词成功')
      lyrics.value = parseLyrics(window.cachedLyrics[currentMusic.value.id])
      return
    }
    
    // 如果内存中没有，从 API 获取
    const url = `${apiConfig.BASE_URL}${apiConfig.MUSIC_LYRICS(currentMusic.value.id)}?t=${Date.now()}`
    console.log('loadLyrics: 从 API 获取歌词，请求URL:', url)
    
    const response = await fetch(url)
    console.log('loadLyrics: 响应状态:', response.status)
    
    const result = await response.json()
    console.log('loadLyrics: 响应数据:', result)
    
    if (result.success && result.data) {
      const lyricsText = result.data
      lyrics.value = parseLyrics(lyricsText)
      // 缓存到内存
      window.cachedLyrics = window.cachedLyrics || {}
      window.cachedLyrics[currentMusic.value.id] = lyricsText
      console.log('loadLyrics: 歌词解析成功，共', lyrics.value.length, '行')
    } else {
      console.log('loadLyrics: 响应中没有歌词数据')
    }
  } catch (error) {
    console.error('loadLyrics: 加载歌词失败:', error)
  }
}

const checkFavoriteStatus = () => {
  if (!currentMusic.value) return
  isFavorite.value = favorites.value.some(f => f.id === currentMusic.value.id)
}

const loadFavorites = async () => {
  const token = localStorage.getItem('token')
  if (!token) return
  
  try {
    const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.USER_FAVORITES}`, {
      headers: { 'Authorization': token }
    })
    
    if (response.ok) {
      const result = await response.json()
      if (result.success && result.favorites) {
        favorites.value = result.favorites
        checkFavoriteStatus()
      }
    }
  } catch (error) {
    console.error('加载收藏列表失败:', error)
  }
}

const minimizeWindow = () => {
  if (window.electronAPI) {
    window.electronAPI.minimize()
  }
}

const maximizeWindow = () => {
  if (window.electronAPI) {
    window.electronAPI.maximize()
  }
}

const closeWindow = () => {
  if (window.electronAPI) {
    window.electronAPI.close()
  }
}

const toggleMute = () => {
  isMuted.value = !isMuted.value
  // 通知 PlayerBar 切换静音
  window.dispatchEvent(new CustomEvent('toggle-mute'))
}

const handleVolumeMouseDown = (event) => {
  isVolumeDragging.value = true
  handleVolumeClick(event)
  event.preventDefault()
}

const handleVolumeMouseMove = (event) => {
  if (isVolumeDragging.value) {
    const volumeSlider = document.querySelector('.volume-slider-vertical')
    if (volumeSlider) {
      const rect = volumeSlider.getBoundingClientRect()
      const y = rect.bottom - event.clientY
      const percentage = Math.max(0, Math.min(100, (y / rect.height) * 100))
      volume.value = Math.round(percentage)
      // 通知 PlayerBar 更新音量
      window.dispatchEvent(new CustomEvent('set-volume', { detail: volume.value }))
    }
  }
}

const handleVolumeMouseUp = () => {
  isVolumeDragging.value = false
}

const handleVolumeClick = (event) => {
  const rect = event.currentTarget.getBoundingClientRect()
  const y = rect.bottom - event.clientY
  const percentage = Math.min(100, Math.max(0, (y / rect.height) * 100))
  volume.value = Math.round(percentage)
  // 通知 PlayerBar 更新音量
  window.dispatchEvent(new CustomEvent('set-volume', { detail: volume.value }))
}

const togglePlaylist = () => {
  // 通知 PlayerBar 切换播放列表面板
  window.dispatchEvent(new CustomEvent('toggle-playlist-panel'))
}

// 统一的 API 请求函数
async function apiRequest(url, options = {}) {
  const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
  return fetch(fullUrl, options)
}

// 显示添加到歌单面板
const showAddToPlaylistModal = async () => {
  if (!currentMusic.value) return

  const token = localStorage.getItem('token')
  if (!token) {
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: { message: '请先登录', type: 'error' }
    }))
    return
  }

  // 加载用户歌单列表
  await loadUserPlaylists()

  if (userPlaylists.value.length === 0) {
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: { message: '暂无歌单，请先创建歌单', type: 'info' }
    }))
    return
  }

  showAddToPlaylistPanel.value = true
}

// 加载用户歌单列表
const loadUserPlaylists = async () => {
  const token = localStorage.getItem('token')
  if (!token) {
    userPlaylists.value = []
    return
  }

  try {
    const timestamp = Date.now()
    const response = await apiRequest(`${apiConfig.PLAYLISTS}?t=${timestamp}`, {
      method: 'GET',
      headers: { 'Authorization': token }
    })

    const data = await response.json()
    if (data.success && data.playlists) {
      const playlistsWithCovers = []

      for (const playlist of data.playlists) {
        try {
          // 获取歌单音乐列表以获取第一首歌的封面
          const musicResponse = await apiRequest(`${apiConfig.PLAYLIST_MUSIC(playlist.id)}?t=${timestamp}`, {
            method: 'GET',
            headers: { 'Authorization': token }
          })

          const musicData = await musicResponse.json()
          if (musicData.success && musicData.musicList && musicData.musicList.length > 0) {
            playlistsWithCovers.push({
              ...playlist,
              firstMusicId: musicData.musicList[0].id
            })
          } else {
            playlistsWithCovers.push(playlist)
          }
        } catch (error) {
          playlistsWithCovers.push(playlist)
        }
      }

      userPlaylists.value = playlistsWithCovers
    }
  } catch (error) {
    console.error('加载用户歌单列表失败:', error)
  }
}

// 添加到用户歌单
const addToUserPlaylist = async (playlistId) => {
  if (!currentMusic.value) return

  const token = localStorage.getItem('token')
  if (!token) {
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: { message: '请先登录', type: 'error' }
    }))
    return
  }

  try {
    const response = await apiRequest(apiConfig.PLAYLIST_MUSIC_ADD, {
      method: 'POST',
      headers: {
        'Authorization': token,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        playlistId: playlistId,
        musicId: currentMusic.value.id
      })
    })

    const data = await response.json()
    if (data.success) {
      window.dispatchEvent(new CustomEvent('show-toast', {
        detail: { message: '添加到歌单成功', type: 'success' }
      }))
      showAddToPlaylistPanel.value = false
    } else {
      window.dispatchEvent(new CustomEvent('show-toast', {
        detail: { message: data.message || '添加失败', type: 'error' }
      }))
    }
  } catch (error) {
    console.error('添加到歌单失败:', error)
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: { message: '网络错误，请重试', type: 'error' }
    }))
  }
}

// 获取歌单封面
const getPlaylistCover = (playlist) => {
  if (playlist.firstMusicId) {
    return `https://music.cnmsb.xin/api/music/cover/${playlist.firstMusicId}`
  }
  return 'https://music.cnmsb.xin/api/user/avatar/default'
}

// 处理创建新歌单
const handleCreateNewPlaylist = async () => {
  if (!newPlaylistName.value.trim()) {
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: { message: '请输入歌单名称', type: 'error' }
    }))
    return
  }

  const token = localStorage.getItem('token')
  if (!token) {
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: { message: '请先登录', type: 'error' }
    }))
    return
  }

  try {
    const response = await apiRequest(apiConfig.PLAYLIST_CREATE, {
      method: 'POST',
      headers: {
        'Authorization': token,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        name: newPlaylistName.value.trim(),
        description: ''
      })
    })

    const data = await response.json()
    if (data.success) {
      window.dispatchEvent(new CustomEvent('show-toast', {
        detail: { message: '歌单创建成功', type: 'success' }
      }))

      // 重新加载歌单列表
      await loadUserPlaylists()

      // 自动将音乐添加到新创建的歌单
      if (data.playlist && data.playlist.id) {
        await addToUserPlaylist(data.playlist.id)
      }

      newPlaylistName.value = ''
    } else {
      window.dispatchEvent(new CustomEvent('show-toast', {
        detail: { message: data.message || '创建失败', type: 'error' }
      }))
    }
  } catch (error) {
    console.error('创建歌单失败:', error)
    window.dispatchEvent(new CustomEvent('show-toast', {
      detail: { message: '网络错误，请重试', type: 'error' }
    }))
  }
}

// 从 localStorage 加载播放状态
const loadPlayerState = () => {
  const savedMusic = localStorage.getItem('currentMusic')
  const savedPlayMode = localStorage.getItem('playMode')
  const savedProgress = localStorage.getItem('playProgress')
  
  if (savedMusic) {
    try {
      currentMusic.value = JSON.parse(savedMusic)
      loadLyrics()
      checkFavoriteStatus()
    } catch (e) {
      console.error('解析当前音乐失败:', e)
    }
  }
  
  if (savedPlayMode) {
    playMode.value = savedPlayMode
  }
  
  if (savedProgress) {
    try {
      const progress = JSON.parse(savedProgress)
      if (progress.musicId === currentMusic.value?.id) {
        currentTime.value = progress.currentTime
        duration.value = progress.duration
      }
    } catch (e) {
      console.error('解析播放进度失败:', e)
    }
  }
}

// 保存播放状态到 localStorage
const savePlayerState = () => {
  if (currentMusic.value) {
    localStorage.setItem('playMode', playMode.value)
    localStorage.setItem('playProgress', JSON.stringify({
      musicId: currentMusic.value.id,
      currentTime: currentTime.value,
      duration: duration.value
    }))
  }
}

// 监听 PlayerBar 的播放状态变化
const handlePlayerStateChange = (event) => {
  if (event.detail?.isPlaying !== undefined) {
    isPlaying.value = event.detail.isPlaying
  }
  if (event.detail?.currentTime !== undefined) {
    currentTime.value = event.detail.currentTime
  }
  if (event.detail?.duration !== undefined) {
    duration.value = event.detail.duration
  }
  if (event.detail?.playMode !== undefined) {
    playMode.value = event.detail.playMode
  }
  if (event.detail?.volume !== undefined) {
    volume.value = event.detail.volume
  }
}

// 处理音乐切换事件
const handleMusicChanged = (event) => {
  const newMusic = event.detail
  if (!newMusic) return
  
  console.log('handleMusicChanged: 音乐已切换到', newMusic.title)
  
  // 更新当前音乐
  currentMusic.value = newMusic
  
  // 重置播放进度
  currentTime.value = 0
  
  // 加载新歌词
  loadLyrics()
  
  // 检查收藏状态
  checkFavoriteStatus()
}

const handleTimeUpdate = () => {
  // 更新歌词高亮
  if (lyrics.value.length > 0) {
    for (let i = lyrics.value.length - 1; i >= 0; i--) {
      if (currentTime.value >= lyrics.value[i].time) {
        currentLyricIndex.value = i
        
        // 滚动歌词到可视区域
        if (lyricsContainer.value) {
          const lyricLines = lyricsContainer.value.querySelectorAll('.lyric-line')
          if (lyricLines[i]) {
            lyricLines[i].scrollIntoView({ behavior: 'smooth', block: 'center' })
          }
        }
        break
      }
    }
  }
  
  // 保存播放进度
  savePlayerState()
}

const handleCoverError = (event) => {
  event.target.src = 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="300" height="300" viewBox="0 0 300 300"><defs><linearGradient id="grad" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" style="stop-color:%23667eea;stop-opacity:1"/><stop offset="100%" style="stop-color:%23764ba2;stop-opacity:1"/></linearGradient></defs><rect width="300" height="300" fill="url(%23grad)" rx="20"/><text x="150" y="160" font-family="Arial" font-size="24" fill="white" text-anchor="middle" font-weight="bold">M</text></svg>'
}

onMounted(() => {
  loadPlayerState()
  loadFavorites()
  
  // 监听 PlayerBar 的播放状态变化
  window.addEventListener('player-state-change', handlePlayerStateChange)
  window.addEventListener('music-play', handlePlayerStateChange)
  
  // 监听音乐切换事件
  window.addEventListener('music-changed', handleMusicChanged)
  
  // 监听音频时间更新（用于歌词同步）
  window.addEventListener('audio-time-update', handleTimeUpdate)
  
  // 全局鼠标事件，处理音量拖动和进度条拖动
  window.addEventListener('mouseup', handleGlobalMouseUp)
  window.addEventListener('mousemove', handleGlobalMouseMove)
  
  // 获取当前播放状态
  window.dispatchEvent(new CustomEvent('get-player-state'))
})

onUnmounted(() => {
  window.removeEventListener('player-state-change', handlePlayerStateChange)
  window.removeEventListener('music-play', handlePlayerStateChange)
  window.removeEventListener('music-changed', handleMusicChanged)
  window.removeEventListener('audio-time-update', handleTimeUpdate)
  window.removeEventListener('mouseup', handleGlobalMouseUp)
  window.removeEventListener('mousemove', handleGlobalMouseMove)

  // 保存播放状态
  savePlayerState()
})
// 监听路由变化，重新加载数据
watch(() => router.currentRoute.value, () => {
  loadPlayerState()
})
</script>

<style scoped>
.player-view {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(135deg, #1a1a2e 0%, #16213e 50%, #0f3460 100%);
  display: flex;
  flex-direction: column;
  z-index: 1000;
  animation: fadeIn 0.3s ease;
}

:root {
  --radius-md: 8px;
  --transition-normal: 0.2s ease;
}

@keyframes fadeIn {
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
}

/* 顶部控制栏 */
.top-bar {
  height: 48px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 16px;
  background: rgba(0, 0, 0, 0.2);
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
  -webkit-app-region: drag;
  user-select: none;
}

.close-btn {
  width: 32px;
  height: 32px;
  border: none;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 6px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  transition: all 0.2s ease;
  -webkit-app-region: no-drag;
}

.close-btn:hover {
  background: rgba(255, 255, 255, 0.2);
}

.window-controls {
  display: flex;
  gap: 8px;
}

.window-control-btn {
  width: 32px;
  height: 32px;
  border: none;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 6px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  transition: all 0.2s ease;
  -webkit-app-region: no-drag;
}

.window-control-btn:hover {
  background: rgba(255, 255, 255, 0.2);
}

.window-control-btn.close:hover {
  background: #ff5f57;
}

.player-content {
  display: grid;
  grid-template-columns: 400px 1fr;
  gap: 0;
  width: 100%;
  height: calc(100% - 120px);
  padding: 0;
}

.cover-section {
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 40px;
}

.cover-container {
  position: relative;
  width: 280px;
  height: 280px;
  border-radius: 16px;
  overflow: hidden;
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.4);
  transition: all 0.3s ease;
}

.album-cover {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.content-section {
  display: flex;
  flex-direction: column;
  padding: 40px 60px 0 0;
  overflow: hidden;
}

.music-info {
  margin-bottom: 24px;
}

.music-title {
  font-size: 36px;
  font-weight: 700;
  color: white;
  margin: 0 0 12px 0;
  line-height: 1.3;
}

.music-meta {
  display: flex;
  align-items: center;
  gap: 12px;
  font-size: 16px;
  color: rgba(255, 255, 255, 0.6);
}

.separator {
  color: rgba(255, 255, 255, 0.3);
}

.lyrics-section {
  flex: 1;
  display: flex;
  align-items: flex-start;
  overflow: hidden;
}

.lyrics-container {
  width: 100%;
  height: 100%;
  overflow-y: auto;
  text-align: center;
  padding: 20px 0;
  scroll-behavior: smooth;
}

.lyrics-content {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.lyric-line {
  font-size: 20px;
  color: rgba(255, 255, 255, 0.4);
  transition: all 0.3s ease;
  cursor: pointer;
  padding: 8px 16px;
  border-radius: 8px;
  line-height: 1.6;
}

.lyric-text {
  display: block;
  margin-bottom: 4px;
}

.lyric-translation {
  display: block;
  font-size: 0.75em;
  opacity: 0.7;
  color: rgba(255, 255, 255, 0.6);
}

.lyric-line:hover {
  color: rgba(255, 255, 255, 0.6);
  background: rgba(255, 255, 255, 0.05);
}

.lyric-line:hover .lyric-translation {
  color: rgba(255, 255, 255, 0.8);
}

.lyric-line.active {
  font-size: 28px;
  font-weight: 600;
  color: white;
  text-shadow: 0 0 20px rgba(102, 126, 234, 0.6);
}

.lyric-line.active .lyric-text {
  color: white;
}

.lyric-line.active .lyric-translation {
  color: rgba(255, 255, 255, 0.9);
  opacity: 1;
}

.no-lyrics {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 16px;
  color: rgba(255, 255, 255, 0.4);
  font-size: 18px;
  height: 200px;
}

/* 底部播放控制 */
.player-controls {
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  height: 90px;
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(10px);
  border-top: 1px solid rgba(255, 255, 255, 0.1);
  display: flex;
  align-items: center;
  padding: 0 24px;
  gap: 24px;
}

.player-info {
  display: flex;
  align-items: center;
  width: 280px;
  flex-shrink: 0;
}

.player-details {
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.player-title {
  font-size: 15px;
  font-weight: 600;
  color: white;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.player-artist {
  font-size: 13px;
  color: rgba(255, 255, 255, 0.7);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  margin-top: 2px;
}

.player-controls-main {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
  min-width: 400px;
}

.control-buttons {
  display: flex;
  align-items: center;
  gap: 12px;
}

.control-btn {
  width: 38px;
  height: 38px;
  border: none;
  background: transparent;
  border-radius: var(--radius-md);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: rgba(255, 255, 255, 0.8);
  transition: all var(--transition-normal);
}

.control-btn svg {
  fill: currentColor;
}

.control-btn img {
  filter: brightness(0) saturate(100%) opacity(0.8);
  transition: all var(--transition-normal);
}

.control-btn:hover {
  background: rgba(102, 126, 234, 0.15);
  color: white;
  transform: scale(1.1);
}

.control-btn:hover img {
  filter: brightness(0) saturate(100%) opacity(1);
}

.control-btn.active {
  color: #667eea;
  background: rgba(102, 126, 234, 0.1);
}

.control-btn.active img {
  filter: brightness(0) saturate(100%) opacity(1) hue-rotate(240deg);
}

.control-btn:disabled {
  opacity: 0.3;
  cursor: not-allowed;
}

.play-btn {
  width: 48px;
  height: 48px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  box-shadow: 0 4px 16px rgba(102, 126, 234, 0.4);
}

.favorite-btn.active {
  color: #ff4545;
  background: rgba(255, 69, 69, 0.2);
}

.favorite-btn:disabled {
  opacity: 0.4;
  cursor: not-allowed;
}

.player-progress {
  width: 100%;
  display: flex;
  align-items: center;
  gap: 12px;
}

.time {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.6);
  min-width: 42px;
  text-align: center;
  font-weight: 500;
}

.progress-bar {
  flex: 1;
  height: 6px;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 3px;
  cursor: pointer;
  position: relative;
  transition: all 0.2s ease;
}

.progress-bar:hover {
  height: 8px;
}

.progress-fill {
  height: 100%;
  background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
  border-radius: 3px;
  position: relative;
  transition: width 0.1s linear;
}

.progress-glow {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
  filter: blur(8px);
  opacity: 0.6;
}

.progress-thumb {
  position: absolute;
  top: 50%;
  transform: translate(-50%, -50%);
  width: 14px;
  height: 14px;
  background: white;
  border-radius: 50%;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
  transition: all 0.2s ease;
}

.thumb-glow {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  width: 24px;
  height: 24px;
  background: white;
  border-radius: 50%;
  filter: blur(8px);
  opacity: 0.5;
}

.progress-bar:hover .progress-thumb {
  width: 18px;
  height: 18px;
}

.player-controls-right {
  display: flex;
  align-items: center;
  gap: 8px;
}

.volume-wrapper {
  position: relative;
  display: flex;
  align-items: center;
}

.volume-panel {
  position: absolute;
  bottom: calc(100% + 12px);
  left: 50%;
  transform: translateX(-50%);
  width: 60px;
  padding: 16px 12px;
  background: rgba(255, 255, 255, 0.95);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.15);
  display: flex;
  flex-direction: column;
  align-items: center;
  z-index: 200;
  opacity: 0;
  visibility: hidden;
  transition: opacity 0.2s ease, visibility 0.2s ease;
}

.volume-wrapper:hover .volume-panel {
  opacity: 1;
  visibility: visible;
}

.volume-slider-vertical {
  width: 8px;
  height: 120px;
  position: relative;
  cursor: pointer;
  user-select: none;
}

.volume-track {
  width: 100%;
  height: 100%;
  background: rgba(0, 0, 0, 0.1);
  border-radius: 4px;
  position: relative;
  overflow: hidden;
}

.volume-fill {
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  background: #FF3B30;
  border-radius: 4px;
  transition: height 0.1s;
}

.volume-thumb {
  position: absolute;
  left: 50%;
  transform: translateX(-50%) translateY(50%);
  width: 20px;
  height: 20px;
  background: white;
  border-radius: 50%;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.2);
  transition: bottom 0.1s;
}

.volume-value {
  margin-top: 8px;
  font-size: 12px;
  font-weight: 500;
  color: var(--text-primary);
}

/* 滚动条样式 */
.lyrics-container::-webkit-scrollbar {
  width: 6px;
}

.lyrics-container::-webkit-scrollbar-track {
  background: transparent;
}

.lyrics-container::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.1);
  border-radius: 3px;
}

.lyrics-container::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.2);
}

.no-music {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 24px;
  color: rgba(255, 255, 255, 0.6);
  height: 100%;
}

.btn-back {
  padding: 12px 32px;
  background: rgba(255, 255, 255, 0.1);
  border: 1px solid rgba(255, 255, 255, 0.2);
  border-radius: 24px;
  color: white;
  font-size: 16px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.btn-back:hover {
  background: rgba(255, 255, 255, 0.2);
}

/* 添加到歌单模态框样式 */
.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.6);
  backdrop-filter: blur(8px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 99999;
  padding: 20px;
}

.modal-content {
  background: linear-gradient(135deg, rgba(42, 42, 42, 0.95) 0%, rgba(30, 30, 30, 0.95) 100%);
  border-radius: 20px;
  padding: 24px;
  width: 100%;
  max-width: 420px;
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.5);
  border: 1px solid rgba(255, 255, 255, 0.1);
}

.modal-content.modal-small {
  max-width: 380px;
}

.modal-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 20px;
}

.modal-title {
  margin: 0;
  font-size: 20px;
  font-weight: 600;
  color: white;
}

.modal-close-btn {
  width: 32px;
  height: 32px;
  border: none;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 8px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: rgba(255, 255, 255, 0.6);
  transition: all 0.2s;
}

.modal-close-btn:hover {
  background: rgba(255, 255, 255, 0.2);
  color: white;
}

.current-music-info {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 12px;
  background: rgba(255, 255, 255, 0.05);
  border-radius: 12px;
  margin-bottom: 20px;
}

.current-music-cover {
  width: 48px;
  height: 48px;
  border-radius: 8px;
  object-fit: cover;
}

.current-music-details {
  flex: 1;
  min-width: 0;
}

.current-music-title {
  display: block;
  font-size: 14px;
  font-weight: 500;
  color: white;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.current-music-artist {
  display: block;
  font-size: 12px;
  color: rgba(255, 255, 255, 0.6);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.playlist-section {
  margin-bottom: 20px;
}

.section-header {
  display: flex;
  align-items: center;
  margin-bottom: 12px;
}

.section-title {
  font-size: 13px;
  font-weight: 600;
  color: rgba(255, 255, 255, 0.6);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.playlist-selector {
  max-height: 200px;
  overflow-y: auto;
  padding: 4px;
}

.playlist-option {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 12px;
  border-radius: 10px;
  cursor: pointer;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
  background: rgba(255, 255, 255, 0.03);
  margin-bottom: 6px;
}

.playlist-option:hover {
  background: rgba(102, 126, 234, 0.15);
  transform: translateX(2px);
}

.playlist-option-cover {
  width: 40px;
  height: 40px;
  border-radius: 8px;
  object-fit: cover;
  flex-shrink: 0;
}

.playlist-option-name {
  flex: 1;
  font-size: 14px;
  color: white;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.playlist-option-count {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.4);
  flex-shrink: 0;
}

.playlists-empty {
  text-align: center;
  padding: 40px 20px;
  color: rgba(255, 255, 255, 0.4);
}

.playlists-empty svg {
  margin-bottom: 12px;
  opacity: 0.6;
}

.playlists-empty p {
  margin: 0;
  font-size: 14px;
}

.create-playlist-section {
  margin-bottom: 20px;
  padding-top: 16px;
  border-top: 1px solid rgba(255, 255, 255, 0.1);
}

.create-playlist-form {
  display: flex;
  gap: 8px;
}

.playlist-name-input {
  flex: 1;
  padding: 10px 14px;
  background: rgba(255, 255, 255, 0.05);
  border: 1px solid rgba(255, 255, 255, 0.1);
  border-radius: 8px;
  color: white;
  font-size: 14px;
  outline: none;
  transition: all 0.2s;
}

.playlist-name-input::placeholder {
  color: rgba(255, 255, 255, 0.4);
}

.playlist-name-input:focus {
  border-color: rgba(102, 126, 234, 0.5);
  background: rgba(255, 255, 255, 0.08);
}

.create-playlist-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 10px 16px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border: none;
  border-radius: 8px;
  color: white;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
  white-space: nowrap;
}

.create-playlist-btn:hover:not(:disabled) {
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
}

.create-playlist-btn:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}

.modal-buttons {
  display: flex;
  gap: 12px;
}

.modal-btn {
  flex: 1;
  padding: 12px;
  border: none;
  border-radius: 8px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}

.modal-btn-secondary {
  background: rgba(255, 255, 255, 0.1);
  color: white;
}

.modal-btn-secondary:hover {
  background: rgba(255, 255, 255, 0.2);
}

.modal-enter-active,
.modal-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.modal-enter-from,
.modal-leave-to {
  opacity: 0;
  transform: scale(0.95);
}

.playlist-selector::-webkit-scrollbar {
  width: 4px;
}

.playlist-selector::-webkit-scrollbar-track {
  background: transparent;
}

.playlist-selector::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.2);
  border-radius: 2px;
}

.playlist-selector::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.3);
}

/* 播放模式图标颜色 */
.play-mode-icon {
  filter: brightness(0) invert(1);
}
</style>