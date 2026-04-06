<template>
  <div class="playlists-view">
    <div class="view-header">
      <h1 class="view-title">{{ t('key.myPlaylists') }}</h1>
      <button class="create-btn" @click="showCreateDialog = true">
        <svg viewBox="0 0 24 24" width="20" height="20">
          <path fill="currentColor" d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
        </svg>
        <span>{{ t('key.createPlaylist') }}</span>
      </button>
    </div>
    
    <div v-if="loading" class="loading">
      <div class="loading-spinner"></div>
      <p>{{ t('key.loading') }}</p>
    </div>
    
    <div v-else-if="key.length === 0" class="empty">
      <div class="empty-icon">
        <svg viewBox="0 0 24 24" width="64" height="64">
          <path fill="currentColor" d="M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4V7h4V3h-6z"/>
        </svg>
      </div>
      <p>{{ t('key.noPlaylists') }}</p>
      <button class="create-btn primary" @click="showCreateDialog = true">{{ t('key.createFirstPlaylist') }}</button>
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
          <p class="playlist-description">{{ playlist.description || t('key.description') }}</p>
          
          <!-- 调试信息 -->
          <div style="font-size: 10px; color: #999; margin: 5px 0;">
            {{ t('key.creator') }}: {{ playlist.creator ? playlist.creator.username : t('key.none') }}
          </div>
          
          <div class="playlist-meta">
            <div class="creator-info" v-if="playlist.creator">
              <img :src="getAvatarUrl(playlist.creator.id)" :alt="t('key.creator')" class="creator-avatar" @error="handleAvatarError" />
              <span class="creator-name">{{ playlist.creator.username }}</span>
            </div>
            <span class="update-time">{{ formatTime(playlist.updatedAt) }}</span>
          </div>
        </div>
        
        <div class="playlist-actions">
          <button class="action-btn" @click.stop="showEditDialog(playlist)" :title="t('key.edit')">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M3 17.25V21h3.75L17.81 9.94l-3.75-3.75L3 17.25zM20.71 7.04c.39-.39.39-1.02 0-1.41l-2.34-2.34c-.39-.39-1.02-.39-1.41 0l-1.83 1.83 3.75 3.75 1.83-1.83z"/>
            </svg>
          </button>
          <button class="action-btn delete" @click.stop="showDeleteDialog(playlist)" :title="t('key.delete')">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM19 4h-3.5l-1-1h-5l-1 1H5v2h14V4z"/>
            </svg>
          </button>
        </div>
      </div>
    </div>
    
    <!-- 创建歌单对话框 -->
    <div v-if="showCreateDialog" class="dialog-overlay" @click.self="showCreateDialog = false">
      <div class="dialog">
        <h2 class="dialog-title">{{ t('key.createPlaylist') }}</h2>
        <form @submit.prevent="createPlaylist">
          <div class="form-group">
            <label>{{ t('key.playlistName') }}</label>
            <input v-model="newPlaylist.name" type="text" :placeholder="t('key.playlistNamePlaceholder')" required maxlength="255" />
          </div>
          <div class="form-group">
            <label>{{ t('key.playlistDescription') }}</label>
            <textarea v-model="newPlaylist.description" :placeholder="t('key.playlistDescPlaceholder')" maxlength="500"></textarea>
          </div>
          <div class="dialog-actions">
            <button type="button" class="btn cancel" @click="showCreateDialog = false">{{ t('key.cancel') }}</button>
            <button type="submit" class="btn primary">{{ t('key.create') }}</button>
          </div>
        </form>
      </div>
    </div>
    
    <!-- 编辑歌单对话框 -->
    <div v-if="showEditDialogFlag" class="dialog-overlay" @click.self="showEditDialogFlag = false">
      <div class="dialog">
        <h2 class="dialog-title">{{ t('key.editPlaylist') }}</h2>
        <form @submit.prevent="updatePlaylist">
          <div class="form-group">
            <label>{{ t('key.playlistName') }}</label>
            <input v-model="editingPlaylist.name" type="text" :placeholder="t('key.playlistNamePlaceholder')" required maxlength="255" />
          </div>
          <div class="form-group">
            <label>{{ t('key.playlistDescription') }}</label>
            <textarea v-model="editingPlaylist.description" :placeholder="t('key.playlistDescPlaceholder')" maxlength="500"></textarea>
          </div>
          <div class="dialog-actions">
            <button type="button" class="btn cancel" @click="showEditDialogFlag = false">{{ t('key.cancel') }}</button>
            <button type="submit" class="btn primary">{{ t('key.save') }}</button>
          </div>
        </form>
      </div>
    </div>
    
    <!-- 删除确认对话框 -->
    <div v-if="showDeleteDialogFlag" class="dialog-overlay" @click.self="showDeleteDialogFlag = false">
      <div class="dialog">
        <h2 class="dialog-title">{{ t('key.deletePlaylist') }}</h2>
        <p class="delete-message">{{ t('key.deleteConfirm') }}</p>
        <div class="dialog-actions">
          <button type="button" class="btn cancel" @click="showDeleteDialogFlag = false">{{ t('key.cancel') }}</button>
          <button type="button" class="btn danger" @click="deletePlaylist">{{ t('key.delete') }}</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'
import { useI18n } from 'vue-i18n'
import apiConfig from '../config/apiConfig'

const router = useRouter()
const { t } = useI18n()
const playlists = ref([])
const loading = ref(false)
const showCreateDialog = ref(false)
const showEditDialogFlag = ref(false)
const showDeleteDialogFlag = ref(false)
const newPlaylist = ref({ name: '', description: '' })
const editingPlaylist = ref({ id: null, name: '', description: '' })
const deletingPlaylist = ref(null)

const fetchPlaylists = async () => {
  const token = localStorage.getItem('token')
  console.log('[PlaylistsView] fetchPlaylists 开始执行，token:', token ? '存在' : '不存在')
  
  if (!token) {
    playlists.value = []
    console.log('[PlaylistsView] 未登录，清空歌单列表')
    return
  }

  loading.value = true
  try {
    // 添加时间戳参数防止浏览器缓存
    const url = `${apiConfig.BASE_URL}${apiConfig.PLAYLISTS}?t=${Date.now()}`
    console.log('[PlaylistsView] 请求URL:', url)
    
    const response = await fetch(url, {
      method: 'GET',
      headers: { 'Authorization': token }
    })

    const data = await response.json()
    console.log('[PlaylistsView] 获取歌单列表响应:', data)
    
    if (data.success && data.playlists) {
      console.log('[PlaylistsView] 歌单数据:', data.playlists)
      playlists.value = data.playlists
      console.log('[PlaylistsView] 更新后的歌单列表长度:', playlists.value.length)
    } else {
      console.log('[PlaylistsView] 响应数据无效:', data)
    }
  } catch (error) {
    console.error('[PlaylistsView] 获取歌单列表失败:', error)
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
  
  if (diff < minute) return t('key.justNow')
  if (diff < hour) return `${Math.floor(diff / minute)}${t('key.minutesAgo')}`
  if (diff < day) return `${Math.floor(diff / hour)}${t('key.hoursAgo')}`
  if (diff < month) return `${Math.floor(diff / day)}${t('key.daysAgo')}`
  if (diff < year) return `${Math.floor(diff / month)}${t('key.monthsAgo')}`
  return `${Math.floor(diff / year)}${t('key.yearsAgo')}`
}

const goToPlaylist = (id) => {
  router.push(`/playlist/${id}`)
}

const createPlaylist = async () => {
  const token = localStorage.getItem('token')
  if (!token) return

  try {
    const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.PLAYLIST_CREATE}`, {
      method: 'POST',
      headers: {
        'Authorization': token,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(newPlaylist.value)
    })

    const data = await response.json()
    if (data.success) {
      showCreateDialog.value = false
      newPlaylist.value = { name: '', description: '' }
      fetchPlaylists()
    }
  } catch (error) {
    console.error('创建歌单失败:', error)
  }
}

const showEditDialog = (playlist) => {
  editingPlaylist.value = { ...playlist }
  showEditDialogFlag.value = true
}

const updatePlaylist = async () => {
  const token = localStorage.getItem('token')
  if (!token) return

  try {
    const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.PLAYLIST_UPDATE}`, {
      method: 'POST',
      headers: {
        'Authorization': token,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        id: editingPlaylist.value.id,
        name: editingPlaylist.value.name,
        description: editingPlaylist.value.description
      })
    })

    const data = await response.json()
    if (data.success) {
      showEditDialogFlag.value = false
      fetchPlaylists()
    }
  } catch (error) {
    console.error('更新歌单失败:', error)
  }
}

const showDeleteDialog = (playlist) => {
  deletingPlaylist.value = playlist
  showDeleteDialogFlag.value = true
}

const deletePlaylist = async () => {
  const token = localStorage.getItem('token')
  if (!token) return

  try {
    const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.PLAYLIST_DELETE}`, {
      method: 'POST',
      headers: {
        'Authorization': token,
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ id: deletingPlaylist.value.id })
    })

    const data = await response.json()
    if (data.success) {
      showDeleteDialogFlag.value = false
      deletingPlaylist.value = null
      fetchPlaylists()
    }
  } catch (error) {
    console.error('删除歌单失败:', error)
  }
}

// 处理用户登录事件
const handleUserLogin = () => {
  console.log('[PlaylistsView] 收到 user-login 事件，开始重新加载歌单')
  fetchPlaylists()
}

// 处理刷新请求事件
const handleRefreshNeeded = () => {
  console.log('[PlaylistsView] 收到 playlist-refresh-needed 事件，开始重新加载歌单')
  fetchPlaylists()
}

// 处理用户登出事件
const handleUserLogout = () => {
  playlists.value = []
}

onMounted(() => {
  console.log('[PlaylistsView] 组件已挂载')
  
  // 检查是否有 token
  const token = localStorage.getItem('token')
  if (token) {
    // 检查是否需要刷新（通过 localStorage 标志）
    const loginTimestamp = localStorage.getItem('loginTimestamp')
    const componentMountedTime = Date.now()
    
    // 如果登录时间与当前时间相差小于 5 秒，说明刚登录，需要刷新
    if (loginTimestamp && (componentMountedTime - parseInt(loginTimestamp)) < 5000) {
      console.log('[PlaylistsView] 检测到刚登录，强制刷新歌单')
      fetchPlaylists()
    } else {
      fetchPlaylists()
    }
  }
  
  // 监听账号变更事件
  window.addEventListener('user-login', handleUserLogin)
  window.addEventListener('user-logout', handleUserLogout)
  // 监听全局刷新事件
  window.addEventListener('playlist-refresh-needed', handleRefreshNeeded)
  console.log('[PlaylistsView] 事件监听器已注册')
})

// 组件卸载时清理事件监听器
onUnmounted(() => {
  window.removeEventListener('user-login', handleUserLogin)
  window.removeEventListener('user-logout', handleUserLogout)
  window.removeEventListener('playlist-refresh-needed', handleRefreshNeeded)
})
</script>

<style scoped>
.playlists-view {
  height: 100%;
  overflow-y: auto;
  padding: 24px;
}

.view-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
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

.create-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 10px 20px;
  font-size: 14px;
  font-weight: 500;
  color: var(--primary);
  background: rgba(102, 126, 234, 0.1);
  border: 1px solid rgba(102, 126, 234, 0.2);
  border-radius: 8px;
  cursor: pointer;
  transition: all 0.2s ease;
}

.create-btn:hover {
  background: rgba(102, 126, 234, 0.2);
  border-color: rgba(102, 126, 234, 0.3);
  transform: translateY(-1px);
}

.create-btn.primary {
  margin-top: 16px;
  padding: 12px 24px;
  font-size: 15px;
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

.playlists-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
  gap: 20px;
}

.playlist-card {
  background: rgba(30, 30, 50, 0.9);
  border-radius: 12px;
  overflow: hidden;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
  transition: all 0.3s ease;
  cursor: pointer;
  position: relative;
  border: 1px solid rgba(255, 255, 255, 0.1);
}

.playlist-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.25);
  border-color: rgba(102, 126, 234, 0.3);
}

.playlist-cover {
  position: relative;
  aspect-ratio: 1;
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.1) 0%, rgba(118, 75, 162, 0.1) 100%);
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

.update-time {
  font-size: 12px;
  color: var(--text-muted);
}

.playlist-actions {
  position: absolute;
  top: 10px;
  right: 10px;
  display: flex;
  gap: 8px;
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
  background: rgba(255, 255, 255, 0.2);
  color: #667eea;
  transform: scale(1.1);
}

.action-btn.delete:hover {
  color: #e91e63;
}

.dialog-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.dialog {
  background: rgba(30, 30, 50, 0.98);
  border-radius: 12px;
  padding: 24px;
  width: 90%;
  max-width: 400px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(255, 255, 255, 0.1);
}

.dialog-title {
  margin: 0 0 20px;
  font-size: 20px;
  font-weight: 600;
  color: white;
}

.form-group {
  margin-bottom: 16px;
}

.form-group label {
  display: block;
  margin-bottom: 8px;
  font-size: 14px;
  font-weight: 500;
  color: rgba(255, 255, 255, 0.9);
}

.form-group input,
.form-group textarea {
  width: 100%;
  padding: 10px 12px;
  border: 1px solid rgba(255, 255, 255, 0.15);
  border-radius: 8px;
  font-size: 14px;
  color: white;
  background: rgba(0, 0, 0, 0.2);
  outline: none;
  transition: border-color 0.2s ease;
  box-sizing: border-box;
}

.form-group input:focus,
.form-group textarea:focus {
  border-color: #667eea;
  background: rgba(0, 0, 0, 0.3);
}

.form-group textarea {
  min-height: 80px;
  resize: vertical;
}

.dialog-actions {
  display: flex;
  gap: 12px;
  justify-content: flex-end;
  margin-top: 24px;
}

.btn {
  padding: 10px 20px;
  border: none;
  border-radius: 8px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
}

.btn.cancel {
  background: #f5f5f5;
  color: var(--text-secondary);
}

.btn.cancel:hover {
  background: #e0e0e0;
}

.btn.primary {
  background: var(--primary);
  color: white;
}

.btn.primary:hover {
  background: #5a7bd6;
}

.btn.danger {
  background: #e91e63;
  color: white;
}

.btn.danger:hover {
  background: #d81b60;
}

.delete-message {
  color: var(--text-secondary);
  margin: 0 0 20px;
  line-height: 1.6;
}

/* 滚动条样式 */
.playlists-view::-webkit-scrollbar {
  width: 8px;
}

.playlists-view::-webkit-scrollbar-track {
  background: transparent;
}

.playlists-view::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 4px;
}

.playlists-view::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}
</style>