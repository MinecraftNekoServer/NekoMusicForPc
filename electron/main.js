import { app, BrowserWindow, ipcMain, Tray, Menu, nativeImage, shell } from 'electron'
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
let tray
app.isQuitting = false  // 声明退出标志

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
      devTools: true,
      sandbox: false,  // 关闭沙箱以允许 localStorage 访问
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

  // 打开开发者工具（开发和生产模式都打开）
  //win.webContents.openDevTools()

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
    
    // Linux特殊处理：先设置一个简单的菜单
    if (process.platform === 'linux') {
      // Linux下尝试不设置contextMenu，改用手动弹出的方式
      console.log('Linux平台：不设置自动菜单，使用手动弹出方式')
    } else {
      // Windows和macOS使用正常的contextMenu
      tray.setContextMenu(contextMenu)
      console.log('托盘菜单已设置到托盘对象')
    }
    
    // 设置工具提示，确保托盘图标可见
    tray.setToolTip('Neko云音乐')
    console.log('托盘工具提示已设置')
    
    // 更新提示信息
    if (music) {
      tray.setToolTip(`正在播放: ${music.title} - ${music.artist}`)
    }
    
    // Linux下保存菜单引用用于手动弹出
    if (process.platform === 'linux') {
      tray.linuxContextMenu = contextMenu
      console.log('Linux平台：菜单已保存用于手动弹出')
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
  
  // 托盘图标右键事件（Linux特别处理）
  tray.on('right-click', (event) => {
    console.log('托盘图标右键被点击，平台:', process.platform)
    if (process.platform === 'linux') {
      // Linux下手动弹出菜单
      if (tray.linuxContextMenu) {
        console.log('Linux平台：手动弹出菜单')
        tray.linuxContextMenu.popup({ window: win })
      } else {
        console.error('Linux平台：找不到菜单引用')
      }
    } else {
      // 其他平台使用默认行为
      tray.popUpContextMenu()
    }
  })
  
  // 监听鼠标按下事件（备用方案）
  tray.on('mouse-down', (event) => {
    console.log('托盘图标鼠标按下事件:', event, 'buttons:', event.buttons)
    if (process.platform === 'linux' && event.buttons === 2) {
      // 右键按下时立即显示菜单
      console.log('Linux平台：右键按下，准备弹出菜单')
      if (tray.linuxContextMenu) {
        tray.linuxContextMenu.popup({ window: win })
      }
    }
  })
  
  // 定期同步状态
  setInterval(updateContextMenu, 5000)
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

// 文件保存 IPC 处理
ipcMain.handle('save-file', async (event, options) => {
  const { fileName, fileType, suggestedPath } = options
  
  // 获取应用运行目录
  let basePath = app.getPath('userData')
  if (suggestedPath) {
    basePath = path.join(basePath, suggestedPath)
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