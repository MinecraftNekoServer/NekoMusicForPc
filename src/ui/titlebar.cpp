/**
 * @file titlebar.cpp
 * @brief 自定义标题栏实现
 *
 * 56px 紧凑，毛玻璃背景 + 底部薰衣草紫微光线。
 */

#include "titlebar.h"
#include "theme/theme.h"
#include "ui/svgicon.h"
#include "core/i18n.h"
#include "core/usermanager.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QStyle>
#include <QEvent>
#include <QGuiApplication>
#include <QWindow>
#include <QToolTip>

namespace {
const QColor kIconNormal = QColor(245, 240, 255, 166);
const QColor kIconActive = QColor(245, 240, 255, 230);
const QColor kCloseNormal = QColor(245, 240, 255, 180);
const QColor kCloseActive = QColor(242, 100, 100, 230);
}

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    setupUi();
    // 安装事件过滤器到 QApplication，捕获标题栏内所有子控件的鼠标事件
    QGuiApplication::instance()->installEventFilter(this);

    // 监听用户状态变化
    connect(&UserManager::instance(), &UserManager::loginStateChanged,
            this, &TitleBar::updateAvatar);
}

TitleBar::~TitleBar() = default;

bool TitleBar::eventFilter(QObject *watched, QEvent *event)
{
    // 只处理 TitleBar 及其子控件的事件
    auto *w = qobject_cast<QWidget *>(watched);
    if (!w || !isAncestorOf(w)) {
        return QObject::eventFilter(watched, event);
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *e = static_cast<QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton) {
            // 按钮和输入框不拦截，让它们自己处理
            if (qobject_cast<QPushButton *>(w) || qobject_cast<QLineEdit *>(w)) {
                return false;
            }
            // 头像点击
            if (w == m_avatar) {
                emit avatarClicked();
                return true;
            }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            if (window()) window()->windowHandle()->startSystemMove();
#endif
            return true;
        }
        break;
    }
    case QEvent::MouseButtonDblClick: {
        auto *e = static_cast<QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton && window()) {
            if (qobject_cast<QPushButton *>(w)) return false;
            window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
            return true;
        }
        break;
    }
    case QEvent::Enter: {
        if (w == m_avatar) {
            QString tip = UserManager::instance().isLoggedIn()
                ? UserManager::instance().userInfo().value("username").toString()
                : I18n::instance().tr("goToLogin");
            QToolTip::showText(QCursor::pos(), tip);
        }
        break;
    }
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && window())
        window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
}

void TitleBar::setupUi()
{
    setFixedHeight(Theme::kTitleBarH);
    setAttribute(Qt::WA_StyledBackground, false);

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(16, 0, 10, 0);
    lay->setSpacing(0);

    // ─── 左侧 Logo + 名称 ────────────────────────────
    auto *left = new QWidget(this);
    left->setFixedWidth(Theme::kSidebarW - 16);
    auto *ll = new QHBoxLayout(left);
    ll->setContentsMargins(0, 0, 0, 0);
    ll->setSpacing(10);

    m_logo = new QLabel(this);
    m_logo->setFixedSize(24, 24);
    m_logo->setPixmap(QPixmap(":/icons/app.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ll->addWidget(m_logo);

    m_name = new QLabel(QStringLiteral("NekoMusic"), this);
    m_name->setObjectName("tbAppName");
    ll->addWidget(m_name);
    ll->addStretch();
    lay->addWidget(left);

    // ─── 中间搜索框 ──────────────────────────────────
    m_search = new QLineEdit(this);
    m_search->setObjectName("tbSearch");
    m_search->setPlaceholderText(I18n::instance().tr("searchPlaceholder"));
    m_search->setFixedHeight(34);
    m_search->setMinimumWidth(180);
    m_search->setMaximumWidth(380);
    connect(m_search, &QLineEdit::returnPressed, this, [this]() {
        if (!m_search->text().trimmed().isEmpty())
            emit searchRequested(m_search->text().trimmed());
    });
    lay->addStretch(1);
    lay->addWidget(m_search, 0, Qt::AlignHCenter);
    lay->addStretch(1);

    // ─── 右侧控制 ────────────────────────────────────
    m_avatar = new QLabel(this);
    m_avatar->setObjectName("tbAvatar");
    m_avatar->setFixedSize(30, 30);
    m_avatar->setCursor(Qt::PointingHandCursor);
    updateAvatar();

    // 头像点击事件
    connect(m_avatar, &QLabel::customContextMenuRequested, this, [this]() {
        emit avatarClicked();
    });
    // 使能鼠标事件
    m_avatar->setContextMenuPolicy(Qt::CustomContextMenu);

    lay->addWidget(m_avatar);

    auto *settingsBtn = new QPushButton(this);
    settingsBtn->setObjectName("tbIconBtn");
    settingsBtn->setFixedSize(34, 34);
    settingsBtn->setIcon(Icons::icon(Icons::kSettings, 18, kIconNormal, kIconActive));
    settingsBtn->setCursor(Qt::PointingHandCursor);
    settingsBtn->setToolTip(I18n::instance().tr("settings"));
    connect(settingsBtn, &QPushButton::clicked, this, &TitleBar::settingsClicked);
    lay->addWidget(settingsBtn);

    // 窗口控制（圆形彩色按钮）
    const QColor kMinHover = QColor(196, 167, 231);  // 薰衣草紫
    const QColor kMaxHover = QColor(126, 200, 200);  // 薄荷绿
    const QColor kCloseHover = QColor(232, 93, 117); // 红色

    auto *minBtn = new QPushButton(this);
    minBtn->setObjectName("tbMinBtn");
    minBtn->setFixedSize(32, 32);
    minBtn->setIcon(Icons::icon(Icons::kMinimize, 20, kIconNormal, kMinHover));
    minBtn->setCursor(Qt::PointingHandCursor);
    minBtn->setToolTip(QStringLiteral("最小化"));
    connect(minBtn, &QPushButton::clicked, this, [this]() { if (window()) window()->showMinimized(); });
    lay->addWidget(minBtn);

    auto *maxBtn = new QPushButton(this);
    maxBtn->setObjectName("tbMaxBtn");
    maxBtn->setFixedSize(32, 32);
    maxBtn->setIcon(Icons::icon(Icons::kMaximize, 20, kIconNormal, kMaxHover));
    maxBtn->setCursor(Qt::PointingHandCursor);
    maxBtn->setToolTip(QStringLiteral("最大化"));
    connect(maxBtn, &QPushButton::clicked, this, [this]() {
        if (window()) window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
    });
    lay->addWidget(maxBtn);

    auto *closeBtn = new QPushButton(this);
    closeBtn->setObjectName("tbCloseBtn");
    closeBtn->setFixedSize(32, 32);
    closeBtn->setIcon(Icons::icon(Icons::kClose, 20, kIconNormal, kCloseHover));
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolTip(QStringLiteral("关闭"));
    connect(closeBtn, &QPushButton::clicked, this, [this]() { if (window()) window()->close(); });
    lay->addWidget(closeBtn);
}

void TitleBar::retranslate()
{
    auto *search = findChild<QLineEdit *>("tbSearch");
    if (search) search->setPlaceholderText(I18n::instance().tr("searchPlaceholder"));

    auto *settingsBtn = findChild<QPushButton *>("tbIconBtn");
    if (settingsBtn) settingsBtn->setToolTip(I18n::instance().tr("settings"));
}

QPoint TitleBar::avatarPos() const
{
    if (m_avatar) {
        return m_avatar->mapToGlobal(QPoint(m_avatar->width() / 2, m_avatar->height()));
    }
    return mapToGlobal(QPoint(width() - 40, height()));
}

void TitleBar::updateAvatar()
{
    if (!m_avatar) return;

    if (UserManager::instance().isLoggedIn()) {
        // 已登录，显示用户名
        QString username = UserManager::instance().userInfo().value("username").toString();
        if (username.isEmpty()) username = "User";
        // 取首字母作为头像
        QString letter = username.left(1).toUpper();
        QPixmap pixmap(30, 30);
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor(242, 172, 185));  // 樱花粉
        p.setPen(Qt::NoPen);
        p.drawEllipse(0, 0, 30, 30);
        p.setPen(QColor(26, 22, 37));
        QFont font = p.font();
        font.setBold(true);
        font.setPixelSize(14);
        p.setFont(font);
        QRect rect(0, 0, 30, 30);
        p.drawText(rect, Qt::AlignCenter, letter);
        m_avatar->setPixmap(pixmap);
        m_avatar->setToolTip(username);
    } else {
        // 未登录，显示默认图标
        m_avatar->setPixmap(QPixmap(":/icons/app.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_avatar->setToolTip(I18n::instance().tr("goToLogin"));
    }
}

void TitleBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 毛玻璃背景
    QColor bg(36, 31, 49, 204);  // kGlassSidebar
    p.fillRect(rect(), bg);

    // 底部薰衣草紫微光线
    QLinearGradient line(rect().topLeft(), rect().topRight());
    line.setColorAt(0.0, QColor(196, 167, 231, 0));
    line.setColorAt(0.5, QColor(196, 167, 231, 40));
    line.setColorAt(1.0, QColor(196, 167, 231, 0));
    p.setPen(QPen(QBrush(line), 1));
    p.drawLine(rect().bottomLeft(), rect().bottomRight());
}
