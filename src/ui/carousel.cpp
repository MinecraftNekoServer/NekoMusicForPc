/**
 * @file carousel.cpp
 * @brief 全屏大图轮播实现
 *
 * 大图背景 + 底部渐变毛玻璃遮罩 + 歌单信息叠加。
 * 5s 自动轮播，左右箭头，底部薄荷绿圆点指示器。
 * 切换时淡出→更新→淡入动画。
 */

#include "carousel.h"
#include "theme/theme.h"
#include "ui/svgicon.h"
#include "core/i18n.h"
#include "core/covercache.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QLabel>
#include <QStyle>
#include <QPropertyAnimation>

namespace {
const QColor kArrowNormal = QColor(245, 240, 255, 160);
const QColor kArrowActive = QColor(126, 200, 200, 230);
}

Carousel::Carousel(QWidget *parent) : QWidget(parent)
{
    setupUi();

    m_timer.setInterval(Theme::kCarouselMs);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        if (m_items.size() > 1 && !m_animating)
            goToIndex((m_currentIndex + 1) % m_items.size());
    });
}

void Carousel::setupUi()
{
    setAttribute(Qt::WA_StyledBackground, false);
    setMinimumHeight(280);

    // 整体淡入淡出效果
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(1.0);
    setGraphicsEffect(m_opacityEffect);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // 大图背景
    m_bgLabel = new QLabel(this);
    m_bgLabel->setObjectName("carouselBg");
    m_bgLabel->setAlignment(Qt::AlignCenter);
    m_bgLabel->setScaledContents(true);
    lay->addWidget(m_bgLabel, 1);

    // 叠加层
    auto *overlay = new QWidget(this);
    overlay->setObjectName("carouselOverlay");
    overlay->setFixedHeight(120);

    auto *olay = new QHBoxLayout(overlay);
    olay->setContentsMargins(32, 0, 32, 16);
    olay->setSpacing(0);

    auto *infoW = new QWidget(overlay);
    auto *ilay = new QVBoxLayout(infoW);
    ilay->setContentsMargins(0, 0, 0, 0);
    ilay->setSpacing(4);

    m_titleLabel = new QLabel(infoW);
    m_titleLabel->setObjectName("carouselTitle");
    ilay->addWidget(m_titleLabel);

    m_descLabel = new QLabel(infoW);
    m_descLabel->setObjectName("carouselDesc");
    ilay->addWidget(m_descLabel);

    olay->addWidget(infoW, 1);

    m_playBtn = new QPushButton(this);
    m_playBtn->setObjectName("carouselPlayBtn");
    m_playBtn->setFixedSize(100, 40);
    m_playBtn->setText(I18n::instance().tr("play"));
    m_playBtn->setCursor(Qt::PointingHandCursor);
    connect(m_playBtn, &QPushButton::clicked, this, [this]() {
        if (m_currentIndex >= 0 && m_currentIndex < m_items.size())
            emit itemClicked(m_items[m_currentIndex].playlistId);
    });
    olay->addWidget(m_playBtn);

    lay->addWidget(overlay);

    // 底部控制
    auto *ctrlW = new QWidget(this);
    ctrlW->setFixedHeight(36);
    auto *clay = new QHBoxLayout(ctrlW);
    clay->setContentsMargins(16, 0, 16, 4);

    auto *prevBtn = createArrowBtn("carouselPrev");
    prevBtn->setIcon(Icons::icon(Icons::kLeft, 18, kArrowNormal, kArrowActive));
    connect(prevBtn, &QPushButton::clicked, this, [this]() {
        if (m_items.size() > 1 && !m_animating)
            goToIndex((m_currentIndex - 1 + m_items.size()) % m_items.size());
    });
    clay->addWidget(prevBtn);

    clay->addStretch();

    m_dotsWidget = new QWidget(ctrlW);
    m_dotsWidget->setObjectName("carouselDots");
    auto *dotsLay = new QHBoxLayout(m_dotsWidget);
    dotsLay->setContentsMargins(0, 0, 0, 0);
    dotsLay->setSpacing(6);
    clay->addWidget(m_dotsWidget, 0, Qt::AlignCenter);

    clay->addStretch();

    auto *nextBtn = createArrowBtn("carouselNext");
    nextBtn->setIcon(Icons::icon(Icons::kRight, 18, kArrowNormal, kArrowActive));
    connect(nextBtn, &QPushButton::clicked, this, [this]() {
        if (m_items.size() > 1 && !m_animating)
            goToIndex((m_currentIndex + 1) % m_items.size());
    });
    clay->addWidget(nextBtn);

    lay->addWidget(ctrlW);
}

QPushButton *Carousel::createArrowBtn(const QString &objName)
{
    auto *btn = new QPushButton(this);
    btn->setObjectName(objName);
    btn->setFixedSize(32, 32);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

void Carousel::setItems(const QList<CarouselItem> &items)
{
    m_items = items;

    auto *dotsLay = qobject_cast<QHBoxLayout *>(m_dotsWidget->layout());
    QLayoutItem *child;
    while ((child = dotsLay->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    for (int i = 0; i < items.size(); ++i) {
        auto *dot = new QLabel(m_dotsWidget);
        dot->setObjectName("carouselDot");
        dot->setFixedSize(8, 8);
        dot->setAlignment(Qt::AlignCenter);
        dotsLay->addWidget(dot);
    }

    if (!items.isEmpty()) {
        m_currentIndex = 0;
        updateDisplay();
        m_timer.start();
    }
}

void Carousel::goToIndex(int index)
{
    if (index == m_currentIndex) return;
    m_currentIndex = index;
    animateTransition();
}

void Carousel::animateTransition()
{
    if (m_animating) return;
    m_animating = true;

    // 淡出
    auto *fadeOut = new QPropertyAnimation(m_opacityEffect, "opacity");
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutCubic);

    connect(fadeOut, &QPropertyAnimation::finished, this, [this]() {
        // 更新内容
        updateDisplay();

        // 淡入
        auto *fadeIn = new QPropertyAnimation(m_opacityEffect, "opacity");
        fadeIn->setDuration(300);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::InCubic);

        connect(fadeIn, &QPropertyAnimation::finished, this, [this]() {
            m_animating = false;
        });
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void Carousel::updateDisplay()
{
    if (m_items.isEmpty()) return;
    const auto &item = m_items[m_currentIndex];

    m_titleLabel->setText(item.title);
    m_descLabel->setText(item.description);

    // 渐变占位
    QPixmap ph(width() > 0 ? width() : 600, qMax(height(), 280));
    ph.fill(Qt::transparent);
    QPainter p(&ph);
    QLinearGradient g(0, 0, ph.width(), ph.height());
    g.setColorAt(0.0, QColor(196, 167, 231, 80));
    g.setColorAt(0.5, QColor(242, 172, 185, 50));
    g.setColorAt(1.0, QColor(36, 31, 49, 200));
    p.fillRect(ph.rect(), g);
    p.end();
    m_bgLabel->setPixmap(ph);

    // 异步加载封面（先查缓存）
    if (!item.coverUrl.isEmpty()) {
        QString musicId = item.coverUrl.mid(item.coverUrl.lastIndexOf(QLatin1Char('/')) + 1);

        QPixmap cached = CoverCache::instance()->get(musicId);
        if (!cached.isNull()) {
            m_bgLabel->setPixmap(cached.scaled(m_bgLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            return;
        }

        connect(CoverCache::instance(), &CoverCache::coverLoaded, this,
                [this, musicId](const QString &id, const QPixmap &pix) {
            if (id == musicId) {
                m_bgLabel->setPixmap(pix.scaled(m_bgLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            }
        });
        CoverCache::instance()->fetchCover(musicId, item.coverUrl);
    }

    // 更新圆点
    auto dots = m_dotsWidget->findChildren<QLabel *>(QStringLiteral("carouselDot"));
    for (int i = 0; i < dots.size(); ++i) {
        bool active = (i == m_currentIndex);
        dots[i]->setProperty("active", active);
        dots[i]->style()->unpolish(dots[i]);
        dots[i]->style()->polish(dots[i]);
    }
}

void Carousel::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    if (!m_items.isEmpty()) updateDisplay();
}
