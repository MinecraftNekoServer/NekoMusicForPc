/**
 * @file mainwindow.cpp
 * @brief 主窗口实现 — 日系动漫风
 *
 * 无边框 + 自定义标题栏拖拽。
 * 深夜紫黑渐变背景，全层级毛玻璃。
 */

#include "mainwindow.h"
#include "ui/titlebar.h"
#include "ui/sidebar.h"
#include "ui/homepage.h"
#include "ui/settingspage.h"
#include "ui/favoritespage.h"
#include "ui/recentpage.h"
#include "ui/playerbar.h"
#include "ui/logindialog.h"
#include "ui/musiclistpage.h"
#include "ui/uploadpage.h"
#include "ui/playerpage.h"
#include "ui/playlistdetailpage.h"
#include "ui/searchpage.h"
#include "ui/addtoplaylistdialog.h"
#include "ui/playlistpanel.h"
#include "ui/toast.h"
#include "ui/updatedialog.h"
#include "ui/searchpage.h"
#include "ui/desktoplrc.h"
#include <QSettings>
#include "core/playerengine.h"
#include "core/i18n.h"
#include "core/apiclient.h"
#include "core/musicdownloader.h"
#include "core/usermanager.h"
#include "core/playlistdb.h"
#include "core/playlistmanager.h"
#include "core/updatechecker.h"
#include "theme/theme.h"
#include "theme/thememanager.h"
#include "ui/glasspaint.h"
#include "version.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <memory>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QFile>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QGraphicsOpacityEffect>
#include <QCloseEvent>
#include <QAction>
#include <QUrl>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
/** 单次远程起播：超过此时长仍未进入 Playing 则计一次失败并重试（最多 3 次）。 */
constexpr int kStreamReadyTimeoutMs = 12000;
constexpr int kStreamRetryDelayMs = 350;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    setWindowIcon(QIcon(QStringLiteral(":/icons/app.png")));
    m_engine = new PlayerEngine(this);
    m_downloader = &MusicDownloader::instance();
    setupUi();
    loadStyleSheet();
    createTrayIcon();
    
    // 连接主题变化信号
    connect(&Theme::ThemeManager::instance(), &Theme::ThemeManager::themeChanged,
            this, &MainWindow::applyTheme);

    setWindowTitle(QStringLiteral("NekoMusic"));
    resize(1200, 800);
    setMinimumSize(960, 640);

    // 延迟检查版本更新
    QTimer::singleShot(2000, this, [this]() { checkForUpdates(false); });
}

MainWindow::~MainWindow()
{
    // 清理下载器连接
    disconnectDownloader();
    
    // 清理托盘图标
    if (m_trayIcon) {
        m_trayIcon->hide();
        delete m_trayIcon;
    }
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    auto *mainV = new QVBoxLayout(central);
    mainV->setContentsMargins(0, 0, 0, 0);
    mainV->setSpacing(0);

    // 标题栏（横跨整个窗口顶部）
    m_titleBar = new TitleBar(this);
    mainV->addWidget(m_titleBar);

    // 中间区域：侧边栏 + 页面
    m_midWidget = new QWidget(this);
    m_midWidget->setObjectName("midWidget");
    m_midWidget->setAttribute(Qt::WA_StyledBackground, true);
    auto *midH = new QHBoxLayout(m_midWidget);
    midH->setContentsMargins(0, 0, 0, 0);
    midH->setSpacing(0);

    m_apiClient = new ApiClient(this);
    m_sidebar = new Sidebar(m_apiClient, this);
    midH->addWidget(m_sidebar);

    m_stack = new QStackedWidget(this);
    m_stack->setObjectName("pageStack");
    m_homePage = new HomePage(this);
    m_settingsPage = new SettingsPage(this);
    m_favoritesPage = new FavoritesPage(m_apiClient, this);
    m_recentPage = new RecentPage(this);
    m_hotMusicPage = new MusicListPage(MusicListPage::Hot, this);
    m_latestMusicPage = new MusicListPage(MusicListPage::Latest, this);
    m_uploadPage = new UploadPage(this);
    m_playlistDetailPage = new PlaylistDetailPage(m_apiClient, this);
    m_searchPage = new SearchPage(m_apiClient, this);
    m_stack->addWidget(m_homePage);
    m_stack->addWidget(m_settingsPage);
    m_stack->addWidget(m_favoritesPage);
    m_stack->addWidget(m_recentPage);
    m_stack->addWidget(m_hotMusicPage);
    m_stack->addWidget(m_latestMusicPage);
    m_stack->addWidget(m_uploadPage);
    m_stack->addWidget(m_playlistDetailPage);
    m_stack->addWidget(m_searchPage);
    midH->addWidget(m_stack, 1);

    mainV->addWidget(m_midWidget, 1);

    // 播放栏（横跨整个窗口底部）
    m_playerBar = new PlayerBar(m_engine, this);
    mainV->addWidget(m_playerBar);

    // 播放页面 — 全屏覆盖层（覆盖侧边栏和标题栏，不覆盖播放栏）
    m_playerPage = new PlayerPage(m_engine, m_midWidget);
    m_playerPage->hide();

    // 播放列表面板
    m_playlistPanel = new PlaylistPanel(this);
    m_playlistPanel->hide();

    // 加载播放队列
    PlaylistManager::instance().load();

    MusicDownloader::purgeLegacyMd5CacheFiles();

    // 恢复上次播放的音乐
    if (PlaylistManager::instance().hasLastPlayed()) {
        auto lastMusic = PlaylistManager::instance().lastPlayedMusic();
        m_playerBar->setCurrentMusicId(lastMusic.id);
        m_playerBar->setSongInfo(lastMusic.title, lastMusic.artist, lastMusic.coverUrl);
        m_playerPage->setMusicInfo(lastMusic.id, lastMusic.title, lastMusic.artist, QString(), lastMusic.coverUrl);
        m_playerPage->loadLyrics(lastMusic.id);
        m_engine->setCurrentMusic(lastMusic);

        // 检查收藏状态（loadFavoritesCache 会在之后异步更新，但这里先设置初始状态）
        bool isFavorited = checkIsFavorited(lastMusic.id);
        m_playerBar->setFavoriteStatus(isFavorited);

        // 与点歌一致：远程起播 + 并行写缓存；起播后暂停等待用户操作
        disconnectDownloader();
        const quint64 restoreSeq = m_enginePlaySeq;
        const QUrl url(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(lastMusic.id));
        m_playerBar->setLoading(true);
        startRemotePlaybackWithBackgroundCache(lastMusic.id, restoreSeq, url, true);
    }

    // 连接导航
    connect(m_sidebar, &Sidebar::navigationRequested, this, [this](const QString &key) {
        if (key == "home") switchPage(m_homePage);
        else if (key == "favorites") {
            m_favoritesPage->refresh();
            switchPage(m_favoritesPage);
        }
        else if (key == "recent") {
            m_recentPage->refresh();
            switchPage(m_recentPage);
        }
        else if (key == "upload") switchPage(m_uploadPage);
        else if (key == "search") switchPage(m_searchPage);
    });
    connect(m_favoritesPage, &FavoritesPage::playRequested, this, &MainWindow::playMusicById);
    connect(&UserManager::instance(), &UserManager::loginStateChanged, this, [this]() {
        if (m_favoritesPage) m_favoritesPage->refresh();
        loadFavoritesCache();
        m_sidebar->loadPlaylists();
    });
    // 启动时可能错过了信号，手动检查一次
    if (UserManager::instance().isLoggedIn()) {
        loadFavoritesCache();
        m_sidebar->loadPlaylists();
    }
    connect(m_recentPage, &RecentPage::playRequested, this, &MainWindow::playMusicById);
    connect(m_sidebar, &Sidebar::playlistClicked, this, &MainWindow::showPlaylistDetailPage);
    connect(m_sidebar, &Sidebar::playlistCreateRequested, this, &MainWindow::createPlaylist);
    connect(m_titleBar, &TitleBar::settingsClicked, this, [this]() {
        switchPage(m_settingsPage);
    });
    connect(m_settingsPage, &SettingsPage::languageChanged, m_homePage, &HomePage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_sidebar, &Sidebar::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_titleBar, &TitleBar::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_playerBar, &PlayerBar::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_playerPage, &PlayerPage::retranslate);

    // 音乐加载器连接 — 由各播放方法按需单独连接

    // QMediaPlayer 缓冲状态监控
    connect(m_engine, &PlayerEngine::stateChanged, this, [this](PlayerEngine::PlaybackState state) {
        if (state == PlayerEngine::Playing) {
            m_playerBar->setLoading(false);
        }
    });

    // 记录最近播放
    connect(m_engine, &PlayerEngine::musicStarted, this, [](const MusicInfo& music) {
        PlaylistDatabase::instance().recordRecentPlay(music);
    });

    // 播放错误处理（远程重试流程由 startRemotePlaybackWithBackgroundCache 专用连接处理，此处不抢 loading）
    connect(m_engine, &PlayerEngine::mediaError, this, [this](const QString &err) {
        qDebug() << "[播放错误]" << err;
        if (m_streamRetryActive)
            return;
        m_playerBar->setLoading(false);
    });

    // 播放完成自动切歌
    connect(m_engine, &PlayerEngine::playbackFinished, this, [this]() {
        auto& manager = PlaylistManager::instance();
        if (manager.count() == 0) return;

        const QString mode = manager.playMode();
        qDebug() << "[播放完成] 当前模式:" << mode;

        if (mode == "single") {
            // 单曲循环：重新播放当前歌曲
            qDebug() << "[单曲循环] 重新播放:" << manager.playlist()[manager.currentIndex()].title;
        }
        // 所有模式都调用 playNext()：
        // - single: nextIndex() 返回相同索引，重新播放当前歌曲
        // - list:   nextIndex() 返回下一首，列表循环
        // - random: nextIndex() 返回随机不同索引，随机播放
        playNext();
    });

    // 头像点击 - 显示登录/登出菜单
    connect(m_titleBar, &TitleBar::avatarClicked, this, [this]() {
        if (UserManager::instance().isLoggedIn()) {
            // 已登录，弹出用户菜单
            QMenu *menu = new QMenu(this);
            menu->setAttribute(Qt::WA_DeleteOnClose);

            QString username = UserManager::instance().userInfo().value("username").toString();
            if (username.isEmpty()) username = "User";
            menu->addAction(username)->setEnabled(false);
            menu->addSeparator();

            auto *favAction = menu->addAction(tr("My Favorites"));
            connect(favAction, &QAction::triggered, this, [this]() {
                switchPage(m_favoritesPage);
            });

            menu->addSeparator();
            auto *logoutAction = menu->addAction(tr("Logout"));
            connect(logoutAction, &QAction::triggered, this, [this, menu]() {
                UserManager::instance().logout();
                menu->close();
            });

            menu->popup(m_titleBar->avatarPos());
        } else {
            // 未登录，显示登录对话框
            LoginDialog dlg(this);
            dlg.exec();
        }
    });

    // 音乐列表页面导航
    connect(m_homePage, &HomePage::navigateToMusicList, this, &MainWindow::showMusicListPage);

    // 音乐列表页面返回
    connect(m_hotMusicPage, &MusicListPage::backRequested, this, [this]() {
        switchPage(m_homePage);
    });
    connect(m_latestMusicPage, &MusicListPage::backRequested, this, [this]() {
        switchPage(m_homePage);
    });

    // 搜索请求
    connect(m_titleBar, &TitleBar::searchRequested, this, [this](const QString &query) {
        m_searchPage->search(query);
        switchPage(m_searchPage);
    });

    // 搜索页面返回
    connect(m_searchPage, &SearchPage::backRequested, this, [this]() {
        switchPage(m_homePage);
    });
    connect(m_searchPage, &SearchPage::playMusic, this, [this](const MusicInfo &info) {
        playMusicById(info.id, info.title, info.artist, info.coverUrl);
    });
    connect(m_searchPage, &SearchPage::playAllRequested, this, [this](const QList<MusicInfo> &results) {
        PlaylistManager::instance().clearPlaylist();
        PlaylistManager::instance().addAllToPlaylist(results);
        if (!results.isEmpty()) {
            const auto &first = results.first();
            playMusicById(first.id, first.title, first.artist, first.coverUrl);
        }
    });
    connect(m_searchPage, &SearchPage::openPlaylist, this, [this](int playlistId) {
        m_playlistDetailPage->loadPlaylist(playlistId);
        switchPage(m_playlistDetailPage);
    });
    // 上传页面返回
    connect(m_uploadPage, &UploadPage::backRequested, this, [this]() {
        switchPage(m_homePage);
    });

    // 音乐列表页面播放
    connect(m_hotMusicPage, &MusicListPage::playMusic, this, [this](const MusicListPage::MusicInfo &info) {
        playMusicById(info.id, info.title, info.artist, info.coverUrl);
    });
    connect(m_latestMusicPage, &MusicListPage::playMusic, this, [this](const MusicListPage::MusicInfo &info) {
        playMusicById(info.id, info.title, info.artist, info.coverUrl);
    });

    // 音乐列表页添加到播放队列
    connect(m_hotMusicPage, &MusicListPage::addToQueue, this, [this](const MusicListPage::MusicInfo &info) {
        MusicInfo mInfo;
        mInfo.id = info.id;
        mInfo.title = info.title;
        mInfo.artist = info.artist;
        mInfo.album = info.album;
        mInfo.duration = info.duration;
        mInfo.coverUrl = info.coverUrl;
        PlaylistManager::instance().addToPlaylist(mInfo);
    });
    connect(m_latestMusicPage, &MusicListPage::addToQueue, this, [this](const MusicListPage::MusicInfo &info) {
        MusicInfo mInfo;
        mInfo.id = info.id;
        mInfo.title = info.title;
        mInfo.artist = info.artist;
        mInfo.album = info.album;
        mInfo.duration = info.duration;
        mInfo.coverUrl = info.coverUrl;
        PlaylistManager::instance().addToPlaylist(mInfo);
    });

    // 音乐列表页添加到歌单
    connect(m_hotMusicPage, &MusicListPage::addToPlaylist, this, [this](const MusicListPage::MusicInfo &info) {
        MusicInfo mInfo;
        mInfo.id = info.id;
        mInfo.title = info.title;
        mInfo.artist = info.artist;
        mInfo.album = info.album;
        mInfo.duration = info.duration;
        mInfo.coverUrl = info.coverUrl;
        showAddToPlaylistDialog(mInfo);
    });
    connect(m_latestMusicPage, &MusicListPage::addToPlaylist, this, [this](const MusicListPage::MusicInfo &info) {
        MusicInfo mInfo;
        mInfo.id = info.id;
        mInfo.title = info.title;
        mInfo.artist = info.artist;
        mInfo.album = info.album;
        mInfo.duration = info.duration;
        mInfo.coverUrl = info.coverUrl;
        showAddToPlaylistDialog(mInfo);
    });

    // 封面点击切换到播放页面（全屏覆盖侧边栏和标题栏）
    connect(m_playerBar, &PlayerBar::coverClicked, this, [this]() {
        m_playerBar->setCoverVisible(false);
        m_playerPage->setGeometry(m_midWidget->rect());
        m_playerPage->move(0, m_midWidget->height());
        m_playerPage->show();
        m_playerPage->raise();

        auto *anim = new QPropertyAnimation(m_playerPage, "pos");
        anim->setDuration(Theme::kAnimNormal);
        anim->setStartValue(QPoint(0, m_midWidget->height()));
        anim->setEndValue(QPoint(0, 0));
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // 播放队列按钮
    connect(m_playerBar, &PlayerBar::playlistClicked, this, &MainWindow::togglePlaylistPanel);

    // 上一首/下一首
    connect(m_playerBar, &PlayerBar::previousClicked, this, &MainWindow::playPrevious);
    connect(m_playerBar, &PlayerBar::nextClicked, this, &MainWindow::playNext);

    // 收藏按钮
    connect(m_playerBar, &PlayerBar::favoriteClicked, this, &MainWindow::toggleFavorite);

    // 播放模式按钮
    connect(m_playerBar, &PlayerBar::playModeClicked, this, [this]() {
        PlaylistManager::instance().togglePlayMode();
        m_playerBar->updatePlayModeBtn(PlaylistManager::instance().playMode());
    });

    // 初始化播放模式按钮状态
    m_playerBar->updatePlayModeBtn(PlaylistManager::instance().playMode());

    // 播放页面返回
    connect(m_playerPage, &PlayerPage::backRequested, this, [this]() {
        auto *anim = new QPropertyAnimation(m_playerPage, "pos");
        anim->setDuration(Theme::kAnimNormal);
        anim->setStartValue(QPoint(0, 0));
        anim->setEndValue(QPoint(0, m_midWidget->height()));
        anim->setEasingCurve(QEasingCurve::InCubic);
        connect(anim, &QPropertyAnimation::finished, this, [this]() {
            m_playerBar->setCoverVisible(true);
            m_playerPage->hide();
            m_playerPage->move(0, 0);
        });
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    // 播放位置变化时更新歌词高亮
    connect(m_engine, &PlayerEngine::positionChanged, m_playerPage, &PlayerPage::updateLyricHighlight);
    // 注意：自动切歌仅连接上方 playbackFinished 的 lambda，勿重复 connect playNext

    // 语言切换
    connect(m_settingsPage, &SettingsPage::languageChanged, m_hotMusicPage, &MusicListPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_latestMusicPage, &MusicListPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_uploadPage, &UploadPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_playlistDetailPage, &PlaylistDetailPage::retranslate);
    connect(m_settingsPage, &SettingsPage::checkForUpdatesRequested, this, [this]() {
        checkForUpdates(true);
    });

    // 播放列表页面返回
    connect(m_playlistDetailPage, &PlaylistDetailPage::backRequested, this, [this]() {
        switchPage(m_homePage);
    });

    // 播放列表页面播放
    connect(m_playlistDetailPage, &PlaylistDetailPage::playMusic, this, [this](const MusicInfo &info) {
        playMusicFromInfo(info);
    });

    // 搜索页面返回
    connect(m_searchPage, &SearchPage::backRequested, this, [this]() {
        switchPage(m_homePage);
    });

    // 搜索页面播放
    connect(m_searchPage, &SearchPage::playMusic, this, [this](const MusicInfo &info) {
        playMusicFromInfo(info);
    });

    // 语言切换
    connect(m_settingsPage, &SettingsPage::languageChanged, m_searchPage, &SearchPage::retranslate);

    // 播放列表页面刷新侧边栏
    connect(m_playlistDetailPage, &PlaylistDetailPage::refreshSidebarPlaylists, this, [this]() {
        m_sidebar->refreshPlaylists();
    });

    // 播放列表面板
    connect(m_playlistPanel, &PlaylistPanel::hideRequested, this, [this]() {
        m_playlistPanel->hidePanel();
    });
    connect(m_playlistPanel, &PlaylistPanel::playRequested, this, [this](int musicId) {
        playMusicFromPlaylist(musicId);
    });

    // 登录状态变化 - 控制上传导航项可见性
    connect(&UserManager::instance(), &UserManager::loginStateChanged, this, [this]() {
        m_sidebar->setUploadVisible(UserManager::instance().isLoggedIn());
    });

    // 创建桌面歌词窗口
    m_desktopLrc = new DesktopLrc(this);
    
    // 连接播放器位置更新到桌面歌词
    connect(m_engine, &PlayerEngine::positionChanged, this, [this](qint64 position) {
        if (m_desktopLrc) {
            m_desktopLrc->updatePosition(position);
        }
    });
    
    // 连接歌曲开始播放时更新桌面歌词
    connect(m_engine, &PlayerEngine::musicStarted, this, [this](const MusicInfo& music) {
        if (m_desktopLrc) {
            m_desktopLrc->setCurrentSong(music.title, music.artist);
            // 这里可以添加歌词文件加载逻辑
            // m_desktopLrc->loadLyricsFile("path/to/lyrics.lrc");
        }
    });
    
    // 连接桌面歌词开关（播放栏「词」按钮）
    connect(m_playerBar, &PlayerBar::desktopLyricsToggled, this, [this](bool enabled) {
        if (m_desktopLrc) {
            if (enabled)
                m_desktopLrc->showWindow();
            else
                m_desktopLrc->hideWindow();
        }
    });
    
    // 初始加载桌面歌词设置
    QSettings settings;
    bool lyricsEnabled = settings.value("desktopLyrics", true).toBool();
    if (m_desktopLrc) {
        if (lyricsEnabled) {
            m_desktopLrc->showWindow();
        } else {
            m_desktopLrc->hideWindow();
        }
    }
}
void MainWindow::loadStyleSheet()
{
    applyTheme();
}

void MainWindow::applyTheme()
{
    QString style = Theme::ThemeManager::instance().currentStyleSheet();
    if (!style.isEmpty()) {
        setStyleSheet(style);
    }
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    GlassPaint::paintMainWindowDeepBackdrop(p, rect(), Theme::ThemeManager::instance().isDarkMode());
}

void MainWindow::switchPage(QWidget *target)
{
    if (m_switching) return;
    if (m_stack->currentWidget() == target) return;

    m_switching = true;
    QWidget *current = m_stack->currentWidget();

    // Make target visible alongside current for cross-fade
    target->show();

    // Opacity effects
    auto *currentEff = new QGraphicsOpacityEffect(current);
    currentEff->setOpacity(1.0);
    current->setGraphicsEffect(currentEff);

    auto *targetEff = new QGraphicsOpacityEffect(target);
    targetEff->setOpacity(0.0);
    target->setGraphicsEffect(targetEff);

    // Parallel cross-fade
    auto *fadeOut = new QPropertyAnimation(currentEff, "opacity");
    fadeOut->setDuration(Theme::kAnimNormal);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);

    auto *fadeIn = new QPropertyAnimation(targetEff, "opacity");
    fadeIn->setDuration(Theme::kAnimNormal);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    connect(fadeIn, &QPropertyAnimation::finished, this, [this, target, current]() {
        m_stack->setCurrentWidget(target);
        current->setGraphicsEffect(nullptr);
        target->setGraphicsEffect(nullptr);
        m_switching = false;
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::showMusicListPage(bool isHot)
{
    if (isHot) {
        m_hotMusicPage->refresh();  // 触发懒加载
        switchPage(m_hotMusicPage);
    } else {
        m_latestMusicPage->refresh();  // 触发懒加载
        switchPage(m_latestMusicPage);
    }
}

void MainWindow::showPlaylistDetailPage(int localId)
{
    m_playlistDetailPage->loadPlaylist(localId);
    switchPage(m_playlistDetailPage);
}

void MainWindow::playMusicFromInfo(const MusicInfo &info)
{
    playMusicById(info.id, info.title, info.artist, info.coverUrl);
}

void MainWindow::createPlaylist()
{
    if (!m_apiClient || !UserManager::instance().isLoggedIn()) {
        return;
    }

    bool ok = false;
    QString name = QInputDialog::getText(this,
        I18n::instance().tr("createPlaylist"),
        I18n::instance().tr("playlistName"),
        QLineEdit::Normal,
        I18n::instance().tr("newPlaylist"),
        &ok);

    if (ok && !name.isEmpty()) {
        m_apiClient->createPlaylist(name, QString(), [this](bool success, const QString &message, const QVariantMap &) {
            if (success) {
                m_sidebar->loadPlaylists();
            }
        });
    }
}

void MainWindow::showAddToPlaylistDialog(const MusicInfo &music)
{
    AddToPlaylistDialog dialog(music, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_sidebar->refreshPlaylists();
    }
}

void MainWindow::togglePlaylistPanel()
{
    m_playlistPanel->togglePanel();
    if (m_playlistPanel->isVisible()) {
        // 定位到播放栏右侧上方
        QPoint pos = mapToGlobal(QPoint(width() - m_playlistPanel->width() - 20,
                                        height() - m_playerBar->height() - m_playlistPanel->height() - 20));
        m_playlistPanel->move(pos);
    }
}

void MainWindow::playMusicFromPlaylist(int musicId)
{
    auto& manager = PlaylistManager::instance();
    const auto& playlist = manager.playlist();
    for (int i = 0; i < playlist.size(); ++i) {
        if (playlist[i].id == musicId) {
            manager.setCurrentIndex(i);
            const auto& info = playlist[i];
            playMusicById(info.id, info.title, info.artist, info.coverUrl);
            break;
        }
    }
}

void MainWindow::cancelStreamWatch()
{
    if (m_streamAttemptTimer) {
        m_streamAttemptTimer->stop();
        m_streamAttemptTimer->deleteLater();
        m_streamAttemptTimer = nullptr;
    }
    if (m_streamPlayConn) {
        disconnect(m_streamPlayConn);
        m_streamPlayConn = QMetaObject::Connection();
    }
    if (m_streamErrorConn) {
        disconnect(m_streamErrorConn);
        m_streamErrorConn = QMetaObject::Connection();
    }
}

void MainWindow::startRemotePlaybackWithBackgroundCache(int musicId, quint64 playSeq, const QUrl &remoteUrl,
                                                        bool pauseWhenReady)
{
    cancelStreamWatch();
    m_streamRetryActive = true;
    m_streamRemoteUrl = remoteUrl;
    m_streamPauseWhenReady = pauseWhenReady;
    m_remoteStreamFailureCount = 0;

    startBackgroundCacheDownload(musicId, playSeq, remoteUrl);
    attachStreamPlaybackGuards(musicId, playSeq);
    m_engine->play(remoteUrl);
}

void MainWindow::startBackgroundCacheDownload(int musicId, quint64 playSeq, const QUrl &url)
{
    if (m_bgCacheFinishedConn) {
        disconnect(m_bgCacheFinishedConn);
        m_bgCacheFinishedConn = QMetaObject::Connection();
    }
    if (m_bgCacheErrorConn) {
        disconnect(m_bgCacheErrorConn);
        m_bgCacheErrorConn = QMetaObject::Connection();
    }

    m_bgCacheFinishedConn = connect(m_downloader, &MusicDownloader::downloadFinished, this,
        [this, playSeq, musicId](const QString &path) {
            if (playSeq != m_enginePlaySeq)
                return;
            qDebug() << "[后台缓存完成] id=" << musicId << path;
        });
    m_bgCacheErrorConn = connect(m_downloader, &MusicDownloader::downloadError, this,
        [this, playSeq, musicId](const QString &err) {
            if (playSeq != m_enginePlaySeq)
                return;
            qDebug() << "[后台缓存失败] id=" << musicId << err;
        });

    m_downloader->download(url, musicId);
}

void MainWindow::attachStreamPlaybackGuards(int musicId, quint64 playSeq)
{
    cancelStreamWatch();
    m_streamFailHandledThisRound = false;

    m_streamAttemptTimer = new QTimer(this);
    m_streamAttemptTimer->setSingleShot(true);
    m_streamAttemptTimer->setInterval(kStreamReadyTimeoutMs);
    connect(m_streamAttemptTimer, &QTimer::timeout, this, [this, musicId, playSeq]() {
        QTimer *t = m_streamAttemptTimer;
        m_streamAttemptTimer = nullptr;
        if (t)
            t->deleteLater();
        if (playSeq != m_enginePlaySeq)
            return;
        if (m_engine->playbackState() == PlayerEngine::Playing)
            return;
        qDebug() << "[Music] 远程起播超时 id=" << musicId;
        handleRemoteStreamFailure(musicId, playSeq);
    });

    m_streamPlayConn = connect(m_engine, &PlayerEngine::stateChanged, this,
        [this, playSeq](PlayerEngine::PlaybackState st) {
            if (playSeq != m_enginePlaySeq || st != PlayerEngine::Playing)
                return;
            m_streamRetryActive = false;
            cancelStreamWatch();
            if (m_streamPauseWhenReady)
                QTimer::singleShot(50, this, [this]() { m_engine->pause(); });
        });

    m_streamErrorConn = connect(m_engine, &PlayerEngine::mediaError, this,
        [this, musicId, playSeq](const QString &err) {
            if (playSeq != m_enginePlaySeq)
                return;
            qDebug() << "[Music] 远程播放错误 id=" << musicId << err;
            handleRemoteStreamFailure(musicId, playSeq);
        });

    m_streamAttemptTimer->start();
}

void MainWindow::handleRemoteStreamFailure(int musicId, quint64 playSeq)
{
    if (playSeq != m_enginePlaySeq)
        return;
    if (m_engine->playbackState() == PlayerEngine::Playing)
        return;
    if (m_streamFailHandledThisRound)
        return;
    m_streamFailHandledThisRound = true;

    m_engine->stop();
    m_remoteStreamFailureCount++;
    if (m_remoteStreamFailureCount >= 3) {
        m_streamRetryActive = false;
        cancelStreamWatch();
        m_playerBar->setLoading(false);
        qDebug() << "[Music] 远程播放失败已达 3 次，切下一首 id=" << musicId;
        playNext();
        return;
    }

    cancelStreamWatch();
    QTimer::singleShot(kStreamRetryDelayMs, this, [this, musicId, playSeq]() {
        if (playSeq != m_enginePlaySeq)
            return;
        attachStreamPlaybackGuards(musicId, playSeq);
        m_engine->play(m_streamRemoteUrl);
    });
}

void MainWindow::disconnectDownloader()
{
    qDebug() << "[MainWindow] disconnectDownloader called";
    cancelStreamWatch();
    if (m_finishedConn) {
        disconnect(m_finishedConn);
        m_finishedConn = QMetaObject::Connection();
    }
    if (m_errorConn) {
        disconnect(m_errorConn);
        m_errorConn = QMetaObject::Connection();
    }
    if (m_bufferConn) {
        disconnect(m_bufferConn);
        m_bufferConn = QMetaObject::Connection();
    }
    if (m_progressConn) {
        disconnect(m_progressConn);
        m_progressConn = QMetaObject::Connection();
    }
    if (m_bgCacheFinishedConn) {
        disconnect(m_bgCacheFinishedConn);
        m_bgCacheFinishedConn = QMetaObject::Connection();
    }
    if (m_bgCacheErrorConn) {
        disconnect(m_bgCacheErrorConn);
        m_bgCacheErrorConn = QMetaObject::Connection();
    }
}

void MainWindow::playNext()
{
    qDebug() << "[MainWindow] playNext called";

    m_isDownloading = false;
    disconnectDownloader();
    m_downloader->cancel();
    m_engine->stop();

    ++m_enginePlaySeq;
    const quint64 playSeq = m_enginePlaySeq;

    auto& manager = PlaylistManager::instance();
    if (manager.count() == 0) return;

    int nextIdx = manager.nextIndex();
    const MusicInfo info = manager.playlist()[nextIdx];
    manager.setCurrentIndex(nextIdx);

    // 与上一曲、点歌一致：立即同步播放栏与播放页（勿等 singleShot，避免播放页滞后）
    m_playerBar->setCurrentMusicId(info.id);
    m_playerBar->setSongInfo(info.title, info.artist, info.coverUrl);
    m_playerBar->setFavoriteStatus(checkIsFavorited(info.id));
    m_playerPage->setMusicInfo(info.id, info.title, info.artist, QString(), info.coverUrl);
    m_playerPage->loadLyrics(info.id);
    m_engine->setCurrentMusic(info);

    // 延后仅用于引擎停播后再起播，避免与 QMediaPlayer 释放竞态
    QTimer::singleShot(50, this, [this, info, playSeq]() {
        if (playSeq != m_enginePlaySeq)
            return;

        m_playerBar->setLoading(true);
        const QUrl url(QStringLiteral("%1/api/music/file/%2").arg(Theme::kApiBase).arg(info.id));
        startRemotePlaybackWithBackgroundCache(info.id, playSeq, url, false);
    });
}

void MainWindow::playPrevious()
{
    qDebug() << "[MainWindow] playPrevious called";
    auto& manager = PlaylistManager::instance();
    if (manager.count() == 0) {
        qDebug() << "[MainWindow] Playlist is empty";
        Toast::show(this, tr("播放列表为空"));
        return;
    }

    m_isDownloading = false;
    disconnectDownloader();
    m_downloader->cancel();
    m_engine->stop();

    ++m_enginePlaySeq;
    const quint64 playSeq = m_enginePlaySeq;

    int prevIdx = manager.previousIndex();
    const MusicInfo info = manager.playlist()[prevIdx];

    qDebug() << "[切歌] 上一曲:" << info.title << "-" << info.artist << "(ID:" << info.id << ")";

    manager.setCurrentIndex(prevIdx);

    m_playerBar->setCurrentMusicId(info.id);
    m_playerBar->setSongInfo(info.title, info.artist, info.coverUrl);
    m_playerBar->setFavoriteStatus(checkIsFavorited(info.id));
    m_playerPage->setMusicInfo(info.id, info.title, info.artist, QString(), info.coverUrl);
    m_playerPage->loadLyrics(info.id);
    m_engine->setCurrentMusic(info);

    QTimer::singleShot(0, this, [this, info, playSeq]() {
        if (playSeq != m_enginePlaySeq)
            return;

        const QUrl url(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(info.id));
        qDebug() << "[音乐加载] 远程起播并并行缓存:" << url.toString();

        m_playerBar->setLoading(true);
        startRemotePlaybackWithBackgroundCache(info.id, playSeq, url, false);
    });
}

void MainWindow::playMusicById(int musicId, const QString &title, const QString &artist, const QString &coverUrl)
{
    if (musicId <= 0) return;

    m_isDownloading = false;
    disconnectDownloader();
    m_downloader->cancel();
    m_engine->stop();

    ++m_enginePlaySeq;
    const quint64 playSeq = m_enginePlaySeq;

    qDebug() << "[音乐加载] 播放音乐:" << title << "-" << artist << "(ID:" << musicId << ")";

    // 自动添加到播放队列（去重）
    MusicInfo mInfo;
    mInfo.id = musicId;
    mInfo.title = title;
    mInfo.artist = artist;
    mInfo.album = QString();
    mInfo.duration = 0;
    mInfo.coverUrl = coverUrl;
    PlaylistManager::instance().addToPlaylist(mInfo);

    // 更新当前索引到刚添加的音乐
    const auto& playlist = PlaylistManager::instance().playlist();
    for (int i = 0; i < playlist.size(); ++i) {
        if (playlist[i].id == musicId) {
            PlaylistManager::instance().setCurrentIndex(i);
            break;
        }
    }

    // 刷新播放列表面板（如果可见）
    if (m_playlistPanel && m_playlistPanel->isVisible()) {
        m_playlistPanel->refresh();
    }

    // Update player bar
    m_playerBar->setCurrentMusicId(musicId);
    m_playerBar->setSongInfo(title, artist, coverUrl);

    // 检查收藏状态
    bool isFavorited = checkIsFavorited(musicId);
    m_playerBar->setFavoriteStatus(isFavorited);

    // Update player page
    m_playerPage->setMusicInfo(musicId, title, artist, QString(), coverUrl);
    m_playerPage->loadLyrics(musicId);

    // Set current music info for recent play tracking
    m_engine->setCurrentMusic(mInfo);

    const QUrl url(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(musicId));
    qDebug() << "[音乐加载] 远程起播并并行缓存:" << url.toString();

    m_playerBar->setLoading(true);
    startRemotePlaybackWithBackgroundCache(musicId, playSeq, url, false);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_playerPage && m_midWidget) m_playerPage->setGeometry(m_midWidget->rect());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 最小化到托盘而不是直接退出
    hide();
    event->ignore();
}

void MainWindow::createTrayIcon()
{
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        m_trayIcon = new QSystemTrayIcon(this);
        m_trayMenu = new QMenu(this);
        
        // 设置托盘图标
        QIcon trayIcon(":/icons/app_icon.png");
        if (trayIcon.isNull()) {
            // 如果资源图标不存在，使用默认图标
            trayIcon = QApplication::windowIcon();
        }
        m_trayIcon->setIcon(trayIcon);
        m_trayIcon->setToolTip("Neko云音乐");
        
        // 创建托盘菜单
        QAction *previousAction = new QAction("上一首", this);
        QAction *playPauseAction = new QAction("播放/暂停", this);
        QAction *nextAction = new QAction("下一首", this);
        QAction *showAction = new QAction("显示主窗口", this);
        QAction *quitAction = new QAction("退出", this);
        
        connect(previousAction, &QAction::triggered, this, &MainWindow::onTrayPrevious);
        connect(playPauseAction, &QAction::triggered, this, &MainWindow::onTrayPlayPause);
        connect(nextAction, &QAction::triggered, this, &MainWindow::onTrayNext);
        connect(showAction, &QAction::triggered, this, &MainWindow::onTrayShow);
        connect(quitAction, &QAction::triggered, this, &MainWindow::onTrayQuit);
        
        m_trayMenu->addAction(previousAction);
        m_trayMenu->addAction(playPauseAction);
        m_trayMenu->addAction(nextAction);
        m_trayMenu->addSeparator();
        m_trayMenu->addAction(showAction);
        m_trayMenu->addSeparator();
        m_trayMenu->addAction(quitAction);
        
        m_trayIcon->setContextMenu(m_trayMenu);
        
        // 连接托盘图标激活信号
        connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
        
        // 显示托盘图标
        m_trayIcon->show();
    }
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        onTrayShow();
    }
}

void MainWindow::onTrayPrevious()
{
    // TODO: 实现上一首功能
    // 暂时留空，后续可以连接到播放器
}

void MainWindow::onTrayPlayPause()
{
    // TODO: 实现播放/暂停功能
    // 暂时留空，后续可以连接到播放器
}

void MainWindow::onTrayNext()
{
    // TODO: 实现下一首功能
    // 暂时留空，后续可以连接到播放器
}

void MainWindow::onTrayShow()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::onTrayQuit()
{
    // 真正退出应用
    QApplication::quit();
}

void MainWindow::toggleFavorite(int musicId)
{
    qDebug() << "[收藏] 切换收藏, musicId =" << musicId;
    qDebug() << "[收藏] m_apiClient =" << (m_apiClient != nullptr) << ", 登录状态 =" << UserManager::instance().isLoggedIn();

    if (musicId <= 0 || !m_apiClient) return;

    bool isFavorited = checkIsFavorited(musicId);
    qDebug() << "[收藏] 当前收藏状态 =" << isFavorited;

    if (isFavorited) {
        qDebug() << "[收藏] 正在取消收藏, musicId =" << musicId;
        QUrl url(QString::fromUtf8("%1/api/user/favorites/%2").arg(Theme::kApiBase).arg(musicId));
        QNetworkRequest req(url);
        req.setRawHeader("Authorization", UserManager::instance().token().toUtf8());

        auto *nam = new QNetworkAccessManager(this);
        auto *reply = nam->deleteResource(req);
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, musicId, nam]() {
            reply->deleteLater();
            nam->deleteLater();
            QByteArray body = reply->readAll();
            qDebug() << "[收藏] 取消收藏响应, error =" << reply->error() << ", body =" << body;
            if (reply->error() == QNetworkReply::NoError) {
                m_favoritesCache.removeAll(musicId);
                m_playerBar->setFavoriteStatus(false);
                Toast::show(this, I18n::instance().tr("cancelFavoriteSuccess"), Toast::Success);
                qDebug() << "[收藏] 已从缓存移除并更新UI";
            } else {
                QString reason = reply->errorString();
                // 尝试从 JSON body 中提取错误信息
                QJsonDocument doc = QJsonDocument::fromJson(body);
                if (!doc.isNull() && doc.isObject()) {
                    QString msg = doc.object().value("message").toString();
                    if (!msg.isEmpty()) reason = msg;
                }
                Toast::show(this, I18n::instance().tr("cancelFavoriteFailed") + ": " + reason, Toast::Error);
            }
        });
    } else {
        qDebug() << "[收藏] 正在添加收藏, musicId =" << musicId;
        QUrl url(QString::fromUtf8("%1/api/user/favorites").arg(Theme::kApiBase));
        QNetworkRequest req(url);
        req.setRawHeader("Authorization", UserManager::instance().token().toUtf8());
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject obj;
        obj.insert("musicId", musicId);
        QJsonDocument doc(obj);

        auto *nam = new QNetworkAccessManager(this);
        auto *reply = nam->post(req, doc.toJson());
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, musicId, nam]() {
            reply->deleteLater();
            nam->deleteLater();
            QByteArray body = reply->readAll();
            qDebug() << "[收藏] 添加收藏响应, error =" << reply->error() << ", body =" << body;
            if (reply->error() == QNetworkReply::NoError) {
                if (!m_favoritesCache.contains(musicId)) {
                    m_favoritesCache.append(musicId);
                }
                m_playerBar->setFavoriteStatus(true);
                Toast::show(this, I18n::instance().tr("favoriteSuccess"), Toast::Success);
                qDebug() << "[收藏] 已加入缓存并更新UI";
            } else {
                QString reason = reply->errorString();
                QJsonDocument doc = QJsonDocument::fromJson(body);
                if (!doc.isNull() && doc.isObject()) {
                    QString msg = doc.object().value("message").toString();
                    if (!msg.isEmpty()) reason = msg;
                }
                Toast::show(this, I18n::instance().tr("favoriteFailed") + ": " + reason, Toast::Error);
            }
        });
    }
}

bool MainWindow::checkIsFavorited(int musicId)
{
    return m_favoritesCache.contains(musicId);
}

void MainWindow::loadFavoritesCache()
{
    qDebug() << "[收藏] 加载收藏缓存, 登录状态 =" << UserManager::instance().isLoggedIn();
    m_favoritesCache.clear();

    if (!UserManager::instance().isLoggedIn()) {
        qDebug() << "[收藏] 未登录，跳过加载";
        return;
    }

    m_apiClient->fetchFavorites([this](bool success, const QList<QVariantMap>& favorites) {
        qDebug() << "[收藏] 获取收藏列表: success =" << success << ", 数量 =" << favorites.size();
        if (success) {
            for (const auto &fav : favorites) {
                int id = fav.value("id").toInt();
                if (id > 0) {
                    m_favoritesCache.append(id);
                }
            }
            qDebug() << "[收藏] 缓存加载完成, 共" << m_favoritesCache.size() << "条";

            // 根据当前播放栏的实际音乐ID更新收藏状态
            if (m_playerBar && m_playerBar->currentMusicId() > 0) {
                int currentId = m_playerBar->currentMusicId();
                bool isFavorited = m_favoritesCache.contains(currentId);
                m_playerBar->setFavoriteStatus(isFavorited);
                qDebug() << "[收藏] 缓存加载后更新收藏状态, id =" << currentId << ", favorited =" << isFavorited;
            }
        }
    });
}

void MainWindow::checkForUpdates(bool showNoUpdateToast)
{
    // 防止重复检查
    if (m_updateChecker && m_updateDialog && m_updateDialog->isVisible()) {
        return;
    }

    m_updateChecker = new UpdateChecker(QString::fromUtf8(APP_VERSION), this);

    connect(m_updateChecker, &UpdateChecker::updateAvailable, this, [this](const UpdateInfo &info) {
        qDebug() << "[更新] 发现新版本:" << info.version << "下载链接:" << info.downloadUrl;
        m_updateDialog = new UpdateDialog(m_updateChecker->currentVersion(), info.version, info.downloadUrl, this);

        connect(m_updateDialog, &UpdateDialog::downloadRequested, this, [this](const QString &url) {
            m_updateChecker->downloadUpdate(url);
        });

        connect(m_updateChecker, &UpdateChecker::downloadProgress, m_updateDialog, &UpdateDialog::showDownloadProgress);
        connect(m_updateChecker, &UpdateChecker::downloadFinished, m_updateDialog, &UpdateDialog::showDownloadFinished);
        connect(m_updateChecker, &UpdateChecker::downloadFailed, m_updateDialog, &UpdateDialog::showDownloadFailed);

        m_updateDialog->exec();
    });

    connect(m_updateChecker, &UpdateChecker::noUpdate, this, [this, showNoUpdateToast]() {
        qDebug() << "[更新] 已是最新版本";
        if (showNoUpdateToast)
            Toast::show(this, I18n::instance().tr("isLatest"), Toast::Info);
    });

    connect(m_updateChecker, &UpdateChecker::checkFailed, this, [](const QString &error) {
        qDebug() << "[更新] 检查失败:" << error;
    });

    m_updateChecker->checkForUpdates();
}
