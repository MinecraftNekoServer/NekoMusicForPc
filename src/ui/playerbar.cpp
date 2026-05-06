/**
 * @file playerbar.cpp
 * @brief 播放栏实现
 *
 * 80px，重度毛玻璃背景，薰衣草紫渐变顶线。
 * 播放按钮薰衣草渐变，进度条薰衣草填充。
 */

#include "playerbar.h"
#include "core/playerengine.h"
#include "core/playlistmanager.h"
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
#include <QFont>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QEvent>
#include <QCursor>
#include <QRect>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QSettings>

namespace {
// 与 old PlayerBar.vue 的 .control-btn / .play-btn 观感对齐：更大、更亮
const QColor kPbIconNormal = QColor(255, 255, 255, 210);
const QColor kPbIconActive = QColor(255, 255, 255, 255);
const QColor kPbIconAccent = QColor(212, 196, 255, 255);
const QColor kPbPlayGlyph = QColor(26, 22, 37, 255);
const QColor kPbHeartOn = QColor(255, 69, 69, 255);
constexpr int kPbCtrlBtn = 38;
constexpr int kPbPlayBtn = 48;
constexpr int kPbCtrlIcon = 22;
constexpr int kPbPlayIcon = 28;
constexpr int kVolumePanelExtraLiftPx = 16;
constexpr int kVolumeLeaveAutoHideMs = 3000;
const QString kSettingsKeyVolume = QStringLiteral("player/volume");

QString formatTime(qint64 ms) {
    qint64 sec = ms / 1000;
    return QString("%1:%2").arg(sec / 60).arg(sec % 60, 2, 10, QChar('0'));
}
}

PlayerBar::PlayerBar(PlayerEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    setFixedHeight(Theme::kPlayerBarH);
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
    
    if (m_engine) {
        connect(m_engine, &PlayerEngine::positionChanged, this, [this](qint64 position) {
            if (m_progress && m_engine) {
                qint64 duration = m_engine->duration();
                if (duration > 0) {
                    int value = static_cast<int>((position * 1000) / duration);
                    m_progress->setValue(value);
                }
            }
        });
        
        connect(m_engine, &PlayerEngine::durationChanged, this, [this](qint64 duration) {
            // 可以更新总时长显示
        });
    }
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

    auto *coverBtn = new QPushButton(this);
    coverBtn->setObjectName("pbCover");
    coverBtn->setFixedSize(48, 48);
    coverBtn->setCursor(Qt::PointingHandCursor);
    coverBtn->setFlat(true);
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
    coverBtn->setIcon(QIcon(ph));
    coverBtn->setIconSize(QSize(48, 48));
    m_cover = coverBtn;
    connect(coverBtn, &QPushButton::clicked, this, [this]() {
        emit coverClicked();
    });
    ll->addWidget(coverBtn);

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
    ctrlL->setSpacing(12);
    ctrlL->setAlignment(Qt::AlignCenter);

    auto *prevBtn = new QPushButton(this);
    prevBtn->setObjectName("pbCtrlBtn");
    prevBtn->setFixedSize(kPbCtrlBtn, kPbCtrlBtn);
    prevBtn->setIconSize(QSize(kPbCtrlIcon, kPbCtrlIcon));
    prevBtn->setIcon(Icons::icon(Icons::kPrev, kPbCtrlIcon, kPbIconNormal, kPbIconAccent));
    prevBtn->setCursor(Qt::PointingHandCursor);
    prevBtn->setToolTip(I18n::instance().tr("previous"));
    connect(prevBtn, &QPushButton::clicked, this, [this]() {
        emit previousClicked();
    });
    ctrlL->addWidget(prevBtn);

    m_playBtn = new QPushButton(this);
    m_playBtn->setObjectName("pbPlayBtn");
    m_playBtn->setFixedSize(kPbPlayBtn, kPbPlayBtn);
    m_playBtn->setIconSize(QSize(kPbPlayIcon, kPbPlayIcon));
    m_playBtn->setIcon(Icons::icon(Icons::kPlay, kPbPlayIcon, kPbPlayGlyph, kPbPlayGlyph));
    m_playBtn->setCursor(Qt::PointingHandCursor);
    m_playBtn->setToolTip(I18n::instance().tr("play"));
    ctrlL->addWidget(m_playBtn);

    auto *nextBtn = new QPushButton(this);
    nextBtn->setObjectName("pbCtrlBtn");
    nextBtn->setFixedSize(kPbCtrlBtn, kPbCtrlBtn);
    nextBtn->setIconSize(QSize(kPbCtrlIcon, kPbCtrlIcon));
    nextBtn->setIcon(Icons::icon(Icons::kNext, kPbCtrlIcon, kPbIconNormal, kPbIconAccent));
    nextBtn->setCursor(Qt::PointingHandCursor);
    nextBtn->setToolTip(I18n::instance().tr("next"));
    connect(nextBtn, &QPushButton::clicked, this, [this]() {
        emit nextClicked();
    });
    ctrlL->addWidget(nextBtn);

    m_playModeBtn = new QPushButton(this);
    m_playModeBtn->setObjectName("pbPlayModeBtn");
    m_playModeBtn->setFixedSize(kPbCtrlBtn, kPbCtrlBtn);
    m_playModeBtn->setIconSize(QSize(24, 24));
    m_playModeBtn->setIcon(QIcon(":/icons/icon_list_loop.png"));
    m_playModeBtn->setCursor(Qt::PointingHandCursor);
    m_playModeBtn->setToolTip(I18n::instance().tr("playModeList"));
    connect(m_playModeBtn, &QPushButton::clicked, this, [this]() {
        emit playModeClicked();
    });
    ctrlL->addWidget(m_playModeBtn);

    cl->addLayout(ctrlL);

    // 进度条
    auto *progL = new QHBoxLayout();
    progL->setSpacing(6);
    auto *curTime = new QLabel(QStringLiteral("0:00"), this);
    curTime->setObjectName("pbTime");
    curTime->setFixedWidth(36);
    m_curTime = curTime;
    progL->addWidget(curTime);

    m_progress = new QSlider(Qt::Horizontal, this);
    m_progress->setObjectName("pbProgress");
    m_progress->setRange(0, 1000);
    m_progress->setValue(0);
    progL->addWidget(m_progress);

    auto *durTime = new QLabel(QStringLiteral("0:00"), this);
    durTime->setObjectName("pbTime");
    durTime->setFixedWidth(36);
    m_durTime = durTime;
    progL->addWidget(durTime);

    cl->addLayout(progL);
    lay->addWidget(center, 1);

    // ─── 右侧：收藏+音量 ─────────────────────────────────
    auto *right = new QWidget(this);
    right->setFixedWidth(184);
    auto *rl = new QHBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    rl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rl->setSpacing(8);

    // 收藏按钮（与 old 相同 SVG 心形路径）
    m_heartBtn = new QPushButton(this);
    m_heartBtn->setObjectName("pbHeartBtn");
    m_heartBtn->setFixedSize(kPbCtrlBtn, kPbCtrlBtn);
    m_heartBtn->setIconSize(QSize(kPbCtrlIcon, kPbCtrlIcon));
    m_heartBtn->setFlat(true);
    m_heartBtn->setIcon(Icons::icon(Icons::kHeart, kPbCtrlIcon, kPbIconNormal, kPbIconAccent));
    m_heartBtn->setCursor(Qt::PointingHandCursor);
    m_heartBtn->setToolTip(I18n::instance().tr("addToFavorites"));
    connect(m_heartBtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "[播放栏] 红心按钮点击, m_currentMusicId =" << m_currentMusicId;
        emit favoriteClicked(m_currentMusicId);
    });
    rl->addWidget(m_heartBtn);

    m_desktopLrcBtn = new QPushButton(QStringLiteral("词"), this);
    m_desktopLrcBtn->setObjectName("pbDesktopLrcBtn");
    m_desktopLrcBtn->setFixedSize(kPbCtrlBtn, kPbCtrlBtn);
    m_desktopLrcBtn->setFlat(true);
    m_desktopLrcBtn->setCheckable(true);
    m_desktopLrcBtn->setCursor(Qt::PointingHandCursor);
    {
        QFont f = m_desktopLrcBtn->font();
        f.setPixelSize(15);
        f.setWeight(QFont::DemiBold);
        m_desktopLrcBtn->setFont(f);
    }
    m_desktopLrcBtn->setToolTip(I18n::instance().tr("desktopLyrics"));
    {
        QSettings lrcSettings;
        m_desktopLrcBtn->setChecked(lrcSettings.value(QStringLiteral("desktopLyrics"), true).toBool());
    }
    connect(m_desktopLrcBtn, &QPushButton::toggled, this, [this](bool on) {
        QSettings s;
        s.setValue(QStringLiteral("desktopLyrics"), on);
        emit desktopLyricsToggled(on);
    });
    rl->addWidget(m_desktopLrcBtn);

    auto *playlistBtn = new QPushButton(this);
    playlistBtn->setObjectName("pbPlaylistBtn");
    playlistBtn->setFixedSize(kPbCtrlBtn, kPbCtrlBtn);
    playlistBtn->setIconSize(QSize(kPbCtrlIcon, kPbCtrlIcon));
    playlistBtn->setIcon(Icons::icon(Icons::kPlaylist, kPbCtrlIcon, kPbIconNormal, kPbIconAccent));
    playlistBtn->setCursor(Qt::PointingHandCursor);
    playlistBtn->setToolTip(I18n::instance().tr("playlist"));
    connect(playlistBtn, &QPushButton::clicked, this, [this]() {
        emit playlistClicked();
    });
    rl->addWidget(playlistBtn);

    // 音量控制
    auto *volWrapper = new QWidget(this);
    volWrapper->setObjectName("pbVolumeWrapper");
    auto *volLay = new QHBoxLayout(volWrapper);
    volLay->setContentsMargins(0, 0, 0, 0);
    volLay->setSpacing(0);

    m_volumeBtn = new QPushButton(this);
    m_volumeBtn->setObjectName("pbVolumeBtn");
    m_volumeBtn->setFixedSize(kPbCtrlBtn, kPbCtrlBtn);
    m_volumeBtn->setIconSize(QSize(kPbCtrlIcon, kPbCtrlIcon));
    m_volumeBtn->setIcon(Icons::icon(Icons::kVolumeHigh, kPbCtrlIcon, kPbIconNormal, kPbIconAccent));
    m_volumeBtn->setCursor(Qt::PointingHandCursor);
    volLay->addWidget(m_volumeBtn);
    rl->addWidget(volWrapper);

    // 音量面板 (垂直弹出) — 父级用顶层窗口，避免面板在播放栏上方时被父控件裁剪
    QWidget *volHost = window();
    if (!volHost)
        volHost = this;
    m_volumePanel = new QWidget(volHost);
    m_volumePanel->setObjectName("pbVolumePanel");
    m_volumePanel->setFixedWidth(40);
    m_volumePanel->setFixedHeight(160);
    m_volumePanel->setFocusPolicy(Qt::NoFocus);
    m_volumePanel->hide();
    m_volumePanel->installEventFilter(this);
    volWrapper->installEventFilter(this);

    auto *vpLay = new QVBoxLayout(m_volumePanel);
    vpLay->setContentsMargins(8, 12, 8, 12);
    vpLay->setSpacing(8);

    m_volumeSlider = new QSlider(Qt::Vertical, m_volumePanel);
    m_volumeSlider->setObjectName("pbVolumeSlider");
    m_volumeSlider->setRange(0, 100);
    vpLay->addWidget(m_volumeSlider, 1, Qt::AlignHCenter);

    QSettings volSettings;
    const int initialVol = qBound(0, volSettings.value(kSettingsKeyVolume, 80).toInt(), 100);

    m_volumeLabel = new QLabel(QStringLiteral("%1%").arg(initialVol), m_volumePanel);
    m_volumeLabel->setObjectName("pbVolumeLabel");
    m_volumeLabel->setAlignment(Qt::AlignCenter);
    vpLay->addWidget(m_volumeLabel);

    // 参考 old 中 .volume-panel 的 opacity 0.2s ease，并加轻微上滑
    m_volumeOpacityFx = new QGraphicsOpacityEffect(m_volumePanel);
    m_volumeOpacityFx->setOpacity(0.0);
    m_volumePanel->setGraphicsEffect(m_volumeOpacityFx);
    m_volumeOpAnim = new QPropertyAnimation(m_volumeOpacityFx, "opacity", this);
    m_volumePosAnim = new QPropertyAnimation(m_volumePanel, "pos", this);
    connect(m_volumeOpAnim, &QPropertyAnimation::finished, this, [this]() {
        if (!m_volumePanelClosing || !m_volumePanel)
            return;
        m_volumePanel->hide();
        m_volumePanelClosing = false;
        removeVolumePanelAppFilter();
    });

    m_volumeLeaveTimer = new QTimer(this);
    m_volumeLeaveTimer->setSingleShot(true);
    m_volumeLeaveTimer->setInterval(kVolumeLeaveAutoHideMs);
    connect(m_volumeLeaveTimer, &QTimer::timeout, this, [this]() {
        if (!m_volumePanel || !m_volumePanel->isVisible() || m_volumePanelClosing)
            return;
        if (!volumePanelHotRectGlobal().contains(QCursor::pos()))
            hideVolumePanelAnimated();
    });

    lay->addWidget(right);

    // 连接引擎
    if (m_engine) {
        connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int v) {
            m_engine->setVolume(v / 100.0f);
            m_volumeLabel->setText(QString("%1%").arg(v));
            updateVolumeIcon(v);
        });
        connect(m_volumeSlider, &QSlider::sliderReleased, this, [this]() {
            if (!m_volumeSlider)
                return;
            QSettings s;
            s.setValue(kSettingsKeyVolume, m_volumeSlider->value());
        });
        connect(m_playBtn, &QPushButton::clicked, this, [this]() {
            if (m_engine->playbackState() == PlayerEngine::Playing) m_engine->fadeOut();
            else m_engine->fadeIn();
        });
        connect(m_engine, &PlayerEngine::stateChanged, this, [this]() { updateState(); });
        // 进度条跟随播放位置
        connect(m_engine, &PlayerEngine::positionChanged, this, [this](qint64 pos) {
            if (m_curTime) m_curTime->setText(formatTime(pos));
            if (m_engine->duration() > 0) {
                m_progress->setValue(static_cast<int>(pos * 1000 / m_engine->duration()));
            }
        });
        connect(m_engine, &PlayerEngine::durationChanged, this, [this](qint64 dur) {
            if (m_durTime) m_durTime->setText(formatTime(dur));
        });
        // 进度条控制播放位置
        connect(m_progress, &QSlider::sliderReleased, this, [this]() {
            // 释放时应用最终位置
            if (m_engine && m_engine->duration() > 0) {
                qint64 position = static_cast<qint64>(m_progress->value()) * m_engine->duration() / 1000;
                m_engine->setPosition(position);
            }
        });
        m_volumeSlider->setValue(initialVol);
        updateVolumeIcon(initialVol);
    }
}

PlayerBar::~PlayerBar()
{
    removeVolumePanelAppFilter();
}

QRect PlayerBar::volumePanelHotRectGlobal() const
{
    if (!m_volumeBtn || !m_volumePanel)
        return {};
    const QRect btnGlobal(m_volumeBtn->mapToGlobal(QPoint(0, 0)), m_volumeBtn->size());
    const QRect panelGlobal(m_volumePanel->mapToGlobal(QPoint(0, 0)), m_volumePanel->size());
    return btnGlobal.united(panelGlobal);
}

void PlayerBar::installVolumePanelAppFilter()
{
    if (m_volumeAppFilterInstalled)
        return;
    if (QCoreApplication *app = QCoreApplication::instance()) {
        app->installEventFilter(this);
        m_volumeAppFilterInstalled = true;
    }
}

void PlayerBar::removeVolumePanelAppFilter()
{
    if (!m_volumeAppFilterInstalled)
        return;
    if (QCoreApplication *app = QCoreApplication::instance())
        app->removeEventFilter(this);
    m_volumeAppFilterInstalled = false;
}

void PlayerBar::showVolumePanelAnimated()
{
    if (!m_volumePanel || !m_volumeBtn || !m_volumeOpacityFx || !m_volumeOpAnim || !m_volumePosAnim)
        return;

    m_volumePanelClosing = false;
    m_volumeOpAnim->stop();
    m_volumePosAnim->stop();
    if (m_volumeLeaveTimer)
        m_volumeLeaveTimer->stop();

    QWidget *host = m_volumePanel->parentWidget();
    if (!host)
        host = window();
    if (!host)
        host = this;
    const QPoint ref = m_volumeBtn->mapTo(host, QPoint(0, 0));
    const int x = ref.x() + (m_volumeBtn->width() - m_volumePanel->width()) / 2;
    const int overlap = 8;
    const int y = ref.y() - m_volumePanel->height() + overlap - kVolumePanelExtraLiftPx;
    const QPoint finalPos(x, y);
    const QPoint startPos(x, y + 10);

    m_volumePanel->move(startPos);
    m_volumeOpacityFx->setOpacity(0.0);
    m_volumePanel->show();
    m_volumePanel->raise();

    m_volumeOpAnim->setDuration(200);
    m_volumeOpAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_volumeOpAnim->setStartValue(0.0);
    m_volumeOpAnim->setEndValue(1.0);

    m_volumePosAnim->setDuration(200);
    m_volumePosAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_volumePosAnim->setStartValue(startPos);
    m_volumePosAnim->setEndValue(finalPos);

    m_volumeOpAnim->start();
    m_volumePosAnim->start();

    installVolumePanelAppFilter();
}

void PlayerBar::hideVolumePanelAnimated()
{
    if (!m_volumePanel || !m_volumeOpacityFx || !m_volumeOpAnim)
        return;
    if (!m_volumePanel->isVisible())
        return;

    if (m_volumeLeaveTimer)
        m_volumeLeaveTimer->stop();

    m_volumeOpAnim->stop();
    if (m_volumePosAnim)
        m_volumePosAnim->stop();

    m_volumePanelClosing = true;
    m_volumeOpAnim->setDuration(180);
    m_volumeOpAnim->setEasingCurve(QEasingCurve::InCubic);
    m_volumeOpAnim->setStartValue(m_volumeOpacityFx->opacity());
    m_volumeOpAnim->setEndValue(0.0);
    m_volumeOpAnim->start();
}

bool PlayerBar::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress && m_volumePanel && m_volumePanel->isVisible()
        && !m_volumePanelClosing) {
        auto *me = static_cast<QMouseEvent *>(event);
        const QPoint g = me->globalPosition().toPoint();
        if (!volumePanelHotRectGlobal().contains(g))
            hideVolumePanelAnimated();
    }

    if (watched->objectName() == "pbVolumeWrapper") {
        if (event->type() == QEvent::Enter) {
            if (m_volumeLeaveTimer)
                m_volumeLeaveTimer->stop();
            if (m_volumePanel && m_volumeBtn)
                showVolumePanelAnimated();
        } else if (event->type() == QEvent::Leave) {
            if (m_volumePanel && m_volumePanel->isVisible()) {
                if (!volumePanelHotRectGlobal().contains(QCursor::pos())) {
                    if (m_volumeLeaveTimer)
                        m_volumeLeaveTimer->start();
                } else if (m_volumeLeaveTimer) {
                    m_volumeLeaveTimer->stop();
                }
            }
        }
    } else if (watched == m_volumePanel) {
        if (event->type() == QEvent::Enter) {
            if (m_volumeLeaveTimer)
                m_volumeLeaveTimer->stop();
        } else if (event->type() == QEvent::Leave) {
            if (!volumePanelHotRectGlobal().contains(QCursor::pos())) {
                if (m_volumeLeaveTimer)
                    m_volumeLeaveTimer->start();
            } else if (m_volumeLeaveTimer) {
                m_volumeLeaveTimer->stop();
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void PlayerBar::updateVolumeIcon(int value)
{
    if (!m_volumeBtn) return;
    const char *path = Icons::kVolumeHigh;
    if (value == 0) path = Icons::kVolumeMute;
    else if (value < 50) path = Icons::kVolumeLow;
    
    m_volumeBtn->setIcon(Icons::icon(path, kPbCtrlIcon, kPbIconNormal, kPbIconAccent));
}

void PlayerBar::retranslate()
{
    // Update default labels if still showing defaults
    if (m_songName && (m_songName->text() == "未在播放" || m_songName->text() == I18n::instance().tr("notPlaying")))
        m_songName->setText(I18n::instance().tr("notPlaying"));
    if (m_artist && (m_artist->text() == "--" || m_artist->text() == I18n::instance().tr("unknown")))
        m_artist->setText(I18n::instance().tr("unknown"));

    if (m_playBtn) {
        bool playing = m_engine && m_engine->playbackState() == PlayerEngine::Playing;
        m_playBtn->setToolTip(playing ? I18n::instance().tr("pause") : I18n::instance().tr("play"));
    }

    // Update prev/next tooltips by finding buttons in order
    auto btns = findChildren<QPushButton *>();
    int ctrlCount = 0;
    for (auto *btn : btns) {
        if (btn->objectName() == "pbCtrlBtn") {
            if (ctrlCount == 0) btn->setToolTip(I18n::instance().tr("previous"));
            else if (ctrlCount == 1) btn->setToolTip(I18n::instance().tr("next"));
            ctrlCount++;
        }
    }

    if (m_desktopLrcBtn)
        m_desktopLrcBtn->setToolTip(I18n::instance().tr("desktopLyrics"));
}

void PlayerBar::setSongInfo(const QString &title, const QString &artist, const QString &coverUrl)
{
    if (m_songName) m_songName->setText(title.isEmpty() ? I18n::instance().tr("unknown") : title);
    if (m_artist) m_artist->setText(artist.isEmpty() ? I18n::instance().tr("unknown") : artist);

    if (m_cover && !coverUrl.isEmpty()) {
        if (coverUrl.startsWith("http")) {
            loadCoverAsync(coverUrl);
        } else {
            QPixmap pm(coverUrl);
            if (!pm.isNull()) setCoverPixmap(pm);
        }
    }
}

void PlayerBar::setCoverVisible(bool visible)
{
    if (m_cover) m_cover->setVisible(visible);
}

void PlayerBar::setCurrentMusicId(int musicId)
{
    qDebug() << "[播放栏] 设置当前音乐ID:" << musicId;
    m_currentMusicId = musicId;
    // 不重置状态，由调用方自行检查收藏状态后设置
}

void PlayerBar::setFavoriteStatus(bool isFavorited)
{
    qDebug() << "[播放栏] setFavoriteStatus:" << isFavorited;
    m_isFavorited = isFavorited;
    if (m_heartBtn) {
        if (isFavorited)
            m_heartBtn->setIcon(Icons::icon(Icons::kHeart, kPbCtrlIcon, kPbHeartOn, QColor(255, 120, 120)));
        else
            m_heartBtn->setIcon(Icons::icon(Icons::kHeart, kPbCtrlIcon, kPbIconNormal, kPbIconAccent));
        m_heartBtn->setToolTip(isFavorited ? I18n::instance().tr("removeFromFavorites") : I18n::instance().tr("addToFavorites"));
    }
}

void PlayerBar::setLoading(bool loading)
{
    if (m_isLoading == loading) return;
    m_isLoading = loading;
    m_loadingAngle = 0;

    if (loading) {
        if (m_playBtn) {
            m_playBtn->setIcon(QIcon());
            m_playBtn->setToolTip(I18n::instance().tr("loading"));
        }
        QTimer *timer = new QTimer(this);
        timer->setObjectName("loadingTimer");
        timer->setInterval(30);
        connect(timer, &QTimer::timeout, this, [this, timer]() {
            if (!m_isLoading) { timer->stop(); return; }
            m_loadingAngle = (m_loadingAngle + 12) % 360;
            update();
        });
        timer->start();
    } else {
        if (QTimer *timer = findChild<QTimer *>("loadingTimer")) {
            timer->stop();
            timer->deleteLater();
        }
        updateState();
    }
}

void PlayerBar::setCoverPixmap(const QPixmap &pm)
{
    auto *btn = qobject_cast<QPushButton *>(m_cover);
    if (!btn || pm.isNull()) return;
    QPixmap scaled = pm.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap rounded(48, 48);
    rounded.fill(Qt::transparent);
    QPainter p(&rounded);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(0, 0, 48, 48, 8, 8);
    p.setClipPath(path);
    p.drawPixmap(0, 0, scaled);
    btn->setIcon(QIcon(rounded));
}

void PlayerBar::loadCoverAsync(const QString &url)
{
    static QNetworkAccessManager nam;
    auto *reply = nam.get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        QPixmap pm;
        if (pm.loadFromData(reply->readAll())) {
            setCoverPixmap(pm);
        }
    });
}

void PlayerBar::updateState()
{
    if (!m_engine) return;
    bool playing = m_engine->playbackState() == PlayerEngine::Playing;
    m_playBtn->setIcon(Icons::render(playing ? Icons::kPause : Icons::kPlay, kPbPlayIcon, kPbPlayGlyph));
    m_playBtn->setToolTip(playing ? I18n::instance().tr("pause") : I18n::instance().tr("play"));
}

void PlayerBar::updatePlayModeBtn(const QString &mode)
{
    if (!m_playModeBtn) return;
    if (mode == "single") {
        m_playModeBtn->setIcon(QIcon(":/icons/icon_single_loop.png"));
        m_playModeBtn->setToolTip(I18n::instance().tr("playModeSingle"));
    } else if (mode == "random") {
        m_playModeBtn->setIcon(QIcon(":/icons/icon_shuffle.png"));
        m_playModeBtn->setToolTip(I18n::instance().tr("playModeRandom"));
    } else {
        m_playModeBtn->setIcon(QIcon(":/icons/icon_list_loop.png"));
        m_playModeBtn->setToolTip(I18n::instance().tr("playModeList"));
    }
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

    // Draw loading spinner on play button area
    if (m_isLoading && m_playBtn) {
        QRect btnRect = m_playBtn->geometry();
        QPoint center = btnRect.center();
        int radius = 18;

        p.save();
        p.translate(center);
        p.rotate(m_loadingAngle);

        QPen pen(QColor(196, 167, 231, 220), 2.5);
        pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);

        // Draw arc (270 degrees, leaving a gap for spinner effect)
        p.drawArc(-radius, -radius, radius * 2, radius * 2, 0, 270 * 16);
        p.restore();
    }
}
