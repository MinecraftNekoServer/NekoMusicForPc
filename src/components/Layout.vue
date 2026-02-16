<template>
  <div class="layout">
    <div class="layout-sidebar glass-dark">
      <div class="sidebar-header">
        <div class="logo-wrapper">
          <img src="/icon.png" alt="Logo" class="app-logo animate-pulse" />
          <div class="logo-glow"></div>
        </div>
        <span class="app-name text-gradient">Neko云音乐</span>
      </div>
      <nav class="sidebar-nav">
        <div 
          v-for="item in navItems" 
          :key="item.key || item.divider"
        >
          <div v-if="item.divider" class="nav-divider"></div>
          <div 
            v-else
            :class="['nav-item', { active: currentRoute === item.key }]"
            @click="navigateTo(item.key)"
          >
            <div class="nav-icon-wrapper">
              <svg class="nav-icon" viewBox="0 0 24 24">
                <path :d="item.icon" fill="currentColor"/>
              </svg>
              <div class="nav-icon-glow" v-if="currentRoute === item.key"></div>
            </div>
            <span>{{ item.label }}</span>
            <div class="nav-indicator" v-if="currentRoute === item.key"></div>
          </div>
        </div>
        
        <div class="sidebar-playlists">
        <div class="playlists-header">
          <span class="playlists-title">我的歌单</span>
          <button class="add-playlist-btn" @click="showCreatePlaylistModal = true" title="创建歌单">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M19 13h-6v6h-2v-6H5v-2h6V5h2v6h6v2z"/>
            </svg>
          </button>
        </div>
        <div class="playlists-list">
          <div 
            v-for="playlist in myPlaylists" 
            :key="playlist.id"
            :class="['playlist-item', { active: currentRoute === 'playlist' && currentPlaylistId === playlist.id }]"
            @click="openPlaylist(playlist.id)"
            @contextmenu.prevent="showPlaylistContextMenu($event, playlist)"
          >
            <img :src="getPlaylistCover(playlist)" alt="封面" class="playlist-cover" />
            <span class="playlist-name">{{ playlist.name }}</span>
          </div>
          <div v-if="myPlaylists.length === 0" class="playlists-empty">
            <span>暂无歌单</span>
          </div>
        </div>
      </div>

      <!-- 收藏的歌单 -->
      <div v-if="favoritePlaylists.length > 0" class="sidebar-playlists">
        <div class="nav-divider"></div>
        <div class="playlists-header">
          <span class="playlists-title">收藏的歌单</span>
        </div>
        <div class="playlists-list">
          <div 
            v-for="playlist in favoritePlaylists" 
            :key="`fav-${playlist.id}`"
            :class="['playlist-item', { active: currentRoute === 'playlist' && currentPlaylistId === playlist.id }]"
            @click="openPlaylist(playlist.id)"
          >
            <img :src="getPlaylistCover(playlist)" alt="封面" class="playlist-cover" />
            <span class="playlist-name">{{ playlist.name }}</span>
          </div>
        </div>
      </div>
      </nav>
      
      <div class="sidebar-footer">
        <div class="user-card" @click="handleUserClick">
          <div class="user-avatar-wrapper">
            <img :src="userAvatar" alt="用户头像" class="user-avatar" />
            <div class="user-avatar-ring"></div>
          </div>
          <div class="user-details">
            <span class="username">{{ username || '未登录' }}</span>
            <span class="user-status">{{ isLoggedIn ? '已登录' : '点击登录' }}</span>
          </div>
          <svg class="user-arrow" viewBox="0 0 24 24" width="16" height="16">
            <path fill="currentColor" d="M8.59 16.59L13.17 12 8.59 7.41 10 6l6 6-6 6-1.41-1.41z"/>
          </svg>
        </div>
      </div>
    </div>
    <div class="layout-main">
      <div class="title-bar glass">
        <div class="title-bar-center">
          <div class="search-box">
            <svg class="search-icon" viewBox="0 0 20 20">
              <path fill="currentColor" d="M8 3a5 5 0 100 10A5 5 0 008 3zM0 8a8 8 0 1114.32 4.906l5.387 5.387a1 1 0 01-1.414 1.414l-5.387-5.387A8 8 0 010 8z"/>
            </svg>
            <input 
              v-model="searchQuery" 
              type="text" 
              placeholder="搜索音乐、艺术家..." 
              @keyup.enter="handleSearch"
            />
            <button class="search-btn" @click="handleSearch" title="搜索">
              <svg viewBox="0 0 24 24" width="18" height="18">
                <path fill="currentColor" d="M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z"/>
              </svg>
            </button>
          </div>
        </div>
        <div class="title-bar-right">
          <button class="action-btn" @click="navigateTo('settings')" title="设置">
            <svg viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54c-.04-.24-.24-.41-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58c-.05.3-.09.63-.09.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z"/>
            </svg>
          </button>
          <div class="window-controls">
            <button class="window-btn" @click="minimize" title="最小化">
              <svg viewBox="0 0 12 12" width="12" height="12">
                <path fill="currentColor" d="M2 5h8v1H2V5z"/>
              </svg>
            </button>
            <button class="window-btn" @click="maximize" title="最大化">
              <svg viewBox="0 0 12 12" width="12" height="12">
                <path fill="none" stroke="currentColor" stroke-width="1" d="M2 2h8v8H2z"/>
              </svg>
            </button>
            <button class="window-btn close-btn" @click="close" title="关闭">
              <svg viewBox="0 0 12 12" width="12" height="12">
                <path fill="currentColor" d="M2 2l8 8M10 2l-8 8" stroke="currentColor" stroke-width="1.5" stroke-linecap="round"/>
              </svg>
            </button>
          </div>
        </div>
      </div>
      <div class="main-content">
        <router-view />
      </div>
      <PlayerBar />
    </div>

    <!-- Toast 通知 -->
    <div class="toast-container">
      <TransitionGroup name="toast">
        <div 
          v-for="toast in toasts" 
          :key="toast.id"
          :class="['toast', `toast-${toast.type}`]"
        >
          <div class="toast-icon">
            <svg v-if="toast.type === 'success'" viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41z"/>
            </svg>
            <svg v-else-if="toast.type === 'error'" viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z"/>
            </svg>
            <svg v-else viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z"/>
            </svg>
          </div>
          <div class="toast-content">
            <div class="toast-message">{{ toast.message }}</div>
          </div>
        </div>
      </TransitionGroup>
    </div>

    <Transition name="modal">
      <div v-if="showLoginModal" class="modal-overlay" @click="showLoginModal = false">
        <div class="modal-content" @click.stop>
          <div class="modal-header">
            <div class="modal-logo">
              <img src="/icon.png" alt="Logo" />
            </div>
            <Transition name="title-fade" mode="out-in">
              <h2 :key="authTab" class="modal-title">{{ authTab === 'login' ? '欢迎回来' : '创建账号' }}</h2>
            </Transition>
            <Transition name="subtitle-fade" mode="out-in">

            </Transition>
          </div>
          
          <div class="modal-tabs">
            <button 
              :class="['tab-btn', { active: authTab === 'login' }]"
              @click="authTab = 'login'"
            >
              登录
            </button>
            <button 
              :class="['tab-btn', { active: authTab === 'register' }]"
              @click="authTab = 'register'"
            >
              注册
            </button>
          </div>
          
          <Transition name="form-slide" mode="out-in">
            <div :key="authTab" class="auth-form">
              <input 
                v-model="formData.username"
                type="text" 
                placeholder="用户名（昵称）"
                class="auth-input"
              />
              <input 
                v-model="formData.password"
                type="password" 
                placeholder="密码"
                class="auth-input"
              />
              <Transition name="field-fade">
                <div v-if="authTab === 'register'" class="email-field">
                  <input 
                    v-model="formData.email"
                    type="email" 
                    placeholder="邮箱"
                    class="auth-input"
                  />
                  <div class="verification-code">
                    <input 
                      v-model="formData.verificationCode"
                      type="text" 
                      placeholder="验证码"
                      class="auth-input"
                    />
                    <button 
                      class="send-code-btn"
                      @click="sendVerificationCode"
                      :disabled="codeSending || countdown > 0"
                    >
                      {{ codeBtnText }}
                    </button>
                  </div>
                </div>
              </Transition>
            </div>
          </Transition>

          <Transition name="button-fade" mode="out-in">
            <div :key="authTab" class="modal-buttons">
              <button class="modal-btn modal-btn-primary" @click="handleSubmit">
                {{ authTab === 'login' ? '立即登录' : '创建账号' }}
              </button>
            </div>
          </Transition>
          
          <p v-if="errorMessage" class="error-message">{{ errorMessage }}</p>
        </div>
      </div>
    </Transition>

    <Transition name="modal">
      <div v-if="showCreatePlaylistModal" class="modal-overlay" @click="showCreatePlaylistModal = false">
        <div class="modal-content modal-small" @click.stop>
          <h2 class="modal-title">创建歌单</h2>
          <input 
            v-model="newPlaylistName"
            type="text" 
            placeholder="输入歌单名称"
            class="auth-input"
            @keyup.enter="handleCreatePlaylist"
          />
          <div class="modal-buttons">
            <button class="modal-btn modal-btn-secondary" @click="showCreatePlaylistModal = false">取消</button>
            <button class="modal-btn modal-btn-primary" @click="handleCreatePlaylist">创建</button>
          </div>
        </div>
      </div>
    </Transition>

    <!-- 右键菜单 -->
    <Transition name="context-menu">
      <div 
        v-if="contextMenu.visible" 
        class="context-menu"
        :style="{ left: contextMenu.x + 'px', top: contextMenu.y + 'px' }"
        @click="contextMenu.visible = false"
      >
        <div class="context-menu-item" @click="handleRenamePlaylist">
          <svg viewBox="0 0 24 24" width="16" height="16">
            <path fill="currentColor" d="M3 17.25V21h3.75L17.81 9.94l-3.75-3.75L3 17.25zM20.71 7.04c.39-.39.39-1.02 0-1.41l-2.34-2.34c-.39-.39-1.02-.39-1.41 0l-1.83 1.83 3.75 3.75 1.83-1.83z"/>
          </svg>
          <span>重命名</span>
        </div>
        <div class="context-menu-item" @click="handleEditPlaylistDescription">
          <svg viewBox="0 0 24 24" width="16" height="16">
            <path fill="currentColor" d="M14.06 9.02l.92.92L5.92 19H5v-.92l9.06-9.06M17.66 3c-.25 0-.51.1-.7.29l-1.83 1.83 3.75 3.75 1.83-1.83c.39-.39.39-1.02 0-1.41l-2.34-2.34c-.2-.2-.45-.29-.71-.29zm-3.6 3.19L3 17.25V21h3.75L17.81 9.94l-3.75-3.75z"/>
          </svg>
          <span>修改描述</span>
        </div>
        <div class="context-menu-item danger" @click="handleDeletePlaylist">
          <svg viewBox="0 0 24 24" width="16" height="16">
            <path fill="currentColor" d="M6 19c0 1.1.9 2 2 2h8c1.1 0 2-.9 2-2V7H6v12zM19 4h-3.5l-1-1h-5l-1 1H5v2h14V4z"/>
          </svg>
          <span>删除歌单</span>
        </div>
      </div>
    </Transition>

    <!-- 编辑歌单模态框 -->
    <Transition name="modal">
      <div v-if="showEditPlaylistModal" class="modal-overlay" @click="showEditPlaylistModal = false">
        <div class="modal-content modal-small" @click.stop>
          <h2 class="modal-title">{{ editMode === 'name' ? '重命名歌单' : '修改歌单描述' }}</h2>
          <input 
            v-model="editPlaylistValue"
            type="text" 
            :placeholder="editMode === 'name' ? '输入新歌单名称' : '输入歌单描述'"
            class="auth-input"
            @keyup.enter="handleSavePlaylistEdit"
          />
          <div class="modal-buttons">
            <button class="modal-btn modal-btn-secondary" @click="showEditPlaylistModal = false">取消</button>
            <button class="modal-btn modal-btn-primary" @click="handleSavePlaylistEdit">保存</button>
          </div>
        </div>
      </div>
    </Transition>

    <!-- 确认对话框 -->
    <Transition name="modal">
      <div v-if="showConfirmDialog" class="modal-overlay" @click="showConfirmDialog = false">
        <div class="modal-content modal-small modal-confirm" @click.stop>
          <div class="confirm-icon-wrapper">
            <svg class="confirm-icon" viewBox="0 0 24 24" width="48" height="48">
              <path fill="currentColor" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-2h2v2zm0-4h-2V7h2v6z"/>
            </svg>
          </div>
          <h2 class="modal-title">{{ confirmDialog.title }}</h2>
          <p class="confirm-message">{{ confirmDialog.message }}</p>
          <div class="modal-buttons">
            <button class="modal-btn modal-btn-secondary" @click="showConfirmDialog = false">取消</button>
            <button class="modal-btn modal-btn-danger" @click="handleConfirmDialog">确认删除</button>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, computed, onMounted, watch } from 'vue'
import { useRouter, useRoute } from 'vue-router'
import PlayerBar from './PlayerBar.vue'
import apiConfig from '../config/apiConfig'

// 统一的 API 请求函数
async function apiRequest(url, options = {}) {
  const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
  return fetch(fullUrl, options)
}

const router = useRouter()
const route = useRoute()
const currentRoute = ref('home')
const searchQuery = ref('')
const username = ref('')
const currentUser = ref(null)
const showLoginModal = ref(false)
const showCreatePlaylistModal = ref(false)
const showEditPlaylistModal = ref(false)
const authTab = ref('login')
const errorMessage = ref('')
const newPlaylistName = ref('')

// 右键菜单
const contextMenu = ref({
  visible: false,
  x: 0,
  y: 0,
  playlist: null
})

// 编辑歌单
const editMode = ref('name') // 'name' 或 'description'
const editPlaylistValue = ref('')
const currentEditPlaylist = ref(null)

// 确认对话框
const showConfirmDialog = ref(false)
const confirmDialog = ref({
  title: '',
  message: '',
  onConfirm: null
})

const formData = ref({
  username: '',
  password: '',
  email: '',
  verificationCode: ''
})
const codeSending = ref(false)
const countdown = ref(0)
const countdownInterval = ref(null)
const toasts = ref([])
let toastId = 0

const codeBtnText = computed(() => {
  return countdown.value > 0 ? `${countdown.value}秒后重发` : '获取验证码'
})

const navItems = [
  { key: 'home', label: '首页', icon: 'M10 20v-6h4v6h5v-8h3L12 3 2 12h3v8z' },
  { divider: true },
  { key: 'favorites', label: '我喜欢的音乐', icon: 'M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5c0 3.78-3.4 6.86-8.55 11.54L12 21.35z' },
  { key: 'recent', label: '最近播放', icon: 'M12 2C6.5 2 2 6.5 2 12s4.5 10 10 10 10-4.5 10-10S17.5 2 12 2zm0 18c-4.41 0-8-3.59-8-8s3.59-8 8-8 8 3.59 8 8-3.59 8-8 8zm.5-13H11v6l5.25 3.15.75-1.23-4.5-2.67z' },
  { divider: true }
]

const myPlaylists = ref([])
const favoritePlaylists = ref([])
const currentPlaylistId = ref(null)

const loadMyPlaylists = async () => {

  const token = localStorage.getItem('token')

  if (!token) {

    myPlaylists.value = []

    return

  }

  try {

    // 添加时间戳参数禁用浏览器缓存
    const timestamp = Date.now()
    const response = await apiRequest(`${apiConfig.PLAYLISTS}?t=${timestamp}`, {

      method: 'GET',

      headers: { 'Authorization': token }

    })

    const data = await response.json()

    if (data.success && data.playlists) {

      const playlistsWithCovers = []

      for (const playlist of data.playlists) {

        try {

          // 添加时间戳参数禁用浏览器缓存
          const musicResponse = await apiRequest(`${apiConfig.PLAYLIST_MUSIC(playlist.id)}?t=${timestamp}`, {

            method: 'GET',

            headers: { 'Authorization': token }

          })

          const musicData = await musicResponse.json()

          if (musicData.success && musicData.musicList && musicData.musicList.length > 0) {

            playlistsWithCovers.push({

              ...playlist,

              firstMusicId: musicData.musicList[0].id

            })

          } else {

            playlistsWithCovers.push(playlist)

          }

        } catch (error) {

          playlistsWithCovers.push(playlist)

        }

      }

      myPlaylists.value = playlistsWithCovers

    }

  } catch (error) {

    console.error('加载我的歌单失败:', error)

  }

}

const loadFavoritePlaylists = async () => {
  const token = localStorage.getItem('token')
  if (!token) {
    favoritePlaylists.value = []
    return
  }

  try {
    const timestamp = Date.now()
    const response = await apiRequest(`${apiConfig.FAVORITE_PLAYLISTS}?t=${timestamp}`, {
      method: 'GET',
      headers: { 'Authorization': token }
    })
    const data = await response.json()
    if (data.success && data.playlists) {
      const playlistsWithCovers = []
      for (const playlist of data.playlists) {
        try {
          const musicResponse = await apiRequest(`${apiConfig.PLAYLIST_MUSIC(playlist.id)}?t=${timestamp}`, {
            method: 'GET',
            headers: { 'Authorization': token }
          })
          const musicData = await musicResponse.json()
          if (musicData.success && musicData.musicList && musicData.musicList.length > 0) {
            playlistsWithCovers.push({
              ...playlist,
              firstMusicId: musicData.musicList[0].id
            })
          } else {
            playlistsWithCovers.push(playlist)
          }
        } catch (error) {
          playlistsWithCovers.push(playlist)
        }
      }
      favoritePlaylists.value = playlistsWithCovers
    }
  } catch (error) {
    console.error('加载收藏歌单失败:', error)
  }
}

const getPlaylistCover = (playlist) => {
  if (playlist.firstMusicId) {
    return `https://music.cnmsb.xin/api/music/cover/${playlist.firstMusicId}`
  }
  return 'https://music.cnmsb.xin/api/user/avatar/default'
}

const openPlaylist = (playlistId) => {
  console.log('点击歌单:', playlistId)
  if (currentPlaylistId.value === playlistId && route.path === `/playlist/${playlistId}`) {
    console.log('已经是当前歌单，跳过')
    return
  }
  currentRoute.value = 'playlist'
  currentPlaylistId.value = playlistId
  console.log('跳转到歌单详情:', `/playlist/${playlistId}`)
  router.push(`/playlist/${playlistId}`)
}

const handleCreatePlaylist = () => {

  if (!newPlaylistName.value.trim()) {

    showToast('请输入歌单名称', 'error')

    return

  }

  createPlaylist(newPlaylistName.value)

}

const createPlaylist = async (name) => {

  const token = localStorage.getItem('token')

  if (!token) {

    showLoginModal.value = true

    return

  }

  try {

    const response = await apiRequest(apiConfig.PLAYLIST_CREATE, {

      method: 'POST',

      headers: { 'Authorization': token, 'Content-Type': 'application/json' },

      body: JSON.stringify({ name: name, description: '' })

    })

    const data = await response.json()

    if (data.success) {

      showToast('歌单创建成功', 'success')

      showCreatePlaylistModal.value = false

      newPlaylistName.value = ''

      loadMyPlaylists()

    } else {

      showToast(data.message || '创建失败', 'error')

    }

  } catch (error) {

    console.error('创建歌单失败:', error)

    showToast('创建失败', 'error')

  }

}

// 显示右键菜单
const showPlaylistContextMenu = (event, playlist) => {
  event.preventDefault()
  event.stopPropagation()
  contextMenu.value = {
    visible: true,
    x: event.clientX,
    y: event.clientY,
    playlist: playlist
  }
}

// 处理重命名歌单
const handleRenamePlaylist = () => {
  if (!contextMenu.value.playlist) return
  editMode.value = 'name'
  editPlaylistValue.value = contextMenu.value.playlist.name
  currentEditPlaylist.value = contextMenu.value.playlist
  showEditPlaylistModal.value = true
}

// 处理修改歌单描述
const handleEditPlaylistDescription = () => {
  if (!contextMenu.value.playlist) return
  editMode.value = 'description'
  editPlaylistValue.value = contextMenu.value.playlist.description || ''
  currentEditPlaylist.value = contextMenu.value.playlist
  showEditPlaylistModal.value = true
}

// 保存歌单编辑
const handleSavePlaylistEdit = async () => {
  if (!currentEditPlaylist.value || !editPlaylistValue.value.trim()) {
    showToast('请输入有效的内容', 'error')
    return
  }

  const token = localStorage.getItem('token')
  if (!token) {
    showLoginModal.value = true
    return
  }

  try {
    const updateData = {
      id: currentEditPlaylist.value.id,
      name: editMode.value === 'name' ? editPlaylistValue.value.trim() : currentEditPlaylist.value.name
    }

    // 只在修改描述时添加 description 字段
    if (editMode.value === 'description') {
      updateData.description = editPlaylistValue.value.trim()
    }

    const response = await apiRequest(apiConfig.PLAYLIST_UPDATE, {
      method: 'POST',
      headers: { 'Authorization': token, 'Content-Type': 'application/json' },
      body: JSON.stringify(updateData)
    })

    const data = await response.json()
    if (data.success) {
      showToast(editMode.value === 'name' ? '歌单重命名成功' : '歌单描述修改成功', 'success')
      showEditPlaylistModal.value = false

      // 直接更新本地数据，立即反映变化
      const playlistIndex = myPlaylists.value.findIndex(p => p.id === currentEditPlaylist.value.id)
      if (playlistIndex !== -1) {
        if (editMode.value === 'name') {
          myPlaylists.value[playlistIndex].name = editPlaylistValue.value.trim()
        } else {
          myPlaylists.value[playlistIndex].description = editPlaylistValue.value.trim()
        }
      }

      // 同时更新当前正在查看的歌单详情（如果正在查看这个歌单）
      if (currentPlaylistId.value === currentEditPlaylist.value.id) {
        // 发送事件通知 PlaylistDetailView 刷新
        window.dispatchEvent(new CustomEvent('playlist-updated', {
          detail: {
            id: currentEditPlaylist.value.id,
            name: editMode.value === 'name' ? editPlaylistValue.value.trim() : myPlaylists.value[playlistIndex].name,
            description: editMode.value === 'description' ? editPlaylistValue.value.trim() : myPlaylists.value[playlistIndex].description
          }
        }))
      }
    } else {
      showToast(data.message || '修改失败', 'error')
    }
  } catch (error) {
    console.error('修改歌单失败:', error)
    showToast('修改失败', 'error')
  }
}

// 处理删除歌单
const handleDeletePlaylist = () => {
  if (!contextMenu.value.playlist) return

  confirmDialog.value = {
    title: '删除歌单',
    message: `确定要删除歌单"${contextMenu.value.playlist.name}"吗？此操作不可恢复。`,
    onConfirm: async () => {
      const token = localStorage.getItem('token')
      if (!token) {
        showLoginModal.value = true
        return
      }

      try {
        const response = await apiRequest(apiConfig.PLAYLIST_DELETE, {
          method: 'POST',
          headers: { 'Authorization': token, 'Content-Type': 'application/json' },
          body: JSON.stringify({ id: contextMenu.value.playlist.id })
        })

        const data = await response.json()
        if (data.success) {
          showToast('歌单删除成功', 'success')
          loadMyPlaylists()
          loadFavoritePlaylists()
          // 如果删除的是当前正在查看的歌单，跳转到首页
          if (currentPlaylistId.value === contextMenu.value.playlist.id) {
            router.push('/home')
          }
        } else {
          showToast(data.message || '删除失败', 'error')
        }
      } catch (error) {
        console.error('删除歌单失败:', error)
        showToast('删除失败', 'error')
      }
    }
  }
  showConfirmDialog.value = true
}

// 处理确认对话框确认
const handleConfirmDialog = () => {
  if (confirmDialog.value.onConfirm) {
    confirmDialog.value.onConfirm()
  }
  showConfirmDialog.value = false
}

// 处理歌单更新事件
const handlePlaylistUpdated = (event) => {
  const { id, action } = event.detail
  
  // 如果是音乐被添加或移除，重新加载歌单列表以更新音乐数量和封面
  if (action === 'music-removed' || action === 'music-added') {
    loadMyPlaylists()
    loadFavoritePlaylists()
  }
}

// 处理收藏歌单更新事件
const handleFavoritePlaylistUpdated = () => {
  console.log('收藏歌单列表已更新')
  loadFavoritePlaylists()
}

const isLoggedIn = computed(() => {
  return currentUser.value !== null
})

const userAvatar = computed(() => {
  if (currentUser.value && currentUser.value.id) {
    return `https://music.cnmsb.xin/api/user/avatar/${currentUser.value.id}`
  }
  return getDefaultAvatar()
})

const getDefaultAvatar = () => {
  return 'https://music.cnmsb.xin/api/user/avatar/default'
}

const showToast = (message, type = 'info') => {
  const id = toastId++
  toasts.value.push({ id, message, type })
  setTimeout(() => {
    toasts.value = toasts.value.filter(t => t.id !== id)
  }, 3000)
}

const sendVerificationCode = async () => {
  if (!formData.value.email) {
    showToast('请先输入邮箱地址', 'error')
    return
  }
  
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/
  if (!emailRegex.test(formData.value.email)) {
    showToast('请输入有效的邮箱地址', 'error')
    return
  }
  
  codeSending.value = true
  try {
    const response = await apiRequest('/api/user/send-verification', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        email: formData.value.email
      })
    })
    
    const result = await response.json()
    if (result.success) {
      showToast('验证码已发送至您的邮箱', 'success')
      startCountdown()
    } else {
      showToast(result.message || '发送验证码失败', 'error')
    }
  } catch (error) {
    showToast('网络错误，请检查服务器连接', 'error')
  } finally {
    codeSending.value = false
  }
}

const startCountdown = () => {
  countdown.value = 60
  countdownInterval.value = setInterval(() => {
    countdown.value--
    if (countdown.value <= 0) {
      clearInterval(countdownInterval.value)
    }
  }, 1000)
}

const navigateTo = (route) => {
  currentRoute.value = route
  router.push(`/${route}`)
}

const handleSearch = () => {
  if (searchQuery.value.trim()) {
    router.push(`/search?q=${encodeURIComponent(searchQuery.value)}`)
  }
}

const handleUserClick = () => {
  if (isLoggedIn.value) {
    navigateTo('settings')
  } else {
    showLoginModal.value = true
  }
}

const handleSubmit = async () => {
  errorMessage.value = ''
  
  if (!formData.value.username || !formData.value.password) {
    errorMessage.value = '请填写用户名和密码'
    return
  }

  if (authTab.value === 'register' && !formData.value.email) {
    errorMessage.value = '请填写邮箱'
    return
  }

  if (authTab.value === 'register' && !formData.value.verificationCode) {
    errorMessage.value = '请填写验证码'
    return
  }

  try {
    if (authTab.value === 'login') {
      const response = await apiRequest(apiConfig.USER_LOGIN, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          username: formData.value.username,
          password: formData.value.password
        })
      })
      
      if (!response.ok) {
        throw new Error('登录失败')
      }
      
      const result = await response.json()
      if (result.success && result.data && result.data.user) {
        const user = result.data.user
        const token = result.data.token
        
        localStorage.setItem('user', JSON.stringify(user))
        localStorage.setItem('token', token)
        currentUser.value = user
        username.value = user.username
        showLoginModal.value = false
        formData.value = { username: '', password: '', email: '' }
        showToast('登录成功，欢迎回来！', 'success')
      } else {
        throw new Error(result.message || '登录失败')
      }
    } else {
      const response = await apiRequest(apiConfig.USER_REGISTER, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          username: formData.value.username,
          password: formData.value.password,
          email: formData.value.email,
          verificationCode: formData.value.verificationCode
        })
      })
      
      if (!response.ok) {
        throw new Error('注册失败')
      }
      
      const result = await response.json()
      if (result.success) {
        authTab.value = 'login'
        errorMessage.value = ''
        formData.value = { username: '', password: '', email: '', verificationCode: '' }
        countdown.value = 0
        if (countdownInterval.value) {
          clearInterval(countdownInterval.value)
        }
        showToast('注册成功，请登录', 'success')
      } else {
        throw new Error(result.message || '注册失败')
      }
    }
  } catch (error) {
    errorMessage.value = error.message || `${authTab.value === 'login' ? '登录' : '注册'}失败，请重试`
    showToast(errorMessage.value, 'error')
  }
}

const minimize = () => {
  console.log('===== Layout minimize clicked =====')
  console.log('electronAPI:', window.electronAPI)
  if (window.electronAPI?.minimize) {
    window.electronAPI.minimize()
    console.log('electronAPI.minimize called')
  } else {
    console.warn('electronAPI.minimize not available')
  }
}

const maximize = () => {
  console.log('===== Layout maximize clicked =====')
  if (window.electronAPI?.maximize) {
    window.electronAPI.maximize()
    console.log('electronAPI.maximize called')
  } else {
    console.warn('electronAPI.maximize not available')
  }
}

const close = () => {
  console.log('===== Layout close clicked =====')
  if (window.electronAPI?.close) {
    window.electronAPI.close()
    console.log('electronAPI.close called')
  } else {
    console.warn('electronAPI.close not available')
  }
}

onMounted(() => {
  const userStr = localStorage.getItem('user')
  if (userStr) {
    try {
      const user = JSON.parse(userStr)
      currentUser.value = user
      username.value = user.username
    } catch (e) {
      console.error('解析用户信息失败:', e)
    }
  }
  
  loadMyPlaylists()
      loadFavoritePlaylists()
  
  window.addEventListener('user-logout', handleUserLogout)
  window.addEventListener('user-login', handleUserLogin)
  window.addEventListener('show-toast', handleShowToast)
  window.addEventListener('playlist-updated', handlePlaylistUpdated)
  window.addEventListener('favorite-playlist-updated', handleFavoritePlaylistUpdated)
  
  // 点击其他地方关闭右键菜单
  document.addEventListener('click', () => {
    contextMenu.value.visible = false
  })
  
  // 路由变化时关闭右键菜单
  router.afterEach(() => {
    contextMenu.value.visible = false
  })
})

const handleShowToast = (event) => {
  const { message, type } = event.detail
  showToast(message, type)
}

const handleUserLogout = () => {
  currentUser.value = null
  username.value = ''
}

const handleUserLogin = (event) => {
  const user = event.detail
  currentUser.value = user
  username.value = user.username
  loadMyPlaylists()
  loadFavoritePlaylists()
}

watch(() => route.path, (newPath) => {
  console.log('路由变化:', newPath)
  const path = newPath.replace(/^\//, '') || 'home'
  currentRoute.value = path
  if (path.startsWith('playlist/')) {
    const playlistId = parseInt(path.split('/')[1])
    if (playlistId) {
      currentPlaylistId.value = playlistId
      console.log('设置当前歌单ID:', playlistId)
    } else {
      currentPlaylistId.value = null
    }
  } else {
    currentPlaylistId.value = null
  }
}, { immediate: true })
</script>

<style scoped>
.layout {
  display: flex;
  width: 100vw;
  height: 100vh;
  overflow: hidden;
  background: var(--bg-main);
}

/* 侧边栏样式 */
.layout-sidebar {
  width: 260px;
  min-height: 100vh;
  display: flex;
  flex-direction: column;
  border-right: 1px solid var(--border-color);
  flex-shrink: 0;
  position: relative;
  overflow: hidden;
}

.layout-sidebar::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: linear-gradient(180deg, rgba(102, 126, 234, 0.1) 0%, transparent 100%);
  pointer-events: none;
}

.sidebar-header {
  padding: 24px 20px;
  display: flex;
  align-items: center;
  gap: 12px;
  flex-shrink: 0;
  position: relative;
  z-index: 1;
}

.logo-wrapper {
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
}

.app-logo {
  width: 32px;
  height: 32px;
  border-radius: 8px;
  position: relative;
  z-index: 2;
}

.logo-glow {
  position: absolute;
  width: 50px;
  height: 50px;
  background: var(--gradient-primary);
  border-radius: 50%;
  filter: blur(20px);
  opacity: 0.5;
  z-index: 1;
}

.app-name {
  font-size: 18px;
  font-weight: 700;
  letter-spacing: 0.5px;
}

.sidebar-nav {
  flex: 1;
  padding: 16px 12px;
  overflow-y: auto;
  position: relative;
  z-index: 1;
}

.nav-item {
  display: flex;
  align-items: center;
  padding: 14px 16px;
  color: var(--text-white-muted);
  cursor: pointer;
  transition: all var(--transition-normal);
  border-radius: var(--radius-md);
  position: relative;
  margin-bottom: 4px;
}

.nav-item:hover {
  background: rgba(255, 255, 255, 0.1);
  color: white;
  transform: translateX(4px);
}

.nav-item.active {
  background: var(--gradient-primary);
  color: white;
  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
}

.nav-icon-wrapper {
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
  width: 24px;
  height: 24px;
  margin-right: 14px;
}

.nav-icon {
  width: 20px;
  height: 20px;
  position: relative;
  z-index: 2;
}

.nav-icon-glow {
  position: absolute;
  width: 30px;
  height: 30px;
  background: rgba(255, 255, 255, 0.3);
  border-radius: 50%;
  filter: blur(8px);
  z-index: 1;
}

.nav-item span {
  font-size: 14px;
  font-weight: 500;
}

.nav-indicator {
  position: absolute;
  right: 0;
  top: 50%;
  transform: translateY(-50%);
  width: 3px;
  height: 60%;
  background: white;
  border-radius: 2px 0 0 2px;
  box-shadow: 0 0 8px rgba(255, 255, 255, 0.5);
}

.nav-divider {
  height: 1px;
  background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.1), transparent);
  margin: 8px 16px;
}

.sidebar-spacer {
  flex: 1;
  min-height: 0;
}

.sidebar-playlists {
  padding: 0 0 12px 0;
}

.playlists-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 16px 8px 16px;
}

.playlists-title {
  font-size: 13px;
  color: var(--text-white-muted);
  font-weight: 500;
}

.add-playlist-btn {
  width: 28px;
  height: 28px;
  border: none;
  background: rgba(255, 255, 255, 0.1);
  border-radius: 50%;
  color: var(--text-white-muted);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  transition: all 0.2s;
}

.add-playlist-btn:hover {
  background: rgba(255, 255, 255, 0.2);
  color: white;
}

.playlists-list {
  max-height: 200px;
  overflow-y: auto;
}

.playlist-item {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 8px 16px;
  cursor: pointer;
  transition: all 0.2s;
  border-left: 3px solid transparent;
}

.playlist-item:hover {
  background: rgba(255, 255, 255, 0.1);
}

.playlist-item.active {
  background: rgba(255, 255, 255, 0.15);
  border-left-color: var(--primary);
}

.playlist-item.active .playlist-name {
  color: white;
  font-weight: 500;
}

.playlist-cover {
  width: 36px;
  height: 36px;
  border-radius: 6px;
  object-fit: cover;
  flex-shrink: 0;
}

.playlist-name {
  font-size: 13px;
  color: var(--text-white-muted);
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  flex: 1;
}

.playlists-empty {
  padding: 20px 16px;
  text-align: center;
  font-size: 12px;
  color: rgba(255, 255, 255, 0.4);
}

.playlists-list::-webkit-scrollbar {
  width: 4px;
}

.playlists-list::-webkit-scrollbar-track {
  background: transparent;
}

.playlists-list::-webkit-scrollbar-thumb {
  background: rgba(255, 255, 255, 0.2);
  border-radius: 2px;
}

.playlists-list::-webkit-scrollbar-thumb:hover {
  background: rgba(255, 255, 255, 0.3);
}

.sidebar-footer {
  padding: 16px 12px;
  border-top: 1px solid var(--border-color);
  flex-shrink: 0;
  position: relative;
  z-index: 1;
}

.user-card {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 12px;
  background: rgba(255, 255, 255, 0.05);
  border-radius: var(--radius-md);
  cursor: pointer;
  transition: all var(--transition-normal);
  border: 1px solid transparent;
}

.user-card:hover {
  background: rgba(255, 255, 255, 0.1);
  border-color: rgba(255, 255, 255, 0.2);
  transform: translateY(-2px);
}

.user-avatar-wrapper {
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
}

.user-avatar {
  width: 42px;
  height: 42px;
  border-radius: 50%;
  object-fit: cover;
  position: relative;
  z-index: 2;
}

.user-avatar-ring {
  position: absolute;
  width: 50px;
  height: 50px;
  border: 2px solid transparent;
  border-radius: 50%;
  background: var(--gradient-primary);
  -webkit-mask: linear-gradient(#fff 0 0) padding-box, linear-gradient(#fff 0 0);
  -webkit-mask-composite: xor;
  mask-composite: exclude;
  animation: rotate 8s linear infinite;
}

.user-details {
  flex: 1;
  min-width: 0;
}

.username {
  font-size: 14px;
  color: white;
  font-weight: 600;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
  display: block;
}

.user-status {
  font-size: 12px;
  color: var(--text-white-muted);
  margin-top: 2px;
  display: block;
}

.user-arrow {
  color: var(--text-white-muted);
  opacity: 0;
  transform: translateX(-8px);
  transition: all var(--transition-normal);
}

.user-card:hover .user-arrow {
  opacity: 1;
  transform: translateX(0);
}

/* 主内容区域 */
.layout-main {
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  position: relative;
}

/* 标题栏 */
.title-bar {
  height: 56px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 20px;
  flex-shrink: 0;
  -webkit-app-region: drag;
  user-select: none;
  position: relative;
  z-index: 10;
}

.title-bar::after {
  content: '';
  position: absolute;
  bottom: 0;
  left: 20px;
  right: 20px;
  height: 1px;
  background: linear-gradient(90deg, transparent, rgba(0, 0, 0, 0.1), transparent);
}

.title-bar-center {
  flex: 1;
  display: flex;
  justify-content: center;
  max-width: 520px;
}

.search-box {
  display: flex;
  align-items: center;
  width: 100%;
  max-width: 480px;
  height: 44px;
  background: white;
  border-radius: var(--radius-lg);
  padding: 0 4px 0 16px;
  box-shadow: var(--shadow-md);
  -webkit-app-region: no-drag;
  transition: all var(--transition-normal);
  border: 2px solid transparent;
  position: relative;
  overflow: hidden;
}

.search-box::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: var(--gradient-primary);
  opacity: 0;
  transition: opacity var(--transition-normal);
  border-radius: var(--radius-lg);
}

.search-box:focus-within {
  box-shadow: 0 8px 24px rgba(102, 126, 234, 0.25);
  border-color: var(--primary);
  transform: translateY(-1px);
}

.search-box:focus-within::before {
  opacity: 0.05;
}

.search-icon {
  width: 20px;
  height: 20px;
  color: var(--text-muted);
  flex-shrink: 0;
  transition: color var(--transition-fast);
  position: relative;
  z-index: 1;
}

.search-box:focus-within .search-icon {
  color: var(--primary);
}

.search-box input {
  flex: 1;
  border: none;
  background: transparent;
  padding: 0 12px;
  font-size: 15px;
  color: var(--text-primary);
  outline: none;
  position: relative;
  z-index: 1;
}

.search-box input::placeholder {
  color: var(--text-muted);
  transition: opacity var(--transition-fast);
}

.search-box:focus-within input::placeholder {
  opacity: 0.6;
}

.search-btn {
  width: 40px;
  height: 36px;
  border: none;
  background: var(--gradient-primary);
  border-radius: var(--radius-md);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  transition: all var(--transition-normal);
  margin-right: 4px;
  position: relative;
  z-index: 1;
  flex-shrink: 0;
}

.search-btn:hover {
  transform: scale(1.05);
  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
}

.search-btn:active {
  transform: scale(0.95);
}

.search-btn svg {
  width: 18px;
  height: 18px;
}

.title-bar-right {
  display: flex;
  align-items: center;
  gap: 8px;
  flex-shrink: 0;
}

.action-btn {
  position: relative;
  width: 40px;
  height: 40px;
  border: none;
  background: transparent;
  border-radius: var(--radius-md);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--text-secondary);
  transition: all var(--transition-normal);
  -webkit-app-region: no-drag;
}

.action-btn:hover {
  background: rgba(102, 126, 234, 0.1);
  color: var(--primary);
  transform: translateY(-2px);
}

.window-controls {
  display: flex;
  align-items: center;
  gap: 4px;
  -webkit-app-region: no-drag;
}

.window-btn {
  width: 36px;
  height: 36px;
  border: none;
  background: transparent;
  border-radius: var(--radius-sm);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: var(--text-secondary);
  transition: all var(--transition-fast);
}

.window-btn:hover {
  background: rgba(0, 0, 0, 0.05);
  color: var(--text-primary);
}

.close-btn:hover {
  background: #e81123;
  color: white;
}

/* 主内容 */
.main-content {
  flex: 1;
  overflow: auto;
  background: transparent;
  position: relative;
}

/* 页面过渡动画 */
.fade-enter-active,
.fade-leave-active {
  transition: all 0.3s ease;
}

.fade-enter-from {
  opacity: 0;
  transform: translateY(10px);
}

.fade-leave-to {
  opacity: 0;
  transform: translateY(-10px);
}

.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.6);
  backdrop-filter: blur(8px);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 99999;
  padding: 20px;
}

.modal-content {
  background: linear-gradient(135deg, rgba(42, 42, 42, 0.95) 0%, rgba(30, 30, 30, 0.95) 100%);
  border-radius: 20px;
  padding: 32px;
  width: 100%;
  max-width: 380px;
  text-align: center;
  box-shadow: 
    0 20px 60px rgba(0, 0, 0, 0.5),
    0 0 0 1px rgba(255, 255, 255, 0.1),
    inset 0 1px 0 rgba(255, 255, 255, 0.1);
  position: relative;
  overflow: hidden;
}

.modal-content.modal-small {
  max-width: 320px;
  padding: 24px;
}

.modal-content::before {
  content: '';
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: radial-gradient(circle, rgba(102, 126, 234, 0.1) 0%, transparent 50%);
  pointer-events: none;
}

.modal-header {
  text-align: center;
  margin-bottom: 24px;
  position: relative;
  z-index: 1;
}

.modal-logo {
  width: 64px;
  height: 64px;
  margin: 0 auto 16px;
  position: relative;
}

.modal-logo img {
  width: 100%;
  height: 100%;
  border-radius: 16px;
  box-shadow: 
    0 8px 24px rgba(102, 126, 234, 0.3),
    inset 0 1px 0 rgba(255, 255, 255, 0.2);
}

.modal-title {
  font-size: 24px;
  font-weight: 700;
  color: white;
  margin-bottom: 8px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

.modal-subtitle {
  font-size: 13px;
  color: rgba(255, 255, 255, 0.5);
  margin: 0;
}

.modal-tabs {
  display: flex;
  gap: 8px;
  margin-bottom: 28px;
  background: rgba(0, 0, 0, 0.3);
  padding: 6px;
  border-radius: 14px;
  position: relative;
  z-index: 1;
}

.tab-btn {
  flex: 1;
  padding: 12px;
  border: none;
  background: transparent;
  color: rgba(255, 255, 255, 0.5);
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  border-radius: 10px;
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
  position: relative;
  z-index: 1;
}

.tab-btn.active {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  box-shadow: 
    0 4px 12px rgba(102, 126, 234, 0.4),
    inset 0 1px 0 rgba(255, 255, 255, 0.2);
  transform: scale(1.02);
}

.auth-form {
  display: flex;
  flex-direction: column;
  gap: 14px;
  margin-bottom: 24px;
  position: relative;
  z-index: 1;
}

.auth-input {
  padding: 14px 18px;
  border: 2px solid rgba(255, 255, 255, 0.08);
  border-radius: 12px;
  background: rgba(0, 0, 0, 0.3);
  color: white;
  font-size: 14px;
  outline: none;
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.auth-input::placeholder {
  color: rgba(255, 255, 255, 0.4);
}

.auth-input:focus {
  border-color: #667eea;
  background: rgba(0, 0, 0, 0.4);
  box-shadow: 
    0 0 0 4px rgba(102, 126, 234, 0.1),
    0 4px 12px rgba(102, 126, 234, 0.2);
  transform: translateY(-1px);
}

.auth-input:hover:not(:focus) {
  border-color: rgba(255, 255, 255, 0.15);
}

.email-field {
  display: flex;
  flex-direction: column;
  gap: 14px;
}

.verification-code {
  display: flex;
  gap: 10px;
}

.verification-code .auth-input {
  flex: 1;
}

.send-code-btn {
  padding: 0 20px;
  border: none;
  border-radius: 12px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s;
  white-space: nowrap;
  min-width: 100px;
  font-weight: 500;
}

.send-code-btn:hover:not(:disabled) {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
}

.send-code-btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
  transform: none;
}

.modal-buttons {
  display: flex;
  flex-direction: column;
  gap: 10px;
  position: relative;
  z-index: 1;
}

.modal-btn {
  padding: 14px 28px;
  border-radius: 12px;
  font-size: 15px;
  font-weight: 600;
  cursor: pointer;
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
  border: none;
  position: relative;
  overflow: hidden;
}

.modal-btn-primary {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  box-shadow: 
    0 4px 16px rgba(102, 126, 234, 0.4),
    inset 0 1px 0 rgba(255, 255, 255, 0.2);
}

.modal-btn-primary::before {
  content: '';
  position: absolute;
  top: 0;
  left: -100%;
  width: 100%;
  height: 100%;
  background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.2), transparent);
  transition: left 0.5s;
}

.modal-btn-primary:hover::before {
  left: 100%;
}

.modal-btn-primary:hover {
  transform: translateY(-2px);
  box-shadow: 
    0 8px 24px rgba(102, 126, 234, 0.5),
    inset 0 1px 0 rgba(255, 255, 255, 0.3);
}

.modal-btn-primary:active {
  transform: translateY(0);
  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.3);
}

.error-message {
  color: #ff6b6b;
  font-size: 13px;
  margin-top: 14px;
  padding: 10px 14px;
  background: rgba(255, 107, 107, 0.1);
  border-radius: 8px;
  border: 1px solid rgba(255, 107, 107, 0.2);
  position: relative;
  z-index: 1;
}

.modal-enter-active,
.modal-leave-active {
  transition: opacity 0.3s ease;
}

.modal-enter-from,
.modal-leave-to {
  opacity: 0;
}

.modal-enter-active .modal-content,
.modal-leave-active .modal-content {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.modal-enter-from .modal-content,
.modal-leave-to .modal-content {
  opacity: 0;
  transform: scale(0.9) translateY(10px);
}

.title-fade-enter-active,
.title-fade-leave-active {
  transition: all 0.3s ease;
}

.title-fade-enter-from,
.title-fade-leave-to {
  opacity: 0;
  transform: translateY(-10px);
}

.subtitle-fade-enter-active,
.subtitle-fade-leave-active {
  transition: all 0.3s ease 0.1s;
}

.subtitle-fade-enter-from,
.subtitle-fade-leave-to {
  opacity: 0;
  transform: translateY(-8px);
}

.form-slide-enter-active,
.form-slide-leave-active {
  transition: all 0.4s cubic-bezier(0.4, 0, 0.2, 1);
}

.form-slide-enter-from {
  opacity: 0;
  transform: translateX(20px);
}

.form-slide-leave-to {
  opacity: 0;
  transform: translateX(-20px);
}

.field-fade-enter-active {
  transition: all 0.3s ease;
}

.field-fade-enter-from {
  opacity: 0;
  transform: translateY(-10px);
}

.field-fade-leave-active {
  transition: all 0.2s ease;
}

.field-fade-leave-to {
  opacity: 0;
  transform: translateY(10px);
}

.button-fade-enter-active,
.button-fade-leave-active {
  transition: all 0.3s ease;
}

.button-fade-enter-from,
.button-fade-leave-to {
  opacity: 0;
  transform: translateY(10px);
}

.toast-container {
  position: fixed;
  top: 80px;
  right: 24px;
  z-index: 999999;
  display: flex;
  flex-direction: column;
  gap: 12px;
  pointer-events: none;
}

.toast {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 16px 20px;
  background: rgba(255, 255, 255, 0.95);
  backdrop-filter: blur(10px);
  border-radius: 12px;
  box-shadow: 
    0 8px 32px rgba(0, 0, 0, 0.15),
    0 0 0 1px rgba(255, 255, 255, 0.5);
  min-width: 300px;
  pointer-events: auto;
  position: relative;
  overflow: hidden;
}

.toast::before {
  content: '';
  position: absolute;
  left: 0;
  top: 0;
  bottom: 0;
  width: 4px;
}

.toast-success::before {
  background: linear-gradient(180deg, #4ade80 0%, #22c55e 100%);
}

.toast-error::before {
  background: linear-gradient(180deg, #f87171 0%, #ef4444 100%);
}

.toast-info::before {
  background: linear-gradient(180deg, #60a5fa 0%, #3b82f6 100%);
}

.toast-icon {
  width: 24px;
  height: 24px;
  flex-shrink: 0;
  display: flex;
  align-items: center;
  justify-content: center;
}

.toast-success .toast-icon {
  color: #22c55e;
}

.toast-error .toast-icon {
  color: #ef4444;
}

.toast-info .toast-icon {
  color: #3b82f6;
}

.toast-content {
  flex: 1;
}

.toast-message {
  font-size: 14px;
  color: #1f2937;
  font-weight: 500;
}

.toast-enter-active,
.toast-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-enter-from {
  opacity: 0;
  transform: translateX(100px);
}

/* 右键菜单 */
.context-menu {
  position: fixed;
  z-index: 999999;
  background: rgba(42, 42, 42, 0.98);
  border-radius: 12px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(255, 255, 255, 0.1);
  min-width: 160px;
  overflow: hidden;
  backdrop-filter: blur(12px);
}

.context-menu-item {
  display: flex;
  align-items: center;
  gap: 10px;
  padding: 12px 16px;
  cursor: pointer;
  transition: all 0.2s;
  color: var(--text-white-muted);
  font-size: 14px;
}

.context-menu-item:hover {
  background: rgba(102, 126, 234, 0.2);
  color: white;
}

.context-menu-item.danger:hover {
  background: rgba(239, 68, 68, 0.2);
  color: #ef4444;
}

.context-menu-item svg {
  flex-shrink: 0;
}

.context-menu-enter-active,
.context-menu-leave-active {
  transition: all 0.2s cubic-bezier(0.4, 0, 0.2, 1);
}

.context-menu-enter-from {
  opacity: 0;
  transform: scale(0.95);
}

.context-menu-leave-to {
  opacity: 0;
  transform: scale(0.95);
}

/* 确认对话框 */
.modal-confirm {
  text-align: center;
}

.confirm-icon-wrapper {
  width: 64px;
  height: 64px;
  margin: 0 auto 20px;
  border-radius: 50%;
  background: linear-gradient(135deg, rgba(239, 68, 68, 0.1) 0%, rgba(220, 38, 38, 0.1) 100%);
  display: flex;
  align-items: center;
  justify-content: center;
}

.confirm-icon {
  color: #ef4444;
}

.confirm-message {
  color: var(--text-secondary);
  font-size: 14px;
  line-height: 1.6;
  margin-bottom: 24px;
}

.modal-btn-danger {
  background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%);
  color: white;
  transition: all var(--transition-normal);
}

.modal-btn-danger:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(239, 68, 68, 0.4);
}

/* ========================================
   响应式适配
   ======================================== */

/* 小屏幕适配 (< 640px) */
@media screen and (max-width: 639px) {
  .layout-sidebar {
    position: fixed;
    left: -100%;
    top: 0;
    height: 100vh;
    z-index: 9999;
    box-shadow: 0 0 0 transparent;
  }
  
  .layout-sidebar.open {
    left: 0;
    box-shadow: 0 0 0 9999px rgba(0, 0, 0, 0.5);
  }
  
  .layout-main {
    margin-left: 0 !important;
  }
  
  .title-bar {
    padding: 0 clamp(8px, 2vw, 12px);
  }
  
  .search-box {
    max-width: clamp(120px, 30vw, 180px);
  }
  
  .search-box input {
    font-size: 12px;
  }
  
  .modal-content {
    padding: clamp(16px, 4vw, 24px);
    max-width: 90vw;
  }
  
  .modal-title {
    font-size: clamp(18px, 4.5vw, 22px);
  }
  
  .toast {
    min-width: auto;
    width: auto;
    max-width: 85vw;
    padding: 12px 16px;
  }
  
  .toast-message {
    font-size: 13px;
  }
  
  .user-avatar,
  .playlist-cover {
    width: clamp(28px, 7vw, 36px);
    height: clamp(28px, 7vw, 36px);
  }
}

/* 中等屏幕适配 (640px - 1023px) */
@media screen and (min-width: 640px) and (max-width: 1023px) {
  .layout-sidebar {
    width: var(--sidebar-width);
  }
  
  .title-bar {
    padding: 0 clamp(12px, 2vw, 16px);
  }
  
  .search-box {
    max-width: clamp(200px, 35vw, 300px);
  }
  
  .nav-item span {
    font-size: 13px;
  }
}

/* 大屏幕适配 (1024px - 1279px) */
@media screen and (min-width: 1024px) and (max-width: 1279px) {
  .layout-sidebar {
    width: var(--sidebar-width);
  }
  
  .modal-content {
    padding: 28px;
  }
}

/* 2K屏幕适配 (1536px - 1919px) */
@media screen and (min-width: 1536px) and (max-width: 1919px) {
  .layout-sidebar {
    width: var(--sidebar-width);
  }
  
  .sidebar-header {
    padding: 24px;
  }
  
  .nav-item {
    padding: 14px 18px;
    gap: 14px;
  }
  
  .nav-item span {
    font-size: 15px;
  }
  
  .modal-content {
    padding: 32px;
    max-width: 480px;
  }
  
  .modal-title {
    font-size: 24px;
  }
  
  .toast {
    padding: 18px 24px;
    min-width: 340px;
  }
  
  .toast-message {
    font-size: 15px;
  }
}

/* 3K屏幕适配 (1920px - 2559px) */
@media screen and (min-width: 1920px) and (max-width: 2559px) {
  .layout-sidebar {
    width: var(--sidebar-width);
  }
  
  .sidebar-header {
    padding: 28px;
    gap: 16px;
  }
  
  .app-name {
    font-size: 18px;
  }
  
  .nav-item {
    padding: 16px 20px;
    gap: 16px;
  }
  
  .nav-item span {
    font-size: 16px;
  }
  
  .modal-content {
    padding: 40px;
    max-width: 520px;
  }
  
  .modal-title {
    font-size: 26px;
  }
  
  .auth-input {
    padding: 16px 20px;
    font-size: 16px;
  }
  
  .modal-btn {
    padding: 16px 28px;
    font-size: 16px;
  }
  
  .toast {
    padding: 20px 28px;
    min-width: 380px;
  }
  
  .toast-message {
    font-size: 16px;
  }
}

/* 4K屏幕适配 (>= 2560px) */
@media screen and (min-width: 2560px) {
  .layout-sidebar {
    width: var(--sidebar-width);
  }
  
  .sidebar-header {
    padding: 36px;
    gap: 20px;
  }
  
  .logo-wrapper {
    width: 48px;
    height: 48px;
  }
  
  .app-name {
    font-size: 22px;
  }
  
  .nav-item {
    padding: 20px 24px;
    gap: 18px;
  }
  
  .nav-icon {
    width: 26px;
    height: 26px;
  }
  
  .nav-icon-wrapper {
    width: 32px;
    height: 32px;
  }
  
  .nav-item span {
    font-size: 18px;
  }
  
  .nav-divider {
    margin: 16px 12px;
  }
  
  .modal-content {
    padding: 56px;
    max-width: 600px;
  }
  
  .modal-title {
    font-size: 32px;
    margin-bottom: 32px;
  }
  
  .auth-input {
    padding: 20px 24px;
    font-size: 18px;
    border-radius: 16px;
  }
  
  .modal-btn {
    padding: 20px 36px;
    font-size: 18px;
    border-radius: 16px;
  }
  
  .toast {
    padding: 24px 32px;
    min-width: 440px;
    border-radius: 20px;
  }
  
  .toast-icon {
    width: 28px;
    height: 28px;
  }
  
  .toast-message {
    font-size: 18px;
  }
  
  .context-menu {
    min-width: 200px;
    border-radius: 16px;
  }
  
  .context-menu-item {
    padding: 16px 20px;
    font-size: 16px;
    gap: 12px;
  }
  
  .playlist-item {
    padding: 16px 20px;
  }
  
  .playlist-cover {
    width: 52px;
    height: 52px;
  }
  
  .playlist-name {
    font-size: 16px;
  }
}

/* 5K屏幕适配 (>= 5120px) */
@media screen and (min-width: 5120px) {
  .layout-sidebar {
    width: var(--sidebar-width);
  }
  
  .sidebar-header {
    padding: 52px;
    gap: 28px;
  }
  
  .logo-wrapper {
    width: 64px;
    height: 64px;
  }
  
  .app-logo {
    border-radius: 16px;
  }
  
  .app-name {
    font-size: 28px;
  }
  
  .nav-item {
    padding: 28px 32px;
    gap: 24px;
  }
  
  .nav-icon {
    width: 36px;
    height: 36px;
  }
  
  .nav-icon-wrapper {
    width: 44px;
    height: 44px;
  }
  
  .nav-item span {
    font-size: 22px;
  }
  
  .modal-content {
    padding: 80px;
    max-width: 720px;
    border-radius: 32px;
  }
  
  .modal-title {
    font-size: 40px;
    margin-bottom: 40px;
  }
  
  .auth-input {
    padding: 28px 32px;
    font-size: 22px;
    border-radius: 20px;
  }
  
  .modal-btn {
    padding: 28px 48px;
    font-size: 22px;
    border-radius: 20px;
  }
  
  .toast {
    padding: 32px 40px;
    min-width: 560px;
    border-radius: 28px;
  }
  
  .toast-icon {
    width: 36px;
    height: 36px;
  }
  
  .toast-message {
    font-size: 22px;
  }
}

/* 高度适配 - 短屏幕 */
@media screen and (max-height: 720px) {
  .sidebar-header {
    padding: 12px;
  }
  
  .nav-item {
    padding: 8px 12px;
  }
  
  .modal-content {
    padding: 20px;
  }
}

/* 高度适配 - 超高屏幕 */
@media screen and (min-height: 1440px) {
  .sidebar-header {
    padding: 32px;
  }
  
  .nav-item {
    padding: 18px 22px;
  }
  
  .modal-content {
    padding: 48px;
  }
}

/* 横屏/竖屏适配 */
@media screen and (orientation: portrait) {
  .layout-sidebar {
    position: fixed;
    left: -100%;
    top: 0;
    height: 100vh;
    z-index: 9999;
  }
  
  .layout-sidebar.open {
    left: 0;
  }
  
  .layout-main {
    margin-left: 0 !important;
  }
}

.modal-btn-danger:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(239, 68, 68, 0.4);
}

.toast-leave-to {
  opacity: 0;
  transform: translateX(100px);
}

.toast-move {
  transition: transform 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}
</style>