<template>
  <div class="music-list">
    <div class="list-header">
      <h2 class="section-title">{{ title }}</h2>
      <div v-if="showSearch" class="search-box">
        <svg class="search-icon" viewBox="0 0 20 20">
          <path fill="currentColor" d="M8 3a5 5 0 100 10A5 5 0 008 3zM0 8a8 8 0 1114.32 4.906l5.387 5.387a1 1 0 01-1.414 1.414l-5.387-5.387A8 8 0 010 8z"/>
        </svg>
        <input 
          v-model="searchQuery" 
          type="text" 
          placeholder="搜索音乐..." 
          @input="handleSearch"
        />
      </div>
    </div>
    
    <div v-if="loading" class="loading">
      <div class="loading-spinner"></div>
      <p>加载中...</p>
    </div>
    
    <div v-else-if="musicList.length === 0" class="empty">
      <div class="empty-icon">
        <svg viewBox="0 0 24 24" width="64" height="64">
          <path fill="currentColor" d="M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4V7h4V3h-6z"/>
        </svg>
      </div>
      <p>暂无音乐</p>
    </div>
    
    <div v-else class="list-container">
      <div class="list-header-row">
        <span class="col-index">#</span>
        <span class="col-title">标题</span>
        <span class="col-artist">艺术家</span>
        <span class="col-album">专辑</span>
        <span class="col-duration">时长</span>
        <span class="col-actions">
          <button class="header-action-btn" @click="addAllToPlaylist" title="全部添加到播放列表">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M14 10H2v2h12v-2zm0-4H2v2h12V6zm4 8v-4h-2v4h-4v2h4v4h2v-4h4v-2h-4zM2 16h8v-2H2v2z"/>
            </svg>
            <span>全部添加</span>
          </button>
        </span>
      </div>
      <div class="music-items">
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
          <span class="col-title">
            <div class="cover-wrapper">
              <img :src="getCoverUrl(music.id)" alt="封面" class="cover" @error="handleCoverError" />
              <div class="cover-overlay">
                <svg class="play-overlay-icon" viewBox="0 0 24 24" width="24" height="24">
                  <path fill="currentColor" d="M8 5v14l11-7z"/>
                </svg>
              </div>
            </div>
            <span class="title-text">{{ music.title }}</span>
          </span>
          <span class="col-artist">{{ music.artist }}</span>
          <span class="col-album">{{ music.album || '-' }}</span>
          <span class="col-duration">{{ formatDuration(music.duration) }}</span>
          <span class="col-actions">
            <button class="action-btn" @click.stop="playMusic(music)" title="播放">
              <svg viewBox="0 0 24 24" width="16" height="16">
                <path fill="currentColor" d="M8 5v14l11-7z"/>
              </svg>
            </button>
            <button class="action-btn" @click.stop="addToPlaylist(music)" title="添加到播放列表">
              <svg viewBox="0 0 24 24" width="16" height="16">
                <path fill="currentColor" d="M14 10H2v2h12v-2zm0-4H2v2h12V6zm4 8v-4h-2v4h-4v2h4v4h2v-4h4v-2h-4zM2 16h8v-2H2v2z"/>
              </svg>
            </button>
            <button v-if="showFavorite" class="action-btn" @click.stop="toggleFavorite(music)" title="收藏">
              <svg viewBox="0 0 24 24" width="16" height="16">
                <path :fill="isFavorite(music.id) ? '#e91e63' : 'currentColor'" d="M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z"/>
              </svg>
            </button>
          </span>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import apiConfig from '../config/apiConfig'

// 统一的 API 请求函数
async function apiRequest(url, options = {}) {
  const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
  return fetch(fullUrl, options)
}

const props = defineProps({
  title: {
    type: String,
    default: '音乐列表'
  },
  showSearch: {
    type: Boolean,
    default: false
  },
  showFavorite: {
    type: Boolean,
    default: false
  },
  fetchFunction: {
    type: Function,
    default: null
  }
})

const router = useRouter()
const musicList = ref([])
const loading = ref(false)
const searchQuery = ref('')
const currentMusic = ref(null)
const favorites = ref([])

const filteredList = computed(() => {
  return musicList.value
})

const fetchMusicList = async () => {
  if (!props.fetchFunction) return
  
  loading.value = true
  try {
    const result = await props.fetchFunction()
    musicList.value = result || []
  } catch (error) {
    console.error('获取音乐列表失败:', error)
  } finally {
    loading.value = false
  }
}

const handleSearch = () => {
  // 搜索逻辑由父组件处理
}

const getCoverUrl = (id) => {
  return `https://music.cnmsb.xin/api/music/cover/${id}`
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

const addToPlaylist = (music) => {
  console.log('addToPlaylist: 尝试添加音乐到播放列表', music)
  window.dispatchEvent(new CustomEvent('add-to-playlist', { detail: music }))
}

const addAllToPlaylist = () => {
  console.log('addAllToPlaylist: 尝试添加全部音乐到播放列表，共', filteredList.value.length, '首')
  window.dispatchEvent(new CustomEvent('add-all-to-playlist', { detail: filteredList.value }))
}

const isFavorite = (musicId) => {
  return favorites.value.some(f => f.id === musicId)
}

const toggleFavorite = async (music) => {
  const token = localStorage.getItem('token')
  if (!token) {
    return
  }

  try {
    if (isFavorite(music.id)) {
      await apiRequest(apiConfig.USER_FAVORITES_DELETE(music.id), {
        method: 'DELETE',
        headers: { 'Authorization': token }
      })
      favorites.value = favorites.value.filter(f => f.id !== music.id)
      // 从当前列表中移除（适用于收藏页面）
      musicList.value = musicList.value.filter(m => m.id !== music.id)
      // 更新本地存储
      localStorage.setItem('favorites', JSON.stringify(favorites.value))
    } else {
      const response = await apiRequest(apiConfig.USER_FAVORITES, {
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
        // 更新本地存储
        localStorage.setItem('favorites', JSON.stringify(favorites.value))
      }
    }
  } catch (error) {
    console.error('收藏操作失败:', error)
  }
}

onMounted(() => {
  const savedMusic = localStorage.getItem('currentMusic')
  if (savedMusic) {
    try {
      currentMusic.value = JSON.parse(savedMusic)
    } catch (e) {
      console.error('解析当前音乐失败:', e)
    }
  }

  // 加载收藏列表
  const localFavorites = localStorage.getItem('favorites')
  if (localFavorites) {
    try {
      favorites.value = JSON.parse(localFavorites)
    } catch (e) {
      console.error('解析收藏列表失败:', e)
    }
  }

  fetchMusicList()
})

defineExpose({
  fetchMusicList
})
</script>

<style scoped>
.music-list {
  padding: 24px;
  height: 100%;
  overflow-y: auto;
}

.list-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 24px;
}

.section-title {
  margin: 0;
  color: var(--text-primary);
  font-size: 28px;
  font-weight: 700;
  background: var(--gradient-primary);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.search-box {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 10px 16px;
  background: white;
  border-radius: var(--radius-full);
  box-shadow: var(--shadow-sm);
  border: 2px solid transparent;
  transition: all var(--transition-normal);
  width: 280px;
}

.search-box:focus-within {
  border-color: var(--primary);
  box-shadow: var(--shadow-md);
}

.search-icon {
  width: 18px;
  height: 18px;
  color: var(--text-muted);
  flex-shrink: 0;
}

.search-box input {
  flex: 1;
  border: none;
  background: transparent;
  font-size: 14px;
  color: var(--text-primary);
  outline: none;
}

.search-box input::placeholder {
  color: var(--text-muted);
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
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.1) 0%, rgba(118, 75, 162, 0.1) 100%);
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 16px;
  color: var(--primary);
}

.list-container {
  background: white;
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-sm);
  overflow: hidden;
}

.list-header-row {
  display: grid;
  grid-template-columns: 50px 1fr 1fr 1fr 80px 140px;
  padding: 16px 20px;
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.05) 0%, rgba(118, 75, 162, 0.05) 100%);
  font-size: 13px;
  color: var(--text-secondary);
  font-weight: 600;
  border-bottom: 1px solid var(--border-light);
}

.header-action-btn {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 6px 12px;
  font-size: 12px;
  font-weight: 500;
  color: var(--primary);
  background: rgba(102, 126, 234, 0.1);
  border: 1px solid rgba(102, 126, 234, 0.2);
  border-radius: 6px;
  cursor: pointer;
  transition: all 0.2s ease;
  margin-left: auto;
}

.header-action-btn:hover {
  background: rgba(102, 126, 234, 0.2);
  border-color: rgba(102, 126, 234, 0.3);
  transform: translateY(-1px);
}

.header-action-btn:active {
  transform: translateY(0);
}

.music-items {
  max-height: calc(100vh - 300px);
  overflow-y: auto;
}

.music-item {
  display: grid;
  grid-template-columns: 50px 1fr 1fr 1fr 80px 120px;
  padding: 14px 20px;
  border-bottom: 1px solid var(--border-light);
  cursor: pointer;
  transition: all var(--transition-normal);
  align-items: center;
  position: relative;
}

.music-item:last-child {
  border-bottom: none;
}

.music-item:hover {
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.03) 0%, rgba(118, 75, 162, 0.03) 100%);
}

.music-item:hover .cover-wrapper .cover-overlay {
  opacity: 1;
}

.music-item.playing {
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.08) 0%, rgba(118, 75, 162, 0.08) 100%);
}

.music-item.playing .title-text {
  color: var(--primary);
  font-weight: 600;
}

.music-item.playing .cover {
  box-shadow: 0 0 20px rgba(102, 126, 234, 0.3);
}

.col-index {
  color: var(--text-muted);
  font-size: 13px;
  text-align: center;
}

.playing-icon {
  color: var(--primary);
  display: flex;
  align-items: center;
  justify-content: center;
}

.col-title {
  display: flex;
  align-items: center;
  gap: 12px;
  overflow: hidden;
}

.cover-wrapper {
  position: relative;
  width: 44px;
  height: 44px;
  flex-shrink: 0;
  border-radius: 8px;
  overflow: hidden;
}

.cover {
  width: 100%;
  height: 100%;
  object-fit: cover;
  transition: all var(--transition-normal);
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
  transition: all var(--transition-fast);
}

.play-overlay-icon {
  color: white;
  transform: scale(0.8);
  transition: all var(--transition-fast);
}

.cover-wrapper:hover .play-overlay-icon {
  transform: scale(1);
}

.title-text {
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  font-size: 14px;
  color: var(--text-primary);
  font-weight: 500;
}

.col-artist, .col-album {
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
  gap: 8px;
  justify-content: center;
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
  transition: all var(--transition-normal);
}

.action-btn:hover {
  background: rgba(102, 126, 234, 0.1);
  color: var(--primary);
  transform: scale(1.1);
}

/* 滚动条样式 */
.music-items::-webkit-scrollbar {
  width: 6px;
}

.music-items::-webkit-scrollbar-track {
  background: transparent;
}

.music-items::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 3px;
}

.music-items::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}

/* ========================================
   响应式适配
   ======================================== */

/* 小屏幕适配 (< 640px) */
@media screen and (max-width: 639px) {
  .music-list {
    padding: clamp(12px, 3vw, 16px);
  }
  
  .list-header {
    flex-direction: column;
    align-items: flex-start;
    gap: 12px;
  }
  
  .section-title {
    font-size: clamp(18px, 4.5vw, 22px);
  }
  
  .search-box {
    width: 100%;
    max-width: none;
  }
  
  .list-container {
    border-radius: var(--radius-md);
  }
  
  .list-header-row {
    display: none;
  }
  
  .music-item {
    display: flex;
    flex-direction: column;
    padding: clamp(10px, 2.5vw, 14px);
    gap: 8px;
  }
  
  .col-index {
    position: absolute;
    top: 8px;
    left: 8px;
    background: rgba(0, 0, 0, 0.3);
    border-radius: 50%;
    width: 20px;
    height: 20px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 10px;
  }
  
  .col-title {
    flex-direction: row;
    padding-left: 32px;
  }
  
  .cover-wrapper {
    width: clamp(40px, 10vw, 48px);
    height: clamp(40px, 10vw, 48px);
  }
  
  .title-text {
    font-size: clamp(13px, 3.5vw, 14px);
  }
  
  .col-artist,
  .col-album,
  .col-duration {
    font-size: clamp(11px, 3vw, 12px);
    padding-left: clamp(44px, 12vw, 56px);
  }
  
  .col-artist::before {
    content: '艺术家: ';
    font-weight: 500;
  }
  
  .col-album::before {
    content: '专辑: ';
    font-weight: 500;
  }
  
  .col-duration::before {
    content: '时长: ';
    font-weight: 500;
  }
  
  .col-actions {
    justify-content: flex-start;
    padding-left: clamp(44px, 12vw, 56px);
  }
  
  .action-btn {
    width: 36px;
    height: 36px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 180px);
  }
}

/* 中等屏幕适配 (640px - 1023px) */
@media screen and (min-width: 640px) and (max-width: 1023px) {
  .music-list {
    padding: 20px;
  }
  
  .section-title {
    font-size: 24px;
  }
  
  .search-box {
    width: clamp(200px, 35vw, 280px);
  }
  
  .list-header-row {
    grid-template-columns: 40px 1fr 0.8fr 0.8fr 70px 120px;
    padding: 14px 16px;
  }
  
  .music-item {
    grid-template-columns: 40px 1fr 0.8fr 0.8fr 70px 100px;
    padding: 12px 16px;
  }
  
  .cover-wrapper {
    width: 40px;
    height: 40px;
  }
  
  .title-text {
    font-size: 13px;
  }
  
  .col-artist,
  .col-album {
    font-size: 12px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 140px);
  }
}

/* 大屏幕适配 (1024px - 1279px) */
@media screen and (min-width: 1024px) and (max-width: 1279px) {
  .music-list {
    padding: 24px;
  }
  
  .section-title {
    font-size: 28px;
  }
  
  .list-header-row {
    padding: 16px 20px;
  }
  
  .music-item {
    padding: 14px 20px;
  }
}

/* 2K屏幕适配 (1536px - 1919px) */
@media screen and (min-width: 1536px) and (max-width: 1919px) {
  .music-list {
    padding: 32px;
  }
  
  .section-title {
    font-size: 32px;
  }
  
  .search-box {
    width: clamp(280px, 25vw, 360px);
    padding: 14px 20px;
  }
  
  .search-box input {
    font-size: 15px;
  }
  
  .list-header-row {
    grid-template-columns: 60px 1.2fr 1fr 1fr 90px 160px;
    padding: 18px 24px;
    font-size: 14px;
  }
  
  .music-item {
    grid-template-columns: 60px 1.2fr 1fr 1fr 90px 140px;
    padding: 16px 24px;
  }
  
  .cover-wrapper {
    width: 50px;
    height: 50px;
    border-radius: 10px;
  }
  
  .title-text {
    font-size: 15px;
  }
  
  .col-artist,
  .col-album {
    font-size: 14px;
  }
  
  .col-duration {
    font-size: 14px;
  }
  
  .action-btn {
    width: 40px;
    height: 40px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 160px);
  }
}

/* 3K屏幕适配 (1920px - 2559px) */
@media screen and (min-width: 1920px) and (max-width: 2559px) {
  .music-list {
    padding: 40px;
  }
  
  .section-title {
    font-size: 36px;
  }
  
  .search-box {
    width: clamp(320px, 28vw, 420px);
    padding: 16px 24px;
  }
  
  .search-box input {
    font-size: 16px;
  }
  
  .list-header-row {
    grid-template-columns: 70px 1.3fr 1.1fr 1.1fr 100px 180px;
    padding: 20px 28px;
    font-size: 15px;
  }
  
  .music-item {
    grid-template-columns: 70px 1.3fr 1.1fr 1.1fr 100px 160px;
    padding: 18px 28px;
  }
  
  .cover-wrapper {
    width: 56px;
    height: 56px;
    border-radius: 12px;
  }
  
  .title-text {
    font-size: 16px;
  }
  
  .col-artist,
  .col-album {
    font-size: 15px;
  }
  
  .col-duration {
    font-size: 15px;
  }
  
  .action-btn {
    width: 44px;
    height: 44px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 180px);
  }
}

/* 4K屏幕适配 (>= 2560px) */
@media screen and (min-width: 2560px) {
  .music-list {
    padding: 48px;
  }
  
  .section-title {
    font-size: 42px;
  }
  
  .search-box {
    width: clamp(380px, 30vw, 500px);
    padding: 18px 28px;
    border-radius: 28px;
  }
  
  .search-box input {
    font-size: 18px;
  }
  
  .list-container {
    border-radius: 24px;
  }
  
  .list-header-row {
    grid-template-columns: 80px 1.4fr 1.2fr 1.2fr 110px 200px;
    padding: 24px 32px;
    font-size: 16px;
  }
  
  .music-item {
    grid-template-columns: 80px 1.4fr 1.2fr 1.2fr 110px 180px;
    padding: 20px 32px;
  }
  
  .cover-wrapper {
    width: 64px;
    height: 64px;
    border-radius: 14px;
  }
  
  .title-text {
    font-size: 18px;
  }
  
  .col-artist,
  .col-album {
    font-size: 16px;
  }
  
  .col-duration {
    font-size: 16px;
  }
  
  .action-btn {
    width: 48px;
    height: 48px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 200px);
  }
}

/* 5K屏幕适配 (>= 5120px) */
@media screen and (min-width: 5120px) {
  .music-list {
    padding: 64px;
  }
  
  .section-title {
    font-size: 52px;
  }
  
  .search-box {
    width: clamp(480px, 35vw, 640px);
    padding: 24px 36px;
    border-radius: 36px;
  }
  
  .search-box input {
    font-size: 22px;
  }
  
  .list-container {
    border-radius: 32px;
  }
  
  .list-header-row {
    grid-template-columns: 100px 1.5fr 1.3fr 1.3fr 130px 240px;
    padding: 32px 40px;
    font-size: 20px;
  }
  
  .music-item {
    grid-template-columns: 100px 1.5fr 1.3fr 1.3fr 130px 220px;
    padding: 28px 40px;
  }
  
  .cover-wrapper {
    width: 80px;
    height: 80px;
    border-radius: 18px;
  }
  
  .title-text {
    font-size: 22px;
  }
  
  .col-artist,
  .col-album {
    font-size: 20px;
  }
  
  .col-duration {
    font-size: 20px;
  }
  
  .action-btn {
    width: 56px;
    height: 56px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 240px);
  }
}

/* 高度适配 - 短屏幕 */
@media screen and (max-height: 720px) {
  .music-list {
    padding: 16px;
  }
  
  .section-title {
    font-size: 20px;
  }
  
  .list-header-row {
    padding: 10px 14px;
  }
  
  .music-item {
    padding: 10px 14px;
  }
  
  .cover-wrapper {
    width: 36px;
    height: 36px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 100px);
  }
}

/* 高度适配 - 超高屏幕 */
@media screen and (min-height: 1440px) {
  .music-list {
    padding: 32px;
  }
  
  .section-title {
    font-size: 30px;
  }
  
  .list-header-row {
    padding: 18px 24px;
  }
  
  .music-item {
    padding: 16px 24px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 200px);
  }
}

/* 横屏适配 */
@media screen and (orientation: landscape) and (max-height: 600px) {
  .list-header {
    margin-bottom: 16px;
  }
  
  .section-title {
    font-size: 18px;
  }
  
  .music-items {
    max-height: calc(100vh - var(--player-bar-height) - 80px);
  }
}

/* 触摸设备适配 */
@media (hover: none) and (pointer: coarse) {
  .music-item {
    padding: 16px;
  }
  
  .action-btn {
    min-width: 44px;
    min-height: 44px;
  }
  
  .header-action-btn {
    padding: 10px 16px;
    min-height: 44px;
  }
}
</style>