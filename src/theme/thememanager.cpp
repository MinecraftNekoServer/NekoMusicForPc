/**
 * @file thememanager.cpp
 * @brief 主题管理器实现
 */

#include "thememanager.h"
#include <QFile>
#include <QApplication>
#include <QStyleHints>

namespace Theme
{

ThemeManager &ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_settings()
{
    loadSettings();
}

ThemeMode ThemeManager::currentMode() const
{
    return m_mode;
}

void ThemeManager::setMode(ThemeMode mode)
{
    if (m_mode == mode)
        return;

    m_mode = mode;
    saveSettings();
    emit themeChanged(mode);
}

QString ThemeManager::currentStyleSheet() const
{
    QString stylePath;
    
    switch (m_mode) {
    case System:
        // 检查系统主题
        if (QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
            stylePath = ":/style.qss";
        } else {
            stylePath = ":/style-light.qss";
        }
        break;
    case Dark:
        stylePath = ":/style.qss";
        break;
    case Light:
        stylePath = ":/style-light.qss";
        break;
    }
    
    return loadStyleSheet(stylePath);
}

bool ThemeManager::isDarkMode() const
{
    switch (m_mode) {
    case System:
        return QApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    case Dark:
        return true;
    case Light:
        return false;
    }
    return true;
}

void ThemeManager::loadSettings()
{
    m_mode = static_cast<ThemeMode>(m_settings.value("themeMode", System).toInt());
}

void ThemeManager::saveSettings()
{
    m_settings.setValue("themeMode", static_cast<int>(m_mode));
}

QString ThemeManager::loadStyleSheet(const QString &path) const
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString::fromUtf8(f.readAll());
    return QString();
}

} // namespace Theme