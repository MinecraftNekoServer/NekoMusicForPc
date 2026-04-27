/**
 * @file sidebar.cpp
 * @brief 侧边栏实现
 *
 * 240px 重度毛玻璃，深紫黑半透 + 薰衣草微光。
 * 选中态：薰衣草背景 + 左侧薄荷绿竖条。
 */

#include "sidebar.h"
#include "theme/theme.h"
#include "ui/svgicon.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QStyle>

namespace {
QColor navIconColor(bool active) {
    return active ? QColor(196, 167, 231) : QColor(245, 240, 255, 166);
}
}

Sidebar::Sidebar(QWidget *parent) : QWidget(parent)
{
    setFixedWidth(Theme::kSidebarW);
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
    setActiveNav("home");
}

void Sidebar::setupUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setObjectName("sbScroll");

    auto *container = new QWidget(scroll);
    container->setObjectName("sbContainer");
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(10, 12, 10, 12);
    lay->setSpacing(2);

    // 主导航（带 SVG 图标）— 当前仅首页可用
    lay->addWidget(createNavItem("home", QStringLiteral("首页"),
                                 Icons::icon(Icons::kHome, 20, navIconColor(false), navIconColor(true))));

    auto *favBtn = new QPushButton(QStringLiteral("收藏"), this);
    favBtn->setObjectName("sbNavItem");
    favBtn->setFixedHeight(42);
    favBtn->setIcon(Icons::render(Icons::kHeart, 20, navIconColor(false)));
    favBtn->setEnabled(false);
    lay->addWidget(favBtn);

    auto *recBtn = new QPushButton(QStringLiteral("最近播放"), this);
    recBtn->setObjectName("sbNavItem");
    recBtn->setFixedHeight(42);
    recBtn->setIcon(Icons::render(Icons::kClock, 20, navIconColor(false)));
    recBtn->setEnabled(false);
    lay->addWidget(recBtn);

    // 分隔线
    auto *div = new QWidget(container);
    div->setObjectName("sbDivider");
    div->setFixedHeight(1);
    lay->addWidget(div);

    // 歌单区域标题
    auto *plHeader = new QLabel(QStringLiteral("我的歌单"), container);
    plHeader->setObjectName("sbPlaylistTitle");
    lay->addWidget(plHeader);

    // 占位
    auto *empty = new QLabel(QStringLiteral("登录后查看"), container);
    empty->setObjectName("sbEmpty");
    empty->setAlignment(Qt::AlignCenter);
    lay->addWidget(empty);

    lay->addStretch();
    scroll->setWidget(container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}

QPushButton *Sidebar::createNavItem(const QString &key, const QString &label, const QIcon &icon)
{
    auto *btn = new QPushButton(label, this);
    btn->setObjectName("sbNavItem");
    btn->setFixedHeight(42);
    btn->setIcon(icon);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("navKey", key);
    connect(btn, &QPushButton::clicked, this, [this, key]() {
        setActiveNav(key);
        emit navigationRequested(key);
    });
    m_navBtns[key] = btn;
    return btn;
}

void Sidebar::setActiveNav(const QString &key)
{
    m_activeKey = key;
    for (auto it = m_navBtns.constBegin(); it != m_navBtns.constEnd(); ++it) {
        bool active = (it.key() == key);
        it.value()->setProperty("active", active);
        it.value()->style()->unpolish(it.value());
        it.value()->style()->polish(it.value());
        // 更新图标颜色
        if (it.key() == "home") {
            it.value()->setIcon(Icons::render(Icons::kHome, 20, navIconColor(active)));
        }
    }
}

void Sidebar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 重度毛玻璃背景
    QColor bg(36, 31, 49, 204);  // 80% 不透明
    p.fillRect(rect(), bg);

    // 薰衣草微光叠加（从左上角辐射）
    QRadialGradient glow(QPointF(0, 0), width() * 1.2);
    glow.setColorAt(0.0, QColor(196, 167, 231, 18));  // 7%
    glow.setColorAt(1.0, QColor(196, 167, 231, 0));
    p.fillRect(rect(), glow);

    // 右侧边线
    p.setPen(QPen(QColor(196, 167, 231, 25), 1));
    p.drawLine(rect().topRight(), rect().bottomRight());
}
