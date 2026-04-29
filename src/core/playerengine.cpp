#include "playerengine.h"
#include <QTimer>

PlayerEngine::PlayerEngine(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
{
    m_player->setAudioOutput(m_audioOutput);

    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &PlayerEngine::onMediaStateChanged);
    connect(m_player, &QMediaPlayer::positionChanged,
            this, &PlayerEngine::positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &PlayerEngine::durationChanged);
    connect(m_player, &QMediaPlayer::errorOccurred,
            this, [this](QMediaPlayer::Error error, const QString &errorString) {
                Q_UNUSED(error);
                emit mediaError(errorString);
            });
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia) {
                    emit playbackFinished();
                }
            });
}

PlayerEngine::~PlayerEngine() = default;

void PlayerEngine::play(const QUrl &url)
{
    m_player->setSource(url);
    m_player->play();
}

void PlayerEngine::play()
{
    m_player->play();
}

void PlayerEngine::pause()
{
    m_player->pause();
}

void PlayerEngine::stop()
{
    m_player->stop();
}

void PlayerEngine::setVolume(float volume)
{
    m_targetVolume = qBound(0.0f, volume, 1.0f);
    m_audioOutput->setVolume(m_targetVolume);
}

void PlayerEngine::fadeIn()
{
    if (m_fadeTimer) { m_fadeTimer->stop(); delete m_fadeTimer; m_fadeTimer = nullptr; }
    m_fadingIn = true;
    m_fadingOut = false;
    m_audioOutput->setVolume(0.0f);
    m_player->play();

    m_fadeTimer = new QTimer(this);
    connect(m_fadeTimer, &QTimer::timeout, this, &PlayerEngine::onFadeTick);
    m_fadeTimer->start(20); // ~50 ticks for 1s fade
}

void PlayerEngine::fadeOut()
{
    if (m_fadeTimer) { m_fadeTimer->stop(); delete m_fadeTimer; m_fadeTimer = nullptr; }
    m_fadingOut = true;
    m_fadingIn = false;

    // Immediately update state so UI shows paused
    m_state = Paused;
    emit stateChanged(m_state);

    m_fadeTimer = new QTimer(this);
    connect(m_fadeTimer, &QTimer::timeout, this, &PlayerEngine::onFadeTick);
    m_fadeTimer->start(20);
}

void PlayerEngine::onFadeTick()
{
    if (!m_audioOutput) return;

    const float step = 0.04f;

    if (m_fadingIn) {
        float vol = m_audioOutput->volume() + step;
        if (vol >= m_targetVolume) {
            vol = m_targetVolume;
            m_fadingIn = false;
            m_fadeTimer->stop();
            delete m_fadeTimer;
            m_fadeTimer = nullptr;
            emit fadeComplete();
        }
        m_audioOutput->setVolume(vol);
    } else if (m_fadingOut) {
        float vol = m_audioOutput->volume() - step;
        if (vol <= 0.0f) {
            vol = 0.0f;
            m_audioOutput->setVolume(vol);
            m_player->pause();
            m_fadingOut = false;
            m_fadeTimer->stop();
            delete m_fadeTimer;
            m_fadeTimer = nullptr;
            emit fadeComplete();
        } else {
            m_audioOutput->setVolume(vol);
        }
    }
}

PlayerEngine::PlaybackState PlayerEngine::playbackState() const
{
    return m_state;
}

qint64 PlayerEngine::duration() const
{
    return m_player->duration();
}

qint64 PlayerEngine::position() const
{
    return m_player->position();
}

float PlayerEngine::volume() const
{
    return m_audioOutput ? m_audioOutput->volume() : 0.0f;
}

void PlayerEngine::setPosition(qint64 position)
{
    if (!m_player) return;
    if (m_seekLimitMs >= 0 && position > m_seekLimitMs) {
        position = m_seekLimitMs;
    }
    m_player->setPosition(position);
}

void PlayerEngine::onMediaStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        m_state = Playing;
        if (m_currentMusic.id > 0) {
            emit musicStarted(m_currentMusic);
        }
        break;
    case QMediaPlayer::PausedState:
        m_state = Paused;
        break;
    case QMediaPlayer::StoppedState:
        m_state = Stopped;
        // 播放结束时发出信号，用于自动切歌
        emit stateChanged(m_state);
        emit playbackFinished();
        break;
    }
    emit stateChanged(m_state);
}

void PlayerEngine::setCurrentMusic(const MusicInfo& music)
{
    m_currentMusic = music;
}
