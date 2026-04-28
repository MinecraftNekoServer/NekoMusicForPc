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

    // Play Queue table
    QString queueSql = R"(
        CREATE TABLE IF NOT EXISTS play_queue (
            queue_id    INTEGER PRIMARY KEY AUTOINCREMENT,
            music_id    INTEGER NOT NULL,
            title       TEXT NOT NULL DEFAULT '',
            artist      TEXT NOT NULL DEFAULT '',
            album       TEXT NOT NULL DEFAULT '',
            duration    INTEGER NOT NULL DEFAULT 0,
            cover_url   TEXT NOT NULL DEFAULT '',
            queue_order INTEGER NOT NULL DEFAULT 0
        )
    )";

    if (!query.exec(queueSql)) {
        qWarning() << "Failed to create play_queue table:" << query.lastError().text();
        return false;
    }

    // Play Queue state table
    QString queueStateSql = R"(
        CREATE TABLE IF NOT EXISTS play_queue_state (
            key         TEXT PRIMARY KEY,
            value       TEXT NOT NULL
        )
    )";

    if (!query.exec(queueStateSql)) {
        qWarning() << "Failed to create play_queue_state table:" << query.lastError().text();
        return false;
    }

    // Recent Play table
    QString recentSql = R"(
        CREATE TABLE IF NOT EXISTS recent_play (
            music_id    INTEGER PRIMARY KEY,
            title       TEXT NOT NULL DEFAULT '',
            artist      TEXT NOT NULL DEFAULT '',
            album       TEXT NOT NULL DEFAULT '',
            duration    INTEGER NOT NULL DEFAULT 0,
            cover_url   TEXT NOT NULL DEFAULT '',
            played_at   TEXT NOT NULL DEFAULT (datetime('now'))
        )
    )";

    if (!query.exec(recentSql)) {
        qWarning() << "Failed to create recent_play table:" << query.lastError().text();
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

// ─── Play Queue Methods ──────────────────────────────────────────────

void PlaylistDatabase::clearQueue() {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.exec("DELETE FROM play_queue");
    query.exec("DELETE FROM play_queue_state");
}

void PlaylistDatabase::addToQueue(const MusicInfo& music) {
    QMutexLocker locker(&m_mutex);

    // Check if already exists
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM play_queue WHERE music_id = :mid");
    checkQuery.bindValue(":mid", music.id);
    checkQuery.exec();
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        return; // Already in queue
    }

    // Get max queue_order
    QSqlQuery orderQuery("SELECT COALESCE(MAX(queue_order), 0) + 1 FROM play_queue");
    orderQuery.exec();
    int order = 0;
    if (orderQuery.next()) {
        order = orderQuery.value(0).toInt();
    }

    QString title = music.title.isEmpty() ? "" : music.title;
    QString artist = music.artist.isEmpty() ? "" : music.artist;
    QString album = music.album.isEmpty() ? "" : music.album;
    QString cover = music.coverUrl.isEmpty() ? "" : music.coverUrl;

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO play_queue (music_id, title, artist, album, duration, cover_url, queue_order) VALUES (:mid, :title, :artist, :album, :duration, :cover, :order)");
    insertQuery.bindValue(":mid", music.id);
    insertQuery.bindValue(":title", title);
    insertQuery.bindValue(":artist", artist);
    insertQuery.bindValue(":album", album);
    insertQuery.bindValue(":duration", music.duration);
    insertQuery.bindValue(":cover", cover);
    insertQuery.bindValue(":order", order);

    if (!insertQuery.exec()) {
        qWarning() << "Failed to add to queue:" << insertQuery.lastError().text();
    }
}

void PlaylistDatabase::removeFromQueue(int queueId) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("DELETE FROM play_queue WHERE queue_id = :qid");
    query.bindValue(":qid", queueId);

    if (!query.exec()) {
        qWarning() << "Failed to remove from queue:" << query.lastError().text();
    }
}

void PlaylistDatabase::setQueueMusic(const QList<MusicInfo>& musicList, int currentIndex) {
    QMutexLocker locker(&m_mutex);

    // Clear existing queue
    QSqlQuery clearQuery("DELETE FROM play_queue");
    clearQuery.exec();

    // Insert all music
    int order = 0;
    for (const auto& music : musicList) {
        QString title = music.title.isEmpty() ? "" : music.title;
        QString artist = music.artist.isEmpty() ? "" : music.artist;
        QString album = music.album.isEmpty() ? "" : music.album;
        QString cover = music.coverUrl.isEmpty() ? "" : music.coverUrl;

        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO play_queue (music_id, title, artist, album, duration, cover_url, queue_order) VALUES (:mid, :title, :artist, :album, :duration, :cover, :order)");
        insertQuery.bindValue(":mid", music.id);
        insertQuery.bindValue(":title", title);
        insertQuery.bindValue(":artist", artist);
        insertQuery.bindValue(":album", album);
        insertQuery.bindValue(":duration", music.duration);
        insertQuery.bindValue(":cover", cover);
        insertQuery.bindValue(":order", order++);

        if (!insertQuery.exec()) {
            qWarning() << "Failed to set queue music:" << insertQuery.lastError().text();
        }
    }

    // Save current index
    QSqlQuery indexQuery;
    indexQuery.prepare("INSERT OR REPLACE INTO play_queue_state (key, value) VALUES ('currentIndex', :value)");
    indexQuery.bindValue(":value", QString::number(currentIndex));
    indexQuery.exec();
}

QList<MusicInfo> PlaylistDatabase::getQueue() {
    QMutexLocker locker(&m_mutex);

    QList<MusicInfo> musicList;
    QSqlQuery query("SELECT queue_id, music_id, title, artist, album, duration, cover_url FROM play_queue ORDER BY queue_order ASC");

    if (query.exec()) {
        while (query.next()) {
            MusicInfo info;
            info.id = query.value(1).toInt();
            info.title = query.value(2).toString();
            info.artist = query.value(3).toString();
            info.album = query.value(4).toString();
            info.duration = query.value(5).toInt();
            info.coverUrl = query.value(6).toString();
            musicList.append(info);
        }
    } else {
        qWarning() << "Failed to get queue:" << query.lastError().text();
    }

    return musicList;
}

int PlaylistDatabase::getQueueCurrentIndex() {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT value FROM play_queue_state WHERE key = 'currentIndex'");
    if (query.exec() && query.next()) {
        return query.value(0).toString().toInt();
    }
    return -1;
}

void PlaylistDatabase::setQueueCurrentIndex(int index) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO play_queue_state (key, value) VALUES ('currentIndex', :value)");
    query.bindValue(":value", QString::number(index));

    if (!query.exec()) {
        qWarning() << "Failed to set queue current index:" << query.lastError().text();
    }
}

QString PlaylistDatabase::getQueuePlayMode() {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT value FROM play_queue_state WHERE key = 'playMode'");
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "list"; // Default mode
}

void PlaylistDatabase::setQueuePlayMode(const QString& mode) {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO play_queue_state (key, value) VALUES ('playMode', :value)");
    query.bindValue(":value", mode);

    if (!query.exec()) {
        qWarning() << "Failed to set queue play mode:" << query.lastError().text();
    }
}

// ─── Recent Play Methods ──────────────────────────────────────────────

void PlaylistDatabase::recordRecentPlay(const MusicInfo& music) {
    QMutexLocker locker(&m_mutex);

    QString title = music.title.isEmpty() ? "" : music.title;
    QString artist = music.artist.isEmpty() ? "" : music.artist;
    QString album = music.album.isEmpty() ? "" : music.album;
    QString cover = music.coverUrl.isEmpty() ? "" : music.coverUrl;

    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO recent_play (music_id, title, artist, album, duration, cover_url, played_at) VALUES (:mid, :title, :artist, :album, :duration, :cover, datetime('now'))");
    query.bindValue(":mid", music.id);
    query.bindValue(":title", title);
    query.bindValue(":artist", artist);
    query.bindValue(":album", album);
    query.bindValue(":duration", music.duration);
    query.bindValue(":cover", cover);

    if (!query.exec()) {
        qWarning() << "Failed to record recent play:" << query.lastError().text();
        return;
    }

    // Keep only last 65535 records
    QSqlQuery cleanupQuery("DELETE FROM recent_play WHERE music_id NOT IN (SELECT music_id FROM recent_play ORDER BY played_at DESC LIMIT 65535)");
    cleanupQuery.exec();
}

QList<MusicInfo> PlaylistDatabase::getRecentPlays(int limit) {
    QMutexLocker locker(&m_mutex);

    QList<MusicInfo> musicList;
    QSqlQuery query;
    query.prepare("SELECT music_id, title, artist, album, duration, cover_url FROM recent_play ORDER BY played_at DESC LIMIT :limit");
    query.bindValue(":limit", limit);

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
        qWarning() << "Failed to get recent plays:" << query.lastError().text();
    }

    return musicList;
}

void PlaylistDatabase::clearRecentPlays() {
    QMutexLocker locker(&m_mutex);

    QSqlQuery query("DELETE FROM recent_play");
    if (!query.exec()) {
        qWarning() << "Failed to clear recent plays:" << query.lastError().text();
    }
}
