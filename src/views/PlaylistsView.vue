<template>
  <div class="playlists-view">
    <div class="view-header">
      <h1 class="view-title">我的歌单</h1>
      <button class="create-btn" @click="showCreateDialog = true">
        <svg viewBox="0 0 24 24" width="20" height="20">
          <path fill="currentColor" d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
        </svg>
        <span>创建歌单</span>
      </button>
    </div>
    
    <div v-if="loading" class="loading">
      <div class="loading-spinner"></div>
      <p>加载中...</p>
    </div>
    
    <div v-else-if="playlists.length === 0" class="empty">
      <div class="empty-icon">
        <svg viewBox="0 0 24 24" width="64" height="64">
          <path fill="currentColor" d="M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4V7h4V3h-6z"/>
        </svg>
      </div>
      <p>暂无歌单</p>
      <button class="create-btn primary" @click="showCreateDialog = true">创建第一个歌单</button>
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
          
          <!-- 调试信息 -->
          <div style="font-size: 10px; color: #999; margin: 5px 0;">
            创建者: {{ playlist.creator ? playlist.creator.username : '无' }}
          </div>
          
          <div class="playlist-meta">
            <div class="creator-info" v-if="playlist.creator">
              <img :src="getAvatarUrl(playlist.creator.id)" alt="创建者头像" class="creator-avatar" @error="handleAvatarError" />
              <span class="creator-name">{{ playlist.creator.username }}</span>
            </div>
            <span class="update-time">{{ formatTime(playlist.updatedAt) }}</span>
          </div>
        </div>
        
        <div class="playlist-actions">
          <button class="action-btn" @click.stop="showEditDialog(playlist)" title="编辑">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M3 17.25V21h3.75L17.81 9.94l-3.75-3.75L3 17.25zM20.71 7.04c.39-.39.39-1.02 0-1.41l-2.34-2.34c-.39-.39-1.02-.39-1.41 0l-1.83 1.83 3.75 3.75 1.83-1.83z"/>
            </svg>
          </button>
          <button class="action-btn delete" @click.stop="showDeleteDialog(playlist)" title="删除">
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
        <h2 class="dialog-title">创建歌单</h2>
        <form @submit.prevent="createPlaylist">
          <div class="form-group">
            <label>歌单名称</label>
            <input v-model="newPlaylist.name" type="text" placeholder="请输入歌单名称" required maxlength="255" />
          </div>
          <div class="form-group">
            <label>歌单描述</label>
            <textarea v-model="newPlaylist.description" placeholder="请输入歌单描述（可选）" maxlength="500"></textarea>
          </div>
          <div class="dialog-actions">
            <button type="button" class="btn cancel" @click="showCreateDialog = false">取消</button>
            <button type="submit" class="btn primary">创建</button>
          </div>
        </form>
      </div>
    </div>
    
    <!-- 编辑歌单对话框 -->
    <div v-if="showEditDialogFlag" class="dialog-overlay" @click.self="showEditDialogFlag = false">
      <div class="dialog">
        <h2 class="dialog-title">编辑歌单</h2>
        <form @submit.prevent="updatePlaylist">
          <div class="form-group">
            <label>歌单名称</label>
            <input v-model="editingPlaylist.name" type="text" placeholder="请输入歌单名称" required maxlength="255" />
          </div>
          <div class="form-group">
            <label>歌单描述</label>
            <textarea v-model="editingPlaylist.description" placeholder="请输入歌单描述（可选）" maxlength="500"></textarea>
          </div>
          <div class="dialog-actions">
            <button type="button" class="btn cancel" @click="showEditDialogFlag = false">取消</button>
            <button type="submit" class="btn primary">保存</button>
          </div>
        </form>
      </div>
    </div>
    
    <!-- 删除确认对话框 -->
    <div v-if="showDeleteDialogFlag" class="dialog-overlay" @click.self="showDeleteDialogFlag = false">
      <div class="dialog">
        <h2 class="dialog-title">删除歌单</h2>
        <p class="delete-message">确定要删除歌单「{{ deletingPlaylist?.name }}」吗？此操作不可恢复。</p>
        <div class="dialog-actions">
          <button type="button" class="btn cancel" @click="showDeleteDialogFlag = false">取消</button>
          <button type="button" class="btn danger" @click="deletePlaylist">删除</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { useRouter } from 'vue-router'
import apiConfig from '../config/apiConfig'

const router = useRouter()
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
  if (!token) {
    playlists.value = []
    return
  }

  loading.value = true
  try {
    const response = await fetch(`${apiConfig.BASE_URL}${apiConfig.PLAYLISTS}`, {
      method: 'GET',
      headers: { 'Authorization': token }
    })

    const data = await response.json()
    console.log('获取歌单列表响应:', data)
    if (data.success && data.playlists) {
      console.log('歌单数据:', data.playlists)
      playlists.value = data.playlists
    }
  } catch (error) {
    console.error('获取歌单列表失败:', error)
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
  
  if (diff < minute) return '刚刚'
  if (diff < hour) return `${Math.floor(diff / minute)}分钟前`
  if (diff < day) return `${Math.floor(diff / hour)}小时前`
  if (diff < month) return `${Math.floor(diff / day)}天前`
  if (diff < year) return `${Math.floor(diff / month)}个月前`
  return `${Math.floor(diff / year)}年前`
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

onMounted(() => {
  fetchPlaylists()
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
  background: white;
  color: var(--primary);
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
  background: white;
  border-radius: 12px;
  padding: 24px;
  width: 90%;
  max-width: 400px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.2);
}

.dialog-title {
  margin: 0 0 20px;
  font-size: 20px;
  font-weight: 600;
  color: var(--text-primary);
}

.form-group {
  margin-bottom: 16px;
}

.form-group label {
  display: block;
  margin-bottom: 8px;
  font-size: 14px;
  font-weight: 500;
  color: var(--text-primary);
}

.form-group input,
.form-group textarea {
  width: 100%;
  padding: 10px 12px;
  border: 1px solid var(--border-color);
  border-radius: 8px;
  font-size: 14px;
  color: var(--text-primary);
  outline: none;
  transition: border-color 0.2s ease;
  box-sizing: border-box;
}

.form-group input:focus,
.form-group textarea:focus {
  border-color: var(--primary);
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