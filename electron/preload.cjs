const { contextBridge, ipcRenderer } = require('electron')

// 暴露安全的 API 给渲染进程
contextBridge.exposeInMainWorld('electronAPI', {
  // 窗口控制
  minimize: () => ipcRenderer.send('window-minimize'),
  maximize: () => ipcRenderer.send('window-maximize'),
  close: () => ipcRenderer.send('window-close'),
  
  // 监听窗口状态变化
  onWindowMaximized: (callback) => ipcRenderer.on('window-maximized', callback),
  onWindowUnmaximized: (callback) => ipcRenderer.on('window-unmaximized', callback),
  
  // 播放器状态通知
  notifyPlayerState: (state) => ipcRenderer.send('player-state-changed', state),
  
  // 文件操作
  saveFile: (options) => ipcRenderer.invoke('save-file', options),
  writeFile: (filePath, data) => ipcRenderer.invoke('write-file', filePath, data),
  openFile: (filePath) => ipcRenderer.invoke('open-file', filePath),
  
  // HTTP 请求（用于处理跨域）
  httpRequest: (url, options) => ipcRenderer.invoke('http-request', url, options),
  
  // 其他可以暴露的 API
  platform: process.platform,
  arch: process.arch
})

// 监听主进程事件
ipcRenderer.on('tray-previous', () => {
  window.dispatchEvent(new CustomEvent('tray-previous'))
})

ipcRenderer.on('tray-play-pause', () => {
  window.dispatchEvent(new CustomEvent('tray-play-pause'))
})

ipcRenderer.on('tray-next', () => {
  window.dispatchEvent(new CustomEvent('tray-next'))
})

ipcRenderer.on('tray-favorite', () => {
  window.dispatchEvent(new CustomEvent('tray-favorite'))
})

ipcRenderer.on('tray-set-play-mode', (event, mode) => {
  window.dispatchEvent(new CustomEvent('tray-set-play-mode', { detail: mode }))
})

ipcRenderer.on('tray-toggle-desktop-lyrics', (event, enabled) => {
  window.dispatchEvent(new CustomEvent('tray-toggle-desktop-lyrics', { detail: enabled }))
})

ipcRenderer.on('navigate-to-settings', () => {
  window.dispatchEvent(new CustomEvent('navigate-to-settings'))
})

// 监听主进程事件
window.addEventListener('DOMContentLoaded', () => {
  console.log('Electron preload script loaded')
})