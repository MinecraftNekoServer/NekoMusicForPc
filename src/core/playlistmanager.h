#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QSettings>
#include "core/musicinfo.h"

class PlaylistManager : public QObject {
    Q_OBJECT

public:
    static PlaylistManager& instance();

    void load();
    void save();

    void addToPlaylist(const MusicInfo& music);
    void addAllToPlaylist(const QList<MusicInfo>& musicList);
    void removeFromPlaylist(int localId);
    void clearPlaylist();

    QList<MusicInfo> playlist() const { return m_playlist; }
    int count() const { return m_playlist.size(); }

    // 恢复上次播放
    bool hasLastPlayed() const { return m_currentIndex >= 0 && !m_playlist.isEmpty(); }
    MusicInfo lastPlayedMusic() const;

    // Play mode: "list" (loop), "single", "random"
    void setPlayMode(const QString& mode);
    QString playMode() const { return m_playMode; }
    void togglePlayMode();

    // Navigation
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);
    int nextIndex() const;
    int previousIndex() const;

signals:
    void playlistChanged();
    void playRequested(int localId);
    void playModeChanged(const QString& mode);

private:
    PlaylistManager() = default;
    ~PlaylistManager() = default;
    PlaylistManager(const PlaylistManager&) = delete;
    PlaylistManager& operator=(const PlaylistManager&) = delete;

    int findIndexByLocalId(int localId) const;

    QList<MusicInfo> m_playlist;
    int m_currentIndex = -1;
    QString m_playMode = "list"; // list, single, random
};

#endif // PLAYLISTMANAGER_H
