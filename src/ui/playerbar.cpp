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
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

namespace {
const QColor kCtrlNormal = QColor(245, 240, 255, 180);
const QColor kCtrlActive = QColor(196, 167, 231, 255);

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
    
    // 创建定时器用于限制UI更新频率
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(100); // 每100ms更新一次UI
    connect(m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_engine) {
            qint64 pos = m_engine->position();
            qint64 dur = m_engine->duration();
            
            // 只有位置或时长发生变化时才更新UI
            if (pos != m_lastPosition) {
                m_lastPosition = pos;
                if (m_curTime) m_curTime->setText(formatTime(pos));
                if (dur > 0 && m_progress) {
                    m_progress->setValue(static_cast<int>(pos * 1000 / dur));
                }
            }
            
            if (dur != m_lastDuration) {
                m_lastDuration = dur;
                if (m_durTime) m_durTime->setText(formatTime(dur));
            }
        }
    });
    
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
    ctrlL->setSpacing(16);
    ctrlL->setAlignment(Qt::AlignCenter);

    auto *prevBtn = new QPushButton(this);
    prevBtn->setObjectName("pbCtrlBtn");
    prevBtn->setFixedSize(32, 32);
    prevBtn->setIcon(Icons::icon(Icons::kPrev, 20, kCtrlNormal, kCtrlActive));
    prevBtn->setCursor(Qt::PointingHandCursor);
    prevBtn->setToolTip(I18n::instance().tr("previous"));
    connect(prevBtn, &QPushButton::clicked, this, [this]() {
        emit previousClicked();
    });
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
    connect(nextBtn, &QPushButton::clicked, this, [this]() {
        emit nextClicked();
    });
    ctrlL->addWidget(nextBtn);

    m_playModeBtn = new QPushButton(this);
    m_playModeBtn->setObjectName("pbPlayModeBtn");
    m_playModeBtn->setFixedSize(32, 32);
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
    right->setFixedWidth(160);
    auto *rl = new QHBoxLayout(right);
    rl->setContentsMargins(0, 0, 0, 0);
    rl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rl->setSpacing(10);

    // 收藏按钮
    m_heartBtn = new QPushButton(this);
    m_heartBtn->setObjectName("pbHeartBtn");
    m_heartBtn->setFixedSize(28, 28);
    m_heartBtn->setIconSize(QSize(20, 20));
    m_heartBtn->setFlat(true);
    m_heartBtn->setIcon(QIcon(":/icons/heart_gray.png"));
    m_heartBtn->setCursor(Qt::PointingHandCursor);
    m_heartBtn->setToolTip(I18n::instance().tr("addToFavorites"));
    connect(m_heartBtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "[播放栏] 红心按钮点击, m_currentMusicId =" << m_currentMusicId;
        emit favoriteClicked(m_currentMusicId);
    });
    rl->addWidget(m_heartBtn);

    auto *playlistBtn = new QPushButton(this);
    playlistBtn->setObjectName("pbPlaylistBtn");
    playlistBtn->setFixedSize(28, 28);
    playlistBtn->setIcon(Icons::icon(Icons::kPlaylist, 18, kCtrlNormal, kCtrlActive));
    playlistBtn->setCursor(Qt::PointingHandCursor);
    playlistBtn->setToolTip(I18n::instance().tr("playlist"));
    connect(playlistBtn, &QPushButton::clicked, this, [this]() {
        emit playlistClicked();
    });
    rl->addWidget(playlistBtn);

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
            if (m_engine->playbackState() == PlayerEngine::Playing) m_engine->fadeOut();
            else m_engine->fadeIn();
        });
        connect(m_engine, &PlayerEngine::stateChanged, this, [this]() { 
            updateState();
            // 根据播放状态启动或停止定时器
            if (m_engine->playbackState() == PlayerEngine::Playing) {
                m_updateTimer->start();
            } else {
                m_updateTimer->stop();
            }
        });
        // 时长变化时更新
        connect(m_engine, &PlayerEngine::durationChanged, this, [this](qint64 dur) {
            m_lastDuration = dur;
            if (m_durTime) m_durTime->setText(formatTime(dur));
        });
        // 进度条拖动时更新预览时间（但不改变播放位置）
        connect(m_progress, &QSlider::sliderMoved, this, [this](int value) {
            if (m_engine && m_engine->duration() > 0) {
                qint64 position = static_cast<qint64>(value) * m_engine->duration() / 1000;
                if (m_curTime) m_curTime->setText(formatTime(position));
            }
        });
        
        // 进度条释放时改变播放位置
        connect(m_progress, &QSlider::sliderReleased, this, [this]() {
            if (m_engine && m_engine->duration() > 0) {
                int value = m_progress->value();
                qint64 position = static_cast<qint64>(value) * m_engine->duration() / 1000;
                m_engine->setPosition(position);
                // 更新最后位置，避免定时器重复更新
                m_lastPosition = position;
            }
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
        m_heartBtn->setIcon(QIcon(isFavorited ? ":/icons/heart_red.png" : ":/icons/heart_gray.png"));
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
    m_playBtn->setIcon(Icons::render(playing ? Icons::kPause : Icons::kPlay, 24, kCtrlNormal));
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
        int radius = 14;

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
