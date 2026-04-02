<template>
  <div class="favorite-playlists-view">
    <div class="view-header">
      <h1 class="view-title">收藏的歌单</h1>
    </div>
    
    <div v-if="loading" class="loading">
      <div class="loading-spinner"></div>
      <p>加载中...</p>
    </div>
    
    <div v-else-if="playlists.length === 0" class="empty">
      <div class="empty-icon">
        <svg viewBox="0 0 24 24" width="64" height="64">
          <path fill="currentColor" d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/>
        </svg>
      </div>
      <p>还没有收藏任何歌单</p>
      <p class="empty-hint">去搜索喜欢的歌单并收藏吧！</p>
    </div>
    
    <div v-else class="playlists-grid">
      <div 
        v-for="playlist in playlists" 
        :key="playlist.id"
        class="playlist-card"
        @click="goToPlaylist(playlist.id)"
      >
        <div class="playlist-cover">
          <img :src="getPlaylistCover(playlist)" alt="歌单封面" @error="handleCoverError" />
          <div class="cover-overlay">
            <svg class="play-icon" viewBox="0 0 24 24" width="32" height="32">
              <path fill="currentColor" d="M8 5v14l11-7z"/>
            </svg>
          </div>
          <div class="music-count">
            <svg viewBox="0 0 24 24" width="14" height="14">
              <path fill="currentColor" d="M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4V7h4V3h-6z"/>
            </svg>
            <span>{{ playlist.musicCount }}</span>
          </div>
        </div>
        
        <div class="playlist-info">
          <h3 class="playlist-name">{{ playlist.name }}</h3>
          <p class="playlist-description">{{ playlist.description || '暂无描述' }}</p>
          
          <div class="playlist-meta">
            <div class="creator-info">
              <img :src="getAvatarUrl(playlist.creator.id)" alt="创建者头像" class="creator-avatar" @error="handleAvatarError" />
              <span class="creator-name">{{ playlist.creator.username }}</span>
            </div>
            <span class="favorite-time">{{ formatTime(playlist.favoriteTime) }}</span>
          </div>
        </div>
        
        <div class="playlist-actions">
          <button 
            v-if="playlist.creator.id !== currentUserId" 
            class="action-btn delete" 
            @click.stop="unfavoritePlaylist(playlist)" 
            title="取消收藏"
          >
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM19 4h-3.5l-1-1h-5l-1 1H5v2h14V4z"/>
            </svg>
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import apiConfig from '../config/apiConfig'

const router = useRouter()
const playlists = ref([])
const loading = ref(false)

// 获取当前用户ID
const currentUserId = computed(() => {
  const userStr = localStorage.getItem('user')
  if (userStr) {
    try {
      const user = JSON.parse(userStr)
      return user.id
    } catch (e) {
      console.error('解析用户信息失败:', e)
    }
  }
  return null
})

const fetchFavoritePlaylists = async () => {
  const token = localStorage.getItem('token')
  if (!token) {
    playlists.value = []
    return
  }

  loading.value = true
  try {
    const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.FAVORITE_PLAYLISTS}?t=${Date.now()}`, {
      method: 'GET',
      headers: { 'Authorization': token }
    })

    const data = await response.json()
    if (data.success && data.playlists) {
      playlists.value = data.playlists
    }
  } catch (error) {
    console.error('获取收藏歌单列表失败:', error)
  } finally {
    loading.value = false
  }
}

const getPlaylistCover = (playlist) => {
  // 如果歌单有音乐，使用第一首歌的封面
  // 否则使用默认封面
  return '/api/user/avatar/default'
}

const getAvatarUrl = (userId) => {
  return `${apiConfig.BASE_URL}${apiConfig.USER_AVATAR(userId)}`
}

const handleCoverError = (e) => {
  e.target.src = '/api/user/avatar/default'
}

const handleAvatarError = (e) => {
  e.target.src = '/api/user/avatar/default'
}

const formatTime = (timestamp) => {
  if (!timestamp) return ''
  const date = new Date(timestamp)
  const now = new Date()
  const diff = now - date
  
  const minute = 60 * 1000
  const hour = 60 * minute
  const day = 24 * hour
  const month = 30 * day
  const year = 365 * day
  
  if (diff < minute) return '刚刚收藏'
  if (diff < hour) return `${Math.floor(diff / minute)}分钟前收藏`
  if (diff < day) return `${Math.floor(diff / hour)}小时前收藏`
  if (diff < month) return `${Math.floor(diff / day)}天前收藏`
  if (diff < year) return `${Math.floor(diff / month)}个月前收藏`
  return `${Math.floor(diff / year)}年前收藏`
}

const goToPlaylist = (id) => {
  router.push(`/playlist/${id}`)
}

const unfavoritePlaylist = async (playlist) => {
  const token = localStorage.getItem('token')
  if (!token) return

  if (!confirm(`确定要取消收藏「${playlist.name}」吗？`)) {
    return
  }

  try {
    const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.FAVORITE_PLAYLISTS_DELETE(playlist.id)}`, {
      method: 'DELETE',
      headers: { 'Authorization': token }
    })

    const data = await response.json()
    if (data.success) {
      playlists.value = playlists.value.filter(p => p.id !== playlist.id)
      window.dispatchEvent(new CustomEvent('show-toast', { 
        detail: { message: '已取消收藏', type: 'success' } 
      }))
      // 触发收藏歌单更新事件，更新侧边栏
      window.dispatchEvent(new CustomEvent('favorite-playlist-updated'))
    }
  } catch (error) {
    console.error('取消收藏失败:', error)
    window.dispatchEvent(new CustomEvent('show-toast', { 
      detail: { message: '取消收藏失败', type: 'error' } 
    }))
  }
}

onMounted(() => {
  fetchFavoritePlaylists()
})
</script>

<style scoped>
.favorite-playlists-view {
  height: 100%;
  overflow-y: auto;
  padding: 24px;
}

.view-header {
  margin-bottom: 24px;
}

.view-title {
  margin: 0;
  color: var(--text-primary);
  font-size: 28px;
  font-weight: 700;
  background: var(--gradient-primary);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.loading, .empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 80px 20px;
  color: var(--text-muted);
}

.loading-spinner {
  width: 48px;
  height: 48px;
  border: 4px solid rgba(102, 126, 234, 0.1);
  border-top: 4px solid var(--primary);
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin-bottom: 16px;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

.empty-icon {
  width: 80px;
  height: 80px;
  border-radius: 50%;
  background: linear-gradient(135deg, rgba(233, 69, 69, 0.1) 0%, rgba(247, 37, 133, 0.1) 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 16px;
  color: #e91e63;
}

.empty-hint {
  font-size: 13px;
  margin-top: 8px;
  color: var(--text-muted);
}

.playlists-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
  gap: 20px;
}

.playlist-card {
  background: white;
  border-radius: 12px;
  overflow: hidden;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.08);
  transition: all 0.3s ease;
  cursor: pointer;
  position: relative;
}

.playlist-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.12);
}

.playlist-cover {
  position: relative;
  aspect-ratio: 1;
  background: linear-gradient(135deg, rgba(233, 69, 69, 0.1) 0%, rgba(247, 37, 133, 0.1) 100%);
}

.playlist-cover img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.cover-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.4);
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0;
  transition: opacity 0.3s ease;
}

.playlist-card:hover .cover-overlay {
  opacity: 1;
}

.play-icon {
  color: white;
  transform: scale(0.8);
  transition: transform 0.3s ease;
}

.playlist-card:hover .play-icon {
  transform: scale(1);
}

.music-count {
  position: absolute;
  top: 10px;
  right: 10px;
  display: flex;
  align-items: center;
  gap: 4px;
  padding: 4px 8px;
  background: rgba(0, 0, 0, 0.5);
  color: white;
  font-size: 12px;
  border-radius: 4px;
}

.playlist-info {
  padding: 16px;
}

.playlist-name {
  margin: 0 0 8px;
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.playlist-description {
  margin: 0 0 12px;
  font-size: 13px;
  color: var(--text-secondary);
  line-height: 1.5;
  display: -webkit-box;
  -webkit-line-clamp: 2;
  -webkit-box-orient: vertical;
  overflow: hidden;
}

.playlist-meta {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.creator-info {
  display: flex;
  align-items: center;
  gap: 8px;
}

.creator-avatar {
  width: 24px;
  height: 24px;
  border-radius: 50%;
  object-fit: cover;
}

.creator-name {
  font-size: 12px;
  color: var(--text-secondary);
}

.favorite-time {
  font-size: 12px;
  color: var(--text-muted);
}

.playlist-actions {
  position: absolute;
  top: 10px;
  right: 10px;
  opacity: 0;
  transition: opacity 0.3s ease;
}

.playlist-card:hover .playlist-actions {
  opacity: 1;
}

.action-btn {
  width: 32px;
  height: 32px;
  border: none;
  background: rgba(255, 255, 255, 0.9);
  border-radius: 50%;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--text-secondary);
  transition: all 0.2s ease;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
}

.action-btn:hover {
  background: white;
  color: #e91e63;
  transform: scale(1.1);
}

/* 滚动条样式 */
.favorite-playlists-view::-webkit-scrollbar {
  width: 8px;
}

.favorite-playlists-view::-webkit-scrollbar-track {
  background: transparent;
}

.favorite-playlists-view::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 4px;
}

.favorite-playlists-view::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}
</style>