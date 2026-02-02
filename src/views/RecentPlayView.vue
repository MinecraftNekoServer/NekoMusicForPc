<template>
  <div class="recent-play-view">
    <!-- 顶部封面区域 -->
    <div class="playlist-header" v-if="recentList.length > 0">
      <div class="cover-wrapper">
        <img :src="getCoverUrl(recentList[0].id)" alt="封面" class="playlist-cover" @error="handleCoverError" />
      </div>
      <div class="playlist-info">
        <div class="playlist-type">歌单</div>
        <h1 class="playlist-title">最近播放</h1>
        <div class="playlist-meta">
          <span class="music-count">歌曲 {{ recentList.length }}</span>
        </div>
        <div class="action-buttons">
          <button class="btn-play" @click="playAll">
            <svg viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M8 5v14l11-7z"/>
            </svg>
            播放全部
          </button>
        </div>
      </div>
    </div>

    <!-- 音乐列表 -->
    <div v-if="recentList.length === 0" class="empty-container">
      <div class="empty-icon">
        <svg viewBox="0 0 24 24" width="64" height="64">
          <path fill="currentColor" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-2 15l-5-5 1.41-1.41L10 14.17l7.59-7.59L19 8l-9 9z"/>
        </svg>
      </div>
      <p>暂无最近播放记录</p>
      <button class="btn-explore" @click="$router.push('/')">去发现音乐</button>
    </div>

    <div v-else class="music-list-container">
      <!-- 列表头部标签 -->
      <div class="list-tabs">
        <div class="tab active">歌曲 {{ recentList.length }}</div>
        <div class="search-wrapper">
          <svg class="search-icon" viewBox="0 0 20 20">
            <path fill="currentColor" d="M8 3a5 5 0 100 10A5 5 0 008 3zM0 8a8 8 0 1114.32 4.906l5.387 5.387a1 1 0 01-1.414 1.414l-5.387-5.387A8 8 0 010 8z"/>
          </svg>
          <input 
            v-model="searchQuery" 
            type="text" 
            placeholder="搜索音乐..." 
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
        <span class="col-info">标题</span>
        <span class="col-album">专辑</span>
        <span class="col-duration">时长</span>
        <span class="col-actions">操作</span>
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
          <button class="action-btn" @click.stop="removeFromRecent(music)" title="删除">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM19 4h-3.5l-1-1h-5l-1 1H5v2h14V4z"/>
            </svg>
          </button>
        </span>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue'

const recentList = ref([])
const currentMusic = ref(null)
const searchQuery = ref('')

const handleSearch = () => {
  // 搜索响应由 computed 属性自动处理，这里留空或添加其他逻辑
}

const clearSearch = () => {
  searchQuery.value = ''
}

const filteredList = computed(() => {
  if (!searchQuery.value) return recentList.value
  const query = searchQuery.value.toLowerCase()
  return recentList.value.filter(music => 
    music.title.toLowerCase().includes(query) ||
    music.artist.toLowerCase().includes(query) ||
    (music.album && music.album.toLowerCase().includes(query))
  )
})

function loadRecentPlay() {
  const recentData = localStorage.getItem('recentPlay')
  if (recentData) {
    try {
      recentList.value = JSON.parse(recentData)
    } catch (e) {
      console.error('解析最近播放列表失败:', e)
      recentList.value = []
    }
  }
}

function saveRecentPlay() {
  localStorage.setItem('recentPlay', JSON.stringify(recentList.value))
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
  
  // 更新最近播放
  addToRecent(music)
  
  window.dispatchEvent(new CustomEvent('add-to-playlist', { detail: music }))
  window.dispatchEvent(new CustomEvent('music-play', { detail: music }))
}

function addToRecent(music) {
  // 移除已存在的相同音乐
  const index = recentList.value.findIndex(m => m.id === music.id)
  if (index !== -1) {
    recentList.value.splice(index, 1)
  }
  
  // 添加到开头
  recentList.value.unshift(music)
  
  saveRecentPlay()
}

function removeFromRecent(music) {
  recentList.value = recentList.value.filter(m => m.id !== music.id)
  saveRecentPlay()
}

function playAll() {
  if (recentList.value.length > 0) {
    window.dispatchEvent(new CustomEvent('add-all-to-playlist', { detail: recentList.value }))
    playMusic(recentList.value[0])
  }
}

function addToPlaylist(music) {
  window.dispatchEvent(new CustomEvent('add-to-playlist', { detail: music }))
}

function handleCoverError(e) {
  e.target.src = 'data:image/svg+xml,<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200" viewBox="0 0 200 200"><rect fill="%23f0f0f0" width="200" height="200"/><text fill="%23999" font-size="16" x="50%25" y="50%25" text-anchor="middle" dy="8">无封面</text></svg>'
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

  loadRecentPlay()
})
</script>

<style scoped>
.recent-play-view {
  height: 100%;
  display: flex;
  flex-direction: column;
  background: linear-gradient(135deg, #f5f7fa 0%, #e4e8ec 100%);
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
  background: white;
  border-radius: 20px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.05);
  border: 1px solid #e5e5e5;
  transition: all 0.2s ease;
  width: 280px;
}

.search-wrapper:focus-within {
  border-color: #ED4040;
  box-shadow: 0 2px 12px rgba(237, 64, 64, 0.15);
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
  border-bottom: 1px solid #f0f0f0;
  cursor: pointer;
  transition: all 0.2s ease;
  align-items: center;
}

.music-item:last-child {
  border-bottom: none;
}

.music-item:hover {
  background: rgba(0, 0, 0, 0.02);
}

.music-item:hover .cover-wrapper-small .cover-overlay {
  opacity: 1;
}

.music-item.playing {
  background: rgba(237, 64, 64, 0.05);
}

.music-item.playing .music-title {
  color: #ED4040;
}

.col-index {
  color: var(--text-muted);
  font-size: 13px;
  text-align: center;
}

.playing-icon {
  color: #ED4040;
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
  background: rgba(0, 0, 0, 0.05);
  color: var(--text-primary);
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