<template>
  <div class="home-view">
    <div class="welcome-section">
      <h1 class="welcome-title">
        <span class="greeting">{{ greeting }}</span>
      </h1>
      <p class="welcome-subtitle">发现好音乐，享受每一刻</p>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'

const greeting = computed(() => {
  const hour = new Date().getHours()
  if (hour < 6) return '夜深了'
  if (hour < 12) return '早上好'
  if (hour < 18) return '下午好'
  return '晚上好'
})
</script>

<style scoped>
.home-view {
  height: 100%;
  overflow-y: auto;
  padding: 32px;
  animation: fadeIn 0.5s ease;
}

@keyframes fadeIn {
  from {
    opacity: 0;
    transform: translateY(20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.welcome-section {
  text-align: center;
  margin-bottom: 40px;
  padding: 40px 20px;
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.1) 0%, rgba(118, 75, 162, 0.1) 100%);
  border-radius: var(--radius-xl);
  position: relative;
  overflow: hidden;
}

.welcome-section::before {
  content: '';
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: radial-gradient(circle, rgba(102, 126, 234, 0.1) 0%, transparent 50%);
  animation: rotate 20s linear infinite;
}

@keyframes rotate {
  from { transform: rotate(0deg); }
  to { transform: rotate(360deg); }
}

.welcome-title {
  margin: 0;
  font-size: 42px;
  font-weight: 800;
  color: var(--text-primary);
  position: relative;
  z-index: 1;
}

.greeting {
  background: var(--gradient-primary);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  background-clip: text;
}

@keyframes bounce {
  0%, 100% { transform: translateY(0); }
  50% { transform: translateY(-10px); }
}

.welcome-subtitle {
  margin-top: 12px;
  font-size: 16px;
  color: var(--text-secondary);
  position: relative;
  z-index: 1;
}

/* 滚动条样式 */
.home-view::-webkit-scrollbar {
  width: 8px;
}

.home-view::-webkit-scrollbar-track {
  background: transparent;
}

.home-view::-webkit-scrollbar-thumb {
  background: rgba(0, 0, 0, 0.1);
  border-radius: 4px;
}

.home-view::-webkit-scrollbar-thumb:hover {
  background: rgba(0, 0, 0, 0.2);
}
</style>