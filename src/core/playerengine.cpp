#include "playerengine.h"
#include <QTimer>
#include <QDebug>

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
                qDebug() << "Media player error:" << error << errorString;
                // Stop playback on error to prevent hanging
                m_player->stop();
                emit stateChanged(Stopped);
                emit errorOccurred(errorString);
            });
}

PlayerEngine::~PlayerEngine() = default;

void PlayerEngine::play(const QUrl &url)
{
    m_player->setSource(url);
    m_player->play();
    
    // Set a timeout to prevent hanging on corrupted files
    QTimer::singleShot(5000, this, [this]() {
        if (m_player->playbackState() == QMediaPlayer::StoppedState && 
            m_player->mediaStatus() == QMediaPlayer::LoadingMedia) {
            qDebug() << "Playback timeout - stopping player";
            m_player->stop();
            emit stateChanged(Stopped);
            emit errorOccurred("播放超时，文件可能已损坏");
        }
    });
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

void PlayerEngine::setPosition(qint64 position)
{
    m_player->setPosition(position);
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
