<template>
  <div class="search-view">
    <div v-if="searchQuery" class="search-content">
      <div class="search-header">
        <h1 class="search-query">{{ searchQuery }}</h1>
        <div class="search-tabs">
          <button 
            v-for="tab in tabs" 
            :key="tab.key"
            :class="['tab-btn', { active: activeTab === tab.key }]"
            @click="activeTab = tab.key"
          >
            {{ tab.label }}
          </button>
        </div>
      </div>

      <div class="search-results">
        <Transition name="tab-content" mode="out-in">
          <div v-if="activeTab === 'music'" class="music-results" :key="'music'">
            <div class="music-list-header">
              <button class="play-all-btn" @click="playAllMusic">
              <svg viewBox="0 0 24 24" width="18" height="18">
                <path fill="currentColor" d="M8 5v14l11-7z"/>
              </svg>
              播放全部
            </button>
            <span class="result-count">{{ musicResults.length }} 首单曲</span>
            </div>
            
            <div v-if="loadingMusic" class="loading">
            <div class="loading-spinner"></div>
            <p>加载中...</p>
          </div>
          
          <div v-else-if="musicResults.length === 0" class="empty">
            <p>暂无单曲</p>
          </div>
          
          <div v-else class="music-list">
            <TransitionGroup name="music-item" tag="div">
              <div 
                v-for="(music, index) in musicResults" 
                :key="music.id"
                :class="['music-item', { playing: currentMusic?.id === music.id }]"
                @dblclick="playMusic(music)"
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
                    @error="handleCoverError"
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

        <div v-else-if="activeTab === 'playlist'" class="playlist-results" :key="'playlist'">
          <div v-if="loadingPlaylists" class="loading">
            <div class="loading-spinner"></div>
            <p>加载中...</p>
          </div>
          <div v-else-if="playlistResults.length === 0" class="empty">
            <p>暂无歌单</p>
          </div>
          <div v-else class="playlist-grid">
            <div 
              v-for="playlist in playlistResults" 
              :key="playlist.id"
              class="playlist-card"
              @click="viewPlaylist(playlist.id)"
            >
              <div class="playlist-cover-wrapper">
                <img 
                  :src="getPlaylistCover(playlist)" 
                  :alt="playlist.name"
                  class="playlist-cover"
                  @error="handlePlaylistCoverError"
                />
              </div>
              <div class="playlist-info">
                <div class="playlist-name">{{ playlist.name }}</div>
                <div class="playlist-description">{{ playlist.description || '暂无描述' }}</div>
                <div class="playlist-meta">
                  <span class="playlist-count">{{ playlist.musicCount }} 首音乐</span>
                </div>
              </div>
            </div>
          </div>
        </div>

        <div v-else-if="activeTab === 'artist'" class="artist-results" :key="'artist'">
          <div v-if="loadingArtists" class="loading">
            <div class="loading-spinner"></div>
            <p>加载中...</p>
          </div>
          <div v-else-if="!artistResults || artistResults.musicCount === 0" class="empty">
            <p>暂无歌手</p>
          </div>
          <div v-else class="artist-card">
            <div class="artist-header">
              <div class="artist-avatar">
                <svg viewBox="0 0 80 80" width="80" height="80">
                  <defs>
                    <linearGradient id="artistGrad" x1="0%" y1="0%" x2="100%" y2="100%">
                      <stop offset="0%" style="stop-color:#667eea;stop-opacity:1"/>
                      <stop offset="100%" style="stop-color:#764ba2;stop-opacity:1"/>
                    </linearGradient>
                  </defs>
                  <circle cx="40" cy="40" r="40" fill="url(#artistGrad)"/>
                  <text x="40" y="48" font-family="Arial" font-size="24" fill="white" text-anchor="middle" font-weight="bold">
                    {{ artistResults.name.charAt(0) }}
                  </text>
                </svg>
              </div>
              <div class="artist-info">
                <h3 class="artist-name">{{ artistResults.name }}</h3>
                <div class="artist-stats">
                  <span>{{ artistResults.musicCount }} 首音乐</span>
                </div>
              </div>
            </div>
            <div class="artist-music-list">
              <div 
                v-for="music in artistResults.musicList" 
                :key="music.id"
                class="artist-music-item"
                @click="playMusic(music)"
              >
                <img 
                  :src="getCoverUrl(music.id)" 
                  :alt="music.title"
                  class="music-cover"
                  @error="handleCoverError"
                />
                <div class="music-info">
                  <div class="music-title">{{ music.title }}</div>
                  <div class="music-artist">{{ music.artist }}</div>
                </div>
              </div>
            </div>
          </div>
          </div>
        </Transition>
      </div>
    </div>
    
    <div v-else class="empty-state">
      <svg class="empty-icon" viewBox="0 0 24 24" width="80" height="80">
        <path fill="currentColor" d="M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z"/>
      </svg>
      <p class="empty-text">输入关键词开始搜索</p>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import apiConfig from '../config/apiConfig'

// 统一的 API 请求函数
async function apiRequest(url, options = {}) {
  const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
  return fetch(fullUrl, options)
}

const route = useRoute()
const router = useRouter()
const searchQuery = ref('')
const activeTab = ref('music')
const playlistResults = ref([])
const artistResults = ref(null)
const musicResults = ref([])
const loadingMusic = ref(false)
const loadingPlaylists = ref(false)
const loadingArtists = ref(false)
const currentMusic = ref(null)
const favorites = ref([])

const tabs = [
  { key: 'music', label: '单曲' },
  { key: 'playlist', label: '歌单' },
  { key: 'artist', label: '歌手' }
]

// 加载收藏列表
const loadFavorites = async () => {
  const token = localStorage.getItem('token')
  if (!token) {
    favorites.value = []
    localStorage.removeItem('favorites')
    return
  }
  
  // 先从本地存储读取
  const localFavorites = localStorage.getItem('favorites')
  if (localFavorites) {
    try {
      favorites.value = JSON.parse(localFavorites)
    } catch (e) {
      console.error('解析本地收藏列表失败:', e)
      favorites.value = []
    }
  }
  
  // 从服务器同步
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
      // 保存到本地存储
      localStorage.setItem('favorites', JSON.stringify(favorites.value))
    }
  } catch (error) {
    console.error('加载收藏列表失败:', error)
  }
}

const fetchMusicResults = async () => {
  if (!searchQuery.value.trim()) return

  loadingMusic.value = true
  try {
    const response = await apiRequest(apiConfig.MUSIC_SEARCH, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        query: searchQuery.value
      })
    })
    
    const data = await response.json()
    if (data.success && data.results) {
      musicResults.value = data.results
    } else {
      musicResults.value = []
    }
  } catch (error) {
    console.error('搜索音乐失败:', error)
    musicResults.value = []
  } finally {
    loadingMusic.value = false
  }
}

const fetchPlaylistResults = async () => {
  if (!searchQuery.value.trim()) return

  loadingPlaylists.value = true
  try {
    const response = await apiRequest(apiConfig.PLAYLISTS_SEARCH, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        query: searchQuery.value
      })
    })
    
    const data = await response.json()
    if (data.success && data.results) {
      playlistResults.value = data.results
    } else {
      playlistResults.value = []
    }
  } catch (error) {
    console.error('搜索歌单失败:', error)
    playlistResults.value = []
  } finally {
    loadingPlaylists.value = false
  }
}

const fetchArtistResults = async () => {
  if (!searchQuery.value.trim()) return

  loadingArtists.value = true
  try {
    const response = await apiRequest(apiConfig.ARTISTS_SEARCH, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        query: searchQuery.value
      })
    })
    
    const data = await response.json()
    if (data.success && data.artist) {
      artistResults.value = data.artist
    } else {
      artistResults.value = { name: '', musicCount: 0, musicList: [] }
    }
  } catch (error) {
    console.error('搜索歌手失败:', error)
    artistResults.value = { name: '', musicCount: 0, musicList: [] }
  } finally {
    loadingArtists.value = false
  }
}

const performSearch = () => {
  if (searchQuery.value.trim()) {
    fetchMusicResults()
    fetchPlaylistResults()
    fetchArtistResults()
  }
}

const getPlaylistCover = (playlist) => {
  if (playlist.firstMusicCover) {
    // 从路径中提取音乐 ID，例如 /music/covers/372.jpg -> 372
    const match = playlist.firstMusicCover.match(/\/(\d+)\.jpg$/)
    if (match) {
      return `https://music.cnmsb.xin/api/music/cover/${match[1]}`
    }
  }
  return 'https://music.cnmsb.xin/api/user/avatar/default'
}

const handlePlaylistCoverError = (event) => {
  event.target.src = 'https://music.cnmsb.xin/api/user/avatar/default'
}

const getCoverUrl = (id) => {
  return `https://music.cnmsb.xin/api/music/cover/${id}`
}

const handleCoverError = (event) => {
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
  
  // 自动添加到播放列表
  window.dispatchEvent(new CustomEvent('add-to-playlist', { detail: music }))
  
  window.dispatchEvent(new CustomEvent('music-play', { detail: music }))
}

const playAllMusic = () => {
  if (musicResults.value.length > 0) {
    console.log('playAllMusic: 清空播放列表并添加所有搜索结果，共', musicResults.value.length, '首')
    
    // 清空播放列表
    window.dispatchEvent(new CustomEvent('clear-playlist'))
    
    // 添加所有搜索结果到播放列表
    window.dispatchEvent(new CustomEvent('add-all-to-playlist', { detail: musicResults.value }))
    
    // 播放第一首
    playMusic(musicResults.value[0])
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
      // 取消收藏
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
      // 添加收藏
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

const viewPlaylist = (playlistId) => {
  router.push(`/playlist/${playlistId}`)
}

watch(() => route.query.q, (newQuery) => {
  if (newQuery) {
    searchQuery.value = decodeURIComponent(newQuery)
    activeTab.value = 'music'
    performSearch()
  }
})

onMounted(() => {
  const query = route.query.q
  if (query) {
    searchQuery.value = decodeURIComponent(query)
    performSearch()
  }
  
  const savedMusic = localStorage.getItem('currentMusic')
  if (savedMusic) {
    try {
      currentMusic.value = JSON.parse(savedMusic)
    } catch (e) {
      console.error('解析当前音乐失败:', e)
    }
  }
  
  // 加载收藏列表
  loadFavorites()
  
  // 监听登录/登出事件，重新加载收藏列表
  window.addEventListener('user-login', loadFavorites)
  window.addEventListener('user-logout', loadFavorites)
  
  // 监听收藏变化
  window.addEventListener('favorite-changed', loadFavorites)
})
</script>

<style scoped>
.search-view {
  height: 100%;
  padding: 24px;
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

.search-header {
  margin-bottom: 24px;
}

.search-query {
  font-size: 28px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 20px 0;
}

.search-tabs {
  display: flex;
  gap: 24px;
  border-bottom: 1px solid var(--border-color);
}

.tab-btn {
  padding: 12px 0;
  font-size: 16px;
  color: var(--text-secondary);
  background: none;
  border: none;
  cursor: pointer;
  position: relative;
  transition: color var(--transition-normal);
}

.tab-btn:hover {
  color: var(--text-primary);
}

.tab-btn.active {
  color: var(--primary);
  font-weight: 600;
}

.tab-btn.active::after {
  content: '';
  position: absolute;
  bottom: -1px;
  left: 0;
  right: 0;
  height: 2px;
  background: var(--primary);
}

.search-results {
  min-height: 400px;
}

.music-results {
  width: 100%;
}

.music-list-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 16px;
}

.play-all-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 20px;
  background: var(--gradient-primary);
  color: white;
  border: none;
  border-radius: var(--radius-full);
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all var(--transition-normal);
}

.play-all-btn:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-md);
}

.result-count {
  font-size: 14px;
  color: var(--text-muted);
}

.music-list {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.music-item {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 12px 16px;
  background: white;
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: all var(--transition-fast);
  box-shadow: var(--shadow-sm);
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

.music-item:hover {
  background: var(--bg-secondary);
  transform: translateX(4px);
}

.music-item.playing {
  background: rgba(102, 126, 234, 0.05);
}

.music-item.playing .music-title {
  color: var(--primary);
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
  width: 56px;
  height: 56px;
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
  width: 32px;
  height: 32px;
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
  font-size: 15px;
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
  font-size: 13px;
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

.action-btn {
  width: 36px;
  height: 36px;
  border: none;
  background: transparent;
  border-radius: var(--radius-md);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--text-secondary);
  transition: all var(--transition-fast);
}

.action-btn:hover {
  background: var(--bg-secondary);
  color: var(--text-primary);
}

.action-btn.is-favorite {
  color: #ff4545;
}

.playlist-results {
  width: 100%;
}

.playlist-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
  gap: 20px;
}

.playlist-card {
  background: white;
  border-radius: var(--radius-lg);
  overflow: hidden;
  cursor: pointer;
  transition: all var(--transition-normal);
  box-shadow: var(--shadow-sm);
}

.playlist-card:hover {
  transform: translateY(-4px);
  box-shadow: var(--shadow-lg);
}

.playlist-cover-wrapper {
  position: relative;
  aspect-ratio: 1;
  overflow: hidden;
}

.playlist-cover {
  width: 100%;
  height: 100%;
  object-fit: cover;
  background: var(--gradient-primary);
  transition: transform var(--transition-normal);
}

.playlist-card:hover .playlist-cover {
  transform: scale(1.05);
}

.playlist-info {
  padding: 16px;
}

.playlist-name {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  margin-bottom: 8px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.playlist-description {
  font-size: 12px;
  color: var(--text-secondary);
  margin-bottom: 12px;
  display: -webkit-box;
  -webkit-line-clamp: 2;
  -webkit-box-orient: vertical;
  overflow: hidden;
  height: 32px;
}

.playlist-meta {
  font-size: 12px;
  color: var(--text-muted);
}

.artist-results {
  width: 100%;
}

.artist-card {
  background: white;
  border-radius: var(--radius-lg);
  padding: 24px;
  box-shadow: var(--shadow-sm);
}

.artist-header {
  display: flex;
  align-items: center;
  gap: 20px;
  margin-bottom: 24px;
  padding-bottom: 24px;
  border-bottom: 1px solid var(--border-color);
}

.artist-avatar {
  width: 120px;
  height: 120px;
  border-radius: 50%;
  overflow: hidden;
  box-shadow: var(--shadow-md);
  flex-shrink: 0;
}

.artist-avatar svg {
  width: 100%;
  height: 100%;
}

.artist-info {
  flex: 1;
}

.artist-name {
  font-size: 24px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 8px 0;
}

.artist-stats {
  font-size: 14px;
  color: var(--text-secondary);
}

.artist-music-list {
  display: flex;
  flex-direction: column;
  gap: 8px;
}

.artist-music-item {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 12px;
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: background var(--transition-fast);
}

.artist-music-item:hover {
  background: var(--bg-secondary);
}

.artist-music-item .music-cover {
  width: 48px;
  height: 48px;
}

.artist-music-item .music-title {
  font-size: 14px;
}

.artist-music-item .music-artist {
  font-size: 12px;
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

.tab-content-enter-active,
.tab-content-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.tab-content-enter-from {
  opacity: 0;
  transform: translateY(10px);
}

.tab-content-leave-to {
  opacity: 0;
  transform: translateY(-10px);
}

.empty-state {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  height: 100%;
  padding: 60px 20px;
  text-align: center;
}

.empty-icon {
  width: 80px;
  height: 80px;
  color: var(--text-muted);
  margin-bottom: 16px;
}

.empty-text {
  font-size: 16px;
  color: var(--text-muted);
  margin: 0;
}
</style>