import { fileURLToPath, URL } from 'node:url'

import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import electron from 'vite-plugin-electron'
import renderer from 'vite-plugin-electron-renderer'

// https://vite.dev/config/
export default defineConfig(({ mode }) => {
  const isProd = mode === 'production'
  
  return {
    plugins: [
      vue(),
      // 生产环境移除 vueDevTools，减少打包体积
      ...(isProd ? [] : [import('vite-plugin-vue-devtools').then(m => m.default)]),
      electron([
        {
          // 主进程入口
          entry: 'electron/main.js',
          onstart({ startup }) {
            startup()
          },
          vite: {
            build: {
              outDir: 'dist-electron',
              rollupOptions: {
                external: ['electron']
              }
            }
          }
        },
        // 注意：preload 脚本不通过 vite 构建，而是在构建后直接复制
      ]),
      renderer()
    ],
    resolve: {
      alias: {
        '@': fileURLToPath(new URL('./src', import.meta.url))
      },
    },
    server: {
      proxy: {
        '/api': {
          target: 'https://music.cnmsb.xin',
          changeOrigin: true,
          configure: (proxy, options) => {
            proxy.on('proxyReq', (proxyReq, req, res) => {
              if (req.headers['authorization']) {
                proxyReq.setHeader('Authorization', req.headers['authorization'])
              }
            })
          },
          rewrite: (path) => path
        },
        '/version.json': {
          target: 'https://music.cnmsb.xin',
          changeOrigin: true,
          rewrite: (path) => path
        }
      },
      headers: {
        'Cache-Control': 'no-cache, no-store, must-revalidate',
        'Pragma': 'no-cache',
        'Expires': '0'
      }
    },
    build: {
      outDir: 'dist',
      emptyOutDir: true,
      base: './',  // 使用相对路径，确保打包后能正确加载资源
      
      // 代码压缩
      minify: 'terser',
      terserOptions: {
        compress: {
          drop_console: true,  // 移除 console.log
          drop_debugger: true,  // 移除 debugger
          pure_funcs: ['console.log', 'console.info', 'console.debug'],  // 移除指定的函数
        },
        format: {
          comments: false,  // 移除注释
        },
      },
      
      // Chunk 优化
      rollupOptions: {
        output: {
          manualChunks: {
            'vue-vendor': ['vue', 'vue-router'],  // 将 Vue 相关代码打包到一个 chunk
          'electron-vendor': ['electron']  // Electron 相关代码
          // 其他代码会自动分包
          },
          // 更紧凑的 chunk 文件名
          chunkFileNames: 'assets/js/[name]-[hash].js',
          entryFileNames: 'assets/js/[name]-[hash].js',
          assetFileNames: 'assets/[ext]/[name]-[hash].[ext]'
        },
      },
      
      // Chunk 大小警告限制（KB）
      chunkSizeWarningLimit: 1000,
      
      // 启用 CSS 代码分割
      cssCodeSplit: true,
      
      // 构建目标
      target: 'esnext',
    },
    clearScreen: false,
  }
})
