<template>
  <Layout />
  <UpdateNotification 
    v-if="updateAvailable && updateInfo"
    :version="updateInfo.version"
    :downloadUrl="updateInfo.downloadUrl"
    @ignore="handleIgnoreUpdate"
    @update="handleUpdate"
  />
  
  <!-- 下载进度 Toast -->
  <Transition name="toast">
    <div v-if="downloading" class="download-toast">
      <div class="toast-icon">
        <svg viewBox="0 0 24 24" width="20" height="20">
          <path fill="currentColor" d="M12 4V1L8 5l4 4V6c3.31 0 6 2.69 6 6 0 1.01-.25 1.97-.7 2.8l1.46 1.46C19.54 15.03 20 13.57 20 12c0-4.42-3.58-8-8-8zm0 14c-3.31 0-6-2.69-6-6 0-1.01.25-1.97.7-2.8L5.24 7.74C4.46 8.97 4 10.43 4 12c0 4.42 3.58 8 8 8v3l4-4-4-4v3z"/>
        </svg>
      </div>
      <div class="download-toast-content">
        <span class="toast-message">下载更新中... {{ downloadProgress }}%</span>
        <div class="toast-progress-bar">
          <div class="toast-progress-fill" :style="{ width: downloadProgress + '%' }"></div>
        </div>
        <span class="toast-speed">{{ downloadSpeed }}</span>
      </div>
    </div>
  </Transition>
  
  <!-- 下载完成确认弹窗 -->
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
          <button class="download-action-btn btn-cancel" @click="handleCancelUpdate">稍后安装</button>
          <button class="download-action-btn btn-install" @click="handleInstallUpdate">立即安装</button>
        </div>
      </div>
    </div>
  </Transition>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import Layout from './components/Layout.vue'
import UpdateNotification from './components/UpdateNotification.vue'
import apiConfig from './config/apiConfig'
import { APP_VERSION } from './version'

const updateAvailable = ref(false)
const updateInfo = ref(null)
const downloading = ref(false)
const downloadProgress = ref(0)
const downloadSpeed = ref('')
const latestVersion = ref('')
const showUpdateConfirm = ref(false)
const downloadedFilePath = ref('')

// 检测系统类型
const getOSType = () => {
  const platform = navigator.platform.toLowerCase()
  if (platform.includes('win')) return 'windows'
  if (platform.includes('mac')) return 'mac'
  if (platform.includes('linux')) return 'linux'
  return 'windows'
}

// 统一的 API 请求函数
async function apiRequest(url, options = {}) {
  console.log('API请求:', url)
  
  // 如果是 Electron 环境，使用 IPC 避免跨域问题
  if (window.electronAPI && window.electronAPI.httpRequest) {
    const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
    const separator = fullUrl.includes('?') ? '&' : '?'
    const urlWithTimestamp = `${fullUrl}${separator}_t=${Date.now()}`
    
    const result = await window.electronAPI.httpRequest(urlWithTimestamp, options)
    
    if (result.success) {
      return {
        ok: result.status >= 200 && result.status < 300,
        status: result.status,
        json: async () => JSON.parse(result.data),
        text: async () => result.data,
        headers: new Headers(result.headers)
      }
    } else {
      throw new Error(result.error || '请求失败')
    }
  }
  
  // 开发环境或非 Electron 环境，使用 fetch
  const fullUrl = url.startsWith('http') ? url : `${apiConfig.BASE_URL}${url}`
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

const checkForUpdates = async () => {
  console.log('开始检查更新，当前版本:', APP_VERSION)
  try {
    const response = await apiRequest(apiConfig.UPDATE_CHECK, {
      cache: 'no-store',
      headers: {
        'Cache-Control': 'no-cache, no-store, must-revalidate',
        'Pragma': 'no-cache',
        'Expires': '0'
      }
    })
    console.log('更新检查响应状态:', response.status)
    const data = await response.json()
    console.log('更新数据:', data)
    
    if (data.pc && data.pc.pc_ver) {
      const remoteVersion = data.pc.pc_ver
      console.log('远程版本:', remoteVersion, '本地版本:', APP_VERSION)
      
      if (remoteVersion !== APP_VERSION) {
        console.log('发现新版本!')
        const osType = getOSType()
        let downloadUrl = ''
        
        if (data.pc[osType]) {
          downloadUrl = data.pc[osType].replace('{pc_ver}', remoteVersion)
        } else {
          downloadUrl = data.pc.windows.replace('{pc_ver}', remoteVersion)
        }
        
        updateAvailable.value = true
        updateInfo.value = {
          version: remoteVersion,
          downloadUrl: downloadUrl
        }
        console.log('更新信息已设置:', updateInfo.value)
      } else {
        console.log('当前已是最新版本')
      }
    }
  } catch (error) {
    console.error('检查更新失败:', error)
  }
}

const handleIgnoreUpdate = () => {
  updateAvailable.value = false
  updateInfo.value = null
}

const handleUpdate = () => {
  console.log('handleUpdate 被调用')
  if (updateInfo.value && updateInfo.value.downloadUrl) {
    console.log('触发下载事件:', updateInfo.value)
    handleDownload(updateInfo.value.downloadUrl, updateInfo.value.version)
    handleIgnoreUpdate()
  }
}

const handleDownload = async (url, version) => {
  console.log('开始下载:', url, version)
  if (!url || downloading.value) return
  
  downloading.value = true
  downloadProgress.value = 0
  downloadSpeed.value = ''
  latestVersion.value = version
  
  try {
    const response = await fetch(url)
    if (!response.ok) throw new Error('下载失败')
    
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
      const fileName = url.split('/').pop()
      const filePath = await window.electronAPI.saveFile({
        fileName: fileName,
        fileType: 'application/octet-stream',
        suggestedPath: 'tmp'
      })
      
      if (filePath) {
        await window.electronAPI.writeFile(filePath, await blob.arrayBuffer())
        downloadedFilePath.value = filePath
        showUpdateConfirm.value = true
      }
    } else {
      const link = document.createElement('a')
      link.href = URL.createObjectURL(blob)
      link.download = url.split('/').pop()
      link.click()
      URL.revokeObjectURL(link.href)
    }
  } catch (error) {
    console.error('下载失败:', error)
    alert('下载失败: ' + error.message)
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

onMounted(() => {
  // 延迟1秒后检查更新，避免影响启动速度
  setTimeout(() => {
    checkForUpdates()
  }, 1000)
})
</script>

<style scoped>
/* 下载 Toast 样式 */
.download-toast {
  position: fixed;
  top: 20px;
  right: 20px;
  z-index: 999999;
  display: flex;
  align-items: flex-start;
  gap: 12px;
  padding: 14px 18px;
  border-radius: 10px;
  background: rgba(102, 126, 234, 0.95);
  border-left: 4px solid #667eea;
  color: white;
  font-size: 14px;
  min-width: 320px;
  box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
  backdrop-filter: blur(10px);
}

.toast-icon {
  flex-shrink: 0;
  margin-top: 2px;
}

.download-toast-content {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  gap: 8px;
  flex: 1;
}

.toast-message {
  font-size: 14px;
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

/* 弹窗样式 */
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
  z-index: 999999;
  backdrop-filter: blur(4px);
  padding: 20px;
}

.download-modal {
  background: white;
  border-radius: 16px;
  padding: 32px;
  width: 100%;
  max-width: 420px;
  box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
}

.download-header {
  text-align: center;
  margin-bottom: 28px;
}

.success-icon {
  color: #4caf50;
  margin-bottom: 16px;
}

.download-header h3 {
  font-size: 22px;
  color: #333;
  margin-bottom: 12px;
  font-weight: 600;
}

.download-header p {
  color: #666;
  font-size: 14px;
  margin: 4px 0;
}

.download-header .file-path {
  font-size: 12px;
  color: #999;
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
  background: #f5f5f5;
  color: #666;
}

.btn-cancel:hover {
  background: #e8e8e8;
  color: #333;
}

.btn-install {
  background: linear-gradient(135deg, #4caf50 0%, #45a049 100%);
  color: white;
}

.btn-install:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(76, 175, 80, 0.4);
}

/* 动画 */
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

.modal-enter-active,
.modal-leave-active {
  transition: opacity 0.3s ease;
}

.modal-enter-from,
.modal-leave-to {
  opacity: 0;
}
</style>
