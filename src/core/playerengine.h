#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

class PlayerEngine : public QObject
{
    Q_OBJECT

public:
    enum PlaybackState {
        Stopped,
        Playing,
        Paused
    };
    Q_ENUM(PlaybackState)

    explicit PlayerEngine(QObject *parent = nullptr);
    ~PlayerEngine() override;

    void play(const QUrl &url);
    void play();
    void pause();
    void stop();
    void setVolume(float volume);

    PlaybackState playbackState() const;

signals:
    void stateChanged(PlaybackState state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);

private:
    void onMediaStateChanged(QMediaPlayer::PlaybackState state);

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    PlaybackState m_state = Stopped;
};
