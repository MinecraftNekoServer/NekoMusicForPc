#include "playerengine.h"

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
    m_audioOutput->setVolume(qBound(0.0f, volume, 1.0f));
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

void PlayerEngine::onMediaStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        m_state = Playing;
        break;
    case QMediaPlayer::PausedState:
        m_state = Paused;
        break;
    case QMediaPlayer::StoppedState:
        m_state = Stopped;
        break;
    }
    emit stateChanged(m_state);
}
