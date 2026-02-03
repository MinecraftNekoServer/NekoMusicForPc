<template>
  <div class="playlist-detail-view">
    <div v-if="loading" class="loading">
      <div class="loading-spinner"></div>
      <p>加载中...</p>
    </div>

    <div v-else-if="playlist" class="playlist-content">
      <div class="playlist-header">
        <div class="playlist-cover-wrapper">
          <img 
            :src="getPlaylistCover()" 
            :alt="playlist.name"
            class="playlist-cover"
            @error="handleCoverError"
          />
        </div>
        <div class="playlist-info">
          <div class="playlist-type">歌单</div>
          <h1 class="playlist-name">{{ playlist.name }}</h1>
          <div class="playlist-description">{{ playlist.description || '暂无描述' }}</div>
          <div class="playlist-meta">
            <div class="creator-info" v-if="playlist.creator">
              <img :src="getAvatarUrl(playlist.creator.id)" alt="创建者头像" class="creator-avatar" @error="handleAvatarError" />
              <span class="creator-name">{{ playlist.creator.username }}</span>
            </div>
            <span class="playlist-count">{{ playlist.musicCount || musicList.length }} 首音乐</span>
          </div>
          <div class="playlist-actions">
            <button class="action-btn play-all" @click="playAll">
              <svg viewBox="0 0 24 24" width="20" height="20">
                <path fill="currentColor" d="M8 5v14l11-7z"/>
              </svg>
              播放全部
            </button>
            <button 
              v-if="!isOwner" 
              class="action-btn collect" 
              @click="toggleCollect" 
              :class="{ collected: isCollected }"
            >
              <svg viewBox="0 0 24 24" width="20" height="20">
                <path :fill="isCollected ? '#ff4545' : 'currentColor'" d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/>
              </svg>
              {{ isCollected ? '已收藏歌单' : '收藏歌单' }}
            </button>
          </div>
        </div>
      </div>

      <div class="music-list-section">
        <div class="list-header">
          <span class="list-title">歌曲列表</span>
          <span class="list-count">{{ musicList.length }} 首歌曲</span>
        </div>

        <div v-if="musicList.length === 0" class="empty">
          <p>歌单暂无音乐</p>
        </div>

        <div v-else class="music-list">
          <TransitionGroup name="music-item" tag="div">
            <div 
              v-for="(music, index) in musicList" 
              :key="music.id"
              :class="['music-item', { playing: currentMusic?.id === music.id }]"
              @dblclick="playMusic(music)"
              @contextmenu.prevent="showContextMenu($event, music)"
            >
              <div class="music-index">
                <span v-if="currentMusic?.id === music.id" class="playing-icon">
                  <svg viewBox="0 0 24 24" width="16" height="16">
                    <path fill="currentColor" d="M6 19h4V5H6v14zm8-14v14h4V5h-4z"/>
                  </svg>
                </span>
                <span v-else class="index-num">{{ index + 1 }}</span>
              </div>
              
              <div class="music-cover-wrapper">
                <img 
                  :src="getCoverUrl(music.id)" 
                  :alt="music.title"
                  class="music-cover"
                  @error="handleMusicCoverError"
                />
                <div class="cover-overlay">
                  <button class="play-btn" @click.stop="playMusic(music)">
                    <svg viewBox="0 0 24 24" width="24" height="24">
                      <path fill="currentColor" d="M8 5v14l11-7z"/>
                    </svg>
                  </button>
                </div>
              </div>
              
              <div class="music-info">
                <div class="music-title">{{ music.title }}</div>
                <div class="music-meta">
                  <span class="music-artist">{{ music.artist }}</span>
                  <span v-if="music.album" class="music-album">{{ music.album }}</span>
                </div>
              </div>
              
              <div class="music-duration">
                {{ formatDuration(music.duration) }}
              </div>
              
              <div class="music-actions">
                <button class="action-btn" @click.stop="toggleFavorite(music)" :class="{ 'is-favorite': isFavorite(music.id) }">
                  <svg viewBox="0 0 24 24" width="18" height="18">
                    <path :fill="isFavorite(music.id) ? '#ff4545' : 'currentColor'" d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/>
                  </svg>
                </button>
              </div>
            </div>
          </TransitionGroup>
        </div>
      </div>
    </div>

    <div v-else class="empty">
      <p>歌单不存在或已被删除</p>
    </div>

    <!-- 右键菜单 -->
    <Transition name="context-menu">
      <div 
        v-if="contextMenu && selectedMusic" 
        class="context-menu"
        :style="{ left: contextMenuPosition.x + 'px', top: contextMenuPosition.y + 'px' }"
      >
        <div 
          class="context-menu-item delete"
          @click.stop="removeMusic(selectedMusic.id)"
        >
          <svg viewBox="0 0 24 24" width="16" height="16">
            <path fill="currentColor" d="M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM19 4h-3.5l-1-1h-5l-1 1H5v2h14V4z"/>
          </svg>
          从歌单中移除
        </div>
      </div>
    </Transition>

    <!-- 确认对话框 -->
    <Transition name="modal">
      <div v-if="showConfirmDialog" class="modal-overlay" @click="handleCancel">
        <div class="modal-content" @click.stop>
          <div class="modal-icon">
            <svg viewBox="0 0 24 24" width="48" height="48">
              <path fill="#ef4444" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-2h2v2zm0-4h-2V7h2v6z"/>
            </svg>
          </div>
          <h3 class="modal-title">确认移除</h3>
          <p class="modal-message">确定要从歌单中移除这首音乐吗？</p>
          <div class="modal-actions">
            <button class="modal-btn cancel" @click="handleCancel">取消</button>
            <button class="modal-btn confirm" @click="handleConfirm">确认移除</button>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, onMounted, computed, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import apiConfig from '../config/apiConfig'

const route = useRoute()
const router = useRouter()

const playlist = ref(null)
const musicList = ref([])
const loading = ref(false)
const currentMusic = ref(null)
const favorites = ref([])
const isCollected = ref(false)
const contextMenu = ref(null)
const contextMenuPosition = ref({ x: 0, y: 0 })
const selectedMusic = ref(null)
const showConfirmDialog = ref(false)
const confirmAction = ref(null)

// 当前用户信息
const currentUser = computed(() => {
  const token = localStorage.getItem('token')
  if (!token) return null
  
  try {
    const userStr = localStorage.getItem('currentUser')
    return userStr ? JSON.parse(userStr) : null
  } catch (e) {
    console.error('解析用户信息失败:', e)
    return null
  }
})

// 判断当前用户是否是歌单所有者
const isOwner = computed(() => {
  if (!currentUser.value || !playlist.value) return false
  return currentUser.value.id === playlist.value.userId
})

// 统一的 API 请求函数
async function apiRequest(url, options = {}) {
  const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
  return fetch(fullUrl, options)
}

// 加载歌单详情
const loadPlaylistDetail = async () => {
  const playlistId = route.params.id
  if (!playlistId) return

  loading.value = true
  try {
    // 添加时间戳参数禁用浏览器缓存
    const timestamp = Date.now()

    // 获取歌单基本信息
    const detailResponse = await apiRequest(`${apiConfig.PLAYLIST_DETAIL(playlistId)}?t=${timestamp}`)
    const detailData = await detailResponse.json()

    console.log('歌单详情响应:', detailData)

    if (detailData.success && detailData.playlist) {
      playlist.value = detailData.playlist

      // 检查是否已收藏此歌单
      await checkIfCollected(playlistId)

      // 获取歌单音乐列表
      const musicResponse = await apiRequest(`${apiConfig.PLAYLIST_MUSIC(playlistId)}?t=${timestamp}`)
      const musicData = await musicResponse.json()

      console.log('歌单音乐列表响应:', musicData)

      if (musicData.success && musicData.musicList) {
        musicList.value = musicData.musicList
      } else {
        musicList.value = []
      }
      console.log('歌单音乐列表:', musicList.value)
    } else {
      playlist.value = null
      musicList.value = []
    }
  } catch (error) {
    console.error('加载歌单详情失败:', error)
    playlist.value = null
    musicList.value = []
  } finally {
    loading.value = false
  }
}

// 检查歌单是否已收藏
const checkIfCollected = async (playlistId) => {
  const token = localStorage.getItem('token')
  if (!token) {
    isCollected.value = false
    return
  }

  try {
    const response = await apiRequest(`${apiConfig.FAVORITE_PLAYLISTS}?t=${Date.now()}`, {
      method: 'GET',
      headers: { 'Authorization': token }
    })

    const data = await response.json()
    if (data.success && data.playlists) {
      isCollected.value = data.playlists.some(p => p.id === parseInt(playlistId))
    } else {
      isCollected.value = false
    }
  } catch (error) {
    console.error('检查收藏状态失败:', error)
    isCollected.value = false
  }
}

// 加载收藏列表
const loadFavorites = async () => {
  const token = localStorage.getItem('token')
  if (!token) {
    favorites.value = []
    localStorage.removeItem('favorites')
    return
  }
  
  const localFavorites = localStorage.getItem('favorites')
  if (localFavorites) {
    try {
      favorites.value = JSON.parse(localFavorites)
    } catch (e) {
      console.error('解析本地收藏列表失败:', e)
      favorites.value = []
    }
  }
  
  try {
    const response = await apiRequest(apiConfig.USER_FAVORITES, {
      method: 'GET',
      headers: {
        'Authorization': token
      }
    })
    
    if (response.ok) {
      const data = await response.json()
      favorites.value = data.favorites || []
      localStorage.setItem('favorites', JSON.stringify(favorites.value))
    }
  } catch (error) {
    console.error('加载收藏列表失败:', error)
  }
}

const getPlaylistCover = () => {
  // 优先使用歌单详情中的第一首音乐 ID 获取封面
  if (playlist.value?.firstMusicId) {
    return `https://music.cnmsb.xin/api/music/cover/${playlist.value.firstMusicId}`
  }
  
  // 如果没有，使用音乐列表中第一首音乐的 ID 获取封面
  if (musicList.value && musicList.value.length > 0) {
    const firstMusic = musicList.value[0]
    if (firstMusic.id) {
      return `https://music.cnmsb.xin/api/music/cover/${firstMusic.id}`
    }
  }
  
  return 'https://music.cnmsb.xin/api/user/avatar/default'
}

const handleCoverError = (event) => {
  event.target.src = 'https://music.cnmsb.xin/api/user/avatar/default'
}

const getCoverUrl = (id) => {
  return `https://music.cnmsb.xin/api/music/cover/${id}`
}

const getAvatarUrl = (userId) => {
  return `https://music.cnmsb.xin/api/user/avatar/${userId}`
}

const handleAvatarError = (event) => {
  event.target.src = 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24"><circle cx="12" cy="8" r="4" fill="%23667eea"/><path d="M12 14c-6.1 0-8 4-8 4v2h16v-2s-1.9-4-8-4z" fill="%23764ba2"/></svg>'
}

const handleMusicCoverError = (event) => {
  event.target.src = 'data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" width="56" height="56" viewBox="0 0 56 56"><defs><linearGradient id="grad" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" style="stop-color:%23667eea;stop-opacity:1"/><stop offset="100%" style="stop-color:%23764ba2;stop-opacity:1"/></linearGradient></defs><rect width="56" height="56" fill="url(%23grad)" rx="8"/><text x="28" y="35" font-family="Arial" font-size="24" fill="white" text-anchor="middle" font-weight="bold">M</text></svg>'
}

const formatDuration = (seconds) => {
  if (!seconds) return '-'
  const mins = Math.floor(seconds / 60)
  const secs = seconds % 60
  return `${mins}:${secs.toString().padStart(2, '0')}`
}

const playMusic = (music) => {
  currentMusic.value = music
  localStorage.setItem('currentMusic', JSON.stringify(music))
  
  window.dispatchEvent(new CustomEvent('add-to-playlist', { detail: music }))
  window.dispatchEvent(new CustomEvent('music-play', { detail: music }))
}

const playAll = () => {
  if (musicList.value.length > 0) {
    window.dispatchEvent(new CustomEvent('clear-playlist'))
    window.dispatchEvent(new CustomEvent('add-all-to-playlist', { detail: musicList.value }))
    playMusic(musicList.value[0])
  }
}

const isFavorite = (musicId) => {
  return favorites.value.some(f => f.id === musicId)
}

const toggleFavorite = async (music) => {
  const token = localStorage.getItem('token')
  if (!token) {
    window.dispatchEvent(new CustomEvent('show-toast', { 
      detail: { message: '请先登录', type: 'error' } 
    }))
    return
  }
  
  try {
    if (isFavorite(music.id)) {
      const response = await fetch(`${apiConfig.BASE_URL}/api/user/favorites/${music.id}`, {
        method: 'DELETE',
        headers: {
          'Authorization': token
        }
      })
      
      if (response.ok) {
        favorites.value = favorites.value.filter(f => f.id !== music.id)
        localStorage.setItem('favorites', JSON.stringify(favorites.value))
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '已取消收藏', type: 'success' } 
        }))
      } else {
        const result = await response.json()
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: result.message || '取消收藏失败', type: 'error' } 
        }))
      }
    } else {
      const response = await apiRequest(apiConfig.USER_FAVORITES, {
        method: 'POST',
        headers: {
          'Authorization': token,
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          musicId: music.id
        })
      })
      
      if (response.ok) {
        favorites.value.push(music)
        localStorage.setItem('favorites', JSON.stringify(favorites.value))
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '收藏成功', type: 'success' } 
        }))
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
      detail: { message: '网络错误，请重试', type: 'error' } 
    }))
  }
}

const toggleCollect = async () => {
  const token = localStorage.getItem('token')
  if (!token) {
    window.dispatchEvent(new CustomEvent('show-toast', { 
      detail: { message: '请先登录', type: 'error' } 
    }))
    return
  }
  
  try {
    if (isCollected.value) {
      // 取消收藏
      const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.FAVORITE_PLAYLISTS_DELETE(playlist.value.id)}`, {
        method: 'DELETE',
        headers: { 'Authorization': token }
      })
      
      const data = await response.json()
      if (data.success) {
        isCollected.value = false
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '已取消收藏歌单', type: 'success' } 
        }))
      } else {
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: data.message || '取消收藏失败', type: 'error' } 
        }))
      }
    } else {
      // 添加收藏
      const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.FAVORITE_PLAYLISTS}`, {
        method: 'POST',
        headers: {
          'Authorization': token,
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ playlistId: playlist.value.id })
      })
      
      const data = await response.json()
      if (data.success) {
        isCollected.value = true
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '已收藏歌单', type: 'success' } 
        }))
      } else {
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: data.message || '收藏失败', type: 'error' } 
        }))
      }
    }
  } catch (error) {
    console.error('收藏操作失败:', error)
    window.dispatchEvent(new CustomEvent('show-toast', { 
      detail: { message: '网络错误，请重试', type: 'error' } 
    }))
  }
}

// 监听路由参数变化，重新加载歌单详情
watch(() => route.params.id, (newId, oldId) => {
  if (newId && newId !== oldId) {
    console.log('歌单ID变化:', oldId, '->', newId)
    loadPlaylistDetail()
  }
})

onMounted(() => {
  loadPlaylistDetail()

  const savedMusic = localStorage.getItem('currentMusic')
  if (savedMusic) {
    try {
      currentMusic.value = JSON.parse(savedMusic)
    } catch (e) {
      console.error('解析当前音乐失败:', e)
    }
  }

  loadFavorites()
  window.addEventListener('favorite-changed', loadFavorites)

  // 监听歌单更新事件
  window.addEventListener('playlist-updated', handlePlaylistUpdated)

  // 点击外部关闭右键菜单
  document.addEventListener('mousedown', (e) => {
    if (contextMenu.value && !e.target.closest('.context-menu')) {
      hideContextMenu()
    }
  })
})

// 处理歌单更新事件
const handlePlaylistUpdated = (event) => {
  const { id, action } = event.detail
  // 如果是当前歌单的音乐发生变化，重新加载歌单详情
  if (playlist.value && playlist.value.id === id && (action === 'music-removed' || action === 'music-added')) {
    loadPlaylistDetail()
  }
}

// 显示右键菜单
const showContextMenu = (event, music) => {
  selectedMusic.value = music
  contextMenuPosition.value = {
    x: event.clientX,
    y: event.clientY
  }
  contextMenu.value = true
  
  console.log('右键菜单显示:', {
    isOwner: isOwner.value,
    currentUser: currentUser.value,
    playlistUserId: playlist.value?.userId,
    selectedMusic: music
  })
}

// 隐藏右键菜单
const hideContextMenu = () => {
  contextMenu.value = false
  selectedMusic.value = null
}

// 从歌单中移除音乐
const removeMusic = async (musicId) => {
  hideContextMenu()
  
  const token = localStorage.getItem('token')
  if (!token) {
    window.dispatchEvent(new CustomEvent('show-toast', { 
      detail: { message: '请先登录', type: 'error' } 
    }))
    return
  }

  // 显示自定义确认对话框
  showConfirmDialog.value = true
  confirmAction.value = async () => {
    try {
      const response = await fetch(`${apiConfig.BASE_URL}/api/user/playlist/music/remove`, {
        method: 'POST',
        headers: {
          'Authorization': token,
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          playlistId: playlist.value.id,
          musicId: musicId
        })
      })

      const result = await response.json()

      if (result.success) {
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: '音乐已从歌单中移除', type: 'success' } 
        }))
        
        // 重新加载歌单详情
        await loadPlaylistDetail()
        
        // 通知侧边栏刷新歌单列表
        window.dispatchEvent(new CustomEvent('playlist-updated', {
          detail: {
            id: playlist.value.id,
            action: 'music-removed'
          }
        }))
      } else {
        window.dispatchEvent(new CustomEvent('show-toast', { 
          detail: { message: result.message || '移除失败', type: 'error' } 
        }))
      }
    } catch (error) {
      console.error('移除音乐失败:', error)
      window.dispatchEvent(new CustomEvent('show-toast', { 
        detail: { message: '网络错误，请重试', type: 'error' } 
      }))
    }
  }
}

const handleConfirm = () => {
  if (confirmAction.value) {
    confirmAction.value()
  }
  showConfirmDialog.value = false
  confirmAction.value = null
}

const handleCancel = () => {
  showConfirmDialog.value = false
  confirmAction.value = null
}
</script>

<style scoped>
.playlist-detail-view {
  height: 100%;
  padding: 24px;
  overflow-y: auto;
}

.loading, .empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 60px 20px;
  color: var(--text-muted);
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 3px solid var(--border-color);
  border-top: 3px solid var(--primary);
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin-bottom: 12px;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

.playlist-content {
  animation: fadeIn 0.3s ease;
}

@keyframes fadeIn {
  from {
    opacity: 0;
    transform: translateY(10px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.playlist-header {
  display: flex;
  gap: 24px;
  padding: 24px;
  background: white;
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-sm);
  margin-bottom: 24px;
}

.playlist-cover-wrapper {
  width: 200px;
  height: 200px;
  flex-shrink: 0;
}

.playlist-cover {
  width: 100%;
  height: 100%;
  border-radius: var(--radius-md);
  object-fit: cover;
  box-shadow: var(--shadow-md);
}

.playlist-info {
  flex: 1;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
}

.playlist-type {
  font-size: 12px;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 1px;
  margin-bottom: 8px;
}

.playlist-name {
  font-size: 28px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 12px 0;
  line-height: 1.3;
}

.playlist-description {
  font-size: 14px;
  color: var(--text-secondary);
  margin-bottom: 16px;
  line-height: 1.6;
  display: -webkit-box;
  -webkit-line-clamp: 2;
  -webkit-box-orient: vertical;
  overflow: hidden;
}

.playlist-meta {
  display: flex;
  gap: 16px;
  font-size: 13px;
  color: var(--text-muted);
  margin-bottom: 20px;
  align-items: center;
}

.creator-info {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 4px 10px;
  background: rgba(102, 126, 234, 0.05);
  border-radius: 16px;
}

.creator-avatar {
  width: 20px;
  height: 20px;
  border-radius: 50%;
  object-fit: cover;
}

.creator-name {
  font-size: 12px;
  color: var(--text-secondary);
  font-weight: 500;
}

.playlist-actions {
  display: flex;
  gap: 12px;
}

.action-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 24px;
  border: none;
  border-radius: var(--radius-full);
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all var(--transition-normal);
}

.action-btn.play-all {
  background: var(--gradient-primary);
  color: #ffc3c3;
  box-shadow: var(--shadow-md);
}

.action-btn.play-all:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-lg);
}

.action-btn.collect {
  background: white;
  color: var(--text-primary);
  border: 1px solid var(--border-color);
}

.action-btn.collect:hover {
  background: var(--bg-secondary);
}

.action-btn.collect.collected {
  color: #ff4545;
  border-color: rgba(255, 69, 69, 0.3);
  background: rgba(255, 69, 69, 0.05);
}

.music-list-section {
  background: white;
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-sm);
  overflow: hidden;
}

.list-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 20px;
  background: var(--bg-secondary);
  border-bottom: 1px solid var(--border-color);
}

.list-title {
  font-size: 15px;
  font-weight: 600;
  color: var(--text-primary);
}

.list-count {
  font-size: 13px;
  color: var(--text-muted);
}

.music-list {
  display: flex;
  flex-direction: column;
}

.music-item {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 12px 20px;
  cursor: pointer;
  transition: background var(--transition-fast);
  border-bottom: 1px solid var(--border-color);
}

.music-item:last-child {
  border-bottom: none;
}

.music-item:hover {
  background: var(--bg-secondary);
}

.music-item.playing {
  background: rgba(102, 126, 234, 0.05);
}

.music-item.playing .music-title {
  color: var(--primary);
}

.music-item-enter-active,
.music-item-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.music-item-enter-from {
  opacity: 0;
  transform: translateY(20px) scale(0.95);
}

.music-item-leave-to {
  opacity: 0;
  transform: translateY(-20px) scale(0.95);
}

.music-item-move {
  transition: transform 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.music-index {
  width: 32px;
  text-align: center;
  flex-shrink: 0;
}

.index-num {
  font-size: 14px;
  color: var(--text-muted);
}

.playing-icon {
  color: var(--primary);
  display: flex;
  align-items: center;
  justify-content: center;
}

.music-cover-wrapper {
  position: relative;
  width: 48px;
  height: 48px;
  flex-shrink: 0;
}

.music-cover {
  width: 100%;
  height: 100%;
  border-radius: var(--radius-sm);
  object-fit: cover;
}

.cover-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.3);
  border-radius: var(--radius-sm);
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0;
  transition: opacity var(--transition-fast);
}

.music-item:hover .cover-overlay {
  opacity: 1;
}

.play-btn {
  width: 28px;
  height: 28px;
  background: var(--primary);
  border: none;
  border-radius: 50%;
  color: white;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all var(--transition-fast);
}

.play-btn:hover {
  transform: scale(1.1);
}

.music-info {
  flex: 1;
  min-width: 0;
}

.music-title {
  font-size: 14px;
  font-weight: 500;
  color: var(--text-primary);
  margin-bottom: 4px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.music-meta {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 12px;
  color: var(--text-secondary);
}

.music-artist,
.music-album {
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.music-album::before {
  content: '-';
  margin-right: 8px;
}

.music-duration {
  width: 60px;
  font-size: 13px;
  color: var(--text-muted);
  text-align: center;
  flex-shrink: 0;
}

.music-actions {
  display: flex;
  gap: 8px;
}

.music-actions .action-btn {
  padding: 8px;
  background: transparent;
  border: none;
}

.music-actions .action-btn:hover {
  background: var(--bg-secondary);
}

.music-actions .action-btn.is-favorite {
  color: #ff4545;
}

.context-menu {
  position: fixed;
  background: white;
  border-radius: var(--radius-md);
  box-shadow: var(--shadow-lg);
  padding: 8px 0;
  z-index: 1000;
  min-width: 180px;
  border: 1px solid var(--border-color);
}

.context-menu-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 10px 16px;
  cursor: pointer;
  transition: all var(--transition-fast);
  font-size: 14px;
  color: var(--text-primary);
}

.context-menu-item:hover {
  background: var(--bg-secondary);
}

.context-menu-item.delete {
  color: #ef4444;
}

.context-menu-item.delete:hover {
  background: rgba(239, 68, 68, 0.1);
}

.context-menu-enter-active,
.context-menu-leave-active {
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
}

.context-menu-enter-from,
.context-menu-leave-to {
  opacity: 0;
  transform: scale(0.95);
}

.context-menu-enter-to,
.context-menu-leave-from {
  opacity: 1;
  transform: scale(1);
}

.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  backdrop-filter: blur(4px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 2000;
}

.modal-content {
  background: white;
  border-radius: var(--radius-lg);
  padding: 32px;
  width: 90%;
  max-width: 400px;
  text-align: center;
  box-shadow: var(--shadow-xl);
}

.modal-icon {
  margin-bottom: 20px;
  display: flex;
  justify-content: center;
}

.modal-title {
  font-size: 20px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0 0 12px 0;
}

.modal-message {
  font-size: 14px;
  color: var(--text-secondary);
  margin: 0 0 24px 0;
  line-height: 1.6;
}

.modal-actions {
  display: flex;
  gap: 12px;
  justify-content: center;
}

.modal-btn {
  padding: 12px 24px;
  border-radius: var(--radius-md);
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all var(--transition-fast);
  border: none;
}

.modal-btn.cancel {
  background: var(--bg-secondary);
  color: var(--text-primary);
}

.modal-btn.cancel:hover {
  background: var(--bg-tertiary);
}

.modal-btn.confirm {
  background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%);
  color: white;
  box-shadow: 0 4px 12px rgba(239, 68, 68, 0.3);
}

.modal-btn.confirm:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 16px rgba(239, 68, 68, 0.4);
}

.modal-enter-active,
.modal-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.modal-enter-from,
.modal-leave-to {
  opacity: 0;
}

.modal-enter-from .modal-content,
.modal-leave-to .modal-content {
  transform: scale(0.9) translateY(20px);
}

.modal-enter-to .modal-content,
.modal-leave-from .modal-content {
  transform: scale(1) translateY(0);
}
</style>