<template>
  <div class="title-bar">
    <div class="title-bar-left">
      <img src="/icon.png" alt="Logo" class="app-logo" />
      <span class="title">Neko云音乐</span>
    </div>
    
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
        <svg class="mic-icon" viewBox="0 0 20 20">
          <path fill="currentColor" d="M7 4a3 3 0 016 0v6a3 3 0 11-6 0V4zm4 10.93A7.001 7.001 0 0017 8a1 1 0 10-2 0A5 5 0 015 8a1 1 0 00-2 0 7.001 7.001 0 006 6.93V17H6a1 1 0 100 2h8a1 1 0 100-2h-3v-2.07z"/>
        </svg>
      </div>
    </div>
    
    <div class="title-bar-right">
      <button class="action-btn" title="消息">
        <svg viewBox="0 0 20 20" width="20" height="20">
          <path fill="currentColor" d="M2.003 5.884L10 9.882l7.997-3.998A2 2 0 0016 4H4a2 2 0 00-1.997 1.884z"/>
          <path fill="currentColor" d="M18 8.118l-8 4-8-4V14a2 2 0 002 2h12a2 2 0 002-2V8.118z"/>
        </svg>
        <span class="badge"></span>
      </button>
      
      <div class="user-section">
        <img :src="userAvatar" alt="用户头像" class="user-avatar" @error="handleAvatarError" />
        <span class="username">{{ username || '未登录' }}</span>
        <svg class="dropdown-icon" viewBox="0 0 20 20" width="12" height="12">
          <path fill="currentColor" d="M5.293 7.293a1 1 0 011.414 0L10 10.586l3.293-3.293a1 1 0 111.414 1.414l-4 4a1 1 0 01-1.414 0l-4-4a1 1 0 010-1.414z"/>
        </svg>
      </div>
      
      <div class="window-controls">
        <button class="window-btn" @click="minimize" title="最小化">
          <svg viewBox="0 0 12 12" width="12" height="12">
            <path fill="currentColor" d="M2 5h8v1H2V5z"/>
          </svg>
        </button>
        <button class="window-btn" @click="maximize" :title="isMaximized ? '还原' : '最大化'">
          <svg v-if="!isMaximized" viewBox="0 0 12 12" width="12" height="12">
            <path fill="none" stroke="currentColor" stroke-width="1" d="M2 2h8v8H2z"/>
          </svg>
          <svg v-else viewBox="0 0 12 12" width="12" height="12">
            <path fill="none" stroke="currentColor" stroke-width="1" d="M2 5h7v5H2zM5 2h5v3H5z"/>
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
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useRouter } from 'vue-router'

const router = useRouter()
const searchQuery = ref('')
const username = ref('')
const isMaximized = ref(false)

const userAvatar = computed(() => {
  const userStr = localStorage.getItem('user')
  if (userStr) {
    try {
      const user = JSON.parse(userStr)
      return `https://music.cnmsb.xin/api/user/avatar/${user.id}`
    } catch (e) {
      return 'https://music.cnmsb.xin/api/user/avatar/default'
    }
  }
  return 'https://music.cnmsb.xin/api/user/avatar/default'
})

const handleAvatarError = (event) => {
  event.target.src = 'https://music.cnmsb.xin/api/user/avatar/default'
}

const handleSearch = () => {
  if (searchQuery.value.trim()) {
    router.push(`/search?q=${encodeURIComponent(searchQuery.value)}`)
  }
}

const minimize = () => {
  console.log('===== minimize clicked =====')
  console.log('electronAPI:', window.electronAPI)
  if (window.electronAPI?.minimize) {
    window.electronAPI.minimize()
    console.log('electronAPI.minimize called')
  } else {
    console.warn('electronAPI.minimize not available - running in browser mode')
    alert('最小化功能仅在 Electron 桌面应用中可用')
  }
}

const maximize = () => {
  console.log('===== maximize clicked =====')
  console.log('isMaximized:', isMaximized.value)
  if (window.electronAPI?.maximize) {
    window.electronAPI.maximize()
    isMaximized.value = !isMaximized.value
    console.log('electronAPI.maximize called')
  } else {
    console.warn('electronAPI.maximize not available - running in browser mode')
    alert('最大化功能仅在 Electron 桌面应用中可用')
  }
}

const close = () => {
  console.log('===== close clicked =====')
  if (window.electronAPI?.close) {
    window.electronAPI.close()
    console.log('electronAPI.close called')
  } else {
    console.warn('electronAPI.close not available - running in browser mode')
    alert('关闭功能仅在 Electron 桌面应用中可用')
  }
}

onMounted(() => {
  const userStr = localStorage.getItem('user')
  if (userStr) {
    try {
      const user = JSON.parse(userStr)
      username.value = user.username
    } catch (e) {
      console.error('解析用户信息失败:', e)
    }
  }
  
  // 监听窗口状态变化
  if (window.electronAPI?.onWindowMaximized) {
    window.electronAPI.onWindowMaximized(() => {
      isMaximized.value = true
    })
  }
  
  if (window.electronAPI?.onWindowUnmaximized) {
    window.electronAPI.onWindowUnmaximized(() => {
      isMaximized.value = false
    })
  }
})
</script>

<style scoped>
.title-bar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  height: 48px;
  padding: 0 16px;
  -webkit-app-region: drag;
  user-select: none;
  flex-shrink: 0;
}

.title-bar-left {
  display: flex;
  align-items: center;
  gap: 8px;
  flex-shrink: 0;
}

.app-logo {
  width: 28px;
  height: 28px;
  border-radius: 6px;
}

.title {
  font-size: 16px;
  font-weight: 500;
  color: #333;
}

.title-bar-center {
  flex: 1;
  display: flex;
  justify-content: center;
  max-width: 500px;
}

.search-box {
  display: flex;
  align-items: center;
  width: 100%;
  max-width: 400px;
  height: 32px;
  background: #f5f5f7;
  border-radius: 16px;
  padding: 0 12px;
  -webkit-app-region: no-drag;
}

.search-icon {
  width: 16px;
  height: 16px;
  color: #999;
  flex-shrink: 0;
}

.search-box input {
  flex: 1;
  border: none;
  background: transparent;
  padding: 0 8px;
  font-size: 13px;
  color: #333;
  outline: none;
}

.search-box input::placeholder {
  color: #999;
}

.mic-icon {
  width: 16px;
  height: 16px;
  color: #d0d0e0;
  flex-shrink: 0;
  cursor: pointer;
}

.mic-icon:hover {
  color: #999;
}

.title-bar-right {
  display: flex;
  align-items: center;
  gap: 16px;
  flex-shrink: 0;
}

.action-btn {
  position: relative;
  width: 36px;
  height: 36px;
  border: none;
  background: transparent;
  border-radius: 50%;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #666;
  transition: all 0.2s;
  -webkit-app-region: no-drag;
}

.action-btn:hover {
  background: #f5f5f7;
  color: #333;
}

.badge {
  position: absolute;
  top: 6px;
  right: 6px;
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: #ff4545;
}

.user-section {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 4px 8px;
  border-radius: 20px;
  cursor: pointer;
  transition: background-color 0.2s;
  -webkit-app-region: no-drag;
}

.user-section:hover {
  background: rgba(0, 0, 0, 0.05);
}

.user-avatar {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  object-fit: cover;
}

.username {
  font-size: 14px;
  color: #333;
  max-width: 100px;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.dropdown-icon {
  color: #999;
  flex-shrink: 0;
}

.window-controls {
  display: flex;
  align-items: center;
  gap: 2px;
  -webkit-app-region: no-drag;
}

.window-btn {
  width: 36px;
  height: 36px;
  border: none;
  background: transparent;
  border-radius: 6px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  color: #666;
  transition: all 0.2s;
}

.window-btn:hover {
  background: #f5f5f7;
  color: #333;
}

.close-btn:hover {
  background: #e81123;
  color: white;
}

/* ========================================
   响应式适配
   ======================================== */

/* 小屏幕适配 (< 640px) */
@media screen and (max-width: 639px) {
  .title-bar {
    height: 44px;
    padding: 0 clamp(8px, 2vw, 12px);
  }
  
  .title-bar-left {
    gap: 6px;
  }
  
  .app-logo {
    width: 24px;
    height: 24px;
  }
  
  .title {
    font-size: 14px;
  }
  
  .title-bar-center {
    max-width: clamp(120px, 40vw, 200px);
  }
  
  .search-box {
    height: 28px;
    padding: 0 10px;
  }
  
  .search-box input {
    font-size: 12px;
  }
  
  .search-icon,
  .mic-icon {
    width: 14px;
    height: 14px;
  }
  
  .title-bar-right {
    gap: 8px;
  }
  
  .action-btn {
    width: 32px;
    height: 32px;
  }
  
  .user-avatar {
    width: 28px;
    height: 28px;
  }
  
  .username {
    font-size: 12px;
    max-width: 60px;
  }
  
  .window-btn {
    width: 32px;
    height: 32px;
  }
}

/* 中等屏幕适配 (640px - 1023px) */
@media screen and (min-width: 640px) and (max-width: 1023px) {
  .title-bar {
    height: 46px;
    padding: 0 14px;
  }
  
  .title-bar-center {
    max-width: clamp(280px, 40vw, 360px);
  }
  
  .search-box {
    height: 30px;
    padding: 0 12px;
  }
  
  .username {
    max-width: 80px;
  }
}

/* 大屏幕适配 (1024px - 1279px) */
@media screen and (min-width: 1024px) and (max-width: 1279px) {
  .title-bar {
    height: 48px;
  }
  
  .title-bar-center {
    max-width: 420px;
  }
}

/* 2K屏幕适配 (1536px - 1919px) */
@media screen and (min-width: 1536px) and (max-width: 1919px) {
  .title-bar {
    height: 52px;
    padding: 0 20px;
  }
  
  .app-logo {
    width: 32px;
    height: 32px;
  }
  
  .title {
    font-size: 17px;
  }
  
  .title-bar-center {
    max-width: 480px;
  }
  
  .search-box {
    height: 36px;
    padding: 0 14px;
    border-radius: 18px;
  }
  
  .search-box input {
    font-size: 14px;
    padding: 0 10px;
  }
  
  .search-icon,
  .mic-icon {
    width: 18px;
    height: 18px;
  }
  
  .action-btn {
    width: 40px;
    height: 40px;
  }
  
  .user-avatar {
    width: 36px;
    height: 36px;
  }
  
  .username {
    font-size: 15px;
    max-width: 120px;
  }
  
  .window-btn {
    width: 40px;
    height: 40px;
  }
}

/* 3K屏幕适配 (1920px - 2559px) */
@media screen and (min-width: 1920px) and (max-width: 2559px) {
  .title-bar {
    height: 56px;
    padding: 0 24px;
  }
  
  .title-bar-left {
    gap: 12px;
  }
  
  .app-logo {
    width: 36px;
    height: 36px;
  }
  
  .title {
    font-size: 18px;
  }
  
  .title-bar-center {
    max-width: 540px;
  }
  
  .search-box {
    height: 38px;
    padding: 0 16px;
    border-radius: 19px;
  }
  
  .search-box input {
    font-size: 15px;
    padding: 0 12px;
  }
  
  .search-icon,
  .mic-icon {
    width: 19px;
    height: 19px;
  }
  
  .action-btn {
    width: 42px;
    height: 42px;
  }
  
  .user-avatar {
    width: 38px;
    height: 38px;
  }
  
  .username {
    font-size: 15px;
    max-width: 140px;
  }
  
  .window-btn {
    width: 42px;
    height: 42px;
  }
}

/* 4K屏幕适配 (>= 2560px) */
@media screen and (min-width: 2560px) {
  .title-bar {
    height: 64px;
    padding: 0 32px;
  }
  
  .title-bar-left {
    gap: 14px;
  }
  
  .app-logo {
    width: 40px;
    height: 40px;
  }
  
  .title {
    font-size: 20px;
  }
  
  .title-bar-center {
    max-width: 600px;
  }
  
  .search-box {
    height: 42px;
    padding: 0 18px;
    border-radius: 21px;
  }
  
  .search-box input {
    font-size: 16px;
    padding: 0 14px;
  }
  
  .search-icon,
  .mic-icon {
    width: 20px;
    height: 20px;
  }
  
  .action-btn {
    width: 44px;
    height: 44px;
  }
  
  .user-avatar {
    width: 40px;
    height: 40px;
  }
  
  .username {
    font-size: 16px;
    max-width: 160px;
  }
  
  .window-btn {
    width: 44px;
    height: 44px;
  }
}

/* 5K屏幕适配 (>= 5120px) */
@media screen and (min-width: 5120px) {
  .title-bar {
    height: 80px;
    padding: 0 48px;
  }
  
  .title-bar-left {
    gap: 20px;
  }
  
  .app-logo {
    width: 52px;
    height: 52px;
  }
  
  .title {
    font-size: 26px;
  }
  
  .title-bar-center {
    max-width: 800px;
  }
  
  .search-box {
    height: 52px;
    padding: 0 24px;
    border-radius: 26px;
  }
  
  .search-box input {
    font-size: 20px;
    padding: 0 18px;
  }
  
  .search-icon,
  .mic-icon {
    width: 24px;
    height: 24px;
  }
  
  .action-btn {
    width: 56px;
    height: 56px;
  }
  
  .user-avatar {
    width: 52px;
    height: 52px;
  }
  
  .username {
    font-size: 20px;
    max-width: 200px;
  }
  
  .window-btn {
    width: 56px;
    height: 56px;
  }
}

/* 高度适配 - 短屏幕 */
@media screen and (max-height: 720px) {
  .title-bar {
    height: 40px;
    padding: 0 12px;
  }
  
  .app-logo {
    width: 22px;
    height: 22px;
  }
  
  .title {
    font-size: 13px;
  }
  
  .search-box {
    height: 26px;
    padding: 0 8px;
  }
  
  .search-box input {
    font-size: 11px;
  }
  
  .action-btn,
  .window-btn {
    width: 30px;
    height: 30px;
  }
  
  .user-avatar {
    width: 26px;
    height: 26px;
  }
  
  .username {
    font-size: 11px;
  }
}

/* 高度适配 - 超高屏幕 */
@media screen and (min-height: 1440px) {
  .title-bar {
    height: 60px;
    padding: 0 28px;
  }
  
  .search-box {
    height: 40px;
  }
}

/* 触摸设备适配 */
@media (hover: none) and (pointer: coarse) {
  .action-btn,
  .window-btn {
    min-width: 44px;
    min-height: 44px;
  }
  
  .search-box {
    height: 44px;
  }
}
</style>