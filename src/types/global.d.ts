declare const __APP_VERSION__: string

// Electron API 类型声明
interface ElectronAPI {
  // 窗口控制
  minimize: () => void
  maximize: () => void
  close: () => void
  
  // 监听窗口状态变化
  onWindowMaximized: (callback: () => void) => void
  onWindowUnmaximized: (callback: () => void) => void
  
  // 播放器状态通知
  notifyPlayerState: (state: any) => void
  
  // 文件操作
  saveFile: (options: { fileName: string; fileType: string; suggestedPath?: string }) => Promise<string>
  writeFile: (filePath: string, data: any) => Promise<{ success: boolean; path?: string; error?: string }>
  openFile: (filePath: string) => Promise<{ success: boolean; error?: string }>
  
  // HTTP 请求
  httpRequest: (url: string, options?: RequestInit) => Promise<{ success: boolean; status?: number; data?: string; headers?: any; error?: string }>
  
  // 下载音乐和歌词
  downloadMusicWithLyrics: (musicData: {
    id: number
    title: string
    artist: string
    fileFormat?: string
    baseUrl: string
  }) => Promise<{ success: boolean; musicPath?: string; lyricsPath?: string; error?: string }>
  
  // 获取系统路径
  getPath: (name: string) => Promise<string>
  
  // 系统信息
  platform: string
  arch: string
}

declare interface Window {
  electronAPI?: ElectronAPI
}