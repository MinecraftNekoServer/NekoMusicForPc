<template>
  <Transition name="notification">
    <div class="update-notification" v-if="show">
      <div class="notification-content">
        <div class="notification-icon">
          <svg viewBox="0 0 24 24" width="48" height="48">
            <path fill="currentColor" d="M12 2C6.48 2 2 6.48 2 12s4.48 10 10 10 10-4.48 10-10S17.52 2 12 2zm1 15h-2v-6h2v6zm0-8h-2V7h2v2z"/>
          </svg>
        </div>
        <div class="notification-text">
          <h3>发现新版本</h3>
          <p>当前版本: {{ currentVersion }}</p>
          <p class="new-version">新版本: {{ version }}</p>
        </div>
        <div class="notification-actions">
          <button class="btn-ignore" @click="handleIgnore">稍后提醒</button>
          <button class="btn-update" @click="handleUpdate">立即更新</button>
        </div>
      </div>
    </div>
  </Transition>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { APP_VERSION } from '../version'

const props = defineProps({
  version: {
    type: String,
    required: true
  },
  downloadUrl: {
    type: String,
    required: true
  }
})

const emit = defineEmits(['ignore', 'update'])

const show = ref(false)
const currentVersion = ref(APP_VERSION)

onMounted(() => {
  setTimeout(() => {
    show.value = true
  }, 100)
})

const handleIgnore = () => {
  emit('ignore')
  show.value = false
}

const handleUpdate = () => {
  console.log('UpdateNotification handleUpdate 被调用')
  emit('update')
  show.value = false
}
</script>

<style scoped>
.update-notification {
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

.notification-content {
  background: rgba(30, 30, 30, 0.95);
  border-radius: 16px;
  padding: 32px;
  max-width: 400px;
  width: 100%;
  text-align: center;
  box-shadow: 
    0 20px 60px rgba(0, 0, 0, 0.5),
    0 0 0 1px rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(20px);
  position: relative;
  overflow: hidden;
}

.notification-content::before {
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

.notification-icon {
  color: #667eea;
  margin-bottom: 20px;
  position: relative;
  z-index: 1;
}

.notification-text {
  margin-bottom: 28px;
  position: relative;
  z-index: 1;
}

.notification-text h3 {
  font-size: 22px;
  color: white;
  margin-bottom: 12px;
  font-weight: 600;
}

.notification-text p {
  color: rgba(255, 255, 255, 0.7);
  font-size: 14px;
  margin: 6px 0;
}

.notification-text .new-version {
  color: #667eea;
  font-size: 16px;
  font-weight: 500;
  margin-top: 8px;
}

.notification-actions {
  display: flex;
  gap: 12px;
  position: relative;
  z-index: 1;
}

.notification-actions button {
  flex: 1;
  padding: 12px 20px;
  border: none;
  border-radius: 10px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s;
}

.btn-ignore {
  background: rgba(255, 255, 255, 0.1);
  color: rgba(255, 255, 255, 0.7);
}

.btn-ignore:hover {
  background: rgba(255, 255, 255, 0.15);
  color: white;
}

.btn-update {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
}

.btn-update:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
}

.notification-enter-active {
  transition: opacity 0.3s ease;
}

.notification-enter-from {
  opacity: 0;
}

.notification-enter-active .notification-content {
  transition: transform 0.3s, opacity 0.3s;
}

.notification-enter-from .notification-content {
  opacity: 0;
  transform: scale(0.9);
}

.notification-leave-active {
  transition: opacity 0.3s ease;
}

.notification-leave-to {
  opacity: 0;
}

.notification-leave-active .notification-content {
  transition: transform 0.3s, opacity 0.3s;
}

.notification-leave-to .notification-content {
  opacity: 0;
  transform: scale(0.9);
}
</style>