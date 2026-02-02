<template>
  <div class="player-bar-wrapper">
    <div class="player-bar glass">
    <div class="player-info">
      <div class="player-cover-wrapper" @click="openPlayer" style="cursor: pointer;">
        <img :src="currentCover" alt="封面" class="player-cover" @error="handleCoverError" />
        <div class="cover-wave" v-if="isPlaying">
          <span></span><span></span><span></span><span></span>
        </div>
      </div>
      <div class="player-details">
        <span class="player-title">{{ currentMusic?.title || '未播放' }}</span>
        <span class="player-artist">{{ currentMusic?.artist || '-' }}</span>
      </div>
    </div>
    
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
        <button class="control-btn play-btn" @click="togglePlay" :title="isPlaying ? '暂停' : '播放'" :disabled="!audioLoaded && currentMusic">
          <svg v-if="!audioLoaded && currentMusic" viewBox="0 0 24 24" width="24" height="24" class="loading-icon">
            <circle cx="12" cy="12" r="10" fill="none" stroke="currentColor" stroke-width="2"/>
            <path d="M12 2 A10 10 0 0 1 22 12" fill="none" stroke="currentColor" stroke-width="2">
              <animateTransform attributeName="transform" type="rotate" from="0 12 12" to="360 12 12" dur="1s" repeatCount="indefinite"/>
            </path>
          </svg>
          <svg v-else-if="!isPlaying" viewBox="0 0 24 24" width="24" height="24">
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
        <button class="control-btn favorite-btn" @click="toggleFavorite" :class="{ 'is-favorite': isFavorite }" :disabled="!currentMusic" :title="isFavorite ? '取消收藏' : '收藏'">
          <svg viewBox="0 0 24 24" width="20" height="20">
            <path :fill="isFavorite ? '#ff4545' : 'currentColor'" d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/>
          </svg>
        </button>
      </div>
      
      <div class="player-progress">
        <span class="time">{{ formatTime(currentTime) }}</span>
        <div class="progress-bar" 
             @mousedown="handleProgressMouseDown" 
             @mousemove="handleProgressMouseMove" 
             @mouseup="handleProgressMouseUp"
             @mouseleave="handleProgressLeave"
             @click="seekTo">
          <div class="progress-fill" :style="{ width: progress + '%' }">
            <div class="progress-glow"></div>
          </div>
          <div class="progress-thumb" :style="{ left: progress + '%' }">
            <div class="thumb-glow"></div>
          </div>
        </div>
        <span class="time">{{ audioLoaded ? formatTime(duration) : '--:--' }}</span>
      </div>
    </div>
    
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
          <div class="volume-slider-vertical" 
               @mousedown="handleVolumeMouseDown" 
               @mousemove="handleVolumeMouseMove">
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
    
    <!-- 播放列表面板 -->
      <Teleport to="body">
      <Transition name="playlist-panel">
      <div class="playlist-panel" v-if="showPlaylistPanel">
        <div class="playlist-header">
                <h3>播放列表</h3>
                <div class="playlist-actions">
                  <span class="playlist-count">{{ playlist.length }} 首歌曲</span>
                </div>
              </div>        <div class="playlist-content">
          <div v-if="playlist.length === 0" class="playlist-empty">
            <svg viewBox="0 0 24 24" width="48" height="48" fill="none" stroke="currentColor" stroke-width="1">
              <path d="M9 18V5l12-2v13"/>
              <circle cx="6" cy="18" r="3"/>
              <circle cx="18" cy="16" r="3"/>
            </svg>
            <p>播放列表为空</p>
            <p class="hint">点击音乐添加到播放列表</p>
          </div>
          <div v-else class="playlist-actions-bar">
            <button class="playlist-action-button" @click="clearPlaylist">
              <svg viewBox="0 0 24 24" width="16" height="16">
                <path fill="currentColor" d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/>
              </svg>
              <span>清空列表</span>
            </button>
          </div>
          <TransitionGroup name="playlist-item" tag="div" class="playlist-items">
            <div 
              v-for="item in playlist" 
              :key="item.localId"
              class="playlist-item"
              :class="{ playing: currentMusic && currentMusic.id === item.id }"
              @click="playFromPlaylist(item.localId)"
            >
              <div class="playlist-item-info">
                <div class="playlist-item-cover">
                  <img :src="`https://music.cnmsb.xin/api/music/cover/${item.id}`" @error="handlePlaylistCoverError" />
                  <div class="playing-indicator" v-if="currentMusic && currentMusic.id === item.id && isPlaying">
                    <span></span><span></span><span></span>
                  </div>
                </div>
                <div class="playlist-item-details">
                  <span class="playlist-item-title">{{ item.title }}</span>
                  <span class="playlist-item-artist">{{ item.artist }}</span>
                </div>
              </div>
              <button class="playlist-item-remove" @click.stop="removeFromPlaylist(item.localId)">
                <svg viewBox="0 0 24 24" width="16" height="16">
                  <path fill="currentColor" d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/>
                </svg>
              </button>
            </div>
          </TransitionGroup>
        </div>
      </div>
      </Transition>
      </Teleport>

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
                <img :src="currentCover" alt="封面" class="current-music-cover" />
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
  </div>
</template>

<script setup>
import { ref, computed, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import apiConfig from '../config/apiConfig'

// 统一的 API 请求函数
async function apiRequest(url, options = {}) {
  const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
  return fetch(fullUrl, options)
}

const router = useRouter()
const currentMusic = ref(null)
const isPlaying = ref(false)
const isMuted = ref(false)
const playMode = ref('list') // list, single, shuffle
const currentTime = ref(0)
const duration = ref(0)
const volume = ref(parseInt(localStorage.getItem('volume')) || 100)
const audioElement = ref(null)
const desktopLyricsEnabled = ref(false)
const fadeInterval = ref(null)
const audioLoaded = ref(false) // 音频是否已加载完成
const playlist = ref([]) // 播放列表
const showPlaylistPanel = ref(false) // 是否显示播放列表面板
const showAddToPlaylistPanel = ref(false) // 是否显示添加到歌单面板
const userPlaylists = ref([]) // 用户歌单列表
const newPlaylistName = ref('') // 新建歌单名称
const FADE_DURATION = 500 // 淡入淡出时长（毫秒）
const FADE_STEPS = 20 // 淡入淡出步数

const playModeTitle = computed(() => {
  const titles = {
    'list': '列表循环',
    'single': '单曲循环',
    'shuffle': '随机播放'
  }
  return titles[playMode.value] || '列表循环'
})

const currentCover = computed(() => {
  if (!currentMusic.value) {
    return 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="56" height="56" viewBox="0 0 56 56"><defs><linearGradient id="grad" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" style="stop-color:%23667eea;stop-opacity:1"/><stop offset="100%" style="stop-color:%23764ba2;stop-opacity:1"/></linearGradient></defs><rect width="56" height="56" fill="url(%23grad)" rx="8"/><text x="28" y="35" font-family="Arial" font-size="24" fill="white" text-anchor="middle" font-weight="bold">M</text></svg>'
  }
  return `https://music.cnmsb.xin/api/music/cover/${currentMusic.value.id}`
})

const isFavorite = ref(false)
const favorites = ref([])

const loadFavorites = async () => {
  const token = localStorage.getItem('token')
  console.log('loadFavorites: token =', token)
  if (!token) {
    console.log('loadFavorites: 未找到 token')
    favorites.value = []
    return
  }
  
  // 先从本地存储读取
  const localFavorites = localStorage.getItem('favorites')
  if (localFavorites) {
    try {
      favorites.value = JSON.parse(localFavorites)
      checkFavoriteStatus()
    } catch (e) {
      console.error('解析本地收藏列表失败:', e)
      favorites.value = []
    }
  }
  
  try {
    const response = await apiRequest(apiConfig.USER_FAVORITES, {
      headers: {
        'Authorization': token
      }
    })
    
    console.log('loadFavorites: response status =', response.status)
    
    if (response.ok) {
      const result = await response.json()
      if (result.success && result.favorites) {
        favorites.value = result.favorites
        // 保存到本地存储
        localStorage.setItem('favorites', JSON.stringify(favorites.value))
        checkFavoriteStatus()
      }
    } else {
      console.log('loadFavorites: 请求失败', response.status)
    }
  } catch (error) {
    console.error('加载收藏列表失败:', error)
  }
}

const checkFavoriteStatus = () => {
  if (!currentMusic.value) return
  isFavorite.value = favorites.value.some(f => f.id === currentMusic.value.id)
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
      // 取消收藏
      const response = await fetch(`${apiConfig.BASE_URL}/api/user/favorites/${currentMusic.value.id}`, {
        method: 'DELETE',
        headers: {
          'Authorization': token
        }
      })
      
      if (response.ok) {
        favorites.value = favorites.value.filter(f => f.id !== currentMusic.value.id)
        isFavorite.value = false
        localStorage.setItem('favorites', JSON.stringify(favorites.value))
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '已取消收藏', type: 'success' } 
        }))
        window.dispatchEvent(new CustomEvent('favorite-changed'))
      } else {
        const result = await response.json()
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: result.message || '取消收藏失败', type: 'error' } 
        }))
      }
    } else {
      // 添加收藏
      const requestBody = {
        musicId: currentMusic.value.id
      }
      console.log('toggleFavorite POST request body:', requestBody)
      console.log('musicId type:', typeof currentMusic.value.id, 'value:', currentMusic.value.id)
      
      const response = await apiRequest(apiConfig.USER_FAVORITES, {
        method: 'POST',
        headers: {
          'Authorization': token,
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(requestBody)
      })
      
      console.log('toggleFavorite POST response status:', response.status)
      const responseText = await response.text()
      console.log('toggleFavorite POST response body:', responseText)
      
      if (response.ok) {
        favorites.value.push(currentMusic.value)
        isFavorite.value = true
        localStorage.setItem('favorites', JSON.stringify(favorites.value))
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '收藏成功', type: 'success' } 
        }))
        window.dispatchEvent(new CustomEvent('favorite-changed'))
      } else {
        const result = await response.json()
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: result.message || '收藏失败', type: 'error' } 
        }))
      }
    }
  } catch (error) {
    console.error('收藏操作失败:', error)
    window.dispatchEvent(new CustomEvent('show-toast', { 
      detail: { message: '网络错误，请稍后重试', type: 'error' } 
    }))
  }
}

const progress = computed(() => {
  if (duration.value === 0) return 0
  return (currentTime.value / duration.value) * 100
})

const formatTime = (seconds) => {
  if (!seconds) return '0:00'
  const mins = Math.floor(seconds / 60)
  const secs = Math.floor(seconds % 60)
  return `${mins}:${secs.toString().padStart(2, '0')}`
}

const handleCoverError = (event) => {
  event.target.src = 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="56" height="56" viewBox="0 0 56 56"><defs><linearGradient id="grad" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" style="stop-color:%23667eea;stop-opacity:1"/><stop offset="100%" style="stop-color:%23764ba2;stop-opacity:1"/></linearGradient></defs><rect width="56" height="56" fill="url(%23grad)" rx="8"/><text x="28" y="35" font-family="Arial" font-size="24" fill="white" text-anchor="middle" font-weight="bold">M</text></svg>'
}

const handlePlaylistCoverError = (event) => {
  event.target.src = 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="40" height="40" viewBox="0 0 40 40"><defs><linearGradient id="grad" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" style="stop-color:%23667eea;stop-opacity:1"/><stop offset="100%" style="stop-color:%23764ba2;stop-opacity:1"/></linearGradient></defs><rect width="40" height="40" fill="url(%23grad)" rx="6"/><text x="20" y="25" font-family="Arial" font-size="18" fill="white" text-anchor="middle" font-weight="bold">M</text></svg>'
}

const togglePlay = () => {
  if (!audioElement.value) return
  
  console.log('togglePlay: 切换播放状态，当前:', isPlaying.value, '音频源:', audioElement.value.src, '音频已加载:', audioLoaded.value, '音频就绪状态:', audioElement.value.readyState)
  
  // 如果要播放但音频未加载完成，不执行播放
  if (!isPlaying.value && !audioLoaded.value) {
    console.log('⚠️ 音频未加载完成，无法播放')
    return
  }
  
  // 立即更新 UI 状态
  isPlaying.value = !isPlaying.value
  notifyPlayerState()
  
  // 执行淡入淡出效果
  if (isPlaying.value) {
    // 淡入播放
    const playPromise = audioElement.value.play()
    if (playPromise !== undefined) {
      playPromise.then(() => {
        console.log('✓ 音频开始播放')
        fadeIn()
      }).catch(err => {
        console.error('✗ 播放失败:', err)
        isPlaying.value = false
      })
    }
  } else {
    // 淡出暂停
    fadeOut(() => {
      audioElement.value.pause()
    })
  }
}

// 淡入效果
const fadeIn = () => {
  if (fadeInterval.value) {
    clearInterval(fadeInterval.value)
  }
  
  // 记录目标音量
  const targetVolume = volume.value / 100
  
  // 从当前音量开始淡入，而不是从0开始
  const startVolume = audioElement.value.volume
  
  // 如果音量已经是目标值或更高，不需要淡入
  if (startVolume >= targetVolume) {
    audioElement.value.volume = targetVolume
    return
  }
  
  const step = (targetVolume - startVolume) / FADE_STEPS
  let currentStep = 0
  
  audioElement.value.volume = startVolume
  
  fadeInterval.value = setInterval(() => {
    currentStep++
    audioElement.value.volume = Math.min(startVolume + currentStep * step, targetVolume)
    
    if (currentStep >= FADE_STEPS) {
      clearInterval(fadeInterval.value)
      fadeInterval.value = null
      audioElement.value.volume = targetVolume
    }
  }, FADE_DURATION / FADE_STEPS)
}

// 淡出效果
const fadeOut = (callback) => {
  if (fadeInterval.value) {
    clearInterval(fadeInterval.value)
  }
  
  const currentVolume = audioElement.value.volume
  const step = currentVolume / FADE_STEPS
  let currentStep = 0
  
  fadeInterval.value = setInterval(() => {
    currentStep++
    audioElement.value.volume = Math.max(currentVolume - currentStep * step, 0)
    
    if (currentStep >= FADE_STEPS) {
      clearInterval(fadeInterval.value)
      fadeInterval.value = null
      if (callback) callback()
    }
  }, FADE_DURATION / FADE_STEPS)
}

const previous = () => {
  if (playlist.value.length === 0) {
    console.log('播放列表为空，无法播放上一首')
    return
  }
  
  if (!currentMusic.value) {
    console.log('没有当前音乐，无法播放上一首')
    return
  }
  
  const currentIndex = playlist.value.findIndex(item => item.id === currentMusic.value.id)
  if (currentIndex === -1) {
    console.log('当前音乐不在播放列表中')
    return
  }
  
  // 播放上一首
  const prevIndex = (currentIndex - 1 + playlist.value.length) % playlist.value.length
  playFromPlaylist(playlist.value[prevIndex].localId)
  
  console.log('播放上一首:', playlist.value[prevIndex].title)
}

const next = () => {
  if (playlist.value.length === 0) {
    console.log('播放列表为空，无法播放下一首')
    return
  }
  
  if (!currentMusic.value) {
    console.log('没有当前音乐，播放第一首')
    playFromPlaylist(playlist.value[0].localId)
    return
  }
  
  const currentIndex = playlist.value.findIndex(item => item.id === currentMusic.value.id)
  if (currentIndex === -1) {
    console.log('当前音乐不在播放列表中，播放第一首')
    playFromPlaylist(playlist.value[0].localId)
    return
  }
  
  let nextIndex = currentIndex + 1
  
  // 根据播放模式决定下一首
  if (playMode.value === 'single') {
    // 单曲循环：重新播放当前音乐
    nextIndex = currentIndex
  } else if (playMode.value === 'shuffle') {
    // 随机播放：随机选择下一首（排除当前）
    do {
      nextIndex = Math.floor(Math.random() * playlist.value.length)
    } while (nextIndex === currentIndex && playlist.value.length > 1)
  } else {
    // 列表循环：播放下一首，到达末尾时循环
    nextIndex = nextIndex % playlist.value.length
  }
  
  playFromPlaylist(playlist.value[nextIndex].localId)
  
  console.log('播放下一首:', playlist.value[nextIndex].title, '播放模式:', playMode.value)
}

const togglePlayMode = () => {
  const modes = ['list', 'single', 'shuffle']
  const currentIndex = modes.indexOf(playMode.value)
  const nextIndex = (currentIndex + 1) % modes.length
  playMode.value = modes[nextIndex]
  notifyPlayerState()
  // 通知 PlayerView 播放模式变化
  window.dispatchEvent(new CustomEvent('player-state-change', {
    detail: { playMode: playMode.value }
  }))
}

const toggleDesktopLyrics = (enabled) => {
  desktopLyricsEnabled.value = enabled
  // 这里可以添加桌面歌词窗口的显示/隐藏逻辑
  console.log('桌面歌词:', enabled ? '开启' : '关闭')
}

// 通知主进程播放状态变化
const notifyPlayerState = () => {
  if (window.electronAPI) {
    window.electronAPI.notifyPlayerState({
      isPlaying: isPlaying.value,
      playMode: playMode.value,
      volume: volume.value,
      desktopLyricsEnabled: desktopLyricsEnabled.value
    })
  }
}

const isDragging = ref(false)
const isVolumeDragging = ref(false)

const seekTo = (event) => {
  if (!audioElement.value || !duration.value) return
  
  const rect = event.currentTarget.getBoundingClientRect()
  const x = event.clientX - rect.left
  const percentage = Math.max(0, Math.min(1, x / rect.width))
  audioElement.value.currentTime = percentage * duration.value
}

const handleProgressMouseDown = (event) => {
  isDragging.value = true
  seekTo(event)
  event.preventDefault()
}

const handleProgressMouseMove = (event) => {
  if (isDragging.value && event.currentTarget) {
    const rect = event.currentTarget.getBoundingClientRect()
    const x = event.clientX - rect.left
    const percentage = Math.max(0, Math.min(1, x / rect.width))
    if (audioElement.value && duration.value) {
      audioElement.value.currentTime = percentage * duration.value
    }
  }
}

const handleGlobalMouseMove = (event) => {
  if (isDragging.value) {
    const progressBar = document.querySelector('.progress-bar')
    if (progressBar) {
      const rect = progressBar.getBoundingClientRect()
      const x = event.clientX - rect.left
      const percentage = Math.max(0, Math.min(1, x / rect.width))
      if (audioElement.value && duration.value) {
        audioElement.value.currentTime = percentage * duration.value
      }
    }
  }
}

const handleProgressMouseUp = () => {
  isDragging.value = false
}

const handleVolumeMouseDown = (event) => {
  isVolumeDragging.value = true
  handleVolumeClick(event)
  event.preventDefault()
}

const handleVolumeMouseMove = (event) => {
  if (isVolumeDragging.value && event.currentTarget) {
    const rect = event.currentTarget.getBoundingClientRect()
    const y = rect.bottom - event.clientY
    const percentage = Math.max(0, Math.min(100, (y / rect.height) * 100))
    volume.value = Math.round(percentage)
    if (audioElement.value) {
      audioElement.value.volume = volume.value / 100
    }
    localStorage.setItem('volume', volume.value.toString())
  }
}

const handleGlobalVolumeMouseMove = (event) => {
  if (isVolumeDragging.value) {
    const volumeSlider = document.querySelector('.volume-slider-vertical')
    if (volumeSlider) {
      const rect = volumeSlider.getBoundingClientRect()
      const y = rect.bottom - event.clientY
      const percentage = Math.max(0, Math.min(100, (y / rect.height) * 100))
      volume.value = Math.round(percentage)
      if (audioElement.value) {
        audioElement.value.volume = volume.value / 100
      }
      localStorage.setItem('volume', volume.value.toString())
    }
  }
}

const handleVolumeMouseUp = () => {
  isVolumeDragging.value = false
}

const handleProgressHover = () => {
  // 可以添加悬停预览功能
}

const handleProgressLeave = () => {
  // 不在这里清除拖动状态，允许拖出进度条
}

const toggleMute = () => {
  isMuted.value = !isMuted.value
  if (audioElement.value) {
    audioElement.value.muted = isMuted.value
  }
  
  // 保存音量设置到本地存储
  localStorage.setItem('volume', volume.value.toString())
}

// 更新媒体信息
const updateMediaInfo = () => {
  // 系统媒体控制已移除，此函数保留但不执行任何操作
}

const handleVolumeChange = () => {
  if (fadeInterval.value) {
    clearInterval(fadeInterval.value)
    fadeInterval.value = null
  }
  
  if (audioElement.value) {
    audioElement.value.volume = volume.value / 100
    isMuted.value = volume.value === 0
  }
  
  // 保存音量设置到本地存储
  localStorage.setItem('volume', volume.value.toString())
}

const togglePlaylist = () => {
  showPlaylistPanel.value = !showPlaylistPanel.value
  console.log('播放列表面板:', showPlaylistPanel.value ? '显示' : '隐藏')

  // 使用 nextTick 检查 DOM 元素
  import('vue').then(({ nextTick }) => {
    nextTick(() => {
      const panel = document.querySelector('.playlist-panel')
      if (panel) {
        console.log('播放列表面板 DOM 元素存在，样式:', window.getComputedStyle(panel).display, 'opacity:', window.getComputedStyle(panel).opacity, 'visibility:', window.getComputedStyle(panel).visibility, 'z-index:', window.getComputedStyle(panel).zIndex)
      } else {
        console.log('播放列表面板 DOM 元素不存在')
      }
    })
  })
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
      
      // 通知侧边栏刷新歌单列表
      window.dispatchEvent(new CustomEvent('playlist-updated', {
        detail: {
          id: playlistId,
          action: 'music-added'
        }
      }))
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

// 添加音乐到播放列表
const addToPlaylist = (music) => {
  console.log('addToPlaylist: 收到添加请求', music)
  
  if (!music) {
    console.log('音乐数据为空')
    return
  }
  
  // 检查是否已存在
  const exists = playlist.value.some(item => item.id === music.id)
  if (exists) {
    console.log('音乐已在播放列表中:', music.title)
    return
  }
  
  // 生成本地 ID
  const localId = Date.now() + Math.random().toString(36).substr(2, 9)
  
  // 添加到播放列表
  playlist.value.push({
    ...music,
    localId: localId
  })
  
  // 保存到 localStorage
  savePlaylist()
  
  console.log('添加到播放列表成功:', music.title, '当前播放列表总数:', playlist.value.length)
}

// 添加多个音乐到播放列表
const addAllToPlaylist = (musicList) => {
  console.log('addAllToPlaylist: 收到添加全部请求，音乐数量:', musicList?.length)
  
  if (!musicList || !Array.isArray(musicList) || musicList.length === 0) {
    console.log('无效的音乐列表')
    return
  }
  
  let addedCount = 0
  
  musicList.forEach(music => {
    console.log('处理音乐:', music.title, 'ID:', music.id)
    
    // 检查是否已存在
    const exists = playlist.value.some(item => item.id === music.id)
    if (!exists) {
      // 生成本地 ID
      const localId = Date.now() + Math.random().toString(36).substr(2, 9)
      
      // 添加到播放列表
      playlist.value.push({
        ...music,
        localId: localId
      })
      
      addedCount++
    } else {
      console.log('音乐已存在，跳过:', music.title)
    }
  })
  
  // 保存到 localStorage
  savePlaylist()
  
  console.log('批量添加到播放列表完成:', addedCount, '首音乐，当前播放列表总数:', playlist.value.length)
}

// 从播放列表移除音乐
const removeFromPlaylist = (localId) => {
  const index = playlist.value.findIndex(item => item.localId === localId)
  if (index > -1) {
    playlist.value.splice(index, 1)
    savePlaylist()
    console.log('从播放列表移除音乐')
  }
}

// 清空播放列表
const clearPlaylist = () => {
  playlist.value = []
  savePlaylist()
  console.log('清空播放列表')
}

// 保存播放列表到 localStorage
const savePlaylist = () => {
  try {
    localStorage.setItem('playlist', JSON.stringify(playlist.value))
    console.log('✓ 播放列表已保存到 localStorage，共', playlist.value.length, '首音乐')
    
    // 验证保存是否成功
    const saved = localStorage.getItem('playlist')
    if (saved) {
      const parsed = JSON.parse(saved)
      console.log('✓ 验证成功: localStorage 中有', parsed.length, '首音乐')
    } else {
      console.error('✗ 验证失败: localStorage 中没有播放列表数据')
    }
  } catch (e) {
    console.error('✗ 保存播放列表失败:', e)
  }
}

// 从 localStorage 加载播放列表
const loadPlaylist = () => {
  try {
    const saved = localStorage.getItem('playlist')
    console.log('loadPlaylist: 从 localStorage 读取播放列表，数据:', saved?.substring(0, 100))
    
    if (saved) {
      playlist.value = JSON.parse(saved)
      console.log('✓ 播放列表加载成功，共', playlist.value.length, '首音乐')
    } else {
      console.log('localStorage 中没有播放列表数据')
    }
  } catch (e) {
    console.error('✗ 加载播放列表失败:', e)
  }
}

// 播放播放列表中的指定音乐
const playFromPlaylist = (localId) => {
  const music = playlist.value.find(item => item.localId === localId)
  if (music) {
    loadMusic(music)
    // 等待音频加载完成后再播放
    const checkAndPlay = () => {
      if (audioLoaded.value) {
        console.log('playFromPlaylist: 音频已加载，开始播放')
        if (audioElement.value) {
          audioElement.value.play().then(() => {
            console.log('✓ 音频开始播放')
            isPlaying.value = true
            fadeIn()
          }).catch(err => {
            console.error('✗ 播放失败:', err)
            isPlaying.value = false
          })
        }
        audioElement.value.removeEventListener('loadedmetadata', checkAndPlay)
      }
    }
    
    // 如果已经加载完成，立即播放
    if (audioLoaded.value) {
      audioElement.value.play().then(() => {
        console.log('✓ 音频开始播放')
        isPlaying.value = true
        fadeIn()
      }).catch(err => {
        console.error('✗ 播放失败:', err)
        isPlaying.value = false
      })
    } else {
      // 否则等待 loadedmetadata 事件
      audioElement.value.addEventListener('loadedmetadata', checkAndPlay)
    }
  }
}

// 播放全部
const playAll = () => {
  if (playlist.value.length === 0) {
    console.log('播放列表为空，无法播放全部')
    return
  }
  
  // 播放第一首
  playFromPlaylist(playlist.value[0].localId)
  console.log('开始播放全部音乐，共', playlist.value.length, '首')
}

const handleVolumeClick = (event) => {
  const rect = event.currentTarget.getBoundingClientRect()
  const y = rect.bottom - event.clientY
  const percentage = Math.min(100, Math.max(0, (y / rect.height) * 100))
  volume.value = Math.round(percentage)
  handleVolumeChange()
}

// 添加到最近播放
const addToRecent = (music) => {
  if (!music) return
  
  try {
    // 获取当前最近播放列表
    const recentData = localStorage.getItem('recentPlay')
    let recentList = []
    
    if (recentData) {
      try {
        recentList = JSON.parse(recentData)
      } catch (e) {
        console.error('解析最近播放列表失败:', e)
        recentList = []
      }
    }
    
    // 移除已存在的相同音乐
    const index = recentList.findIndex(m => m.id === music.id)
    if (index !== -1) {
      recentList.splice(index, 1)
    }
    
    // 添加到开头
    recentList.unshift(music)
    
    // 保存到 localStorage
    localStorage.setItem('recentPlay', JSON.stringify(recentList))
    
    console.log('✓ 已添加到最近播放:', music.title, '当前最近播放总数:', recentList.length)
  } catch (e) {
    console.error('添加到最近播放失败:', e)
  }
}

const loadMusic = async (music) => {
  if (!music) return

  console.log('loadMusic: 加载音乐', music.title)
  
  currentMusic.value = music
  isPlaying.value = false
  currentTime.value = 0
  duration.value = music.duration || 0
  audioLoaded.value = false // 重置加载状态
  
  // 检查收藏状态
  checkFavoriteStatus()
  
  // 保存到 localStorage
  localStorage.setItem('currentMusic', JSON.stringify(music))
  
  // 添加到最近播放
  addToRecent(music)
  
  // 通知 PlayerView 音乐已切换
  window.dispatchEvent(new CustomEvent('music-changed', {
    detail: music
  }))
  
  // 加载歌词并缓存到内存
  try {
    console.log('loadMusic: 开始加载歌词')
    const lyricsUrl = `${apiConfig.BASE_URL}${apiConfig.MUSIC_LYRICS(music.id)}`
    const response = await fetch(lyricsUrl)
    const result = await response.json()
    
    if (result.success && result.data) {
      console.log('loadMusic: 歌词加载成功')
      window.cachedLyrics = window.cachedLyrics || {}
      window.cachedLyrics[music.id] = result.data
    }
  } catch (error) {
    console.error('loadMusic: 加载歌词失败:', error)
  }
  
  if (audioElement.value) {
    audioElement.value.src = `https://music.cnmsb.xin/api/music/file/${music.id}`
    console.log('✓ 音频已加载到元素，等待 loadedmetadata 事件')
    audioElement.value.load()
  }
  
  // 通知主进程 (已移除)
}
const handleTimeUpdate = () => {
  if (audioElement.value && audioLoaded.value) {
    currentTime.value = audioElement.value.currentTime
    
    // 通知播放页面更新
    window.dispatchEvent(new CustomEvent('player-state-change', {
      detail: {
        isPlaying: isPlaying.value,
        currentTime: currentTime.value,
        duration: duration.value,
        playMode: playMode.value,
        volume: volume.value
      }
    }))
    
    // 广播音频时间更新（用于歌词同步）
    window.dispatchEvent(new CustomEvent('audio-time-update', {
      detail: {
        currentTime: currentTime.value,
        duration: duration.value
      }
    }))
  }
}

const handleLoadedMetadata = () => {
  if (audioElement.value) {
    duration.value = audioElement.value.duration
    audioLoaded.value = true
    console.log('✓ 音频元数据已加载完成，时长:', duration.value)
  }
}

const handleCanPlay = () => {
  audioLoaded.value = true
  console.log('✓ 音频可以播放')
}

const handleError = (error) => {
  console.error('✗ 音频加载错误:', error)
  audioLoaded.value = false
  isPlaying.value = false
}

const handleEnded = () => {
  console.log('音乐播放完成，当前播放模式:', playMode.value)
  
  // 根据播放模式决定下一步操作
  if (playMode.value === 'single') {
    // 单曲循环：重新播放当前音乐
    console.log('单曲循环：重新播放')
    if (audioElement.value && currentMusic.value) {
      audioElement.value.currentTime = 0
      audioElement.value.play()
    }
  } else {
    // 列表循环或随机播放：播放下一首
    next()
  }
}

const handleMusicPlay = (event) => {
  const music = event.detail
  loadMusic(music)
  // 添加到最近播放
  addToRecent(music)
  // 等待音频加载完成后再播放
  if (audioElement.value) {
    const checkAndPlay = () => {
      if (audioLoaded.value) {
        console.log('handleMusicPlay: 音频已加载，开始播放')
        // 使用淡入效果播放
        audioElement.value.play().then(() => {
          console.log('✓ 音频开始播放')
          isPlaying.value = true
          fadeIn()
        }).catch(err => {
          console.error('✗ 播放失败:', err)
          isPlaying.value = false
        })
        audioElement.value.removeEventListener('loadedmetadata', checkAndPlay)
      }
    }
    
    // 如果已经加载完成，立即播放
    if (audioLoaded.value) {
      audioElement.value.play().then(() => {
        console.log('✓ 音频开始播放')
        isPlaying.value = true
        fadeIn()
      }).catch(err => {
        console.error('✗ 播放失败:', err)
        isPlaying.value = false
      })
    } else {
      // 否则等待 loadedmetadata 事件
      audioElement.value.addEventListener('loadedmetadata', checkAndPlay)
    }
  }
}

onMounted(() => {
  audioElement.value = new Audio()
  audioElement.value.volume = volume.value / 100
  
  audioElement.value.addEventListener('timeupdate', handleTimeUpdate)
  audioElement.value.addEventListener('loadedmetadata', handleLoadedMetadata)
  audioElement.value.addEventListener('canplay', handleCanPlay)
  audioElement.value.addEventListener('error', handleError)
  audioElement.value.addEventListener('ended', handleEnded)
  
  window.addEventListener('music-play', handleMusicPlay)
  window.addEventListener('add-to-playlist', (event) => {
    addToPlaylist(event.detail)
  })
  window.addEventListener('add-all-to-playlist', (event) => {
    addAllToPlaylist(event.detail)
  })
  window.addEventListener('clear-playlist', () => {
    clearPlaylist()
  })
  
  // 监听播放页面的同步请求
  window.addEventListener('get-player-state', () => {
    window.dispatchEvent(new CustomEvent('player-state-change', {
      detail: {
        isPlaying: isPlaying.value,
        currentTime: currentTime.value,
        duration: duration.value,
        playMode: playMode.value
      }
    }))
  })
  
  // 监听播放页面的同步
  window.addEventListener('player-state-sync', (event) => {
    if (event.detail?.isPlaying !== undefined) {
      isPlaying.value = event.detail.isPlaying
    }
    if (event.detail?.currentTime !== undefined) {
      currentTime.value = event.detail.currentTime
      if (audioElement.value) {
        audioElement.value.currentTime = event.detail.currentTime
      }
    }
    if (event.detail?.duration !== undefined) {
      duration.value = event.detail.duration
    }
    if (event.detail?.playMode !== undefined) {
      playMode.value = event.detail.playMode
    }
  })
  
  // 监听播放页面的控制请求
  window.addEventListener('toggle-play', () => {
    togglePlay()
  })
  
  window.addEventListener('seek-to', (event) => {
    if (audioElement.value && duration.value) {
      audioElement.value.currentTime = event.detail
    }
  })
  
  window.addEventListener('toggle-mute', () => {
    toggleMute()
  })
  
  window.addEventListener('set-volume', (event) => {
    volume.value = event.detail
    if (audioElement.value) {
      audioElement.value.volume = volume.value / 100
    }
  })
  
  window.addEventListener('toggle-playlist-panel', () => {
    togglePlaylist()
  })
  
  loadFavorites()
  
  window.addEventListener('user-login', loadFavorites)
  window.addEventListener('user-logout', () => {
    favorites.value = []
    isFavorite.value = false
  })
  
  // 监听托盘事件
  window.addEventListener('tray-previous', previous)
  window.addEventListener('tray-play-pause', togglePlay)
  window.addEventListener('tray-next', next)
  window.addEventListener('tray-favorite', handleTrayFavorite)
  window.addEventListener('tray-set-play-mode', (event) => {
    playMode.value = event.detail
    notifyPlayerState()
    // 通知 PlayerView 播放模式变化
    window.dispatchEvent(new CustomEvent('player-state-change', {
      detail: { playMode: playMode.value }
    }))
  })
  window.addEventListener('tray-toggle-desktop-lyrics', (event) => {
    toggleDesktopLyrics(event.detail)
  })
  window.addEventListener('navigate-to-settings', handleNavigateToSettings)
  
  // 全局鼠标事件，处理拖动进度条和音量
  window.addEventListener('mouseup', (event) => {
    handleProgressMouseUp()
    handleVolumeMouseUp()
  })
  window.addEventListener('mousemove', (event) => {
    handleGlobalMouseMove(event)
    handleGlobalVolumeMouseMove(event)
  })
  
  // 恢复之前播放的音乐
  const savedMusic = localStorage.getItem('currentMusic')
  if (savedMusic) {
    try {
      const music = JSON.parse(savedMusic)
      loadMusic(music)
      isPlaying.value = false // 恢复时默认暂停
    } catch (e) {
      console.error('解析音乐失败:', e)
    }
  }
  
  // 加载播放列表
  loadPlaylist()
})

onUnmounted(() => {
  if (fadeInterval.value) {
    clearInterval(fadeInterval.value)
    fadeInterval.value = null
  }
  
  if (audioElement.value) {
    audioElement.value.removeEventListener('timeupdate', handleTimeUpdate)
    audioElement.value.removeEventListener('loadedmetadata', handleLoadedMetadata)
    audioElement.value.removeEventListener('canplay', handleCanPlay)
    audioElement.value.removeEventListener('error', handleError)
    audioElement.value.removeEventListener('ended', handleEnded)
    audioElement.value.pause()
  }
  window.removeEventListener('music-play', handleMusicPlay)
  window.removeEventListener('add-to-playlist', addToPlaylist)
  window.removeEventListener('add-all-to-playlist', addAllToPlaylist)
  window.removeEventListener('clear-playlist', clearPlaylist)
  window.removeEventListener('tray-previous', previous)
  window.removeEventListener('tray-play-pause', togglePlay)
  window.removeEventListener('tray-next', next)
  window.removeEventListener('tray-favorite', handleTrayFavorite)
  window.removeEventListener('tray-set-play-mode', togglePlayMode)
  window.removeEventListener('tray-toggle-desktop-lyrics', toggleDesktopLyrics)
  window.removeEventListener('navigate-to-settings', handleNavigateToSettings)
  window.removeEventListener('media-play-pause', togglePlay)
  })

const handleTrayFavorite = () => {
  if (currentMusic.value) {
    toggleFavorite(currentMusic.value)
  }
}

const openPlayer = () => {
  if (currentMusic.value) {
    router.push('/player')
  }
}

const handleNavigateToSettings = () => {
  const router = useRouter()
  router.push('/settings')
}
</script>

<style scoped>
.player-bar-wrapper {
  /* 不设置 position: relative，避免创建层叠上下文 */
}

.player-bar {
  height: 90px;
  border-top: 1px solid var(--border-light);
  display: flex;
  align-items: center;
  padding: 0 24px;
  gap: 24px;
  position: relative;
  z-index: 100;
}

.player-bar::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  height: 1px;
  background: linear-gradient(90deg, transparent, rgba(102, 126, 234, 0.3), transparent);
}

.player-info {
  display: flex;
  align-items: center;
  gap: 14px;
  width: 280px;
  flex-shrink: 0;
}

.player-cover-wrapper {
  position: relative;
  width: 60px;
  height: 60px;
  flex-shrink: 0;
}

.player-cover {
  width: 100%;
  height: 100%;
  border-radius: 12px;
  object-fit: cover;
  box-shadow: var(--shadow-md);
  transition: all var(--transition-normal);
}

.player-cover-wrapper:hover .player-cover {
  transform: scale(1.05);
  box-shadow: var(--shadow-lg);
}

.cover-wave {
  position: absolute;
  bottom: -8px;
  left: 50%;
  transform: translateX(-50%);
  display: flex;
  gap: 3px;
  align-items: flex-end;
  height: 12px;
}

.cover-wave span {
  width: 3px;
  background: var(--gradient-primary);
  border-radius: 2px;
  animation: wave 1s ease-in-out infinite;
}

.cover-wave span:nth-child(1) { animation-delay: 0s; height: 6px; }
.cover-wave span:nth-child(2) { animation-delay: 0.1s; height: 10px; }
.cover-wave span:nth-child(3) { animation-delay: 0.2s; height: 8px; }
.cover-wave span:nth-child(4) { animation-delay: 0.3s; height: 12px; }

@keyframes wave {
  0%, 100% { transform: scaleY(1); }
  50% { transform: scaleY(0.5); }
}

.player-details {
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.player-title {
  font-size: 15px;
  font-weight: 600;
  color: var(--text-primary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.player-artist {
  font-size: 13px;
  color: var(--text-secondary);
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
  color: var(--text-secondary);
  transition: all var(--transition-normal);
}

.control-btn svg {
  fill: currentColor;
}

.control-btn img {
  filter: brightness(0) saturate(100%) opacity(0.6);
  transition: all var(--transition-normal);
}

.control-btn:hover {
  background: rgba(102, 126, 234, 0.1);
  color: var(--primary);
  transform: scale(1.1);
}

.control-btn:hover img {
  filter: brightness(0) saturate(100%) opacity(1) hue-rotate(240deg);
}

.control-btn.active {
  color: var(--primary);
  background: rgba(102, 126, 234, 0.1);
}

.control-btn.active img {
  filter: brightness(0) saturate(100%) opacity(1) hue-rotate(240deg);
}

.play-btn {
  width: 48px;
  height: 48px;
  background: var(--gradient-primary);
  color: white;
  box-shadow: 0 4px 16px rgba(102, 126, 234, 0.4);
}

.favorite-btn:hover:not(:disabled) {
  background: rgba(255, 69, 69, 0.1);
}

.favorite-btn.is-favorite {
  color: #ff4545;
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
  color: var(--text-muted);
  min-width: 42px;
  text-align: center;
  font-weight: 500;
}

.progress-bar {
  flex: 1;
  height: 6px;
  background: rgba(0, 0, 0, 0.1);
  border-radius: 3px;
  cursor: pointer;
  position: relative;
  overflow: hidden;
  user-select: none;
}

.loading-text {
  font-size: 11px;
  color: var(--text-secondary);
  font-weight: 500;
}

.loading-icon {
  animation: spin 1s linear infinite;
}

.progress-fill {
  height: 100%;
  background: var(--gradient-primary);
  border-radius: 3px;
  position: relative;
  transition: width 0.1s;
}

.progress-glow {
  position: absolute;
  top: -4px;
  left: 0;
  right: 0;
  bottom: -4px;
  background: var(--gradient-primary);
  filter: blur(8px);
  opacity: 0.5;
}

.progress-thumb {
  position: absolute;
  top: 50%;
  width: 14px;
  height: 14px;
  background: white;
  border-radius: 50%;
  transform: translate(-50%, -50%);
  box-shadow: var(--shadow-md);
  opacity: 0;
  transition: all var(--transition-fast);
}

.progress-bar:hover .progress-thumb {
  opacity: 1;
}

.thumb-glow {
  position: absolute;
  top: -4px;
  left: -4px;
  right: -4px;
  bottom: -4px;
  background: var(--gradient-primary);
  border-radius: 50%;
  filter: blur(6px);
  opacity: 0.6;
}

.player-controls-right {
  display: flex;
  align-items: center;
  gap: 8px;
  flex-shrink: 0;
}

.volume-wrapper {
  position: relative;
  display: flex;
  align-items: center;
}

.volume-wrapper:hover .volume-panel {
  opacity: 1;
  visibility: visible;
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

.playlist-panel {
  position: fixed;
  bottom: 100px;
  right: 24px;
  width: 380px;
  max-height: 500px;
  background: rgba(255, 255, 255, 0.95);
  backdrop-filter: blur(20px);
  border-radius: 16px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.15);
  display: flex;
  flex-direction: column;
  z-index: 100000;
  overflow: hidden;
  border: 1px solid rgba(255, 255, 255, 0.5);
  pointer-events: auto;
}

.playlist-panel-enter-active,
.playlist-panel-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.playlist-panel-enter-from {
  opacity: 0;
  transform: translateY(20px) scale(0.95);
}

.playlist-panel-leave-to {
  opacity: 0;
  transform: translateY(20px) scale(0.95);
}

.playlist-header {
  padding: 16px 20px;
  border-bottom: 1px solid rgba(0, 0, 0, 0.1);
  display: flex;
  justify-content: space-between;
  align-items: center;
  background: rgba(102, 126, 234, 0.05);
}

.playlist-header h3 {
  margin: 0;
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
}

.playlist-actions {
  display: flex;
  align-items: center;
  gap: 12px;
}

.playlist-count {
  font-size: 13px;
  color: var(--text-secondary);
  font-weight: 500;
}

.playlist-action-btn {
  padding: 6px 14px;
  font-size: 12px;
  font-weight: 500;
  color: var(--text-secondary);
  background: rgba(0, 0, 0, 0.05);
  border: none;
  border-radius: 8px;
  cursor: pointer;
  transition: all var(--transition-fast);
}

.playlist-action-btn:hover {
  background: rgba(255, 59, 48, 0.1);
  color: #ff3b30;
  transform: translateY(-1px);
}

.playlist-content {
  flex: 1;
  overflow-y: auto;
  max-height: 400px;
  padding: 8px 0;
}

.playlist-content::-webkit-scrollbar {
  width: 6px;
}

.playlist-content::-webkit-scrollbar-track {
  background: transparent;
}

.playlist-content::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 3px;
}

.playlist-content::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}

.playlist-actions-bar {
  display: flex;
  gap: 8px;
  padding: 12px 16px;
  border-bottom: 1px solid rgba(0, 0, 0, 0.05);
}

.playlist-action-button {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 6px;
  padding: 8px 12px;
  font-size: 13px;
  font-weight: 500;
  color: var(--text-primary);
  background: rgba(102, 126, 234, 0.1);
  border: 1px solid rgba(102, 126, 234, 0.2);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.playlist-action-button:hover {
  background: rgba(102, 126, 234, 0.2);
  border-color: rgba(102, 126, 234, 0.3);
  transform: translateY(-1px);
  box-shadow: 0 2px 8px rgba(102, 126, 234, 0.2);
}

.playlist-action-button:active {
  transform: translateY(0);
}

.playlist-action-button svg {
  flex-shrink: 0;
}

.playlist-empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 60px 20px;
  color: var(--text-muted);
}

.playlist-empty svg {
  margin-bottom: 16px;
  opacity: 0.4;
  filter: drop-shadow(0 2px 4px rgba(0, 0, 0, 0.1));
}

.playlist-empty p {
  margin: 4px 0;
  font-size: 14px;
}

.playlist-empty .hint {
  font-size: 12px;
  opacity: 0.7;
}

.playlist-items {
  display: flex;
  flex-direction: column;
  padding: 0 8px;
}

.playlist-item {
  display: flex;
  align-items: center;
  padding: 10px 12px;
  margin: 4px 0;
  cursor: pointer;
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
  border-radius: 12px;
  position: relative;
  overflow: hidden;
}

.playlist-item::before {
  content: '';
  position: absolute;
  left: 0;
  top: 0;
  bottom: 0;
  width: 3px;
  background: var(--gradient-primary);
  border-radius: 3px 0 0 3px;
  opacity: 0;
  transition: opacity 0.2s ease;
}

.playlist-item:hover {
  background: rgba(102, 126, 234, 0.08);
  transform: translateX(2px);
}

.playlist-item:hover::before {
  opacity: 0.5;
}

.playlist-item.playing {
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.15), rgba(118, 75, 162, 0.15));
  box-shadow: 0 2px 8px rgba(102, 126, 234, 0.2);
}

.playlist-item.playing::before {
  opacity: 1;
}

.playlist-item-enter-active,
.playlist-item-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.playlist-item-enter-from {
  opacity: 0;
  transform: translateX(30px);
}

.playlist-item-leave-to {
  opacity: 0;
  transform: translateX(-30px);
  height: 0;
  padding: 0;
  margin: 0;
}

.playlist-item-info {
  flex: 1;
  display: flex;
  align-items: center;
  gap: 12px;
  min-width: 0;
}

.playlist-item-cover {
  position: relative;
  width: 44px;
  height: 44px;
  border-radius: 10px;
  overflow: hidden;
  flex-shrink: 0;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
  transition: transform 0.2s ease;
}

.playlist-item:hover .playlist-item-cover {
  transform: scale(1.05);
}

.playlist-item-cover img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.playing-indicator {
  position: absolute;
  bottom: 0;
  left: 50%;
  transform: translateX(-50%);
  display: flex;
  gap: 2px;
  align-items: flex-end;
  height: 10px;
  padding-bottom: 2px;
}

.playing-indicator span {
  width: 2px;
  background: white;
  border-radius: 1px;
  animation: playlist-wave 1s ease-in-out infinite;
  box-shadow: 0 0 4px rgba(0, 0, 0, 0.3);
}

.playing-indicator span:nth-child(1) { animation-delay: 0s; height: 4px; }
.playing-indicator span:nth-child(2) { animation-delay: 0.1s; height: 6px; }
.playing-indicator span:nth-child(3) { animation-delay: 0.2s; height: 8px; }

@keyframes playlist-wave {
  0%, 100% { transform: scaleY(1); }
  50% { transform: scaleY(0.5); }
}

.playlist-item-details {
  flex: 1;
  min-width: 0;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.playlist-item-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  margin-bottom: 2px;
}

.playlist-item.playing .playlist-item-title {
  color: var(--primary);
}

.playlist-item-artist {
  font-size: 12px;
  color: var(--text-secondary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.playlist-item-remove {
  width: 32px;
  height: 32px;
  border: none;
  background: transparent;
  border-radius: 8px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--text-muted);
  transition: all 0.2s ease;
  flex-shrink: 0;
  margin-left: 8px;
  opacity: 0;
  transform: translateX(10px);
}

.playlist-item:hover .playlist-item-remove {
  opacity: 1;
  transform: translateX(0);
}

.playlist-item-remove:hover {
  background: rgba(255, 59, 48, 0.15);
  color: #ff3b30;
  transform: scale(1.1) !important;
}

.playlist-item-remove:active {
  transform: scale(0.95) !important;
}
</style>

<style>
/* 全局样式 - 确保播放列表面板正确显示 */
.playlist-panel {
  position: fixed !important;
  bottom: 100px !important;
  right: 24px !important;
  z-index: 2147483647 !important;
  pointer-events: auto !important;
  transform: translateZ(0) !important;
  isolation: isolate !important;
}

/* 添加到歌单模态框 */
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
  color: var(--text-muted);
  transition: all 0.2s;
}

.modal-close-btn:hover {
  background: rgba(255, 255, 255, 0.2);
  color: white;
}

/* 当前音乐信息 */
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
  color: var(--text-secondary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

/* 歌单区域 */
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
  color: var(--text-secondary);
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
  color: var(--text-muted);
  flex-shrink: 0;
}

.playlists-empty {
  text-align: center;
  padding: 40px 20px;
  color: var(--text-muted);
}

.playlists-empty svg {
  margin-bottom: 12px;
  opacity: 0.4;
}

.playlists-empty p {
  margin: 0;
  font-size: 14px;
}

/* 新建歌单区域 */
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
  color: var(--text-muted);
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
  background: var(--gradient-primary);
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

/* 滚动条样式 */
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
  filter: brightness(0);
}

/* ========================================
   响应式适配
   ======================================== */

/* 小屏幕适配 (< 640px) */
@media screen and (max-width: 639px) {
  .player-bar-wrapper {
    position: fixed;
    bottom: 0;
    left: 0;
    right: 0;
    z-index: 9998;
  }
  
  .player-bar {
    height: var(--player-bar-height);
    padding: clamp(6px, 1.5vw, 10px);
    gap: clamp(8px, 2vw, 12px);
    flex-wrap: nowrap;
    overflow: hidden;
  }
  
  .player-info {
    width: clamp(120px, 30vw, 160px);
    gap: clamp(8px, 2vw, 12px);
  }
  
  .player-cover-wrapper {
    width: clamp(36px, 10vw, 44px);
    height: clamp(36px, 10vw, 44px);
    flex-shrink: 0;
  }
  
  .player-cover {
    border-radius: 6px;
  }
  
  .player-details {
    margin-left: 0;
    min-width: 0;
    flex: 1;
  }
  
  .player-title {
    font-size: clamp(12px, 3vw, 14px);
    max-width: 100%;
  }
  
  .player-artist {
    font-size: clamp(10px, 2.5vw, 12px);
  }
  
  .player-controls-main {
    min-width: 0;
    flex: 1;
    gap: clamp(4px, 1vw, 6px);
  }
  
  .control-buttons {
    gap: clamp(2px, 0.5vw, 6px);
  }
  
  .control-btn {
    width: clamp(28px, 7vw, 36px);
    height: clamp(28px, 7vw, 36px);
    flex-shrink: 0;
  }
  
  .play-btn {
    width: clamp(36px, 9vw, 44px);
    height: clamp(36px, 9vw, 44px);
  }
  
  .player-progress {
    gap: clamp(4px, 1vw, 8px);
    min-width: 0;
  }
  
  .time {
    font-size: clamp(10px, 2.5vw, 12px);
    min-width: clamp(24px, 6vw, 32px);
  }
  
  .progress-bar {
    height: 3px;
    flex: 1;
    min-width: 60px;
  }
  
  .progress-thumb {
    width: 8px;
    height: 8px;
  }
  
  .player-controls-right {
    gap: clamp(4px, 1vw, 6px);
    flex-shrink: 0;
  }
  
  .volume-wrapper {
    display: none;
  }
  
  .volume-panel {
    display: none;
  }
  
  .playlist-panel {
    bottom: var(--player-bar-height);
    max-height: calc(100vh - var(--player-bar-height) - 20px);
  }
  
  .playlist-content {
    padding: clamp(12px, 3vw, 16px);
  }
  
  .playlist-item {
    padding: clamp(10px, 2.5vw, 14px);
  }
  
  .playlist-item-cover {
    width: clamp(32px, 8vw, 40px);
    height: clamp(32px, 8vw, 40px);
  }
  
  .playlist-item-title {
    font-size: clamp(12px, 3vw, 14px);
  }
  
  .playlist-item-artist {
    font-size: clamp(10px, 2.5vw, 12px);
  }
}

/* 超小屏幕适配 (< 480px) */
@media screen and (max-width: 479px) {
  .player-bar {
    padding: 6px;
    gap: 6px;
  }
  
  .player-info {
    width: 100px;
    gap: 6px;
  }
  
  .player-cover-wrapper {
    width: 36px;
    height: 36px;
  }
  
  .player-title {
    font-size: 12px;
  }
  
  .player-artist {
    font-size: 11px;
  }
  
  .player-controls-main {
    gap: 4px;
  }
  
  .control-buttons {
    gap: 2px;
  }
  
  .control-btn {
    width: 28px;
    height: 28px;
  }
  
  .play-btn {
    width: 32px;
    height: 32px;
  }
  
  .control-btn svg,
  .play-btn svg {
    width: 16px;
    height: 16px;
  }
  
  .player-progress {
    gap: 4px;
  }
  
  .time {
    font-size: 10px;
    min-width: 24px;
  }
  
  .progress-bar {
    min-width: 50px;
  }
  
  .progress-thumb {
    width: 6px;
    height: 6px;
  }
  
  .player-controls-right {
    gap: 4px;
  }
  
  .player-controls-right .control-btn {
    width: 28px;
    height: 28px;
  }
  
  .player-controls-right .control-btn svg {
    width: 16px;
    height: 16px;
  }
}

/* 极小屏幕适配 (< 380px) */
@media screen and (max-width: 379px) {
  .player-bar {
    padding: 4px;
    gap: 4px;
  }
  
  .player-info {
    width: 90px;
    gap: 4px;
  }
  
  .player-cover-wrapper {
    width: 32px;
    height: 32px;
  }
  
  .player-title {
    font-size: 11px;
  }
  
  .player-artist {
    font-size: 10px;
  }
  
  .control-buttons {
    gap: 1px;
  }
  
  .control-btn {
    width: 24px;
    height: 24px;
  }
  
  .play-btn {
    width: 28px;
    height: 28px;
  }
  
  .control-btn svg,
  .play-btn svg {
    width: 14px;
    height: 14px;
  }
  
  .time {
    font-size: 9px;
    min-width: 20px;
  }
  
  .progress-bar {
    min-width: 40px;
  }
  
  .player-controls-right .control-btn {
    width: 24px;
    height: 24px;
  }
}

/* 中等屏幕适配 (640px - 1023px) */
@media screen and (min-width: 640px) and (max-width: 1023px) {
  .player-bar {
    padding: 12px clamp(16px, 3vw, 20px);
  }
  
  .player-cover-wrapper {
    width: 48px;
    height: 48px;
  }
  
  .player-details {
    margin-left: 12px;
  }
  
  .player-title {
    font-size: 14px;
    max-width: 180px;
  }
  
  .player-artist {
    font-size: 12px;
  }
  
  .volume-panel {
    display: none;
  }
  
  .playlist-panel {
    right: clamp(10px, 2vw, 20px);
    bottom: calc(var(--player-bar-height) + 10px);
    width: clamp(300px, 40vw, 400px);
  }
}

/* 大屏幕适配 (1024px - 1279px) */
@media screen and (min-width: 1024px) and (max-width: 1279px) {
  .player-bar {
    padding: 14px 24px;
  }
  
  .player-cover-wrapper {
    width: 52px;
    height: 52px;
  }
  
  .player-title {
    max-width: 200px;
  }
}

/* 2K屏幕适配 (1536px - 1919px) */
@media screen and (min-width: 1536px) and (max-width: 1919px) {
  .player-bar {
    padding: 18px 32px;
    height: var(--player-bar-height);
  }
  
  .player-cover-wrapper {
    width: 60px;
    height: 60px;
  }
  
  .player-cover {
    border-radius: 12px;
  }
  
  .player-details {
    margin-left: 16px;
  }
  
  .player-title {
    font-size: 16px;
    max-width: 240px;
  }
  
  .player-artist {
    font-size: 14px;
  }
  
  .control-btn {
    width: 44px;
    height: 44px;
  }
  
  .play-btn {
    width: 52px;
    height: 52px;
  }
  
  .control-buttons {
    gap: 10px;
  }
  
  .progress-bar {
    height: 5px;
  }
  
  .progress-thumb {
    width: 12px;
    height: 12px;
  }
  
  .time {
    font-size: 14px;
    min-width: 42px;
  }
  
  .volume-panel {
    width: 40px;
    height: 120px;
    padding: 8px;
  }
  
  .volume-thumb {
    width: 14px;
    height: 14px;
  }
  
  .playlist-panel {
    width: 420px;
    max-height: 500px;
  }
  
  .playlist-item {
    padding: 14px 16px;
  }
  
  .playlist-item-cover {
    width: 44px;
    height: 44px;
  }
}

/* 3K屏幕适配 (1920px - 2559px) */
@media screen and (min-width: 1920px) and (max-width: 2559px) {
  .player-bar {
    padding: 20px 40px;
    height: var(--player-bar-height);
  }
  
  .player-cover-wrapper {
    width: 68px;
    height: 68px;
  }
  
  .player-cover {
    border-radius: 14px;
  }
  
  .player-details {
    margin-left: 18px;
  }
  
  .player-title {
    font-size: 17px;
    max-width: 280px;
  }
  
  .player-artist {
    font-size: 15px;
  }
  
  .control-btn {
    width: 48px;
    height: 48px;
  }
  
  .play-btn {
    width: 58px;
    height: 58px;
  }
  
  .control-buttons {
    gap: 12px;
  }
  
  .progress-bar {
    height: 6px;
  }
  
  .progress-thumb {
    width: 14px;
    height: 14px;
  }
  
  .time {
    font-size: 15px;
    min-width: 48px;
  }
  
  .volume-panel {
    width: 44px;
    height: 140px;
    padding: 10px;
  }
  
  .volume-thumb {
    width: 16px;
    height: 16px;
  }
  
  .playlist-panel {
    width: 480px;
    max-height: 560px;
  }
  
  .playlist-item {
    padding: 16px 18px;
  }
  
  .playlist-item-cover {
    width: 48px;
    height: 48px;
  }
  
  .playlist-item-title {
    font-size: 15px;
  }
  
  .playlist-item-artist {
    font-size: 13px;
  }
}

/* 4K屏幕适配 (>= 2560px) */
@media screen and (min-width: 2560px) {
  .player-bar {
    padding: 24px 48px;
    height: var(--player-bar-height);
  }
  
  .player-cover-wrapper {
    width: 80px;
    height: 80px;
  }
  
  .player-cover {
    border-radius: 16px;
  }
  
  .player-details {
    margin-left: 20px;
  }
  
  .player-title {
    font-size: 20px;
    max-width: 320px;
  }
  
  .player-artist {
    font-size: 17px;
  }
  
  .control-btn {
    width: 54px;
    height: 54px;
  }
  
  .play-btn {
    width: 66px;
    height: 66px;
  }
  
  .control-buttons {
    gap: 14px;
  }
  
  .progress-bar {
    height: 8px;
  }
  
  .progress-thumb {
    width: 16px;
    height: 16px;
  }
  
  .time {
    font-size: 17px;
    min-width: 56px;
  }
  
  .volume-panel {
    width: 48px;
    height: 160px;
    padding: 12px;
  }
  
  .volume-thumb {
    width: 18px;
    height: 18px;
  }
  
  .playlist-panel {
    width: 560px;
    max-height: 640px;
    border-radius: 20px;
  }
  
  .playlist-item {
    padding: 18px 20px;
  }
  
  .playlist-item-cover {
    width: 56px;
    height: 56px;
  }
  
  .playlist-item-title {
    font-size: 17px;
  }
  
  .playlist-item-artist {
    font-size: 15px;
  }
  
  .playlist-header h3 {
    font-size: 20px;
  }
}

/* 5K屏幕适配 (>= 5120px) */
@media screen and (min-width: 5120px) {
  .player-bar {
    padding: 36px 64px;
    height: var(--player-bar-height);
  }
  
  .player-cover-wrapper {
    width: 100px;
    height: 100px;
  }
  
  .player-cover {
    border-radius: 20px;
  }
  
  .player-details {
    margin-left: 28px;
  }
  
  .player-title {
    font-size: 26px;
    max-width: 400px;
  }
  
  .player-artist {
    font-size: 22px;
  }
  
  .control-btn {
    width: 68px;
    height: 68px;
  }
  
  .play-btn {
    width: 84px;
    height: 84px;
  }
  
  .control-buttons {
    gap: 18px;
  }
  
  .progress-bar {
    height: 10px;
  }
  
  .progress-thumb {
    width: 20px;
    height: 20px;
  }
  
  .time {
    font-size: 22px;
    min-width: 72px;
  }
  
  .volume-panel {
    width: 56px;
    height: 200px;
    padding: 16px;
  }
  
  .volume-thumb {
    width: 22px;
    height: 22px;
  }
  
  .playlist-panel {
    width: 680px;
    max-height: 800px;
    border-radius: 28px;
  }
  
  .playlist-item {
    padding: 24px 28px;
  }
  
  .playlist-item-cover {
    width: 72px;
    height: 72px;
  }
  
  .playlist-item-title {
    font-size: 22px;
  }
  
  .playlist-item-artist {
    font-size: 18px;
  }
}

/* 高度适配 - 短屏幕 */
@media screen and (max-height: 720px) {
  .player-bar {
    padding: 8px 16px;
  }
  
  .player-cover-wrapper {
    width: 40px;
    height: 40px;
  }
  
  .control-btn {
    width: 36px;
    height: 36px;
  }
  
  .play-btn {
    width: 44px;
    height: 44px;
  }
  
  .volume-panel {
    display: none;
  }
}

/* 高度适配 - 超高屏幕 */
@media screen and (min-height: 1440px) {
  .player-bar {
    padding: 22px 44px;
  }
  
  .player-cover-wrapper {
    width: 72px;
    height: 72px;
  }
}

/* 触摸设备适配 */
@media (hover: none) and (pointer: coarse) {
  .control-btn,
  .volume-btn {
    min-width: 48px;
    min-height: 48px;
  }
  
  .progress-bar {
    height: 10px;
  }
  
  .progress-thumb {
    width: 20px;
    height: 20px;
  }
}
</style>