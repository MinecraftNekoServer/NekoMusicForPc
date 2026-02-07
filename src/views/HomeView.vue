<template>
  <div class="home-view">
    <div class="welcome-section">
      <h1 class="welcome-title">
        <span class="greeting">{{ greeting }}</span>
      </h1>
      <p class="welcome-subtitle">发现好音乐，享受每一刻</p>
    </div>

    <!-- 推荐歌单 -->
    <div class="recommendation-section">
      <h2 class="section-title">推荐歌单</h2>
      <div v-if="playlistsLoading" class="loading">加载中...</div>
      <div v-else class="playlist-scroll">
        <!-- 热门音乐整体卡片 -->
        <div
          class="playlist-item hot-music-card"
          @click="goToRanking"
        >
          <div class="playlist-cover hot-music-cover">
            <div class="hot-music-mosaic">
              <img
                v-for="(item, index) in hotMusicList.slice(0, 4)"
                :key="index"
                :src="item.coverUrl"
                :alt="item.title"
                class="mosaic-img"
                @error="handleImageError"
              />
            </div>
            <div class="playlist-count hot-music-count">{{ hotMusicCount || 0 }}首</div>
          </div>
          <div class="playlist-info">
            <h3 class="playlist-name">热门音乐</h3>
            <p class="playlist-description">播放次数最高的热门歌曲</p>
          </div>
        </div>

        <!-- 歌单卡片 -->
        <div
          v-for="playlist in playlistList"
          :key="playlist.id"
          class="playlist-item"
          @click="goToPlaylist(playlist.id)"
        >
          <div class="playlist-cover">
            <img
              :src="playlist.firstMusicCover"
              :alt="playlist.name"
              class="playlist-cover-img"
              @error="handlePlaylistCoverError"
            />
            <div class="playlist-count">{{ playlist.musicCount }}首</div>
          </div>
          <div class="playlist-info">
            <h3 class="playlist-name">{{ playlist.name }}</h3>
            <p class="playlist-description">{{ playlist.description || '暂无描述' }}</p>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed, ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import apiConfig from '@/config/apiConfig.js'
const router = useRouter()

const greeting = computed(() => {
  const hour = new Date().getHours()
  if (hour < 6) return '夜深了'
  if (hour < 12) return '早上好'
  if (hour < 18) return '下午好'
  return '晚上好'
})

const playlistList = ref([])
const playlistsLoading = ref(false)
const hotMusicList = ref([])
const hotMusicCount = ref(0)

// 获取推荐歌单
const fetchPlaylists = async () => {
  playlistsLoading.value = true
  try {
    const response = await fetch(`${apiConfig.BASE_URL}/api/playlists/search`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        query: ''
      })
    })
    const data = await response.json()

    if (data.success && data.results) {
      // 为歌单封面添加正确的URL
      playlistList.value = data.results.slice(0, 8).map(playlist => ({
        ...playlist,
        firstMusicCover: playlist.firstMusicCover
          ? (playlist.firstMusicCover.startsWith('http') || playlist.firstMusicCover.startsWith('/api'))
            ? playlist.firstMusicCover
            : `${apiConfig.BASE_URL}${playlist.firstMusicCover}`
          : '/api/defaultIcon'
      }))
      console.log('歌单加载成功:', data.results.length, '个')
    } else {
      console.error('获取歌单失败:', data.message)
      playlistList.value = []
    }
  } catch (error) {
    console.error('歌单请求失败:', error)
    playlistList.value = []
  } finally {
    playlistsLoading.value = false
  }
}

// 获取热门音乐（用于拼接封面）
const fetchHotMusic = async () => {
  try {
    const response = await fetch(`${apiConfig.BASE_URL}/api/music/ranking`)
    const data = await response.json()

    if (data.success && data.data && data.data.length > 0) {
      // 保存完整的音乐数量
      hotMusicCount.value = data.data.length
      // 只取前4首用于拼接封面
      hotMusicList.value = data.data.slice(0, 4).map(item => ({
        ...item,
        coverUrl: `${apiConfig.BASE_URL}/api/music/cover/${item.id}`
      }))
      console.log('热门音乐加载成功:', hotMusicCount.value, '首')
    } else {
      console.log('没有热门音乐数据')
      hotMusicList.value = []
      hotMusicCount.value = 0
    }
  } catch (error) {
    console.error('获取热门音乐失败:', error)
    hotMusicList.value = []
    hotMusicCount.value = 0
  }
}

// 跳转到歌单详情
const goToPlaylist = (playlistId) => {
  router.push(`/playlist/${playlistId}`)
}

// 跳转到排行榜
const goToRanking = () => {
  router.push('/ranking')
}

// 处理图片加载错误
const handleImageError = (event) => {
  event.target.src = '/api/defaultIcon'
}

// 处理歌单封面加载错误
const handlePlaylistCoverError = (event) => {
  event.target.src = '/api/defaultIcon'
}

onMounted(() => {
  fetchPlaylists()
  fetchHotMusic()
})
</script>

<style scoped>
.home-view {
  height: 100%;
  overflow-y: auto;
  padding: 32px;
  animation: fadeIn 0.5s ease;
}

@keyframes fadeIn {
  from {
    opacity: 0;
    transform: translateY(20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.welcome-section {
  text-align: center;
  margin-bottom: 40px;
  padding: 40px 20px;
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.1) 0%, rgba(118, 75, 162, 0.1) 100%);
  border-radius: var(--radius-xl);
  position: relative;
  overflow: hidden;
}

.welcome-section::before {
  content: '';
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: radial-gradient(circle, rgba(102, 126, 234, 0.1) 0%, transparent 50%);
  animation: rotate 20s linear infinite;
}

@keyframes rotate {
  from { transform: rotate(0deg); }
  to { transform: rotate(360deg); }
}

.welcome-title {
  margin: 0;
  font-size: 42px;
  font-weight: 800;
  color: var(--text-primary);
  position: relative;
  z-index: 1;
}

.greeting {
  background: var(--gradient-primary);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

@keyframes bounce {
  0%, 100% { transform: translateY(0); }
  50% { transform: translateY(-10px); }
}

.welcome-subtitle {
  margin-top: 12px;
  font-size: 16px;
  color: var(--text-secondary);
  position: relative;
  z-index: 1;
}

/* 推荐歌单样式 */
.recommendation-section {
  background: var(--bg-card);
  border-radius: var(--radius-xl);
  padding: 28px;
  box-shadow: var(--shadow-md);
  border: 1px solid var(--border-light);
  backdrop-filter: blur(10px);
  -webkit-backdrop-filter: blur(10px);
}

.section-title {
  font-size: 22px;
  font-weight: 700;
  color: var(--text-primary);
  margin: 0 0 24px 0;
}

.loading {
  text-align: center;
  color: var(--text-secondary);
  padding: 40px;
  font-size: 14px;
}

.playlist-scroll {
  display: flex;
  gap: 20px;
  overflow-x: auto;
  padding: 0 0 12px 0;
  scroll-behavior: smooth;
}

.playlist-scroll::-webkit-scrollbar {
  height: 6px;
}

.playlist-scroll::-webkit-scrollbar-track {
  background: transparent;
  border-radius: 3px;
}

.playlist-scroll::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 3px;
}

.playlist-scroll::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}

.playlist-item {
  flex-shrink: 0;
  width: 204px;
  cursor: pointer;
  transition: all var(--transition-normal);
}

.playlist-item:hover {
  transform: translateY(-6px);
}

.playlist-cover {
  position: relative;
  width: 180px;
  height: 180px;
  border-radius: var(--radius-lg);
  overflow: hidden;
  margin: 0 auto;
  box-shadow: var(--shadow-md);
  background: var(--gradient-primary);
}

.playlist-cover-img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  transition: transform var(--transition-normal);
}

.playlist-item:hover .playlist-cover-img {
  transform: scale(1.05);
}

.playlist-count {
  position: absolute;
  bottom: 0;
  left: 0;
  right: 0;
  background: linear-gradient(to top, rgba(0, 0, 0, 0.8), transparent);
  color: var(--text-white);
  padding: 20px 10px 8px;
  font-size: 12px;
  font-weight: 600;
}

/* 热门音乐卡片样式 */
.hot-music-card {
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.05) 0%, rgba(118, 75, 162, 0.05) 100%);
  border-radius: var(--radius-lg);
  padding: 12px;
  transition: all var(--transition-normal);
}

.hot-music-card:hover {
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.1) 0%, rgba(118, 75, 162, 0.1) 100%);
}

.hot-music-cover {
  background: var(--gradient-primary);
  padding: 3px;
}

.hot-music-mosaic {
  display: grid;
  grid-template-columns: 1fr 1fr;
  grid-template-rows: 1fr 1fr;
  gap: 3px;
  width: 100%;
  height: 100%;
  overflow: hidden;
  border-radius: 8px;
}

.mosaic-img {
  width: 100%;
  height: 100%;
  object-fit: cover;
  border-radius: 3px;
}

.hot-music-count {
  background: rgba(102, 126, 234, 0.9);
  color: var(--text-white);
  font-weight: 700;
}

.playlist-info {
  padding: 12px 4px 0 4px;
}

.playlist-name {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
  margin: 0 0 6px 0;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.playlist-description {
  font-size: 12px;
  color: var(--text-secondary);
  margin: 0;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.no-content {
  text-align: center;
  color: var(--text-secondary);
  padding: 40px;
  font-size: 14px;
}

/* 滚动条样式 */
.home-view::-webkit-scrollbar {
  width: 8px;
}

.home-view::-webkit-scrollbar-track {
  background: transparent;
}

.home-view::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 4px;
}

.home-view::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}
</style>