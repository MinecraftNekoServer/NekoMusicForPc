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
#include "ui/addtoplaylistdialog.h"
#include "ui/playlistpanel.h"
#include "ui/searchpage.h"
#include "core/playerengine.h"
#include "core/i18n.h"
#include "core/apiclient.h"
#include "core/musicdownloader.h"
#include "core/usermanager.h"
#include "core/playlistdb.h"
#include "core/playlistmanager.h"
#include "theme/theme.h"

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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    m_engine = new PlayerEngine(this);
    m_downloader = new MusicDownloader(this);
    setupUi();
    loadStyleSheet();
    createTrayIcon();

    setWindowTitle(QStringLiteral("NekoMusic"));
    resize(1200, 800);
    setMinimumSize(960, 640);
}

MainWindow::~MainWindow()
{
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

    m_sidebar = new Sidebar(this);
    midH->addWidget(m_sidebar);

    m_apiClient = new ApiClient(this);
    m_stack = new QStackedWidget(this);
    m_stack->setObjectName("pageStack");
    m_homePage = new HomePage(this);
    m_settingsPage = new SettingsPage(this);
    m_favoritesPage = new FavoritesPage(m_apiClient, this);
    m_recentPage = new RecentPage(this);
    m_hotMusicPage = new MusicListPage(MusicListPage::Hot, this);
    m_latestMusicPage = new MusicListPage(MusicListPage::Latest, this);
    m_uploadPage = new UploadPage(this);
    m_playlistDetailPage = new PlaylistDetailPage(this);
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
        m_playerPage->setMusicInfo(lastMusic.id, lastMusic.title, lastMusic.artist, QString(), lastMusic.coverUrl);
        m_engine->setCurrentMusic(lastMusic);

        // 预加载音频文件：下载完成后自动播放然后立即暂停，这样音频源已加载，用户点播放即可
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
    });
    connect(m_favoritesPage, &FavoritesPage::playRequested, this, &MainWindow::playMusicById);
    connect(&UserManager::instance(), &UserManager::loginStateChanged, this, [this]() {
        if (m_favoritesPage) m_favoritesPage->refresh();
    });
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

    // 音乐下载器连接
    connect(m_downloader, &MusicDownloader::downloadFinished, this, [this](const QString &localPath) {
        m_engine->play(QUrl::fromLocalFile(localPath));
    });
    connect(m_downloader, &MusicDownloader::downloadError, this, [](const QString &err) {
        qDebug() << "Music download error:" << err;
    });

    // 记录最近播放
    connect(m_engine, &PlayerEngine::musicStarted, this, [](const MusicInfo& music) {
        PlaylistDatabase::instance().recordRecentPlay(music);
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

    // 语言切换
    connect(m_settingsPage, &SettingsPage::languageChanged, m_hotMusicPage, &MusicListPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_latestMusicPage, &MusicListPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_uploadPage, &UploadPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_playlistDetailPage, &PlaylistDetailPage::retranslate);

    // 播放列表页面返回
    connect(m_playlistDetailPage, &PlaylistDetailPage::backRequested, this, [this]() {
        switchPage(m_homePage);
    });

    // 播放列表页面播放
    connect(m_playlistDetailPage, &PlaylistDetailPage::playMusic, this, [this](const MusicInfo &info) {
        playMusicFromInfo(info);
    });

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
    bool ok = false;
    QString name = QInputDialog::getText(this,
        I18n::instance().tr("createPlaylist"),
        I18n::instance().tr("playlistName"),
        QLineEdit::Normal,
        I18n::instance().tr("newPlaylist"),
        &ok);

    if (ok && !name.isEmpty()) {
        int playlistId = PlaylistDatabase::instance().createPlaylist(name);
        if (playlistId > 0) {
            m_sidebar->refreshPlaylists();
        }
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

void MainWindow::playNext()
{
    auto& manager = PlaylistManager::instance();
    if (manager.count() == 0) return;

    int nextIdx = manager.nextIndex();
    manager.setCurrentIndex(nextIdx);
    MusicInfo info = manager.playlist()[nextIdx];  // Copy by value to avoid use-after-free
    playMusicById(info.id, info.title, info.artist, info.coverUrl);
}

void MainWindow::playPrevious()
{
    auto& manager = PlaylistManager::instance();
    if (manager.count() == 0) return;

    int prevIdx = manager.previousIndex();
    manager.setCurrentIndex(prevIdx);
    MusicInfo info = manager.playlist()[prevIdx];  // Copy by value to avoid use-after-free
    playMusicById(info.id, info.title, info.artist, info.coverUrl);
}

void MainWindow::playMusicById(int musicId, const QString &title, const QString &artist, const QString &coverUrl)
{
    if (musicId <= 0) return;

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

    // Update player page
    m_playerPage->setMusicInfo(musicId, title, artist, QString(), coverUrl);
    m_playerPage->loadLyrics(musicId);

    // Set current music info for recent play tracking
    m_engine->setCurrentMusic(mInfo);

    // Build music URL and start buffered download
    QUrl url(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(musicId));
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
