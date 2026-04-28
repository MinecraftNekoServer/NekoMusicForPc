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
#include "core/playerengine.h"
#include "core/i18n.h"
#include "core/usermanager.h"
#include "theme/theme.h"

#include <QApplication>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QFile>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QCloseEvent>
#include <QAction>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    m_engine = new PlayerEngine(this);
    setupUi();
    loadStyleSheet();

    setWindowTitle(QStringLiteral("NekoMusic"));
    resize(1200, 800);
    setMinimumSize(960, 640);
}

MainWindow::~MainWindow() = default;

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
    auto *midH = new QHBoxLayout();
    midH->setContentsMargins(0, 0, 0, 0);
    midH->setSpacing(0);

    m_sidebar = new Sidebar(this);
    midH->addWidget(m_sidebar);

    m_stack = new QStackedWidget(this);
    m_stack->setObjectName("pageStack");
    m_homePage = new HomePage(this);
    m_settingsPage = new SettingsPage(this);
    m_favoritesPage = new FavoritesPage(this);
    m_recentPage = new RecentPage(this);
    m_hotMusicPage = new MusicListPage(MusicListPage::Hot, this);
    m_latestMusicPage = new MusicListPage(MusicListPage::Latest, this);
    m_uploadPage = new UploadPage(this);
    m_stack->addWidget(m_homePage);
    m_stack->addWidget(m_settingsPage);
    m_stack->addWidget(m_favoritesPage);
    m_stack->addWidget(m_recentPage);
    m_stack->addWidget(m_hotMusicPage);
    m_stack->addWidget(m_latestMusicPage);
    m_stack->addWidget(m_uploadPage);
    midH->addWidget(m_stack, 1);

    mainV->addLayout(midH, 1);

    // 播放栏（横跨整个窗口底部）
    m_playerBar = new PlayerBar(m_engine, this);
    mainV->addWidget(m_playerBar);

    // 连接导航
    connect(m_sidebar, &Sidebar::navigationRequested, this, [this](const QString &key) {
        if (key == "home") switchPage(m_homePage);
        else if (key == "favorites") switchPage(m_favoritesPage);
        else if (key == "recent") switchPage(m_recentPage);
        else if (key == "upload") switchPage(m_uploadPage);
    });
    connect(m_titleBar, &TitleBar::settingsClicked, this, [this]() {
        switchPage(m_settingsPage);
    });
    connect(m_settingsPage, &SettingsPage::languageChanged, m_homePage, &HomePage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_sidebar, &Sidebar::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_titleBar, &TitleBar::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_playerBar, &PlayerBar::retranslate);

    // 头像点击 - 显示登录对话框
    connect(m_titleBar, &TitleBar::avatarClicked, this, [this]() {
        if (UserManager::instance().isLoggedIn()) {
            // 已登录，显示登出确认
            // TODO: 可以添加用户菜单
            UserManager::instance().logout();
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

    // 上传页面返回
    connect(m_uploadPage, &UploadPage::backRequested, this, [this]() {
        switchPage(m_homePage);
    });

    // 音乐列表页面播放
    connect(m_hotMusicPage, &MusicListPage::playMusic, this, [this](const MusicListPage::MusicInfo &info) {
        playMusicById(info.id, info.title, info.artist);
    });
    connect(m_latestMusicPage, &MusicListPage::playMusic, this, [this](const MusicListPage::MusicInfo &info) {
        playMusicById(info.id, info.title, info.artist);
    });

    // 语言切换
    connect(m_settingsPage, &SettingsPage::languageChanged, m_hotMusicPage, &MusicListPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_latestMusicPage, &MusicListPage::retranslate);
    connect(m_settingsPage, &SettingsPage::languageChanged, m_uploadPage, &UploadPage::retranslate);

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

void MainWindow::playMusicById(int musicId, const QString &title, const QString &artist)
{
    if (musicId <= 0) return;

    // Update player bar
    m_playerBar->setSongInfo(title, artist);

    // 构建音乐URL
    QUrl url(QString::fromUtf8("%1/api/music/file/%2").arg(Theme::kApiBase).arg(musicId));
    m_engine->play(url);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Directly close the application
    event->accept();
    QApplication::quit();
}
