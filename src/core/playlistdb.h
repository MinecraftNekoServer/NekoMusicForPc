#ifndef PLAYLISTDB_H
#define PLAYLISTDB_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QString>
#include <QMutex>

#include "core/musicinfo.h"

struct LocalPlaylistInfo {
    int localId = 0;
    QString name;
    QString description;
    int coverMusicId = 0;
    QString createdAt;
    QString updatedAt;
};

class PlaylistDatabase {
public:
    static PlaylistDatabase& instance();

    bool init();
    void close();

    // Playlist CRUD
    int createPlaylist(const QString& name = "未命名歌单", const QString& description = "");
    bool deletePlaylist(int localId);
    bool updatePlaylist(int localId, const QString& name, const QString& description);
    QList<LocalPlaylistInfo> getAllPlaylists();
    LocalPlaylistInfo getPlaylist(int localId);

    // Playlist Music Operations
    bool addMusic(int playlistId, const MusicInfo& music);
    bool removeMusic(int playlistId, int musicId);
    QList<MusicInfo> getPlaylistMusic(int playlistId);
    int getPlaylistMusicCount(int playlistId);

    // Play Queue Operations
    void clearQueue();
    void addToQueue(const MusicInfo& music);
    void removeFromQueue(int queueId);
    void setQueueMusic(const QList<MusicInfo>& musicList, int currentIndex);
    QList<MusicInfo> getQueue();
    int getQueueCurrentIndex();
    void setQueueCurrentIndex(int index);
    QString getQueuePlayMode();
    void setQueuePlayMode(const QString& mode);

    // Recent Play Operations
    void recordRecentPlay(const MusicInfo& music);
    QList<MusicInfo> getRecentPlays(int limit = 65535);
    void clearRecentPlays();

private:
    PlaylistDatabase() = default;
    ~PlaylistDatabase() = default;
    PlaylistDatabase(const PlaylistDatabase&) = delete;
    PlaylistDatabase& operator=(const PlaylistDatabase&) = delete;

    bool createTables();

    QSqlDatabase m_db;
    QMutex m_mutex;
};

#endif // PLAYLISTDB_H
