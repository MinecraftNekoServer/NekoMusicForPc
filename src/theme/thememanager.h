#pragma once

/**
 * @file thememanager.h
 * @brief 主题管理器
 */

#include <QObject>
#include <QSettings>

namespace Theme
{

enum ThemeMode
{
    System = 0,  // 跟随系统
    Dark = 1,    // 暗色模式
    Light = 2    // 亮色模式
};

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager &instance();

    ThemeMode currentMode() const;
    void setMode(ThemeMode mode);
    
    QString currentStyleSheet() const;
    bool isDarkMode() const;

signals:
    void themeChanged(ThemeMode mode);

private:
    ThemeManager(QObject *parent = nullptr);
    ThemeManager(const ThemeManager &) = delete;
    ThemeManager &operator=(const ThemeManager &) = delete;

    void loadSettings();
    void saveSettings();
    QString loadStyleSheet(const QString &path) const;

    ThemeMode m_mode = System;
    QSettings m_settings;
};

} // namespace Theme