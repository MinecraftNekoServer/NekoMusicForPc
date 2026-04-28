#pragma once

/**
 * @file mainwindow.h
 * @brief 主窗口 — 日系动漫风
 *
 * 无边框窗口：
 * TitleBar(56) + Sidebar(240) | HomePage + PlayerBar(80)
 */

#include <QMainWindow>
#include <QStackedWidget>

class QCloseEvent;
class TitleBar;
class Sidebar;
class HomePage;
class SettingsPage;
class FavoritesPage;
class RecentPage;
class PlayerBar;
class PlayerEngine;
class MusicDownloader;
class MusicListPage;
class UploadPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void paintEvent(QPaintEvent *) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void setupUi();
    void loadStyleSheet();
    void switchPage(QWidget *target);
    void showMusicListPage(bool isHot);
    void playMusicById(int musicId, const QString &title, const QString &artist, const QString &coverUrl = QString());

    bool m_switching = false;
    TitleBar *m_titleBar = nullptr;
    Sidebar *m_sidebar = nullptr;
    HomePage *m_homePage = nullptr;
    SettingsPage *m_settingsPage = nullptr;
    FavoritesPage *m_favoritesPage = nullptr;
    RecentPage *m_recentPage = nullptr;
    MusicListPage *m_hotMusicPage = nullptr;
    MusicListPage *m_latestMusicPage = nullptr;
    UploadPage *m_uploadPage = nullptr;
    PlayerBar *m_playerBar = nullptr;
    QStackedWidget *m_stack = nullptr;
    PlayerEngine *m_engine = nullptr;
    MusicDownloader *m_downloader = nullptr;
};
