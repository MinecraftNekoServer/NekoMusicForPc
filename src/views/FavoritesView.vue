<template>
  <div class="favorites-view">
    <!-- 顶部封面区域 -->
    <div class="playlist-header" v-if="musicList.length > 0">
      <div class="cover-wrapper">
        <img :src="getCoverUrl(musicList[0].id)" :alt="t('key.album')" class="playlist-cover" @error="handleCoverError" />
      </div>
      <div class="playlist-info">
        <div class="playlist-type">{{ t('key.playlists') }}</div>
        <h1 class="playlist-title">{{ t('favorites.myFavorites') }}</h1>
        <div class="playlist-meta">
          <span class="music-count">{{ t('key.songs') }} {{ musicList.length }}</span>
        </div>
        <div class="action-buttons">
          <button class="btn-play" @click="playAll">
            <svg viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M8 5v14l11-7z"/>
            </svg>
            {{ t('player.playAll') }}
          </button>
        </div>
      </div>
    </div>

    <!-- 音乐列表 -->
    <div v-if="musicList.length === 0" class="empty-container">
      <div class="empty-icon">
        <svg viewBox="0 0 24 24" width="64" height="64">
          <path fill="currentColor" d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/>
        </svg>
      </div>
      <p>{{ t('favorites.noFavorites') }}</p>
      <button class="btn-explore" @click="$router.push('/')">{{ t('home.discoverMusic') }}</button>
    </div>

    <div v-else class="music-list-container">
      <!-- 列表头部标签 -->
      <div class="list-tabs">
        <div class="tab active">{{ t('key.songs') }} {{ musicList.length }}</div>
        <div class="search-wrapper">
          <svg class="search-icon" viewBox="0 0 20 20">
            <path fill="currentColor" d="M8 3a5 5 0 100 10A5 5 0 008 3zM0 8a8 8 0 1114.32 4.906l5.387 5.387a1 1 0 01-1.414 1.414l-5.387-5.387A8 8 0 010 8z"/>
          </svg>
          <input 
            v-model="searchQuery" 
            type="text" 
            :placeholder="t('key.searchPlaceholder')" 
            class="search-input"
            @input="handleSearch"
          />
          <button v-if="searchQuery" class="clear-btn" @click="clearSearch">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/>
            </svg>
          </button>
        </div>
      </div>

      <!-- 歌曲列表 -->
      <div class="list-header-row">
        <span class="col-index">#</span>
        <span class="col-info">{{ t('key.title') }}</span>
        <span class="col-album">{{ t('key.album') }}</span>
        <span class="col-duration">{{ t('key.duration') }}</span>
        <span class="col-actions">{{ t('key.actions') }}</span>
      </div>
      <div 
        v-for="(music, index) in filteredList" 
        :key="music.id"
        :class="['music-item', { playing: currentMusic?.id === music.id }]"
        @dblclick="playMusic(music)"
      >
        <span class="col-index">
          <span v-if="currentMusic?.id === music.id" class="playing-icon">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M6 19h4V5H6v14zm8-14v14h4V5h-4z"/>
            </svg>
          </span>
          <span v-else>{{ index + 1 }}</span>
        </span>
        <span class="col-info">
          <div class="cover-wrapper-small">
            <img :src="getCoverUrl(music.id)" alt="封面" class="cover-small" @error="handleCoverError" />
            <div class="cover-overlay">
              <svg class="play-overlay-icon" viewBox="0 0 24 24" width="20" height="20">
                <path fill="currentColor" d="M8 5v14l11-7z"/>
              </svg>
            </div>
          </div>
          <div class="info-text">
            <div class="music-title">{{ music.title }}</div>
            <div class="music-artist">{{ music.artist }}</div>
          </div>
        </span>
        <span class="col-album">{{ music.album || '-' }}</span>
        <span class="col-duration">{{ formatDuration(music.duration) }}</span>
        <span class="col-actions">
          <button class="action-btn favorite-btn" @click.stop="toggleFavorite(music)" :class="{ active: isFavorite(music.id) }" title="喜欢">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/>
            </svg>
          </button>
          <button class="action-btn" @click.stop="playMusic(music)" title="播放">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M8 5v14l11-7z"/>
            </svg>
          </button>
          <button class="action-btn" @click.stop="addToPlaylist(music)" title="添加到播放列表">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
            </svg>
          </button>
        </span>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, computed, onUnmounted } from 'vue'
import { useI18n } from 'vue-i18n'
import apiConfig from '../config/apiConfig'

const { t } = useI18n()

const musicList = ref([])
const currentMusic = ref(null)
const favorites = ref([])
const searchQuery = ref('')

const handleSearch = () => {
  // 搜索响应由 computed 属性自动处理，这里留空或添加其他逻辑
}

const clearSearch = () => {
  searchQuery.value = ''
}

const filteredList = computed(() => {
  if (!searchQuery.value) return musicList.value
  const query = searchQuery.value.toLowerCase()
  return musicList.value.filter(music => 
    music.title.toLowerCase().includes(query) ||
    music.artist.toLowerCase().includes(query) ||
    (music.album && music.album.toLowerCase().includes(query))
  )
})

async function fetchFavorites(forceRefresh = false) {
  const token = localStorage.getItem('token')
  if (!token) {
    console.log('[FavoritesView] fetchFavorites: 未登录，跳过获取收藏列表')
    return []
  }

  console.log('[FavoritesView] fetchFavorites: 开始获取收藏列表，forceRefresh =', forceRefresh)

  try {
    // 如果不是强制刷新，先尝试从本地存储读取
    if (!forceRefresh) {
      const localFavorites = localStorage.getItem('favorites')
      if (localFavorites) {
        try {
          const parsed = JSON.parse(localFavorites)
          if (parsed && parsed.length > 0) {
            console.log('[FavoritesView] fetchFavorites: 从本地读取收藏列表，数量:', parsed.length)
            favorites.value = parsed
            return parsed
          }
        } catch (e) {
          console.error('[FavoritesView] 解析本地收藏列表失败:', e)
        }
      }
    }

    // 从服务器同步（添加时间戳参数防止缓存）
    const fullUrl = `${apiConfig.BASE_URL}${apiConfig.USER_FAVORITES}?t=${Date.now()}`
    console.log('[FavoritesView] fetchFavorites: 请求URL:', fullUrl)
    const response = await fetch(fullUrl, {
      headers: { 'Authorization': token }
    })

    console.log('[FavoritesView] fetchFavorites: 响应状态:', response.status)
    
    if (!response.ok) {
      throw new Error('获取收藏列表失败')
    }

    const result = await response.json()
    console.log('[FavoritesView] fetchFavorites: 服务器响应:', result)
    
    if (result.success && result.favorites) {
      console.log('[FavoritesView] fetchFavorites: 从服务器获取收藏列表成功，数量:', result.favorites.length)
      // 保存到本地存储
      localStorage.setItem('favorites', JSON.stringify(result.favorites))
      favorites.value = result.favorites
      return result.favorites
    } else {
      console.log('[FavoritesView] fetchFavorites: 服务器返回数据格式不正确，result =', result)
    }
    return []
  } catch (error) {
    console.error('[FavoritesView] fetchFavorites: 获取收藏列表失败:', error)
    return []
  }
}

function getCoverUrl(id) {
  return `https://music.cnmsb.xin/api/music/cover/${id}`
}

function formatDuration(seconds) {
  if (!seconds) return '-'
  const mins = Math.floor(seconds / 60)
  const secs = seconds % 60
  return `${mins}:${secs.toString().padStart(2, '0')}`
}

function playMusic(music) {
  currentMusic.value = music
  localStorage.setItem('currentMusic', JSON.stringify(music))
  window.dispatchEvent(new CustomEvent('add-to-playlist', { detail: music }))
  window.dispatchEvent(new CustomEvent('music-play', { detail: music }))
}

function playAll() {
  if (musicList.value.length > 0) {
    window.dispatchEvent(new CustomEvent('add-all-to-playlist', { detail: musicList.value }))
    playMusic(musicList.value[0])
  }
}

function addToPlaylist(music) {
  window.dispatchEvent(new CustomEvent('add-to-playlist', { detail: music }))
}

function isFavorite(musicId) {
  return favorites.value.some(f => f.id === musicId)
}

async function toggleFavorite(music) {
  const token = localStorage.getItem('token')
  if (!token) return

  try {
    if (isFavorite(music.id)) {
      const fullUrl = `${apiConfig.BASE_URL}${apiConfig.USER_FAVORITES_DELETE(music.id)}`
      await fetch(fullUrl, {
        method: 'DELETE',
        headers: { 'Authorization': token }
      })
      favorites.value = favorites.value.filter(f => f.id !== music.id)
      musicList.value = musicList.value.filter(m => m.id !== music.id)
      localStorage.setItem('favorites', JSON.stringify(favorites.value))
    } else {
      const fullUrl = `${apiConfig.BASE_URL}${apiConfig.USER_FAVORITES}`
      const response = await fetch(fullUrl, {
        method: 'POST',
        headers: { 
          'Authorization': token,
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ musicId: music.id })
      })
      const result = await response.json()
      if (result.success && result.data) {
        favorites.value.push(result.data)
        localStorage.setItem('favorites', JSON.stringify(favorites.value))
      }
    }
  } catch (error) {
    console.error('收藏操作失败:', error)
  }
}

function handleCoverError(e) {
  e.target.src = 'data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 200 200"><rect fill="%23f0f0f0" width="200" height="200"/><text fill="%23999" font-size="16" x="50%25" y="50%25" text-anchor="middle" dy="8">无封面</text></svg>'
}

// 处理用户登录事件
const handleUserLogin = async () => {
  console.log('[FavoritesView] 收到 user-login 事件，开始重新加载收藏列表')
  // 清除本地缓存的收藏数据
  localStorage.removeItem('favorites')
  favorites.value = []
  console.log('[FavoritesView] 已清除本地缓存，开始强制刷新')
  const result = await fetchFavorites(true)  // 强制从服务器刷新
  console.log('[FavoritesView] 收藏列表刷新完成，结果数量:', result.length)
  musicList.value = result
}

// 处理刷新请求事件
const handleRefreshNeeded = async () => {
  console.log('[FavoritesView] 收到 favorites-refresh-needed 事件，开始重新加载收藏列表')
  // 清除本地缓存的收藏数据
  localStorage.removeItem('favorites')
  favorites.value = []
  console.log('[FavoritesView] 已清除本地缓存，开始强制刷新')
  const result = await fetchFavorites(true)  // 强制从服务器刷新
  console.log('[FavoritesView] 收藏列表刷新完成，结果数量:', result.length)
  musicList.value = result
}

// 处理用户登出事件
const handleUserLogout = () => {
  musicList.value = []
  favorites.value = []
  currentMusic.value = null
  // 清除本地缓存的收藏数据
  localStorage.removeItem('favorites')
}

onMounted(async () => {
  console.log('[FavoritesView] 组件已挂载')
  
  const savedMusic = localStorage.getItem('currentMusic')
  if (savedMusic) {
    try {
      currentMusic.value = JSON.parse(savedMusic)
    } catch (e) {
      console.error('解析当前音乐失败:', e)
    }
  }

  // 检查是否有 token
  const token = localStorage.getItem('token')
  console.log('[FavoritesView] 检查 token:', token ? '已登录' : '未登录')
  
  if (token) {
    // 检查是否需要刷新（通过 localStorage 标志）
    const loginTimestamp = localStorage.getItem('loginTimestamp')
    const componentMountedTime = Date.now()
    
    console.log('[FavoritesView] 检查登录时间戳:', loginTimestamp)
    
    // 如果登录时间与当前时间相差小于 5 秒，说明刚登录，需要刷新
    if (loginTimestamp && (componentMountedTime - parseInt(loginTimestamp)) < 5000) {
      console.log('[FavoritesView] 检测到刚登录，强制刷新收藏列表')
      // 清除本地缓存的收藏数据
      localStorage.removeItem('favorites')
      favorites.value = []
      const result = await fetchFavorites(true)
      console.log('[FavoritesView] 刚登录刷新完成，结果数量:', result.length)
      musicList.value = result
    } else {
      console.log('[FavoritesView] 使用缓存或正常加载收藏列表')
      musicList.value = await fetchFavorites()
    }
  } else {
    console.log('[FavoritesView] 未登录，跳过加载收藏列表')
  }

  // 监听账号变更事件
  window.addEventListener('user-login', handleUserLogin)
  window.addEventListener('user-logout', handleUserLogout)
  // 监听全局刷新事件
  window.addEventListener('favorites-refresh-needed', handleRefreshNeeded)
  console.log('[FavoritesView] 事件监听器已注册')
})

// 组件卸载时清理事件监听器
onUnmounted(() => {
  window.removeEventListener('user-login', handleUserLogin)
  window.removeEventListener('user-logout', handleUserLogout)
  window.removeEventListener('favorites-refresh-needed', handleRefreshNeeded)
})
</script>

<style scoped>
.favorites-view {
  height: 100%;
  display: flex;
  flex-direction: column;
  background: var(--bg-main);
  overflow-y: auto;
}

/* 顶部封面区域 */
.playlist-header {
  display: flex;
  gap: 32px;
  padding: 40px 32px 24px 32px;
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.1) 0%, rgba(118, 75, 162, 0.1) 100%);
}

.cover-wrapper {
  flex-shrink: 0;
}

.playlist-cover {
  width: 200px;
  height: 200px;
  border-radius: 8px;
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.15);
  object-fit: cover;
}

.playlist-info {
  flex: 1;
  display: flex;
  flex-direction: column;
  justify-content: center;
}

.playlist-type {
  font-size: 14px;
  color: var(--text-secondary);
  margin-bottom: 8px;
}

.playlist-title {
  margin: 0 0 16px 0;
  font-size: 32px;
  font-weight: 700;
  color: var(--text-primary);
  line-height: 1.3;
}

.playlist-meta {
  display: flex;
  align-items: center;
  gap: 16px;
  margin-bottom: 24px;
  font-size: 14px;
  color: var(--text-secondary);
}

.music-count {
  font-weight: 500;
}

.action-buttons {
  display: flex;
  gap: 12px;
}

.btn-play {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 24px;
  font-size: 14px;
  font-weight: 600;
  color: white;
  background: #ED4040;
  border: none;
  border-radius: 20px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.btn-play:hover {
  background: #d93636;
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(237, 64, 64, 0.3);
}



/* 空状态 */
.empty-container {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 80px 20px;
  color: var(--text-muted);
}

.empty-icon {
  width: 80px;
  height: 80px;
  border-radius: 50%;
  background: rgba(102, 126, 234, 0.1);
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 16px;
  color: var(--primary);
}

.btn-explore {
  margin-top: 16px;
  padding: 10px 24px;
  font-size: 14px;
  font-weight: 500;
  color: var(--primary);
  background: rgba(102, 126, 234, 0.1);
  border: 1px solid rgba(102, 126, 234, 0.2);
  border-radius: 20px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.btn-explore:hover {
  background: rgba(102, 126, 234, 0.2);
}

/* 音乐列表容器 */
.music-list-container {
  padding: 0 32px 32px 32px;
  overflow-y: auto;
  flex: 1;
}

.list-tabs {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 16px 0;
  border-bottom: 2px solid transparent;
}

.search-wrapper {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 8px 16px;
  background: var(--bg-white);
  border-radius: 20px;
  box-shadow: var(--shadow-sm);
  border: 1px solid var(--border-input);
  transition: all 0.2s ease;
  width: 280px;
}

.search-wrapper:focus-within {
  border-color: var(--primary);
  box-shadow: 0 2px 12px rgba(102, 126, 234, 0.15);
}

.search-icon {
  width: 16px;
  height: 16px;
  color: var(--text-muted);
  flex-shrink: 0;
}

.search-input {
  flex: 1;
  border: none;
  background: transparent;
  font-size: 14px;
  color: var(--text-primary);
  outline: none;
}

.search-input::placeholder {
  color: var(--text-muted);
}

.clear-btn {
  width: 20px;
  height: 20px;
  border: none;
  background: transparent;
  border-radius: 50%;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--text-muted);
  transition: all 0.2s ease;
  padding: 0;
}

.clear-btn:hover {
  background: rgba(0, 0, 0, 0.05);
  color: var(--text-primary);
}

.tab {
  font-size: 16px;
  font-weight: 600;
  color: var(--text-primary);
  padding: 8px 0;
  cursor: pointer;
  position: relative;
}

.tab.active {
  color: #ED4040;
}

.tab.active::after {
  content: '';
  position: absolute;
  bottom: -18px;
  left: 0;
  right: 0;
  height: 3px;
  background: #ED4040;
  border-radius: 2px;
}

.list-header-row {
  display: grid;
  grid-template-columns: 50px 1fr 1fr 80px 120px;
  padding: 12px 0;
  font-size: 13px;
  color: var(--text-secondary);
  font-weight: 600;
}

.music-item {
  display: grid;
  grid-template-columns: 50px 1fr 1fr 80px 120px;
  padding: 12px 0;
  border-bottom: 1px solid var(--border-divider);
  cursor: pointer;
  transition: all 0.2s ease;
  align-items: center;
}

.music-item:last-child {
  border-bottom: none;
}

.music-item:hover {
  background: var(--bg-hover);
}

.music-item:hover .cover-wrapper-small .cover-overlay {
  opacity: 1;
}

.music-item.playing {
  background: rgba(102, 126, 234, 0.15);
}

.music-item.playing .music-title {
  color: #667eea;
}

.col-index {
  color: rgba(255, 255, 255, 0.5);
  font-size: 13px;
  text-align: center;
}

.playing-icon {
  color: #667eea;
  display: flex;
  align-items: center;
  justify-content: center;
}

.col-info {
  display: flex;
  align-items: center;
  gap: 12px;
  overflow: hidden;
}

.cover-wrapper-small {
  position: relative;
  width: 40px;
  height: 40px;
  flex-shrink: 0;
  border-radius: 6px;
  overflow: hidden;
}

.cover-small {
  width: 100%;
  height: 100%;
  object-fit: cover;
  transition: all 0.2s ease;
}

.cover-overlay {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  opacity: 0;
  transition: all 0.2s ease;
}

.play-overlay-icon {
  color: white;
  transform: scale(0.8);
  transition: all 0.2s ease;
}

.cover-wrapper-small:hover .play-overlay-icon {
  transform: scale(1);
}

.info-text {
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.music-title {
  font-size: 14px;
  color: var(--text-primary);
  font-weight: 500;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.music-artist {
  font-size: 12px;
  color: var(--text-secondary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.col-album {
  font-size: 13px;
  color: var(--text-secondary);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.col-duration {
  font-size: 13px;
  color: var(--text-muted);
  text-align: center;
  font-weight: 500;
}

.col-actions {
  display: flex;
  gap: 4px;
  justify-content: flex-end;
}

.action-btn {
  width: 32px;
  height: 32px;
  border: none;
  background: transparent;
  border-radius: 50%;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--text-secondary);
  transition: all 0.2s ease;
}

.action-btn:hover {
  background: var(--bg-hover);
  color: var(--text-primary);
}

.favorite-btn.active {
  color: #ED4040;
}

.favorite-btn.active:hover {
  background: rgba(237, 64, 64, 0.15);
}

/* 滚动条样式 */
.music-list-container::-webkit-scrollbar {
  width: 6px;
}

.music-list-container::-webkit-scrollbar-track {
  background: transparent;
}

.music-list-container::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 3px;
}

.music-list-container::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}
</style>