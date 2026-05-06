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
#include <QSystemTrayIcon>
#include "core/musicinfo.h"
#include "theme/thememanager.h"

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
class PlayerPage;
class QMenu;
class PlaylistDetailPage;
class AddToPlaylistDialog;
class PlaylistPanel;
class SearchPage;
class ApiClient;
class UpdateChecker;
class UpdateDialog;
class SearchPage;
class DesktopLrc;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onTrayPrevious();
    void onTrayPlayPause();
    void onTrayNext();
    void onTrayShow();
    void onTrayQuit();

private:
    void setupUi();
    void loadStyleSheet();
    void applyTheme();
    void switchPage(QWidget *target);
    void showMusicListPage(bool isHot);
    void showPlaylistDetailPage(int localId);
    void playMusicById(int musicId, const QString &title, const QString &artist, const QString &coverUrl = QString());
    void playMusicFromInfo(const MusicInfo &info);
    void createTrayIcon();
    void createPlaylist();
    void showAddToPlaylistDialog(const MusicInfo &music);
    void togglePlaylistPanel();
    void playMusicFromPlaylist(int musicId);
    void playNext();
    void playPrevious();
    void toggleFavorite(int musicId);
    /** @param showNoUpdateToast 为 true 时表示用户从设置页手动检查，已是最新版本时弹出 Toast */
    void checkForUpdates(bool showNoUpdateToast = false);

private:
    bool checkIsFavorited(int musicId);
    void loadFavoritesCache();
    void disconnectDownloader();

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
    PlayerPage *m_playerPage = nullptr;
    PlaylistDetailPage *m_playlistDetailPage = nullptr;
    SearchPage *m_searchPage = nullptr;
    PlaylistPanel *m_playlistPanel = nullptr;
    PlayerBar *m_playerBar = nullptr;
    QWidget *m_midWidget = nullptr;
    QStackedWidget *m_stack = nullptr;
    PlayerEngine *m_engine = nullptr;
    MusicDownloader *m_downloader = nullptr;
    ApiClient *m_apiClient = nullptr;
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    QList<int> m_favoritesCache;  // 缓存已收藏的音乐ID
    UpdateChecker *m_updateChecker = nullptr;
    UpdateDialog *m_updateDialog = nullptr;
    DesktopLrc *m_desktopLrc = nullptr;
    
    // Download state
    bool m_isDownloading = false;
    /** 每次切歌递增；延后回调里若与当前不一致则丢弃，避免叠多个 singleShot 播错文件。 */
    quint64 m_enginePlaySeq = 0;
    /** 当前曲是否已因 bufferReady 从 .part 起播（用于 downloadFinished 时是否带进度切到正式文件）。 */
    int m_lastBufferedMusicId = 0;

    // Downloader signal connections
    QMetaObject::Connection m_finishedConn;
    QMetaObject::Connection m_errorConn;
    QMetaObject::Connection m_bufferConn;
    QMetaObject::Connection m_progressConn;
};
