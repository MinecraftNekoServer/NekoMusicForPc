<template>
  <div class="ranking-view">
    <div class="ranking-header">
      <h1 class="ranking-title">热门音乐</h1>
      <button @click="playAll" class="play-all-btn">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="currentColor">
          <path d="M8 5v14l11-7z"/>
        </svg>
        播放全部
      </button>
    </div>

    <div v-if="loading" class="loading">加载中...</div>
    <div v-else-if="rankingList && rankingList.length > 0" class="ranking-list">
      <div
        v-for="(item, index) in rankingList"
        :key="item.id"
        class="ranking-item"
      >
        <div class="ranking-number" :class="`rank-${index + 1}`">
          {{ index + 1 }}
        </div>
        <div class="ranking-cover">
          <img
            :src="item.coverUrl"
            :alt="item.title"
            class="cover-img"
            @error="handleImageError"
          />
        </div>
        <div class="ranking-info">
          <h3 class="music-title">{{ item.title }}</h3>
          <p class="music-artist">{{ item.artist }}</p>
        </div>
        <div class="ranking-play-count">{{ formatPlayCount(item.playCount) }}</div>
        <div class="ranking-actions">
          <button @click="playMusic(item)" class="action-btn play-btn" title="播放">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="currentColor">
              <path d="M8 5v14l11-7z"/>
            </svg>
          </button>
          <button @click="downloadMusic(item)" class="action-btn download-btn" title="下载">
            <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path>
              <polyline points="7 10 12 15 17 10"></polyline>
              <line x1="12" y1="15" x2="12" y2="3"></line>
            </svg>
          </button>
        </div>
      </div>
    </div>
    <div v-else class="no-content">
      <p>暂无热门音乐</p>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import apiConfig from '@/config/apiConfig.js'

const rankingList = ref([])
const loading = ref(false)

// 获取排行榜
const fetchRanking = async () => {
  loading.value = true
  try {
    const timestamp = Date.now()
    const response = await fetch(`${apiConfig.BASE_URL}/api/music/ranking?t=${timestamp}`)
    const data = await response.json()

    if (data.success && data.data) {
      rankingList.value = data.data.map(item => ({
        ...item,
        coverUrl: `${apiConfig.BASE_URL}/api/music/cover/${item.id}`
      }))
      console.log('排行榜加载成功:', rankingList.value.length, '首')
    } else {
      console.error('获取排行榜失败:', data.message)
      rankingList.value = []
    }
  } catch (error) {
    console.error('排行榜请求失败:', error)
    rankingList.value = []
  } finally {
    loading.value = false
  }
}

// 播放音乐
const playMusic = (music) => {
  window.location.hash = `#play=${encodeURIComponent(JSON.stringify(music))}`
}

// 播放全部
const playAll = () => {
  window.location.hash = `#playlist=${encodeURIComponent(JSON.stringify(rankingList.value))}&index=0`
}

// 下载音乐
const downloadMusic = (music) => {
  const downloadUrl = `${apiConfig.BASE_URL}/api/music/file/${music.id}`
  const link = document.createElement('a')
  link.href = downloadUrl
  link.download = `${music.title}.mp3`
  document.body.appendChild(link)
  link.click()
  document.body.removeChild(link)
}

// 格式化播放次数
const formatPlayCount = (count) => {
  if (count >= 10000) {
    return (count / 10000).toFixed(1) + '万'
  }
  return count.toString()
}

// 处理图片加载错误
const handleImageError = (event) => {
  event.target.src = '/api/defaultIcon'
}

onMounted(() => {
  fetchRanking()
})
</script>

<style scoped>
.ranking-view {
  height: 100%;
  overflow-y: auto;
  padding: 32px;
}

.ranking-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 32px;
}

.ranking-title {
  font-size: 28px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0;
}

.play-all-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 20px;
  background: var(--gradient-primary);
  color: white;
  border: none;
  border-radius: var(--radius-md);
  font-size: 14px;
  font-weight: 600;
  cursor: pointer;
  transition: all var(--transition-normal);
}

.play-all-btn:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-md);
}

.play-all-btn:active {
  transform: translateY(0);
}

.loading {
  text-align: center;
  color: var(--text-secondary);
  padding: 60px;
  font-size: 14px;
}

.ranking-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.ranking-item {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 16px;
  background: var(--bg-card);
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-sm);
  border: 1px solid var(--border-light);
  transition: all var(--transition-normal);
}

.ranking-item:hover {
  background: rgba(102, 126, 234, 0.03);
  transform: translateX(4px);
  box-shadow: var(--shadow-md);
}

.ranking-number {
  width: 40px;
  height: 40px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 18px;
  font-weight: 800;
  color: var(--text-secondary);
  border-radius: var(--radius-md);
  background: rgba(0, 0, 0, 0.03);
}

.rank-1 {
  background: linear-gradient(135deg, #FFD700 0%, #FFA500 100%);
  color: white;
}

.rank-2 {
  background: linear-gradient(135deg, #C0C0C0 0%, #A0A0A0 100%);
  color: white;
}

.rank-3 {
  background: linear-gradient(135deg, #CD7F32 0%, #B8860B 100%);
  color: white;
}

.ranking-cover {
  flex-shrink: 0;
  width: 56px;
  height: 56px;
  border-radius: var(--radius-md);
  overflow: hidden;
  box-shadow: var(--shadow-sm);
}

.cover-img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  transition: transform var(--transition-normal);
}

.ranking-item:hover .cover-img {
  transform: scale(1.05);
}

.ranking-info {
  flex: 1;
  min-width: 0;
}

.music-title {
  font-size: 15px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0 0 4px 0;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.music-artist {
  font-size: 13px;
  color: var(--text-secondary);
  margin: 0;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.ranking-play-count {
  flex-shrink: 0;
  font-size: 13px;
  color: var(--text-muted);
  padding: 6px 12px;
  background: rgba(0, 0, 0, 0.03);
  border-radius: var(--radius-sm);
}

.ranking-actions {
  display: flex;
  gap: 8px;
  flex-shrink: 0;
}

.action-btn {
  width: 36px;
  height: 36px;
  display: flex;
  align-items: center;
  justify-content: center;
  border: none;
  border-radius: var(--radius-md);
  background: var(--bg-glass);
  color: var(--text-secondary);
  cursor: pointer;
  transition: all var(--transition-normal);
}

.action-btn:hover {
  background: var(--primary);
  color: white;
  transform: scale(1.05);
}

.action-btn:active {
  transform: scale(0.95);
}

.no-content {
  text-align: center;
  color: var(--text-secondary);
  padding: 60px;
  font-size: 14px;
}

/* 滚动条样式 */
.ranking-view::-webkit-scrollbar {
  width: 8px;
}

.ranking-view::-webkit-scrollbar-track {
  background: transparent;
}

.ranking-view::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 4px;
}

.ranking-view::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}
</style>