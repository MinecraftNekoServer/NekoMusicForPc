#pragma once

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

#include "core/musicinfo.h"

class QTimer;

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
    float volume() const;
    void setPosition(qint64 position);
    void fadeIn();
    void fadeOut();
    void setCurrentMusic(const MusicInfo& music);

    PlaybackState playbackState() const;
    qint64 duration() const;
    qint64 position() const;

signals:
    void stateChanged(PlaybackState state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void fadeComplete();
    void musicStarted(const MusicInfo& music);

private:
    void onMediaStateChanged(QMediaPlayer::PlaybackState state);
    void onFadeTick();

    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    PlaybackState m_state = Stopped;
    float m_targetVolume = 1.0f;
    QTimer *m_fadeTimer = nullptr;
    bool m_fadingIn = false;
    bool m_fadingOut = false;
    MusicInfo m_currentMusic;
};
