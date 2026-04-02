import { createI18n } from 'vue-i18n'
import zhCN from './zh-CN'
import enUS from './en-US'

// 从 localStorage 获取保存的语言设置，默认为中文
const savedLanguage = localStorage.getItem('app-language') || 'zh-CN'

const i18n = createI18n({
  legacy: false,
  locale: savedLanguage,
  fallbackLocale: 'zh-CN',
  messages: {
    'zh-CN': zhCN,
    'en-US': enUS
  }
})

// 切换语言
export function setLanguage(locale) {
  i18n.global.locale.value = locale
  localStorage.setItem('app-language', locale)
}

// 获取当前语言
export function getCurrentLanguage() {
  return i18n.global.locale.value
}

export default i18n