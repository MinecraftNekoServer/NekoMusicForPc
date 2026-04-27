/**
 * @file recentpage.cpp
 * @brief "最近播放"页面 — 空页面，仅显示标题
 */

#include "recentpage.h"
#include "core/i18n.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>

RecentPage::RecentPage(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
}

void RecentPage::setupUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scroll);
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(32, 32, 32, 32);
    lay->setSpacing(16);

    m_titleLabel = new QLabel(I18n::instance().tr("recentPlay"), container);
    m_titleLabel->setObjectName("recentTitle");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    lay->addWidget(m_titleLabel);

    lay->addStretch();

    scroll->setWidget(container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}

void RecentPage::paintEvent(QPaintEvent *)
{
    // 透明，由父窗口渐变背景透出
}
