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
      // 生产环境完全禁用 vueDevTools
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
          pure_funcs: ['console.log', 'console.info', 'console.debug', 'console.warn', 'console.trace', 'console.error'],  // 移除指定的函数
          dead_code: true,  // 移除死代码
          unused: true,  // 移除未使用的变量
          passes: 2,  // 多次压缩
          unsafe: true,  // 激进优化
          unsafe_comps: true,
          unsafe_Function: true,
          unsafe_math: true,
          unsafe_proto: true,
          unsafe_regexp: true,
          conditionals: true,
          comparisons: true,
          evaluate: true,
          booleans: true,
          loops: true,
          keep_fargs: false,
          hoist_funs: true,
          hoist_vars: true,
          if_return: true,
          join_vars: true,
          side_effects: true,
          negate_iife: true,
          sequences: true,
          switches: true,
          typeofs: true,
        },
        mangle: {
          safari10: true,  // 兼容 Safari 10
          properties: {
            regex: /^_/,  // 混淆以 _ 开头的属性
          },
          toplevel: true,  // 混淆顶级作用域
          keep_classnames: false,
          keep_fnames: false,
        },
        format: {
          comments: false,  // 移除注释
          beautify: false,
          ecma: 2020,
        },
        ecma: 2020,
        module: true,
        toplevel: true,
      },
      
      // Chunk 优化
      rollupOptions: {
        output: {
          manualChunks: {
            'vue-vendor': ['vue', 'vue-router'],  // 将 Vue 相关代码打包到一个 chunk
          },
          // 更紧凑的 chunk 文件名
          chunkFileNames: 'assets/js/[name]-[hash].js',
          entryFileNames: 'assets/js/[name]-[hash].js',
          assetFileNames: 'assets/[ext]/[name]-[hash].[ext]',
          // 优化 chunk 拆分策略
          inlineDynamicImports: false,
          compact: true,
        },
        // 外部依赖排除
        external: [],
        treeshake: {
          moduleSideEffects: false,
          propertyReadSideEffects: false,
          unknownGlobalSideEffects: false,
        },
      },
      
      // Chunk 大小警告限制（KB）
      chunkSizeWarningLimit: 500,
      
      // 启用 CSS 代码分割
      cssCodeSplit: true,
      cssMinify: true,
      
      // 构建目标
      target: 'es2020',
      
      // 资源内联限制
      assetsInlineLimit: 1024,
      
      // 源码映射
      sourcemap: false,
      
      // 报告压缩
      reportCompressedSize: false,
      
      // 优化模块预构建
      optimizeDeps: {
        include: ['vue', 'vue-router'],
        exclude: []
      }
    },
    clearScreen: false,
  }
})
