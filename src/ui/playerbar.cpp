/**
 * @file playerbar.cpp
 * @brief 播放栏实现
 *
 * 80px，重度毛玻璃背景，薰衣草紫渐变顶线。
 * 播放按钮薰衣草渐变，进度条薰衣草填充。
 */

#include "playerbar.h"
#include "core/playerengine.h"
#include "theme/theme.h"
#include "ui/svgicon.h"
#include "core/i18n.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QStyle>

namespace {
const QColor kCtrlNormal = QColor(245, 240, 255, 180);
const QColor kCtrlActive = QColor(196, 167, 231, 255);
}

PlayerBar::PlayerBar(PlayerEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    setFixedHeight(Theme::kPlayerBarH);
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
}

void PlayerBar::setupUi()
{
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(20, 0, 20, 0);
    lay->setSpacing(0);

    // ─── 左侧：封面+信息 ────────────────────────────
    auto *left = new QWidget(this);
    left->setFixedWidth(240);
    auto *ll = new QHBoxLayout(left);
    ll->setContentsMargins(0, 0, 0, 0);
    ll->setSpacing(10);

    auto *cover = new QLabel(this);
    cover->setObjectName("pbCover");
    cover->setFixedSize(48, 48);
    QPixmap ph(48, 48);
    ph.fill(Qt::transparent);
    QPainter pp(&ph);
    QPainterPath ppp;
    ppp.addRoundedRect(0, 0, 48, 48, 8, 8);
    QLinearGradient g(0, 0, 48, 48);
    g.setColorAt(0.0, QColor(196, 167, 231));
    g.setColorAt(1.0, QColor(168, 139, 212));
    pp.fillPath(ppp, g);
    pp.end();
    cover->setPixmap(ph);
    ll->addWidget(cover);

    auto *infoL = new QVBoxLayout();
    infoL->setSpacing(2);
    m_songName = new QLabel(I18n::instance().tr("notPlaying"), this);
    m_songName->setObjectName("pbSong");
    infoL->addWidget(m_songName);
    m_artist = new QLabel(I18n::instance().tr("unknown"), this);
    m_artist->setObjectName("pbArtist");
    infoL->addWidget(m_artist);
    ll->addLayout(infoL);
    lay->addWidget(left);

    // ─── 中间：控制+进度 ────────────────────────────
    auto *center = new QWidget(this);
    auto *cl = new QVBoxLayout(center);
    cl->setContentsMargins(0, 6, 0, 6);
    cl->setSpacing(2);

    auto *ctrlL = new QHBoxLayout();
    ctrlL->setSpacing(16);
    ctrlL->setAlignment(Qt::AlignCenter);

    auto *prevBtn = new QPushButton(this);
    prevBtn->setObjectName("pbCtrlBtn");
    prevBtn->setFixedSize(32, 32);
    prevBtn->setIcon(Icons::icon(Icons::kPrev, 20, kCtrlNormal, kCtrlActive));
    prevBtn->setCursor(Qt::PointingHandCursor);
    prevBtn->setToolTip(I18n::instance().tr("previous"));
    ctrlL->addWidget(prevBtn);

    m_playBtn = new QPushButton(this);
    m_playBtn->setObjectName("pbPlayBtn");
    m_playBtn->setFixedSize(40, 40);
    m_playBtn->setIcon(Icons::icon(Icons::kPlay, 24, kCtrlNormal, kCtrlActive));
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->setToolTip(I18n::instance().tr("play"));
    ctrlL->addWidget(m_playBtn);

    auto *nextBtn = new QPushButton(this);
    nextBtn->setObjectName("pbCtrlBtn");
    nextBtn->setFixedSize(32, 32);
    nextBtn->setIcon(Icons::icon(Icons::kNext, 20, kCtrlNormal, kCtrlActive));
    nextBtn->setCursor(Qt::PointingHandCursor);
    nextBtn->setToolTip(I18n::instance().tr("next"));
    ctrlL->addWidget(nextBtn);

    cl->addLayout(ctrlL);

    // 进度条
    auto *progL = new QHBoxLayout();
    progL->setSpacing(6);
    auto *curTime = new QLabel(QStringLiteral("0:00"), this);
    curTime->setObjectName("pbTime");
    curTime->setFixedWidth(36);
    progL->addWidget(curTime);

    m_progress = new QSlider(Qt::Horizontal, this);
    m_progress->setObjectName("pbProgress");
    m_progress->setRange(0, 1000);
    m_progress->setValue(0);
    progL->addWidget(m_progress);

    auto *durTime = new QLabel(QStringLiteral("0:00"), this);
    durTime->setObjectName("pbTime");
    durTime->setFixedWidth(36);
    progL->addWidget(durTime);

    cl->addLayout(progL);
    lay->addWidget(center, 1);

    // ─── 右侧：音量 ─────────────────────────────────
    auto *right = new QWidget(this);
    right->setFixedWidth(160);
    auto *rl = new QHBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    rl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto *modeBtn = new QPushButton(this);
    modeBtn->setObjectName("pbCtrlBtn");
    modeBtn->setFixedSize(28, 28);
    modeBtn->setIcon(Icons::icon(Icons::kShuffle, 18, kCtrlNormal, kCtrlActive));
    modeBtn->setCursor(Qt::PointingHandCursor);
    modeBtn->setToolTip(I18n::instance().tr("playModeList"));
    rl->addWidget(modeBtn);

    m_volume = new QSlider(Qt::Horizontal, this);
    m_volume->setObjectName("pbVolume");
    m_volume->setFixedWidth(90);
    m_volume->setRange(0, 100);
    m_volume->setValue(80);
    rl->addWidget(m_volume);

    lay->addWidget(right);

    // 连接引擎
    if (m_engine) {
        connect(m_volume, &QSlider::valueChanged, this, [this](int v) {
            m_engine->setVolume(v / 100.0f);
        });
        connect(m_playBtn, &QPushButton::clicked, this, [this]() {
            if (m_engine->playbackState() == PlayerEngine::Playing) m_engine->pause();
            else m_engine->play();
        });
        connect(m_engine, &PlayerEngine::stateChanged, this, [this]() { updateState(); });
        // 进度条跟随播放位置
        connect(m_engine, &PlayerEngine::positionChanged, this, [this](qint64 pos) {
            if (m_engine->duration() > 0)
                m_progress->setValue(static_cast<int>(pos * 1000 / m_engine->duration()));
        });
        connect(m_engine, &PlayerEngine::durationChanged, this, [this](qint64 dur) {
            Q_UNUSED(dur)
        });
    }
}

void PlayerBar::retranslate()
{
    // Update default labels if still showing defaults
    if (m_songName && (m_songName->text() == "未在播放" || m_songName->text() == I18n::instance().tr("notPlaying")))
        m_songName->setText(I18n::instance().tr("notPlaying"));
    if (m_artist && (m_artist->text() == "--" || m_artist->text() == I18n::instance().tr("unknown")))
        m_artist->setText(I18n::instance().tr("unknown"));

    // Update tooltips by objectName
    auto *prevBtn = findChild<QPushButton *>("pbCtrlBtn", Qt::FindDirectChildrenOnly);
    if (prevBtn) prevBtn->setToolTip(I18n::instance().tr("previous"));

    if (m_playBtn) {
        bool playing = m_engine && m_engine->playbackState() == PlayerEngine::Playing;
        m_playBtn->setToolTip(playing ? I18n::instance().tr("pause") : I18n::instance().tr("play"));
    }

    auto *nextBtn = findChild<QPushButton *>("pbCtrlBtn", Qt::FindChildrenRecursively);
    // find all pbCtrlBtn buttons and set appropriate tooltips
    auto btns = findChildren<QPushButton *>();
    int ctrlCount = 0;
    for (auto *btn : btns) {
        if (btn->objectName() == "pbCtrlBtn") {
            if (ctrlCount == 0) btn->setToolTip(I18n::instance().tr("previous"));
            else if (ctrlCount == 1) btn->setToolTip(I18n::instance().tr("next"));
            else if (ctrlCount == 2) btn->setToolTip(I18n::instance().tr("playModeList"));
            ctrlCount++;
        }
    }
}

void PlayerBar::updateState()
{
    if (!m_engine) return;
    bool playing = m_engine->playbackState() == PlayerEngine::Playing;
    m_playBtn->setIcon(Icons::render(playing ? Icons::kPause : Icons::kPlay, 24, kCtrlNormal));
    m_playBtn->setToolTip(playing ? I18n::instance().tr("pause") : I18n::instance().tr("play"));
}

void PlayerBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QColor bg(36, 31, 49, 218);
    p.fillRect(rect(), bg);

    QLinearGradient line(rect().topLeft(), rect().topRight());
    line.setColorAt(0.0, QColor(196, 167, 231, 0));
    line.setColorAt(0.5, QColor(196, 167, 231, 50));
    line.setColorAt(1.0, QColor(196, 167, 231, 0));
    p.setPen(QPen(QBrush(line), 1));
    p.drawLine(rect().topLeft(), rect().topRight());
}
