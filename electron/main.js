import { app, BrowserWindow, ipcMain, Tray, Menu, nativeImage, shell, screen } from 'electron'
import path from 'path'
import fs from 'fs'
import { fileURLToPath } from 'url'

// 在 ES 模块中获取 __dirname 的等效值
const __dirname = path.dirname(fileURLToPath(import.meta.url))

// Linux托盘支持检查
if (process.platform === 'linux') {
  console.log('Linux平台检测到，检查托盘支持...')
  console.log('提示：如果托盘图标不显示或右键无反应，请安装系统库：')
  console.log('  sudo apt-get install libayatana-appindicator3-1')
  console.log('  或者设置环境变量：export XDG_CURRENT_DESKTOP=Unity')
}



let win
let lyricsWindow
let tray
app.isQuitting = false  // 声明退出标志

// 启用 GPU 加速，减少 CPU 负担
app.commandLine.appendSwitch('enable-gpu-rasterization')
app.commandLine.appendSwitch('enable-zero-copy')
app.commandLine.appendSwitch('ignore-gpu-blocklist')
app.commandLine.appendSwitch('enable-native-gpu-memory-buffers')
app.commandLine.appendSwitch('enable-gpu-memory-buffer-compositor-resources')

// 禁用不需要的功能，减少资源占用
app.commandLine.appendSwitch('disable-dev-shm-usage')
app.commandLine.appendSwitch('disable-background-networking')
app.commandLine.appendSwitch('disable-background-timer-throttling')
app.commandLine.appendSwitch('disable-renderer-backgrounding')
app.commandLine.appendSwitch('disable-breakpad')
app.commandLine.appendSwitch('disable-component-extensions-with-background-pages')
app.commandLine.appendSwitch('disable-domain-reliability')
app.commandLine.appendSwitch('disable-sync')
app.commandLine.appendSwitch('disable-backgrounding-occluded-windows')
app.commandLine.appendSwitch('disable-ipc-flooding-protection')
app.commandLine.appendSwitch('disable-features', 'Translate')
app.commandLine.appendSwitch('disable-features', 'SpeechSynthesis')
app.commandLine.appendSwitch('disable-features', 'Autofill')

// 防止多实例运行
const gotTheLock = app.requestSingleInstanceLock()

if (!gotTheLock) {
  console.log('已经有实例在运行，退出新实例')
  app.quit()
  process.exit(0)
} else {
  app.on('second-instance', () => {
    // 如果有第二个实例尝试启动，聚焦到现有窗口
    if (win) {
      if (win.isMinimized()) win.restore()
      win.focus()
    }
  })
}

function createWindow() {
  console.log('createWindow: 开始创建窗口')
  // 打包后图标在 app.asar 的 public 目录中，开发环境在 public 目录
  const isDev = process.env.NODE_ENV === 'development'
  const iconPath = isDev
    ? path.join(__dirname, '../public/icon.png')
    : path.join(app.getAppPath(), 'public/icon.png')
  // 打包后 preload.cjs 在 dist-electron 目录下，使用相对路径
  const preloadPath = path.join(__dirname, './preload.cjs')
  console.log('createWindow: 图标路径 =', iconPath)
  console.log('createWindow: preload 路径 =', preloadPath)
  
  win = new BrowserWindow({
    width: 1280,
    height: 720,
    minWidth: 1280,
    minHeight: 720,
    frame: false,
    autoHideMenuBar: true,
    icon: iconPath,
    title: 'Neko云音乐',
    webPreferences: {
      preload: preloadPath,
      nodeIntegration: false,
      contextIsolation: true,
      devTools: false,
      sandbox: false,  // 关闭沙箱以允许 localStorage 访问
      nativeWindowOpen: true,  // 允许使用原生 window.open
    },
    backgroundColor: '#667eea',
  })
  
  // 监听窗口最大化事件
  win.on('maximize', () => {
    win.webContents.send('window-maximized')
  })
  
  // 监听窗口还原事件
  win.on('unmaximize', () => {
    win.webContents.send('window-unmaximized')
  })

  // 禁用 DevTools 和全屏快捷键
  win.webContents.on('before-input-event', (event, input) => {
    // 禁用 DevTools 快捷键
    if (input.control && input.shift && (input.key === 'I' || input.key === 'i')) {
      event.preventDefault()
    }
    if (input.control && (input.key === 'F12' || input.key === 'f12')) {
      event.preventDefault()
    }
    if (input.alt && input.key === 'F12' || input.alt && input.key === 'f12') {
      event.preventDefault()
    }
    // 禁用 F11 全屏快捷键
    if (input.key === 'F11' || input.key === 'f11') {
      event.preventDefault()
    }
  })

  // 拦截所有新窗口的打开，强制在系统浏览器中打开
  win.webContents.setWindowOpenHandler(({ url }) => {
    shell.openExternal(url).catch(err => {
      console.error('Failed to open external URL:', err)
    })
    return { action: 'deny' }
  })

  // 根据环境判断加载开发服务器还是生产文件
  if (isDev || !app.isPackaged) {
    console.log('createWindow: 加载开发服务器 http://localhost:5173')
    win.loadURL('http://localhost:5173')
  } else {
    console.log('createWindow: 加载生产文件')
    const appPath = app.getAppPath()
    const prodPath = path.join(appPath, 'dist/index.html')
    console.log('生产文件路径:', prodPath)
    win.loadFile(prodPath)
  }

  // 只在开发环境打开开发者工具
  if (isDev || !app.isPackaged) {
    win.webContents.openDevTools()
  }

  win.on('close', (event) => {
    if (!app.isQuitting) {
      event.preventDefault()
      win.hide()
    }
  })

  win.on('closed', () => {
    win = null
  })
}

// 创建桌面歌词窗口
function createLyricsWindow() {
  if (lyricsWindow) {
    console.log('歌词窗口已存在，显示窗口')
    lyricsWindow.show()
    lyricsWindow.focus()
    return
  }

  console.log('createLyricsWindow: 开始创建歌词窗口')

  const isDev = process.env.NODE_ENV === 'development' || !app.isPackaged
  let lyricsHtmlPath = path.join(__dirname, './lyrics.html')
  console.log('歌词HTML路径:', lyricsHtmlPath)

  // 检查文件是否存在
  if (!fs.existsSync(lyricsHtmlPath)) {
    console.error('歌词HTML文件不存在:', lyricsHtmlPath)
    // 尝试从 dist-electron 目录加载
    const altPath = path.join(__dirname, '../../dist-electron/lyrics.html')
    if (fs.existsSync(altPath)) {
      console.log('使用备用路径:', altPath)
      lyricsHtmlPath = altPath
    } else {
      console.error('无法找到歌词HTML文件')
      return
    }
  }

  try {
    lyricsWindow = new BrowserWindow({
      width: 500,
      height: 300,  // 增加高度以适应最大字体和控件
      frame: false,
      transparent: true,
      alwaysOnTop: true,
      skipTaskbar: true,
      resizable: false,
      movable: false,
      webPreferences: {
        nodeIntegration: true,
        contextIsolation: false,
        devTools: isDev // 只在开发环境开启DevTools
      },
      show: false,
      backgroundColor: '#00000000'
    })
  } catch (error) {
    console.error('创建歌词窗口失败:', error)
    lyricsWindow = null
    return
  }

  // 设置窗口位置到屏幕中央
  const { width, height } = lyricsWindow.getBounds()
  const { workAreaSize } = screen.getPrimaryDisplay()
  const x = Math.floor((workAreaSize.width - width) / 2)
  const y = Math.floor((workAreaSize.height - height) / 2)
  lyricsWindow.setPosition(x, y)
  console.log('歌词窗口位置:', { x, y })

  lyricsWindow.loadFile(lyricsHtmlPath)

  // 监听加载失败
  lyricsWindow.webContents.on('did-fail-load', (event, errorCode, errorDescription) => {
    console.error('歌词窗口加载失败:', errorCode, errorDescription)
    if (lyricsWindow) {
      lyricsWindow.close()
      lyricsWindow = null
    }
  })

  // 监听窗口无响应
  lyricsWindow.on('unresponsive', () => {
    console.warn('歌词窗口无响应，尝试恢复')
    if (lyricsWindow) {
      lyricsWindow.reload()
    }
  })

  // 监听窗口恢复响应
  lyricsWindow.on('responsive', () => {
    console.log('歌词窗口已恢复响应')
  })

  // 等待窗口加载完成后再显示
  lyricsWindow.webContents.on('did-finish-load', () => {
    console.log('歌词窗口加载完成，准备显示')
    // 加载保存的字体大小
    const lyricsDataPath = path.join(app.getPath('userData'), 'lyrics-window-data.json')
    if (fs.existsSync(lyricsDataPath)) {
      try {
        const lyricsData = JSON.parse(fs.readFileSync(lyricsDataPath, 'utf-8'))
        if (lyricsData.fontSize) {
          lyricsWindow.webContents.send('update-font-size', lyricsData.fontSize)
        }
        // 加载保存的位置（如果有）
        if (lyricsData.position) {
          lyricsWindow.setPosition(lyricsData.position.x, lyricsData.position.y)
        }
      } catch (e) {
        console.error('解析歌词窗口数据失败:', e)
      }
    }
    // 延迟显示，确保窗口完全准备好
    setTimeout(() => {
      if (lyricsWindow && !lyricsWindow.isDestroyed()) {
        lyricsWindow.show()
        lyricsWindow.focus()
        console.log('歌词窗口已显示')
      }
    }, 100)
  })

  lyricsWindow.on('closed', () => {
    console.log('歌词窗口已关闭')
    lyricsWindow = null
  })

  // 添加节流，避免频繁保存位置
  let savePositionTimeout = null
  lyricsWindow.on('moved', () => {
    if (savePositionTimeout) {
      clearTimeout(savePositionTimeout)
    }
    // 延迟500ms保存，等待拖动结束
    savePositionTimeout = setTimeout(() => {
      try {
        if (lyricsWindow && !lyricsWindow.isDestroyed()) {
          const [x, y] = lyricsWindow.getPosition()
          saveLyricsWindowData({ position: { x, y } })
          console.log('歌词窗口移动到:', { x, y })
        }
      } catch (error) {
        console.error('保存歌词窗口位置失败:', error)
      }
    }, 500)
  })

  console.log('歌词窗口创建成功')
}

// 保存歌词窗口数据
function saveLyricsWindowData(data) {
  const lyricsDataPath = path.join(app.getPath('userData'), 'lyrics-window-data.json')
  try {
    let existingData = {}
    if (fs.existsSync(lyricsDataPath)) {
      existingData = JSON.parse(fs.readFileSync(lyricsDataPath, 'utf-8'))
    }
    const updatedData = { ...existingData, ...data }
    fs.writeFileSync(lyricsDataPath, JSON.stringify(updatedData))
  } catch (e) {
    console.error('保存歌词窗口数据失败:', e)
  }
}

// 显示歌词窗口
function showLyricsWindow() {
  console.log('showLyricsWindow: 被调用')
  if (!lyricsWindow) {
    console.log('showLyricsWindow: 创建新窗口')
    createLyricsWindow()
    // 等待窗口创建完成
    return
  }
  console.log('showLyricsWindow: 显示窗口')
  lyricsWindow.show()
  lyricsWindow.focus()
  console.log('showLyricsWindow: 窗口已显示')
}

// 隐藏歌词窗口
function hideLyricsWindow() {
  if (lyricsWindow) {
    lyricsWindow.hide()
  }
}

// 全局状态
let playerState = {
  currentMusic: null,
  isPlaying: false,
  playMode: 'list', // list, single, shuffle
  volume: 80,
  lyricsEnabled: false,
  desktopLyricsEnabled: false
}

function createTray() {
  console.log('createTray: 开始创建托盘')
  
  // 优化图标路径处理，支持asar包
  const getIconPath = (filename) => {
    // 优先从resources目录查找（打包后）
    const resourcesPath = process.resourcesPath
      ? path.join(process.resourcesPath, filename)
      : null
    
    // 其次从app路径查找（开发环境）
    const appPath = path.join(app.getAppPath(), 'public', filename)
    
    // 再次从相对路径查找
    const relativePath = path.join(__dirname, '../public', filename)
    
    // 检查哪个路径存在
    if (resourcesPath && fs.existsSync(resourcesPath)) {
      console.log(`使用resources路径: ${resourcesPath}`)
      return resourcesPath
    } else if (fs.existsSync(appPath)) {
      console.log(`使用app路径: ${appPath}`)
      return appPath
    } else if (fs.existsSync(relativePath)) {
      console.log(`使用相对路径: ${relativePath}`)
      return relativePath
    }
    
    // 默认使用app路径
    console.log(`使用默认app路径: ${appPath}`)
    return appPath
  }
  
  const iconPath = getIconPath('icon.png')
  const trayIcon = nativeImage.createFromPath(iconPath)
  
  // 不调整图标尺寸，让系统自动处理所有平台
  console.log('托盘图标路径:', iconPath)
  console.log('托盘图标是否为空:', trayIcon.isEmpty())
  
  // 移除旧的托盘（如果存在）
  if (tray) {
    tray.destroy()
    tray = null
  }
  
  tray = new Tray(trayIcon)
  
  // 设置工具提示（这对Linux很重要）
  tray.setToolTip('Neko云音乐')

  // 加载图标
  const loadIcons = () => {
    const icons = {}
    const iconList = [
      'tray-previous',
      'tray-play',
      'tray-pause',
      'tray-next',
      'tray-favorite',
      'tray-shuffle',
      'tray-setting',
      'tray-list-loop',
      'tray-single-loop',
      'tray-minimize',
      'tray-lyrics',
      'tray-exit'
    ]

    iconList.forEach(name => {
      try {
        const iconPath = getIconPath(`${name}.png`)
        const icon = nativeImage.createFromPath(iconPath)
        icon.resize({ width: 18, height: 18 })
        icons[name] = icon
      } catch (e) {
        console.warn(`Failed to load icon: ${name}`, e)
      }
    })

    return icons
  }
  
  const icons = loadIcons()
  
  // 获取播放器状态
  const syncPlayerState = async () => {
    if (!win) return
    
    try {
      const musicJson = await win.webContents.executeJavaScript('localStorage.getItem("currentMusic")')
      if (musicJson) {
        playerState.currentMusic = JSON.parse(musicJson)
      } else {
        playerState.currentMusic = null
      }
      
      // 同步播放状态（通过 window 对象获取实时状态）
      const state = await win.webContents.executeJavaScript(`
        (function() {
          const audio = document.querySelector('audio');
          return {
            isPlaying: audio ? !audio.paused : false,
            currentTime: audio ? audio.currentTime : 0,
            duration: audio ? audio.duration : 0
          };
        })()
      `)
      
      if (state) {
        playerState.isPlaying = state.isPlaying
      }
    } catch (e) {
      console.error('同步播放状态失败:', e)
    }
  }
  
  // 更新托盘菜单
  const updateContextMenu = async () => {
    await syncPlayerState()
    
    const music = playerState.currentMusic
    
    // 构建当前播放信息标题
    let playingLabel = '暂无播放'
    if (music) {
      const title = music.title || '未知歌曲'
      const artist = music.artist || '未知艺术家'
      // 截断过长文本
      const displayTitle = title.length > 15 ? title.substring(0, 15) + '...' : title
      playingLabel = `${displayTitle} - ${artist}`
    }
    
    const menuTemplate = [
      // 顶部：当前播放信息
      {
        label: playingLabel,
        enabled: false
      },
      { type: 'separator' },
      
      // 显示窗口
      {
        label: '显示窗口',
        click: () => {
          if (win) {
            win.show()
            win.focus()
          }
        }
      },
      
      // 隐藏窗口
      {
        label: '隐藏窗口',
        click: () => {
          if (win) {
            win.hide()
          }
        }
      },
      { type: 'separator' },
      
      // 退出
      {
        label: '退出',
        click: () => {
          app.isQuitting = true
          app.quit()
        }
      }
    ]
    
    const contextMenu = Menu.buildFromTemplate(menuTemplate)
    
    console.log('托盘菜单已构建，包含', menuTemplate.length, '个菜单项')
    
    // 所有平台统一使用 setContextMenu
    tray.setContextMenu(contextMenu)
    console.log('托盘菜单已设置到托盘对象')
    
    // 设置工具提示，确保托盘图标可见
    tray.setToolTip('Neko云音乐')
    console.log('托盘工具提示已设置')
    
    // 更新提示信息
    if (music) {
      tray.setToolTip(`正在播放: ${music.title} - ${music.artist}`)
    }
  }
  
  // 初始化菜单
  updateContextMenu()
  
  // 监听渲染进程状态变化
  ipcMain.on('player-state-changed', (event, state) => {
    if (state) {
      playerState = { ...playerState, ...state }
      updateContextMenu()
    }
  })
  
  // 监听音乐播放事件
  ipcMain.on('music-play', (event, music) => {
    playerState.currentMusic = music
    playerState.isPlaying = true
    updateContextMenu()
  })
  
  // 监听播放状态变化
  ipcMain.on('play-state-changed', (event, isPlaying) => {
    playerState.isPlaying = isPlaying
    updateContextMenu()
  })

// 托盘图标点击事件（左键）
  tray.on('click', () => {
    console.log('托盘图标被点击')
    if (win) {
      if (win.isVisible()) {
        if (win.isFocused()) {
          win.hide()
        } else {
          win.focus()
        }
      } else {
        win.show()
        win.focus()
      }
    }
  })
  
  // 托盘图标双击事件
  tray.on('double-click', () => {
    console.log('托盘图标被双击')
    if (win) {
      if (win.isVisible()) {
        win.focus()
      } else {
        win.show()
        win.focus()
      }
    }
  })
  
  
  
  // 移除定时器，改为通过 IPC 事件更新托盘菜单
  // setInterval(updateContextMenu, 5000)
}

// 窗口控制 IPC 处理
ipcMain.on('window-minimize', () => {
  if (win) win.minimize()
})

ipcMain.on('window-maximize', () => {
  if (win) {
    if (win.isMaximized()) {
      win.unmaximize()
    } else {
      win.maximize()
    }
  }
})

ipcMain.on('window-close', () => {
  if (win) win.hide()
})

// 歌词窗口控制 IPC 处理
ipcMain.on('show-lyrics-window', () => {
  console.log('IPC: 收到 show-lyrics-window 请求')
  showLyricsWindow()
})

ipcMain.on('hide-lyrics-window', () => {
  hideLyricsWindow()
})

ipcMain.on('toggle-lyrics-window', () => {
  if (lyricsWindow && lyricsWindow.isVisible()) {
    hideLyricsWindow()
  } else {
    showLyricsWindow()
  }
})

ipcMain.on('update-lyrics', (event, { lyric, translation }) => {
  try {
    console.log('主进程: 收到歌词更新请求', { lyric, translation, lyricsWindowExists: !!lyricsWindow })
    if (lyricsWindow && !lyricsWindow.isDestroyed()) {
      console.log('主进程: 发送歌词到窗口')
      lyricsWindow.webContents.send('update-lyrics', { lyric, translation })
    } else {
      console.log('主进程: 歌词窗口不存在或已销毁')
    }
  } catch (error) {
    console.error('更新歌词失败:', error)
  }
})

ipcMain.on('lyrics-window-move', (event, { x, y }) => {
  try {
    if (lyricsWindow && !lyricsWindow.isDestroyed()) {
      // 确保坐标是有效的数字
      const posX = Math.round(Number(x))
      const posY = Math.round(Number(y))
      if (!isNaN(posX) && !isNaN(posY)) {
        lyricsWindow.setPosition(posX, posY)
      }
    }
  } catch (error) {
    console.error('移动歌词窗口失败:', error)
  }
})

ipcMain.on('lyrics-window-reset-position', () => {
  if (lyricsWindow) {
    const defaultPosition = { x: 500, y: 100 }
    lyricsWindow.setPosition(defaultPosition.x, defaultPosition.y)
    saveLyricsWindowData({ position: defaultPosition })
  }
})

ipcMain.on('lyrics-window-toggle-minimize', () => {
  if (lyricsWindow) {
    lyricsWindow.webContents.send('toggle-minimize')
  }
})

ipcMain.on('lyrics-window-update-font-size', (event, size) => {
  if (lyricsWindow) {
    lyricsWindow.webContents.send('update-font-size', size)
    saveLyricsWindowData({ fontSize: size })
  }
})

ipcMain.on('lyrics-window-hide', () => {
  hideLyricsWindow()
  // 通知主窗口更新状态
  if (win) {
    win.webContents.send('lyrics-window-hidden')
  }
})

ipcMain.on('lyrics-window-open-devtools', () => {
  if (lyricsWindow) {
    lyricsWindow.webContents.openDevTools()
  }
})

// 文件保存 IPC 处理
ipcMain.handle('get-path', async (event, name) => {
  return app.getPath(name)
})

ipcMain.handle('open-external', async (event, url) => {
  const { shell } = await import('electron')
  try {
    await shell.openExternal(url)
    return { success: true }
  } catch (error) {
    console.error('Failed to open external URL:', error)
    return { success: false, error: error.message }
  }
})

ipcMain.handle('save-file', async (event, options) => {
  const { fileName, fileType, suggestedPath } = options
  
  let basePath
  
  // 如果 suggestedPath 包含 NekoMusic，使用下载目录作为基础路径
  if (suggestedPath && suggestedPath.includes('NekoMusic')) {
    basePath = app.getPath('downloads')
    if (suggestedPath) {
      basePath = path.join(basePath, suggestedPath)
    }
  } else {
    // 否则使用应用数据目录
    basePath = app.getPath('userData')
    if (suggestedPath) {
      basePath = path.join(basePath, suggestedPath)
    }
  }
  
  // 确保目录存在
  if (!fs.existsSync(basePath)) {
    fs.mkdirSync(basePath, { recursive: true })
  }
  
  // 构建完整文件路径
  const filePath = path.join(basePath, fileName)
  
  return filePath
})

ipcMain.handle('write-file', async (event, filePath, data) => {
  try {
    const buffer = Buffer.from(data)
    fs.writeFileSync(filePath, buffer)
    return { success: true, path: filePath }
  } catch (error) {
    console.error('写入文件失败:', error)
    return { success: false, error: error.message }
  }
})

ipcMain.handle('open-file', async (event, filePath) => {
  try {
    await shell.openPath(filePath)
    return { success: true }
  } catch (error) {
    console.error('打开文件失败:', error)
    return { success: false, error: error.message }
  }
})

ipcMain.handle('http-request', async (event, url, options = {}) => {
  try {
    const response = await fetch(url, {
      ...options,
      headers: {
        'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36',
        ...(options.headers || {})
      }
    })
    
    const data = await response.text()
    return {
      success: true,
      status: response.status,
      data: data,
      headers: Object.fromEntries(response.headers.entries())
    }
  } catch (error) {
    console.error('HTTP请求失败:', error)
    return { success: false, error: error.message }
  }
})

// 下载音乐和歌词到指定目录
ipcMain.handle('download-music-with-lyrics', async (event, musicData) => {
  console.log('下载音乐和歌词 - 接收到的数据:', musicData)
  
  try {
    const { id, title, artist, fileFormat, baseUrl } = musicData
    
    if (!id || !title || !artist || !baseUrl) {
      console.error('缺少必要参数:', { id, title, artist, baseUrl })
      return { success: false, error: '缺少必要参数' }
    }
    
    // 确保下载目录存在
    const downloadsPath = app.getPath('downloads')
    const nekoMusicPath = path.join(downloadsPath, 'NekoMusic')
    console.log('下载目录:', nekoMusicPath)
    
    if (!fs.existsSync(nekoMusicPath)) {
      fs.mkdirSync(nekoMusicPath, { recursive: true })
      console.log('创建下载目录成功')
    }
    
    // 生成文件名（清理特殊字符）
    const sanitizedTitle = title.replace(/[<>:"/\\|?*]/g, '_')
    const sanitizedArtist = artist.replace(/[<>:"/\\|?*]/g, '_')
    const fileName = `${sanitizedArtist} - ${sanitizedTitle}`
    console.log('文件名:', fileName)
    
    // 下载音乐文件
    const musicUrl = `${baseUrl}/api/music/file/${id}`
    console.log('音乐URL:', musicUrl)
    
    const musicResponse = await fetch(musicUrl)
    console.log('音乐响应状态:', musicResponse.status)
    
    if (!musicResponse.ok) {
      throw new Error(`下载音乐文件失败: ${musicResponse.status}`)
    }
    
    const musicBuffer = await musicResponse.arrayBuffer()
    const musicExt = fileFormat || 'mp3'
    const musicFilePath = path.join(nekoMusicPath, `${fileName}.${musicExt}`)
    fs.writeFileSync(musicFilePath, Buffer.from(musicBuffer))
    console.log('音乐文件保存成功:', musicFilePath)
    
    // 下载歌词文件
    let lyricsFilePath = null
    try {
      const lyricsUrl = `${baseUrl}/api/music/lyrics/${id}`
      console.log('歌词URL:', lyricsUrl)
      
      const lyricsResponse = await fetch(lyricsUrl)
      console.log('歌词响应状态:', lyricsResponse.status)
      
      if (lyricsResponse.ok) {
        const lyricsData = await lyricsResponse.json()
        console.log('歌词数据:', lyricsData)
        
        if (lyricsData.success && lyricsData.data) {
          lyricsFilePath = path.join(nekoMusicPath, `${fileName}.lrc`)
          fs.writeFileSync(lyricsFilePath, lyricsData.data, 'utf-8')
          console.log('歌词文件保存成功:', lyricsFilePath)
        } else {
          console.log('没有歌词数据')
        }
      } else {
        console.log('歌词不可用')
      }
    } catch (error) {
      console.warn('下载歌词失败:', error)
    }
    
    console.log('下载完成:', { musicPath: musicFilePath, lyricsPath: lyricsFilePath })
    
    return {
      success: true,
      musicPath: musicFilePath,
      lyricsPath: lyricsFilePath
    }
  } catch (error) {
    console.error('下载音乐和歌词失败:', error)
    return { success: false, error: error.message }
  }
})

app.on('ready', () => {
  // 如果已经退出，不创建窗口
  if (app.isQuitting) {
    console.log('应用已退出，跳过窗口创建')
    return
  }
  
  // 如果窗口已存在，不重复创建
  if (win) {
    console.log('窗口已存在，显示窗口')
    win.show()
    win.focus()
    return
  }
  
  console.log('创建新窗口，NODE_ENV:', process.env.NODE_ENV)
  createWindow()
  createTray()
})

app.on('will-quit', () => {
  // 清理资源
  if (lyricsWindow) {
    lyricsWindow.close()
  }
})

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', () => {
  if (win === null) {
    createWindow()
  } else {
    win.show()
  }
})