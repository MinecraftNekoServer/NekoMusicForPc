<template>
  <div class="settings-view">
    <div class="settings-container">
      <h2>{{ t('settings.settings') }}</h2>
      
      <div class="settings-section">
        <h3>{{ t('settings.accountInfo') }}</h3>
        <div v-if="currentUser" class="account-info">
          <div class="account-avatar">
            <img :src="userAvatar" :alt="t('common.username')" />
          </div>
          <div class="account-details">
            <div class="account-item">
              <span class="label">{{ t('common.username') }}</span>
              <span class="value">{{ currentUser.username }}</span>
            </div>
            <div class="account-item">
              <span class="label">{{ t('common.email') }}</span>
              <span class="value">{{ currentUser.email || t('common.noData') }}</span>
            </div>
            <div class="account-item">
              <span class="label">{{ t('settings.registerTime') }}</span>
              <span class="value">{{ formatDate(currentUser.createdAt) }}</span>
            </div>
          </div>
          <button class="logout-btn" @click="handleLogout">
            <svg viewBox="0 0 24 24" width="16" height="16">
              <path fill="currentColor" d="M17 7l-1.41 1.41L18.17 11H8v2h10.17l-2.58 2.58L17 17l5-5zM4 5h8V3H4c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h8v-2H4V5z"/>
            </svg>
            {{ t('settings.logout') }}
          </button>
        </div>
        <div v-else class="no-login">
          <p>{{ t('settings.notLoggedIn') }}</p>
          <button class="login-btn" @click="showLoginModal = true">{{ t('settings.goToLogin') }}</button>
        </div>
      </div>
      
      <div class="settings-section">
        <h3>{{ t('settings.playSettings') }}</h3>
        <div class="setting-item">
          <div class="setting-info">
            <span class="setting-label">{{ t('settings.musicCache') }}</span>
            <span class="setting-desc">{{ t('settings.musicCacheDesc') }}</span>
          </div>
          <label class="toggle-switch">
            <input type="checkbox" v-model="musicCacheEnabled" @change="handleCacheToggle" />
            <span class="toggle-slider"></span>
          </label>
        </div>
        <div class="cache-path-info">
          <span class="cache-path-label">{{ t('settings.cachePath') }}：</span>
          <span class="cache-path-value">{{ cachePath }}</span>
        </div>
      </div>
      
      <div class="settings-section">
        <h3>{{ t('settings.languageSettings') }}</h3>
        <div class="setting-item">
          <div class="setting-info">
            <span class="setting-label">{{ t('settings.language') }}</span>
            <span class="setting-desc">{{ t('settings.language') }}</span>
          </div>
          <select 
            class="language-select" 
            :value="currentLanguage"
            @change="handleLanguageChange($event.target.value)"
          >
            <option 
              v-for="option in languageOptions" 
              :key="option.value" 
              :value="option.value"
            >
              {{ option.label }}
            </option>
          </select>
        </div>
      </div>
      
      <div class="settings-section">
        <h3>{{ t('settings.about') }}</h3>
        <p>NekoMusic {{ getDisplayOSType() }} {{ t('settings.appVersion') }}</p>
        <p>{{ t('settings.appVersion') }}: {{ APP_VERSION }}</p>
        <div class="links-section">
          <a href="#" @click.prevent="openExternalLink('https://github.com/FantasyNetworkCN/NekoMusicForPc')" class="link-btn">
            <svg width="16" height="16" viewBox="0 0 16 16" fill="currentColor">
              <path d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0016 8c0-4.42-3.58-8-8-8z"/>
            </svg>
            {{ t('settings.githubRepo') }}
          </a>
          <a href="#" @click.prevent="openExternalLink('https://github.com/NyaNyagulugulu/NekoMusicDocs')" class="link-btn">
            <svg width="16" height="16" viewBox="0 0 16 16" fill="currentColor">
              <path d="M1 2.5A1.5 1.5 0 012.5 1h3A1.5 1.5 0 017 2.5v3A1.5 1.5 0 015.5 7h-3A1.5 1.5 0 011 5.5v-3zM2.5 2a.5.5 0 00-.5.5v3a.5.5 0 00.5.5h3a.5.5 0 00.5-.5v-3a.5.5 0 00-.5-.5h-3zm6.5.5A1.5 1.5 0 0110.5 1h3A1.5 1.5 0 0115 2.5v3A1.5 1.5 0 0113.5 7h-3A1.5 1.5 0 019 5.5v-3zm1.5-.5a.5.5 0 00-.5.5v3a.5.5 0 00.5.5h3a.5.5 0 00.5-.5v-3a.5.5 0 00-.5-.5h-3zM1 10.5A1.5 1.5 0 012.5 9h3A1.5 1.5 0 017 10.5v3A1.5 1.5 0 015.5 15h-3A1.5 1.5 0 011 13.5v-3zm1.5-.5a.5.5 0 00-.5.5v3a.5.5 0 00.5.5h3a.5.5 0 00.5-.5v-3a.5.5 0 00-.5-.5h-3zm6.5.5A1.5 1.5 0 0110.5 9h3a1.5 1.5 0 011.5 1.5v3a1.5 1.5 0 01-1.5 1.5h-3A1.5 1.5 0 019 13.5v-3zm1.5-.5a.5.5 0 00-.5.5v3a.5.5 0 00.5.5h3a.5.5 0 00.5-.5v-3a.5.5 0 00-.5-.5h-3z"/>
            </svg>
            {{ t('settings.apiDocs') }}
          </a>
        </div>
        <div class="update-section">
          <button class="check-update-btn" @click="checkForUpdates" :disabled="checkingUpdate">
            {{ checkingUpdate ? t('settings.checking') : t('settings.checkUpdate') }}
          </button>
          <div v-if="updateAvailable" class="update-available">
            <p class="update-message">{{ t('settings.updateAvailable') }}: {{ latestVersion }}</p>
            <button class="download-btn" @click="handleDownload" :disabled="downloading">
              {{ downloading ? t('settings.downloading') : t('settings.downloadUpdate') }}
            </button>
          </div>
          <p v-if="noUpdate" class="no-update">{{ t('settings.isLatest') }}</p>
        </div>
      </div>
    </div>

    <Transition name="modal">
      <div v-if="showLoginModal" class="modal-overlay" @click="showLoginModal = false">
        <div class="modal-content" @click.stop>
          <div class="modal-header">
            <img src="/icon.png" alt="Logo" class="modal-logo" />
            <h3 class="modal-title">{{ t('login.welcome') }}</h3>
            <p class="modal-subtitle">{{ t('login.login') }} {{ t('settings.accountSettings') }}</p>
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
                :placeholder="authTab === 'login' ? t('settings.email') : t('common.nickname')"
                class="auth-input"
              />
              <input 
                v-model="formData.password"
                type="password" 
                placeholder="t('settings.password')"
                class="auth-input"
              />
              <Transition name="field-fade">
                <div v-if="authTab === 'register'" class="email-field">
                  <input 
                    v-model="formData.email"
                    type="email" 
                    placeholder="t('settings.email')"
                    class="auth-input"
                  />
                  <div class="verification-code">
                    <input 
                      v-model="formData.verificationCode"
                      type="text" 
                      placeholder="t('settings.verificationCode')"
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
              <button class="submit-btn" @click="handleSubmit">
                {{ authTab === 'login' ? t('common.login') : t('common.register') }}
              </button>
            </div>
          </Transition>
          
          <Transition name="error-fade">
            <div v-if="errorMessage" class="error-message">
              {{ errorMessage }}
            </div>
          </Transition>
        </div>
      </div>
    </Transition>

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
              <path fill="currentColor" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-2h2v2zm0-4h-2V7h2v6z"/>
            </svg>
            <svg v-else viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z"/>
            </svg>
          </div>
          <span class="toast-message">{{ toast.message }}</span>
        </div>
        
        <div v-if="downloading" key="download-progress" class="toast toast-download">
          <div class="toast-icon">
            <svg viewBox="0 0 24 24" width="20" height="20">
              <path fill="currentColor" d="M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 1.01-.25 1.97-.7 2.8l1.46 1.46C19.54 15.03 20 13.57 20 12c0-4.42-3.58-8-8-8zm0 14c-3.31 0-6-2.69-6-6 0-1.01.25-1.97.7-2.8L5.24 7.74C4.46 8.97 4 10.43 4 12c0 4.42 3.58 8 8 8v3l4-4-4-4v3z"/>
            </svg>
          </div>
          <div class="download-toast-content">
            <span class="toast-message">{{ t('settings.downloadProgress') }} {{ downloadProgress }}%</span>
            <div class="toast-progress-bar">
              <div class="toast-progress-fill" :style="{ width: downloadProgress + '%' }"></div>
            </div>
            <span class="toast-speed">{{ downloadSpeed }}</span>
          </div>
        </div>
      </TransitionGroup>
    </div>

    <Transition name="modal">
      <div v-if="showUpdateConfirm" class="modal-overlay" @click.self="showUpdateConfirm = false">
        <div class="download-modal">
          <div class="download-header">
            <div class="success-icon">
              <svg viewBox="0 0 24 24" width="64" height="64">
                <path fill="currentColor" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm-2 15l-5-5 1.41-1.41L10 14.17l7.59-7.59L19 8l-9 9z"/>
              </svg>
            </div>
            <h3>更新下载完成</h3>
            <p>版本 {{ latestVersion }} 已下载完成</p>
            <p class="file-path">{{ downloadedFilePath }}</p>
          </div>
          <div class="download-actions">
            <button class="download-action-btn btn-cancel" @click="handleCancelUpdate">{{ t('settings.installLater') }}</button>
            <button class="download-action-btn btn-install" @click="handleInstallUpdate">{{ t('settings.installNow') }}</button>
          </div>
        </div>
      </div>
    </Transition>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import { useI18n } from 'vue-i18n'
import { setLanguage } from '../locales'
import apiConfig from '../config/apiConfig'
import { APP_VERSION } from '../version'

const { t, locale } = useI18n()

// 音乐缓存设置
const musicCacheEnabled = ref(localStorage.getItem('musicCacheEnabled') !== 'false')

// 语言设置
const currentLanguage = ref(locale.value)

const languageOptions = [
  { value: 'zh-CN', label: t('settings.languageChinese') },
  { value: 'en-US', label: t('settings.languageEnglish') }
]

const handleLanguageChange = async (newLanguage) => {
  try {
    await setLanguage(newLanguage)
    currentLanguage.value = newLanguage
    showToast(t('settings.language') + ' ' + t('settings.languageSettings') + ' ' + t('common.save'), 'success')
  } catch (error) {
    console.error('切换语言失败:', error)
    showToast(t('common.switchLanguageFailed'), 'error')
  }
}

// 获取系统下载目录路径
const cachePath = ref('~/Downloads/NekoMusic/Music_temp')

// 在组件挂载时获取实际路径
onMounted(async () => {
  console.log('SettingsView mounted, electronAPI:', window.electronAPI)
  console.log('getPath exists:', window.electronAPI?.getPath)
  
  if (window.electronAPI && window.electronAPI.getPath) {
    try {
      console.log('开始获取下载目录...')
      const downloadsPath = await window.electronAPI.getPath('downloads')
      console.log('下载目录:', downloadsPath)
      cachePath.value = `${downloadsPath}/NekoMusic/Music_temp`
      console.log('缓存路径:', cachePath.value)
    } catch (error) {
      console.error('获取下载目录失败:', error)
    }
  } else {
    console.log('electronAPI 或 getPath 不存在')
  }
})

const handleCacheToggle = () => {
  localStorage.setItem('musicCacheEnabled', musicCacheEnabled.value.toString())
  window.dispatchEvent(new CustomEvent('cache-setting-changed', {
    detail: { enabled: musicCacheEnabled.value }
  }))
  showToast(musicCacheEnabled.value ? '音乐缓存已开启' : '音乐缓存已关闭', 'success')
}

// 在外部浏览器中打开链接
const openExternalLink = (url) => {
  // 检测是否在 Electron 环境中
  const isElectron = window.electronAPI || (window.process && window.process.type === 'renderer')
  
  if (isElectron && window.electronAPI && window.electronAPI.openExternal) {
    // 在 Electron 环境中，使用 shell.openExternal 在系统浏览器中打开
    window.electronAPI.openExternal(url)
  } else {
    // 在纯浏览器环境中，使用 window.open
    window.open(url, '_blank', 'noopener,noreferrer')
  }
}

// 检测系统类型
const getOSType = () => {
  const platform = navigator.platform.toLowerCase()
  if (platform.includes('win')) return 'windows'
  if (platform.includes('mac')) return 'mac'
  if (platform.includes('linux')) return 'linux'
  return 'windows'
}

const getDisplayOSType = () => {
  const platform = navigator.platform.toLowerCase()
  if (platform.includes('win')) return 'Windows'
  if (platform.includes('mac')) return 'macOS'
  if (platform.includes('linux')) return 'Linux'
  return 'Unknown'
}

const osType = ref(getOSType())

// 更新检测
const checkingUpdate = ref(false)
const updateAvailable = ref(false)
const noUpdate = ref(false)
const latestVersion = ref('')
const downloadUrl = ref('')
const downloading = ref(false)
const downloadProgress = ref(0)
const downloadSpeed = ref('')
const showDownloadModal = ref(false)
const showUpdateConfirm = ref(false)
const downloadedFilePath = ref('')

// 监听全局下载事件
onMounted(() => {
  const userStr = localStorage.getItem('user')
  if (userStr) {
    try {
      const user = JSON.parse(userStr)
      currentUser.value = user
    } catch (e) {
      console.error('解析用户信息失败:', e)
    }
  }
  
  // 监听来自 App.vue 的下载事件
  window.addEventListener('start-download', (event) => {
    console.log('收到下载事件:', event.detail)
    const { url, version } = event.detail
    downloadUrl.value = url
    latestVersion.value = version
    console.log('开始下载:', url, version)
    handleDownload()
  })
})

// 统一的 API 请求函数
async function apiRequest(url, options = {}) {
  const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
  // 添加时间戳参数防止缓存
  const separator = fullUrl.includes('?') ? '&' : '?'
  const urlWithTimestamp = `${fullUrl}${separator}_t=${Date.now()}`
  
  return fetch(urlWithTimestamp, {
    ...options,
    headers: {
      'Cache-Control': 'no-cache, no-store, must-revalidate',
      'Pragma': 'no-cache',
      'Expires': '0',
      ...(options.headers || {})
    },
    cache: 'no-store'
  })
}

const currentUser = ref(null)
const showLoginModal = ref(false)
const authTab = ref('login')
const errorMessage = ref('')
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
  return countdown.value > 0 ? `${countdown.value}${t('settings.sendCodeAfter')}` : t('settings.getCode')
})

const userAvatar = computed(() => {
  if (currentUser.value && currentUser.value.id) {
    return `https://music.cnmsb.xin/api/user/avatar/${currentUser.value.id}`
  }
  return 'https://music.cnmsb.xin/api/user/avatar/default'
})

const formatDate = (dateStr) => {
  if (!dateStr) return t('settings.unknown')
  const date = new Date(dateStr)
  return date.toLocaleDateString('zh-CN', {
    year: 'numeric',
    month: 'long',
    day: 'numeric'
  })
}

const showToast = (message, type = 'info') => {
  const id = ++toastId
  toasts.value.push({ id, message, type })
  setTimeout(() => {
    toasts.value = toasts.value.filter(t => t.id !== id)
  }, 3000)
}

const checkForUpdates = async () => {
  checkingUpdate.value = true
  updateAvailable.value = false
  noUpdate.value = false
  
  try {
    const response = await apiRequest(apiConfig.UPDATE_CHECK, {
      cache: 'no-store',
      headers: {
        'Cache-Control': 'no-cache, no-store, must-revalidate',
        'Pragma': 'no-cache',
        'Expires': '0'
      }
    })
    const data = await response.json()
    
    if (data.pc && data.pc.pc_ver) {
      const remoteVersion = data.pc.pc_ver
      latestVersion.value = remoteVersion
      
      if (remoteVersion !== APP_VERSION) {
        updateAvailable.value = true
        const osKey = osType.value
        if (data.pc[osKey]) {
          downloadUrl.value = data.pc[osKey].replace('{pc_ver}', remoteVersion)
        } else {
          downloadUrl.value = data.pc.windows.replace('{pc_ver}', remoteVersion)
        }
      } else {
        noUpdate.value = true
      }
    }
  } catch (error) {
    showToast(t('common.checkUpdateFailed'), 'error')
  } finally {
    checkingUpdate.value = false
  }
}

const handleDownload = async () => {
  if (!downloadUrl.value || downloading.value) return
  
  downloading.value = true
  downloadProgress.value = 0
  downloadSpeed.value = ''
  
  try {
    const response = await fetch(downloadUrl.value)
    if (!response.ok) throw new Error(t('settings.downloadFailed'))
    
    const contentLength = response.headers.get('content-length')
    const total = parseInt(contentLength, 10)
    let loaded = 0
    
    const reader = response.body.getReader()
    const chunks = []
    const startTime = Date.now()
    
    while (true) {
      const { done, value } = await reader.read()
      
      if (done) break
      
      chunks.push(value)
      loaded += value.length
      
      if (total) {
        downloadProgress.value = Math.round((loaded / total) * 100)
        
        const elapsed = (Date.now() - startTime) / 1000
        if (elapsed > 0) {
          const speed = loaded / elapsed
          const speedMB = (speed / 1024 / 1024).toFixed(2)
          downloadSpeed.value = `${speedMB} MB/s`
        }
      }
    }
    
    const blob = new Blob(chunks)
    
    if (window.electronAPI && window.electronAPI.saveFile) {
      const fileName = downloadUrl.value.split('/').pop()
      const filePath = await window.electronAPI.saveFile({
        fileName: fileName,
        fileType: 'application/octet-stream',
        suggestedPath: 'tmp'
      })
      
      if (filePath) {
        await window.electronAPI.writeFile(filePath, await blob.arrayBuffer())
        downloadedFilePath.value = filePath
        showUpdateConfirm.value = true
      } else {
        showToast(t('common.downloadCancelled'), 'info')
      }
    } else {
      const link = document.createElement('a')
      link.href = URL.createObjectURL(blob)
      link.download = downloadUrl.value.split('/').pop()
      link.click()
      URL.revokeObjectURL(link.href)
      showToast(t('common.downloadComplete'), 'success')
    }
  } catch (error) {
    showToast(t('common.downloadFailed') + ': ' + error.message, 'error')
  } finally {
    downloading.value = false
    downloadProgress.value = 0
    downloadSpeed.value = ''
  }
}

const handleInstallUpdate = () => {
  if (downloadedFilePath.value && window.electronAPI) {
    window.electronAPI.openFile(downloadedFilePath.value)
    showUpdateConfirm.value = false
  }
}

const handleCancelUpdate = () => {
  showUpdateConfirm.value = false
  downloadedFilePath.value = ''
}

const sendVerificationCode = async () => {
  if (!formData.value.email) {
    showToast(t('common.inputEmail'), 'error')
    return
  }
  
  const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/
  if (!emailRegex.test(formData.value.email)) {
    showToast(t('common.inputValidEmail'), 'error')
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
      showToast(t('common.verifyCodeSent'), 'success')
      startCountdown()
    } else {
      showToast(result.message || t('common.sendVerifyCodeFailed'), 'error')
    }
  } catch (error) {
    showToast(t('common.networkError'), 'error')
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

const handleLogout = () => {
  localStorage.removeItem('user')
  localStorage.removeItem('token')
  localStorage.removeItem('loginTimestamp')
  currentUser.value = null
  window.dispatchEvent(new CustomEvent('user-logout'))
  showToast(t('common.logoutSuccess'), 'success')
}

const handleSubmit = async () => {
  errorMessage.value = ''
  
  if (!formData.value.username || !formData.value.password) {
    errorMessage.value = t('settings.fillUsernameAndPassword')
    return
  }

  if (authTab.value === 'register' && !formData.value.email) {
    errorMessage.value = t('settings.fillEmail')
    return
  }

  if (authTab.value === 'register' && !formData.value.verificationCode) {
    errorMessage.value = t('settings.fillVerificationCode')
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
        throw new Error(t('common.loginFailed'))
      }
      
      const result = await response.json()
      if (result.success && result.data && result.data.user) {
        const user = result.data.user
        const token = result.data.token
        
        console.log('[SettingsView] 登录成功，用户信息:', user)
        localStorage.setItem('user', JSON.stringify(user))
        localStorage.setItem('token', token)
        // 设置登录时间戳，用于检测刚登录后需要刷新的组件
        localStorage.setItem('loginTimestamp', Date.now().toString())
        
        currentUser.value = user
        showLoginModal.value = false
        formData.value = { username: '', password: '', email: '' }
        
        console.log('[SettingsView] 派发 user-login 事件')
        window.dispatchEvent(new CustomEvent('user-login', { detail: user }))
        
        showToast(t('common.loginSuccess'), 'success')
      } else {
        throw new Error(result.message || t('common.loginFailed'))
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
        throw new Error(t('common.registerFailed'))
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
        showToast(t('common.registerSuccess'), 'success')
      } else {
        throw new Error(result.message || t('common.registerFailed'))
      }
    }
  } catch (error) {
    errorMessage.value = error.message || `${authTab.value === 'login' ? t('common.loginFailed') : t('common.registerFailed')}`
    showToast(errorMessage.value, 'error')
  }
}
</script>

<style scoped>
.settings-view {
  height: 100%;
  padding: 20px;
  overflow-y: auto;
}

.settings-container {
  max-width: 800px;
  margin: 0 auto;
}

.settings-container h2 {
  margin-bottom: 20px;
  color: var(--text-dark);
  font-size: 24px;
  font-weight: 600;
}

.settings-section {
  background: var(--bg-white);
  padding: 24px;
  border-radius: 12px;
  box-shadow: var(--shadow-md);
  margin-bottom: 20px;
}

.settings-section h3 {
  margin-bottom: 16px;
  color: var(--text-dark);
  font-size: 16px;
  font-weight: 600;
}

.settings-section p {
  color: var(--text-gray);
  margin: 8px 0;
}

.links-section {
  display: flex;
  gap: 12px;
  margin: 16px 0;
}

.link-btn {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  padding: 8px 16px;
  background: var(--primary-color);
  color: white;
  text-decoration: none;
  border-radius: 8px;
  font-size: 14px;
  transition: all 0.2s ease;
}

.link-btn:hover {
  background: var(--primary-color-dark);
  transform: translateY(-2px);
  box-shadow: var(--shadow-md);
}

.link-btn:active {
  transform: translateY(0);
}

.setting-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 0;
}

.setting-info {
  flex: 1;
}

.setting-label {
  display: block;
  color: var(--text-dark);
  font-size: 14px;
  font-weight: 500;
  margin-bottom: 4px;
}

.setting-desc {
  display: block;
  color: var(--text-light);
  font-size: 12px;
}

.toggle-switch {
  position: relative;
  display: inline-block;
  width: 48px;
  height: 26px;
  margin-left: 16px;
}

.toggle-switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

.toggle-slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  transition: 0.3s;
  border-radius: 26px;
}

.toggle-slider:before {
  position: absolute;
  content: "";
  height: 20px;
  width: 20px;
  left: 3px;
  bottom: 3px;
  background-color: white;
  transition: 0.3s;
  border-radius: 50%;
}

.toggle-switch input:checked + .toggle-slider {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
}

.toggle-switch input:checked + .toggle-slider:before {
  transform: translateX(22px);
}

.toggle-switch input:focus + .toggle-slider {
  box-shadow: 0 0 0 2px rgba(102, 126, 234, 0.3);
}

.cache-path-info {
  margin-top: 12px;
  padding: 12px;
  background: var(--bg-hover);
  border-radius: 8px;
  font-size: 13px;
}

.cache-path-label {
  color: var(--text-gray);
  font-weight: 500;
}

.cache-path-value {
  color: var(--text-dark);
  font-family: 'Courier New', monospace;
  word-break: break-all;
  display: block;
  margin-top: 4px;
}

.language-select {
  padding: 8px 12px;
  border: 1px solid var(--border-input);
  border-radius: 8px;
  background: var(--bg-input);
  color: var(--text-dark);
  font-size: 14px;
  cursor: pointer;
  transition: border-color 0.2s ease;
  min-width: 150px;
}

.language-select:focus {
  outline: none;
  border-color: var(--primary);
}

.language-select:hover {
  border-color: var(--border-focus);
}

.update-section {
  margin-top: 16px;
}

.check-update-btn {
  padding: 10px 20px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border: none;
  border-radius: 8px;
  color: white;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s;
}

.check-update-btn:hover:not(:disabled) {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
}

.check-update-btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.update-available {
  margin-top: 12px;
  padding: 12px;
  background: var(--bg-success);
  border-radius: 8px;
  border-left: 4px solid var(--text-success);
}

.update-message {
  color: var(--text-success);
  font-size: 14px;
  margin: 0 0 8px 0;
}

.download-btn {
  display: inline-block;
  padding: 8px 16px;
  background: #4caf50;
  color: white;
  text-decoration: none;
  border-radius: 6px;
  font-size: 13px;
  transition: all 0.2s;
  border: none;
  cursor: pointer;
}

.download-btn:hover:not(:disabled) {
  background: #45a049;
  transform: translateY(-1px);
}

.download-btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
  transform: none;
}

.no-update {
  color: #4caf50;
  font-size: 14px;
  margin: 12px 0 0 0;
}

.download-modal {
  background: var(--bg-white);
  border-radius: 16px;
  padding: 32px;
  width: 100%;
  max-width: 420px;
  box-shadow: var(--shadow-xl);
}

.download-header {
  text-align: center;
  margin-bottom: 28px;
}

.success-icon {
  color: var(--text-success);
  margin-bottom: 16px;
}

.download-header h3 {
  font-size: 22px;
  color: var(--text-dark);
  margin-bottom: 12px;
  font-weight: 600;
}

.download-header p {
  color: var(--text-gray);
  font-size: 14px;
  margin: 4px 0;
}

.download-header .file-path {
  font-size: 12px;
  color: var(--text-light);
  word-break: break-all;
  margin-top: 8px;
}

.download-actions {
  display: flex;
  gap: 12px;
}

.download-action-btn {
  flex: 1;
  padding: 14px 20px;
  border: none;
  border-radius: 8px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-cancel {
  background: var(--bg-hover);
  color: var(--text-gray);
}

.btn-cancel:hover {
  background: var(--bg-active);
  color: var(--text-dark);
}

.btn-install {
  background: linear-gradient(135deg, #4caf50 0%, #45a049 100%);
  color: white;
}

.btn-install:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(76, 175, 80, 0.4);
}

.account-info {
  display: flex;
  gap: 20px;
  align-items: flex-start;
}

.account-avatar {
  flex-shrink: 0;
}

.account-avatar img {
  width: 80px;
  height: 80px;
  border-radius: 50%;
  object-fit: cover;
  border: 3px solid var(--border-divider);
}

.account-details {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.account-item {
  display: flex;
  justify-content: space-between;
  padding: 10px 0;
  border-bottom: 1px solid var(--border-divider);
}

.account-item:last-child {
  border-bottom: none;
}

.account-item .label {
  color: var(--text-light);
  font-size: 14px;
}

.account-item .value {
  color: var(--text-dark);
  font-size: 14px;
  font-weight: 500;
}

.logout-btn {
  flex-shrink: 0;
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 10px 20px;
  background: var(--bg-hover);
  border: none;
  border-radius: 8px;
  color: var(--text-gray);
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s;
  align-self: flex-start;
  margin-top: 10px;
}

.logout-btn:hover {
  background: var(--bg-active);
  color: var(--text-dark);
}

.no-login {
  text-align: center;
  padding: 40px 20px;
}

.no-login p {
  color: var(--text-light);
  margin-bottom: 16px;
}

.login-btn {
  padding: 10px 30px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  border: none;
  border-radius: 8px;
  color: white;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s;
}

.login-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
}

.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 99999;
  padding: 20px;
  backdrop-filter: blur(4px);
}

.modal-content {
  background: rgba(30, 30, 30, 0.95);
  border-radius: 16px;
  padding: 32px;
  width: 100%;
  max-width: 400px;
  text-align: center;
  box-shadow: 
    0 20px 60px rgba(0, 0, 0, 0.5),
    0 0 0 1px rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(20px);
  position: relative;
  overflow: hidden;
}

.modal-content::before {
  content: '';
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: radial-gradient(circle at 30% 30%, rgba(102, 126, 234, 0.15) 0%, transparent 50%),
                radial-gradient(circle at 70% 70%, rgba(118, 75, 162, 0.15) 0%, transparent 50%);
  pointer-events: none;
}

.modal-header {
  margin-bottom: 24px;
  position: relative;
  z-index: 1;
}

.modal-logo {
  width: 48px;
  height: 48px;
  border-radius: 12px;
  margin-bottom: 16px;
}

.modal-title {
  font-size: 22px;
  color: white;
  margin-bottom: 6px;
  font-weight: 600;
}

.modal-subtitle {
  font-size: 13px;
  color: rgba(255, 255, 255, 0.6);
  margin-bottom: 24px;
}

.modal-tabs {
  display: flex;
  gap: 8px;
  margin-bottom: 24px;
  background: rgba(255, 255, 255, 0.05);
  padding: 4px;
  border-radius: 10px;
  position: relative;
  z-index: 1;
}

.tab-btn {
  flex: 1;
  padding: 10px;
  border: none;
  background: transparent;
  border-radius: 8px;
  color: rgba(255, 255, 255, 0.6);
  font-size: 14px;
  cursor: pointer;
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.tab-btn:hover {
  color: white;
}

.tab-btn.active {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  font-weight: 500;
  box-shadow: 0 2px 8px rgba(102, 126, 234, 0.3);
}

.auth-form {
  display: flex;
  flex-direction: column;
  gap: 14px;
  position: relative;
  z-index: 1;
}

.auth-input {
  padding: 14px 16px;
  border: 2px solid rgba(255, 255, 255, 0.1);
  border-radius: 10px;
  background: rgba(255, 255, 255, 0.05);
  color: white;
  font-size: 14px;
  outline: none;
  transition: all 0.2s;
}

.auth-input::placeholder {
  color: rgba(255, 255, 255, 0.4);
}

.auth-input:hover {
  border-color: rgba(255, 255, 255, 0.2);
}

.auth-input:focus {
  border-color: #667eea;
  box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.2);
}

.submit-btn {
  padding: 14px;
  border: none;
  border-radius: 10px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  font-size: 15px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
  position: relative;
  overflow: hidden;
  margin-top: 8px;
}

.submit-btn::before {
  content: '';
  position: absolute;
  top: 0;
  left: -100%;
  width: 100%;
  height: 100%;
  background: linear-gradient(90deg, transparent, rgba(255, 255, 255, 0.2), transparent);
  transition: left 0.5s;
}

.submit-btn:hover {
  transform: translateY(-2px);
  box-shadow: 0 6px 20px rgba(102, 126, 234, 0.5);
}

.submit-btn:hover::before {
  left: 100%;
}

.submit-btn:active {
  transform: translateY(0);
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
  border-radius: 10px;
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  font-size: 14px;
  cursor: pointer;
  transition: all 0.2s;
  white-space: nowrap;
  min-width: 100px;
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
  transition: transform 0.3s, opacity 0.3s;
}

.modal-enter-from .modal-content,
.modal-leave-to .modal-content {
  opacity: 0;
  transform: scale(0.9);
}

.form-slide-enter-active,
.form-slide-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.form-slide-enter-from {
  opacity: 0;
  transform: translateX(20px);
}

.form-slide-leave-to {
  opacity: 0;
  transform: translateX(-20px);
}

.field-fade-enter-active,
.field-fade-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.field-fade-enter-from {
  opacity: 0;
  transform: translateY(-10px);
}

.field-fade-leave-to {
  opacity: 0;
  transform: translateY(10px);
}

.error-fade-enter-active,
.error-fade-leave-active {
  transition: all 0.2s ease;
}

.error-fade-enter-from,
.error-fade-leave-to {
  opacity: 0;
  transform: translateY(-5px);
}

.toast-container {
  position: fixed;
  top: 20px;
  right: 20px;
  z-index: 999999;
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.toast {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 14px 18px;
  border-radius: 10px;
  color: white;
  font-size: 14px;
  min-width: 280px;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
  backdrop-filter: blur(10px);
}

.toast-success {
  background: rgba(76, 175, 80, 0.9);
  border-left: 4px solid #4caf50;
}

.toast-error {
  background: rgba(244, 67, 54, 0.9);
  border-left: 4px solid #f44336;
}

.toast-info {
  background: rgba(33, 150, 243, 0.9);
  border-left: 4px solid #2196f3;
}

.toast-download {
  background: rgba(102, 126, 234, 0.95);
  border-left: 4px solid #667eea;
  min-width: 320px;
}

.download-toast-content {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 8px;
  flex: 1;
}

.toast-progress-bar {
  width: 100%;
  height: 4px;
  background: rgba(255, 255, 255, 0.2);
  border-radius: 2px;
  overflow: hidden;
}

.toast-progress-fill {
  height: 100%;
  background: linear-gradient(90deg, rgba(255, 255, 255, 0.9) 0%, rgba(255, 255, 255, 0.7) 100%);
  transition: width 0.3s ease;
  border-radius: 2px;
}

.toast-speed {
  font-size: 12px;
  color: rgba(255, 255, 255, 0.8);
}

.toast-icon {
  flex-shrink: 0;
}

.toast-enter-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-enter-from {
  opacity: 0;
  transform: translateX(100px);
}

.toast-leave-active {
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
}

.toast-leave-to {
  opacity: 0;
  transform: translateX(100px);
}
</style>