#include "core/playlistmanager.h"

#include <QRandomGenerator>
#include <QDebug>

PlaylistManager& PlaylistManager::instance() {
    static PlaylistManager manager;
    return manager;
}

void PlaylistManager::load() {
    QSettings settings;
    int size = settings.beginReadArray("playlist");
    m_playlist.clear();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        MusicInfo info;
        info.id = settings.value("id").toInt();
        info.title = settings.value("title").toString();
        info.artist = settings.value("artist").toString();
        info.album = settings.value("album").toString();
        info.duration = settings.value("duration").toInt();
        info.coverUrl = settings.value("coverUrl").toString();
        m_playlist.append(info);
    }
    settings.endArray();

    m_currentIndex = settings.value("currentIndex", -1).toInt();
    m_playMode = settings.value("playMode", "list").toString();
}

void PlaylistManager::save() {
    QSettings settings;
    settings.beginWriteArray("playlist");
    for (int i = 0; i < m_playlist.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("id", m_playlist[i].id);
        settings.setValue("title", m_playlist[i].title);
        settings.setValue("artist", m_playlist[i].artist);
        settings.setValue("album", m_playlist[i].album);
        settings.setValue("duration", m_playlist[i].duration);
        settings.setValue("coverUrl", m_playlist[i].coverUrl);
    }
    settings.endArray();

    settings.setValue("currentIndex", m_currentIndex);
    settings.setValue("playMode", m_playMode);
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
    save();
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
        }
    }
    if (m_currentIndex == -1 && !m_playlist.isEmpty()) {
        m_currentIndex = 0;
    }
    save();
    emit playlistChanged();
}

void PlaylistManager::removeFromPlaylist(int localId) {
    int index = findIndexByLocalId(localId);
    if (index >= 0) {
        m_playlist.removeAt(index);
        if (m_currentIndex >= m_playlist.size()) {
            m_currentIndex = m_playlist.isEmpty() ? -1 : m_playlist.size() - 1;
        }
        save();
        emit playlistChanged();
    }
}

void PlaylistManager::clearPlaylist() {
    m_playlist.clear();
    m_currentIndex = -1;
    save();
    emit playlistChanged();
}

void PlaylistManager::setPlayMode(const QString& mode) {
    m_playMode = mode;
    save();
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
    save();
    emit playModeChanged(m_playMode);
}

void PlaylistManager::setCurrentIndex(int index) {
    m_currentIndex = index;
    save();
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
