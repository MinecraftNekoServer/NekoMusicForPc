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

#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QStyle>

namespace {
const QColor kIconNormal = QColor(245, 240, 255, 166);
const QColor kIconActive = QColor(245, 240, 255, 230);
const QColor kCloseNormal = QColor(245, 240, 255, 180);
const QColor kCloseActive = QColor(242, 100, 100, 230);
}

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
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
    auto *avatar = new QLabel(this);
    avatar->setObjectName("tbAvatar");
    avatar->setFixedSize(30, 30);
    avatar->setPixmap(QPixmap(":/icons/app.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    lay->addWidget(avatar);

    auto *settingsBtn = new QPushButton(this);
    settingsBtn->setObjectName("tbIconBtn");
    settingsBtn->setFixedSize(34, 34);
    settingsBtn->setIcon(Icons::icon(Icons::kSettings, 18, kIconNormal, kIconActive));
    settingsBtn->setCursor(Qt::PointingHandCursor);
    settingsBtn->setToolTip(I18n::instance().tr("settings"));
    connect(settingsBtn, &QPushButton::clicked, this, &TitleBar::settingsClicked);
    lay->addWidget(settingsBtn);

    // 窗口控制
    auto *minBtn = new QPushButton(this);
    minBtn->setObjectName("tbWinBtn");
    minBtn->setFixedSize(30, 30);
    minBtn->setIcon(Icons::icon(Icons::kMinimize, 14, kIconNormal, kIconActive));
    minBtn->setCursor(Qt::PointingHandCursor);
    connect(minBtn, &QPushButton::clicked, this, [this]() { if (window()) window()->showMinimized(); });
    lay->addWidget(minBtn);

    auto *maxBtn = new QPushButton(this);
    maxBtn->setObjectName("tbWinBtn");
    maxBtn->setFixedSize(30, 30);
    maxBtn->setIcon(Icons::icon(Icons::kMaximize, 14, kIconNormal, kIconActive));
    maxBtn->setCursor(Qt::PointingHandCursor);
    connect(maxBtn, &QPushButton::clicked, this, [this]() {
        if (window()) window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
    });
    lay->addWidget(maxBtn);

    auto *closeBtn = new QPushButton(this);
    closeBtn->setObjectName("tbWinClose");
    closeBtn->setFixedSize(30, 30);
    closeBtn->setIcon(Icons::icon(Icons::kClose, 14, kCloseNormal, kCloseActive));
    closeBtn->setCursor(Qt::PointingHandCursor);
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

void TitleBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPos = e->globalPosition().toPoint() - window()->pos();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging && window()) {
        if (window()->isMaximized()) {
            window()->showNormal();
            m_dragPos = QPoint(window()->width() / 2, Theme::kTitleBarH / 2);
        }
        window()->move(e->globalPosition().toPoint() - m_dragPos);
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent *) { m_dragging = false; }

void TitleBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && window())
        window()->isMaximized() ? window()->showNormal() : window()->showMaximized();
}
