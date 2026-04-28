#include "core/playlistmanager.h"
#include "core/playlistdb.h"

#include <QRandomGenerator>

PlaylistManager& PlaylistManager::instance() {
    static PlaylistManager manager;
    return manager;
}

void PlaylistManager::load() {
    // Load from SQLite
    m_playlist = PlaylistDatabase::instance().getQueue();
    m_currentIndex = PlaylistDatabase::instance().getQueueCurrentIndex();
    m_playMode = PlaylistDatabase::instance().getQueuePlayMode();
}

void PlaylistManager::save() {
    // Save to SQLite
    PlaylistDatabase::instance().setQueueMusic(m_playlist, m_currentIndex);
    PlaylistDatabase::instance().setQueuePlayMode(m_playMode);
}

void PlaylistManager::addToPlaylist(const MusicInfo& music) {
    // 去重：按音乐 ID 检查是否已存在
    for (const auto& item : m_playlist) {
        if (item.id == music.id) {
            return; // 已存在，不重复添加
        }
    }
    m_playlist.append(music);
    if (m_currentIndex == -1) {
        m_currentIndex = 0;
    }
    PlaylistDatabase::instance().addToQueue(music);
    emit playlistChanged();
}

void PlaylistManager::addAllToPlaylist(const QList<MusicInfo>& musicList) {
    for (const auto& music : musicList) {
        bool exists = false;
        for (const auto& item : m_playlist) {
            if (item.id == music.id) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            m_playlist.append(music);
            PlaylistDatabase::instance().addToQueue(music);
        }
    }
    if (m_currentIndex == -1 && !m_playlist.isEmpty()) {
        m_currentIndex = 0;
    }
    emit playlistChanged();
}

void PlaylistManager::removeFromPlaylist(int localId) {
    int index = findIndexByLocalId(localId);
    if (index >= 0) {
        m_playlist.removeAt(index);
        if (m_currentIndex >= m_playlist.size()) {
            m_currentIndex = m_playlist.isEmpty() ? -1 : m_playlist.size() - 1;
        }
        // Rebuild queue in DB
        save();
        emit playlistChanged();
    }
}

void PlaylistManager::clearPlaylist() {
    m_playlist.clear();
    m_currentIndex = -1;
    PlaylistDatabase::instance().clearQueue();
    emit playlistChanged();
}

void PlaylistManager::setPlayMode(const QString& mode) {
    m_playMode = mode;
    PlaylistDatabase::instance().setQueuePlayMode(mode);
    emit playModeChanged(mode);
}

void PlaylistManager::togglePlayMode() {
    if (m_playMode == "list") {
        m_playMode = "single";
    } else if (m_playMode == "single") {
        m_playMode = "random";
    } else {
        m_playMode = "list";
    }
    PlaylistDatabase::instance().setQueuePlayMode(m_playMode);
    emit playModeChanged(m_playMode);
}

void PlaylistManager::setCurrentIndex(int index) {
    m_currentIndex = index;
    PlaylistDatabase::instance().setQueueCurrentIndex(index);
}

int PlaylistManager::nextIndex() const {
    if (m_playlist.isEmpty()) return -1;

    if (m_playMode == "single") {
        return m_currentIndex;
    } else if (m_playMode == "random") {
        if (m_playlist.size() <= 1) return m_currentIndex;
        int next;
        do {
            next = QRandomGenerator::global()->bounded(m_playlist.size());
        } while (next == m_currentIndex);
        return next;
    } else {
        // list mode: loop
        return (m_currentIndex + 1) % m_playlist.size();
    }
}

int PlaylistManager::previousIndex() const {
    if (m_playlist.isEmpty()) return -1;
    // 列表循环：向前一个
    return (m_currentIndex - 1 + m_playlist.size()) % m_playlist.size();
}

int PlaylistManager::findIndexByLocalId(int localId) const {
    for (int i = 0; i < m_playlist.size(); ++i) {
        if (m_playlist[i].id == localId) {
            return i;
        }
    }
    return -1;
}

MusicInfo PlaylistManager::lastPlayedMusic() const {
    if (m_currentIndex >= 0 && m_currentIndex < m_playlist.size()) {
        return m_playlist[m_currentIndex];
    }
    return MusicInfo();
}
