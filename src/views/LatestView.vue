<template>
  <div class="latest-view">
    <div class="latest-header">
      <h1 class="latest-title">最新音乐</h1>
      <button @click="playAll" class="play-all-btn">
        <svg width="16" height="16" viewBox="0 0 24 24" fill="currentColor">
          <path d="M8 5v14l11-7z"/>
        </svg>
        播放全部
      </button>
    </div>

    <div v-if="loading" class="loading">加载中...</div>
    <div v-else-if="latestList && latestList.length > 0" class="latest-list">
      <div
        v-for="(item, index) in latestList"
        :key="item.id"
        class="latest-item"
      >
        <div class="latest-number">
          {{ index + 1 }}
        </div>
        <div class="latest-cover">
          <img
            :src="item.coverUrl"
            :alt="item.title"
            class="cover-img"
            @error="handleImageError"
          />
        </div>
        <div class="latest-info">
          <h3 class="music-title">{{ item.title }}</h3>
          <p class="music-artist">{{ item.artist }}</p>
        </div>
        <div class="latest-time">{{ formatTime(item.createdAt) }}</div>
        <div class="latest-actions">
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
      <p>暂无最新音乐</p>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import apiConfig from '@/config/apiConfig.js'

const latestList = ref([])
const loading = ref(false)
const currentMusic = ref(null)

// 获取最新音乐
const fetchLatest = async () => {
  loading.value = true
  try {
    const timestamp = Date.now()
    const response = await fetch(`${apiConfig.BASE_URL}/api/music/latest?limit=300&t=${timestamp}`)
    const data = await response.json()

    if (data.success && data.data) {
      latestList.value = data.data.map(item => ({
        ...item,
        coverUrl: `${apiConfig.BASE_URL}/api/music/cover/${item.id}`
      }))
      console.log('最新音乐加载成功:', latestList.value.length, '首')
    } else {
      console.error('获取最新音乐失败:', data.message)
      latestList.value = []
    }
  } catch (error) {
    console.error('最新音乐请求失败:', error)
    latestList.value = []
  } finally {
    loading.value = false
  }
}

// 播放音乐
const playMusic = (music) => {
  currentMusic.value = music
  localStorage.setItem('currentMusic', JSON.stringify(music))
  
  // 添加到播放列表
  window.dispatchEvent(new CustomEvent('add-to-playlist', { detail: music }))
  
  // 触发播放
  window.dispatchEvent(new CustomEvent('play-music', { detail: music }))
}

// 播放全部
const playAll = () => {
  if (latestList.value.length === 0) return
  
  currentMusic.value = latestList.value[0]
  localStorage.setItem('currentMusic', JSON.stringify(latestList.value[0]))
  
  // 设置播放列表
  localStorage.setItem('playlist', JSON.stringify(latestList.value))
  
  // 触发播放全部
  window.dispatchEvent(new CustomEvent('play-all', { detail: latestList.value }))
}

// 下载音乐
const downloadMusic = async (music) => {
  try {
    const downloadUrl = `${apiConfig.BASE_URL}/api/music/file/${music.id}`

    // 使用fetch获取文件数据
    const response = await fetch(downloadUrl)
    if (!response.ok) {
      throw new Error('下载失败')
    }

    // 将响应转换为blob
    const blob = await response.blob()

    // 创建blob URL
    const blobUrl = URL.createObjectURL(blob)

    // 创建下载链接
    const link = document.createElement('a')
    link.href = blobUrl
    link.download = `${music.title}-${music.artist}.mp3`
    document.body.appendChild(link)
    link.click()

    // 清理
    document.body.removeChild(link)
    URL.revokeObjectURL(blobUrl)
  } catch (error) {
    console.error('下载失败:', error)
    alert('下载失败，请重试')
  }
}

// 处理图片加载错误
const handleImageError = (event) => {
  event.target.src = '/api/defaultIcon'
}

// 格式化时间
const formatTime = (timestamp) => {
  if (!timestamp) return '未知时间'
  const date = new Date(timestamp)
  const now = new Date()
  const diff = now - date

  const seconds = Math.floor(diff / 1000)
  const minutes = Math.floor(seconds / 60)
  const hours = Math.floor(minutes / 60)
  const days = Math.floor(hours / 24)

  if (seconds < 60) return '刚刚'
  if (minutes < 60) return `${minutes}分钟前`
  if (hours < 24) return `${hours}小时前`
  if (days < 7) return `${days}天前`

  return date.toLocaleDateString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit'
  })
}

onMounted(() => {
  fetchLatest()
})
</script>

<style scoped>
.latest-view {
  height: 100%;
  overflow-y: auto;
  padding: 32px;
}

.latest-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 32px;
  background: var(--bg-card);
  border-radius: var(--radius-xl);
  padding: 24px 28px;
  box-shadow: var(--shadow-md);
  border: 1px solid var(--border-light);
}

.latest-title {
  margin: 0;
  font-size: 28px;
  font-weight: 700;
  color: var(--text-primary);
}

.play-all-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 20px;
  background: var(--gradient-primary);
  color: var(--text-white);
  border: none;
  border-radius: var(--radius-lg);
  font-size: 14px;
  font-weight: 600;
  cursor: pointer;
  transition: all var(--transition-normal);
  box-shadow: var(--shadow-sm);
}

.play-all-btn:hover {
  transform: translateY(-2px);
  box-shadow: var(--shadow-md);
}

.loading {
  text-align: center;
  color: var(--text-secondary);
  padding: 60px;
  font-size: 14px;
}

.latest-list {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.latest-item {
  display: flex;
  align-items: center;
  padding: 16px 20px;
  background: var(--bg-card);
  border-radius: var(--radius-lg);
  box-shadow: var(--shadow-sm);
  border: 1px solid var(--border-light);
  transition: all var(--transition-normal);
}

.latest-item:hover {
  background: var(--bg-card-hover);
  transform: translateX(4px);
  box-shadow: var(--shadow-md);
}

.latest-number {
  font-size: 20px;
  font-weight: 700;
  width: 40px;
  text-align: center;
  color: var(--text-secondary);
}

.latest-cover {
  width: 56px;
  height: 56px;
  border-radius: var(--radius-md);
  overflow: hidden;
  margin-right: 16px;
  flex-shrink: 0;
  background: var(--gradient-primary);
}

.cover-img {
  width: 100%;
  height: 100%;
  object-fit: cover;
}

.latest-info {
  flex: 1;
  min-width: 0;
}

.music-title {
  margin: 0 0 6px 0;
  font-size: 15px;
  font-weight: 600;
  color: var(--text-primary);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.music-artist {
  margin: 0;
  font-size: 13px;
  color: var(--text-secondary);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.latest-time {
  font-size: 12px;
  color: var(--text-tertiary);
  margin-right: 16px;
  flex-shrink: 0;
}

.latest-actions {
  display: flex;
  gap: 8px;
  flex-shrink: 0;
}

.action-btn {
  width: 36px;
  height: 36px;
  border-radius: var(--radius-md);
  border: none;
  background: var(--bg-secondary);
  color: var(--text-primary);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all var(--transition-normal);
}

.action-btn:hover {
  background: var(--gradient-primary);
  color: var(--text-white);
  transform: scale(1.1);
}

.no-content {
  text-align: center;
  color: var(--text-secondary);
  padding: 60px;
  font-size: 14px;
}

/* 滚动条样式 */
.latest-view::-webkit-scrollbar {
  width: 8px;
}

.latest-view::-webkit-scrollbar-track {
  background: transparent;
}

.latest-view::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 4px;
}

.latest-view::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}
</style>