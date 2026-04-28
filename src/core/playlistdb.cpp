#include "core/playlistdb.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

PlaylistDatabase& PlaylistDatabase::instance() {
    static PlaylistDatabase db;
    return db;
}

bool PlaylistDatabase::init() {
    QString dbDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbDir);
    QString dbPath = dbDir + "/playlists.db";

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        qWarning() << "Failed to open playlist database:" << m_db.lastError().text();
        return false;
    }

    return createTables();
}

void PlaylistDatabase::close() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool PlaylistDatabase::createTables() {
    QSqlQuery query;

    QString playlistsSql = R"(
        CREATE TABLE IF NOT EXISTS playlists (
            local_id       INTEGER PRIMARY KEY AUTOINCREMENT,
            name           TEXT NOT NULL DEFAULT '未命名歌单',
            description    TEXT NOT NULL DEFAULT '',
            cover_music_id INTEGER NOT NULL DEFAULT 0,
            created_at     TEXT NOT NULL DEFAULT (datetime('now')),
            updated_at     TEXT NOT NULL DEFAULT (datetime('now'))
        )
    )";

    if (!query.exec(playlistsSql)) {
        qWarning() << "Failed to create playlists table:" << query.lastError().text();
        return false;
    }

    QString musicSql = R"(
        CREATE TABLE IF NOT EXISTS playlist_music (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            playlist_id INTEGER NOT NULL REFERENCES playlists(local_id) ON DELETE CASCADE,
            music_id    INTEGER NOT NULL,
            title       TEXT NOT NULL DEFAULT '',
            artist      TEXT NOT NULL DEFAULT '',
            album       TEXT NOT NULL DEFAULT '',
            duration    INTEGER NOT NULL DEFAULT 0,
            cover_url   TEXT NOT NULL DEFAULT '',
            added_at    TEXT NOT NULL DEFAULT (datetime('now')),
            UNIQUE(playlist_id, music_id)
        )
    )";

    if (!query.exec(musicSql)) {
        qWarning() << "Failed to create playlist_music table:" << query.lastError().text();
        return false;
    }

    QString indexSql = "CREATE INDEX IF NOT EXISTS idx_playlist_music_pid ON playlist_music(playlist_id)";
    if (!query.exec(indexSql)) {
        qWarning() << "Failed to create index:" << query.lastError().text();
        return false;
    }

    return true;
}

int PlaylistDatabase::createPlaylist(const QString& name, const QString& description) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("INSERT INTO playlists (name, description) VALUES (:name, :description)");
    query.bindValue(":name", name);
    query.bindValue(":description", description);

    if (query.exec()) {
        return query.lastInsertId().toInt();
    } else {
        qWarning() << "Failed to create playlist:" << query.lastError().text();
        return -1;
    }
}

bool PlaylistDatabase::deletePlaylist(int localId) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("DELETE FROM playlists WHERE local_id = :id");
    query.bindValue(":id", localId);

    if (!query.exec()) {
        qWarning() << "Failed to delete playlist:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool PlaylistDatabase::updatePlaylist(int localId, const QString& name, const QString& description) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("UPDATE playlists SET name = :name, description = :description, updated_at = datetime('now') WHERE local_id = :id");
    query.bindValue(":name", name);
    query.bindValue(":description", description);
    query.bindValue(":id", localId);

    if (!query.exec()) {
        qWarning() << "Failed to update playlist:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

QList<LocalPlaylistInfo> PlaylistDatabase::getAllPlaylists() {
    QMutexLocker locker(&m_mutex);

    QList<LocalPlaylistInfo> playlists;
    QSqlQuery query("SELECT local_id, name, description, cover_music_id, created_at, updated_at FROM playlists ORDER BY updated_at DESC");

    while (query.next()) {
        LocalPlaylistInfo info;
        info.localId = query.value(0).toInt();
        info.name = query.value(1).toString();
        info.description = query.value(2).toString();
        info.coverMusicId = query.value(3).toInt();
        info.createdAt = query.value(4).toString();
        info.updatedAt = query.value(5).toString();
        playlists.append(info);
    }

    return playlists;
}

LocalPlaylistInfo PlaylistDatabase::getPlaylist(int localId) {
    QMutexLocker locker(&m_mutex);

    LocalPlaylistInfo info;
    QSqlQuery query;
    query.prepare("SELECT local_id, name, description, cover_music_id, created_at, updated_at FROM playlists WHERE local_id = :id");
    query.bindValue(":id", localId);

    if (query.exec() && query.next()) {
        info.localId = query.value(0).toInt();
        info.name = query.value(1).toString();
        info.description = query.value(2).toString();
        info.coverMusicId = query.value(3).toInt();
        info.createdAt = query.value(4).toString();
        info.updatedAt = query.value(5).toString();
    }

    return info;
}

bool PlaylistDatabase::addMusic(int playlistId, const MusicInfo& music) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("INSERT INTO playlist_music (playlist_id, music_id, title, artist, album, duration, cover_url) VALUES (:pid, :mid, :title, :artist, :album, :duration, :cover)");
    query.bindValue(":pid", playlistId);
    query.bindValue(":mid", music.id);
    query.bindValue(":title", music.title);
    query.bindValue(":artist", music.artist);
    query.bindValue(":album", music.album);
    query.bindValue(":duration", music.duration);
    query.bindValue(":cover", music.coverUrl);

    if (!query.exec()) {
        qWarning() << "Failed to add music to playlist:" << query.lastError().text();
        return false;
    }

    // Update playlist's updated_at
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE playlists SET updated_at = datetime('now') WHERE local_id = :id");
    updateQuery.bindValue(":id", playlistId);
    updateQuery.exec();

    return true;
}

bool PlaylistDatabase::removeMusic(int playlistId, int musicId) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("DELETE FROM playlist_music WHERE playlist_id = :pid AND music_id = :mid");
    query.bindValue(":pid", playlistId);
    query.bindValue(":mid", musicId);

    if (!query.exec()) {
        qWarning() << "Failed to remove music from playlist:" << query.lastError().text();
        return false;
    }

    // Update playlist's updated_at
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE playlists SET updated_at = datetime('now') WHERE local_id = :id");
    updateQuery.bindValue(":id", playlistId);
    updateQuery.exec();

    return query.numRowsAffected() > 0;
}

QList<MusicInfo> PlaylistDatabase::getPlaylistMusic(int playlistId) {
    QMutexLocker locker(&m_mutex);

    QList<MusicInfo> musicList;
    QSqlQuery query;
    query.prepare("SELECT music_id, title, artist, album, duration, cover_url FROM playlist_music WHERE playlist_id = :pid ORDER BY added_at ASC");
    query.bindValue(":pid", playlistId);

    if (query.exec()) {
        while (query.next()) {
            MusicInfo info;
            info.id = query.value(0).toInt();
            info.title = query.value(1).toString();
            info.artist = query.value(2).toString();
            info.album = query.value(3).toString();
            info.duration = query.value(4).toInt();
            info.coverUrl = query.value(5).toString();
            musicList.append(info);
        }
    } else {
        qWarning() << "Failed to get playlist music:" << query.lastError().text();
    }

    return musicList;
}

int PlaylistDatabase::getPlaylistMusicCount(int playlistId) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM playlist_music WHERE playlist_id = :pid");
    query.bindValue(":pid", playlistId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}
