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
#include <QCryptographicHash>
#include <QStandardPaths>
#include "core/i18n.h"
#include "core/apiclient.h"
#include "core/musicdownloader.h"
#include "core/usermanager.h"
#include "core/playlistdb.h"
#include "core/playlistmanager.h"
#include "core/updatechecker.h"
#include "theme/theme.h"
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
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    setWindowIcon(QIcon(QStringLiteral(":/icons/app.png")));
    QMetaObject::Connection m_bufferConn;
    QMetaObject::Connection m_progressConn;
    m_engine = new PlayerEngine(this);
    m_downloader = &MusicDownloader::instance();
    setupUi();
    loadStyleSheet();
    createTrayIcon();

    setWindowTitle(QStringLiteral("NekoMusic"));
    resize(1200, 800);
    setMinimumSize(960, 640);

    // 延迟检查版本更新
    QTimer::singleShot(2000, this, &MainWindow::checkForUpdates);
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

    // 恢复上次播放的音乐
    if (PlaylistManager::instance().hasLastPlayed()) {
        auto lastMusic = PlaylistManager::instance().lastPlayedMusic();
        m_playerBar->setSongInfo(lastMusic.title, lastMusic.artist, lastMusic.coverUrl);
        m_playerBar->setCurrentMusicId(lastMusic.id);
        m_playerPage->setMusicInfo(lastMusic.id, lastMusic.title, lastMusic.artist, QString(), lastMusic.coverUrl);
        m_playerPage->loadLyrics(lastMusic.id);
        m_engine->setCurrentMusic(lastMusic);

        // 检查收藏状态（loadFavoritesCache 会在之后异步更新，但这里先设置初始状态）
        bool isFavorited = checkIsFavorited(lastMusic.id);
        m_playerBar->setFavoriteStatus(isFavorited);

        // 预加载音频文件：下载完成后暂停，等待用户操作
        disconnectDownloader();
        QUrl url(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(lastMusic.id));
        auto restoreConn = std::make_shared<QMetaObject::Connection>();
        *restoreConn = connect(m_downloader, &MusicDownloader::downloadFinished, this, [this, restoreConn](const QString &localPath) {
            disconnect(*restoreConn);
            m_engine->play(QUrl::fromLocalFile(localPath));
            // 立即暂停，等待用户操作
            QTimer::singleShot(50, this, [this]() {
                m_engine->pause();
            });
        });
        m_downloader->download(url);
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

    // 播放错误处理
    connect(m_engine, &PlayerEngine::mediaError, this, [this](const QString &err) {
        qDebug() << "[播放错误]" << err;
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
    
    // 播放结束时自动切歌
    connect(m_engine, &PlayerEngine::playbackFinished, this, &MainWindow::playNext);

    // 语言切换
    connect(m_settingsPage, &SettingsPage::languageChanged, m_hotMusicPage, &MusicListPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_latestMusicPage, &MusicListPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_uploadPage, &UploadPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_playlistDetailPage, &PlaylistDetailPage::retranslate);
    connect(m_settingsPage, &SettingsPage::checkForUpdatesRequested, this, &MainWindow::checkForUpdates);

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
    
    // 连接桌面歌词设置变化
    connect(m_settingsPage, &SettingsPage::desktopLyricsChanged, this, [this](bool enabled) {
        if (m_desktopLrc) {
            if (enabled) {
                m_desktopLrc->showWindow();
            } else {
                m_desktopLrc->hideWindow();
            }
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
    QFile f(":/style.qss");
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        setStyleSheet(QString::fromUtf8(f.readAll()));
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QLinearGradient bg(rect().topLeft(), QPoint(rect().width() * 0.3, rect().height()));
    bg.setColorAt(0.0, QColor(26, 22, 37));   // #1A1625
    bg.setColorAt(1.0, QColor(36, 31, 49));   // #241F31
    p.fillRect(rect(), bg);
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

void MainWindow::disconnectDownloader()
{
    qDebug() << "[MainWindow] disconnectDownloader called";
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
}

void MainWindow::playNext()
{
    // 1. 立即拦截频繁调用（防抖）
    static QElapsedTimer lastCallTimer;
    if (lastCallTimer.isValid() && lastCallTimer.elapsed() < 300) return;
    lastCallTimer.restart();

    qDebug() << "[MainWindow] playNext called";

    // 2. 同步状态清理
    m_isDownloading = false; // 重置状态，准备新任务
    disconnectDownloader();  // 确保这里彻底 disconnect 了 m_downloader 的所有信号
    m_downloader->cancel();
    m_engine->stop();

    auto& manager = PlaylistManager::instance();
    if (manager.count() == 0) return;

    int nextIdx = manager.nextIndex();
    const MusicInfo info = manager.playlist()[nextIdx];
    manager.setCurrentIndex(nextIdx);

    // 3. 移除 singleShot，或者将状态检查移入异步块
    // 如果一定要用 singleShot 解决 QMediaPlayer 的释放问题：
    QTimer::singleShot(50, this, [this, info]() {
        // 更新 UI
        m_playerBar->setSongInfo(info.title, info.artist, info.coverUrl);
        m_playerBar->setCurrentMusicId(info.id);

        // 缓存检查
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/nekomusic-cache";
        QString hash = QCryptographicHash::hash(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(info.id).toUtf8(), QCryptographicHash::Md5).toHex();
        QString cachedPath = cacheDir + "/" + hash;

        if (QFile::exists(cachedPath)) {
            m_engine->play(QUrl::fromLocalFile(cachedPath));
            return;
        }

        // --- 开始下载逻辑 ---
        if (m_isDownloading) return; // 二次检查
        m_isDownloading = true;

        // 重新建立连接（确保旧的已断开）
        // 建议使用 Qt::UniqueConnection 确保连接唯一性
        m_bufferConn = connect(m_downloader, &MusicDownloader::bufferReady, this, [this](auto path){
            m_engine->play(QUrl::fromLocalFile(path));
        }, Qt::UniqueConnection);

        // ... 其他 connect 也要加 Qt::UniqueConnection ...

        m_downloader->download(QUrl(QString("%1/api/music/file/%2").arg(Theme::kApiBase).arg(info.id)));
    });
}

void MainWindow::playPrevious()
{
    qDebug() << "[MainWindow] playPrevious called";
    auto& manager = PlaylistManager::instance();
    if (manager.count() == 0) {
        qDebug() << "[MainWindow] Playlist is empty";
        // 显示提示信息
        Toast::show(this, tr("播放列表为空"));
        return;
    }

    int prevIdx = manager.previousIndex();
    const MusicInfo info = manager.playlist()[prevIdx];

    qDebug() << "[切歌] 上一曲:" << info.title << "-" << info.artist << "(ID:" << info.id << ")";

    m_engine->stop();
    manager.setCurrentIndex(prevIdx);

    // Defer to next event loop iteration
    QTimer::singleShot(0, this, [this, info]() {
        m_playerBar->setSongInfo(info.title, info.artist, info.coverUrl);
        m_playerBar->setCurrentMusicId(info.id);
        m_playerPage->setMusicInfo(info.id, info.title, info.artist, QString(), info.coverUrl);
        m_playerPage->loadLyrics(info.id);
        m_engine->setCurrentMusic(info);

        // 检查文件是否已缓存
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/nekomusic-cache";
        QString hash = QCryptographicHash::hash(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(info.id).toUtf8(), 
                                                QCryptographicHash::Md5).toHex();
        QString cachedPath = cacheDir + "/" + hash;
        
        if (QFile::exists(cachedPath)) {
            qDebug() << "[缓存命中] 文件已缓存:" << cachedPath;
            m_playerBar->setLoading(false);
            m_engine->play(QUrl::fromLocalFile(cachedPath));
            return;
        }

        QUrl url(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(info.id));
        qDebug() << "[音乐加载] 开始下载:" << url.toString();

        disconnectDownloader();
        m_downloader->cancel();
        m_playerBar->setLoading(true);

        m_finishedConn = connect(m_downloader, &MusicDownloader::downloadFinished, this,
            [this](const QString &finalPath) {
                qDebug() << "[下载完成] 文件已就绪，开始播放:" << finalPath;
                m_playerBar->setLoading(false);
                m_engine->play(QUrl::fromLocalFile(finalPath));
            });

        m_errorConn = connect(m_downloader, &MusicDownloader::downloadError, this,
            [this](const QString &err) {
                qDebug() << "[下载失败]" << err;
                m_playerBar->setLoading(false);
            });

        m_downloader->download(url);
    });
}

void MainWindow::playMusicById(int musicId, const QString &title, const QString &artist, const QString &coverUrl)
{
    if (musicId <= 0) return;

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
    m_playerBar->setSongInfo(title, artist, coverUrl);
    m_playerBar->setCurrentMusicId(musicId);

    // 检查收藏状态
    bool isFavorited = checkIsFavorited(musicId);
    m_playerBar->setFavoriteStatus(isFavorited);

    // Update player page
    m_playerPage->setMusicInfo(musicId, title, artist, QString(), coverUrl);
    m_playerPage->loadLyrics(musicId);

    // Set current music info for recent play tracking
    m_engine->setCurrentMusic(mInfo);

    // 检查文件是否已缓存
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/nekomusic-cache";
    QString hash = QCryptographicHash::hash(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(musicId).toUtf8(), 
                                            QCryptographicHash::Md5).toHex();
    QString cachedPath = cacheDir + "/" + hash;
    
    if (QFile::exists(cachedPath)) {
        qDebug() << "[缓存命中] 文件已缓存:" << cachedPath;
        m_playerBar->setLoading(false);
        m_engine->play(QUrl::fromLocalFile(cachedPath));
        return;
    }

    QUrl url(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(musicId));
    qDebug() << "[音乐加载] 开始下载:" << url.toString();

    disconnectDownloader();
    m_downloader->cancel();
    m_playerBar->setLoading(true);

    m_finishedConn = connect(m_downloader, &MusicDownloader::downloadFinished, this,
        [this](const QString &finalPath) {
            qDebug() << "[下载完成] 文件已就绪，开始播放:" << finalPath;
            m_playerBar->setLoading(false);
            m_engine->play(QUrl::fromLocalFile(finalPath));
        });

    m_errorConn = connect(m_downloader, &MusicDownloader::downloadError, this,
        [this](const QString &err) {
            qDebug() << "[下载失败]" << err;
            m_playerBar->setLoading(false);
        });

    m_downloader->download(url);
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

void MainWindow::checkForUpdates()
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

    connect(m_updateChecker, &UpdateChecker::noUpdate, this, []() {
        qDebug() << "[更新] 已是最新版本";
    });

    connect(m_updateChecker, &UpdateChecker::checkFailed, this, [](const QString &error) {
        qDebug() << "[更新] 检查失败:" << error;
    });

    m_updateChecker->checkForUpdates();
}
