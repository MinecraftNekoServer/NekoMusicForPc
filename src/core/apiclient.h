#pragma once

/**
 * @file apiclient.h
 * @brief 后端 API 客户端
 */

#include <QObject>
#include <QNetworkAccessManager>
#include <functional>

class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);

    // ─── 音乐相关 ────────────────────────────────────
    using PlaylistsCb = std::function<void(bool, const QList<QVariantMap>&)>;
    void fetchPlaylists(const QString &query, PlaylistsCb cb);

    using MusicListCb = std::function<void(bool, const QList<QVariantMap>&)>;
    void fetchRanking(MusicListCb cb);
    void fetchLatest(int limit, MusicListCb cb);
    void fetchFavorites(MusicListCb cb);
    
    // 音乐搜索
    using MusicSearchCb = std::function<void(bool, int, int, int, const QList<QVariantMap>&)>;
    void searchMusic(const QString &query, int page = 1, int pageSize = 20, MusicSearchCb cb = nullptr);
    
    // 获取音乐信息
    using MusicInfoCb = std::function<void(bool, const QVariantMap&)>;
    void fetchMusicInfo(int musicId, MusicInfoCb cb);
    
    // 获取歌词
    using LyricsCb = std::function<void(bool, const QString&)>;
    void fetchLyrics(int musicId, LyricsCb cb);
    
    // 搜索歌手
    using ArtistSearchCb = std::function<void(bool, const QVariantMap&)>;
    void searchArtists(const QString &query, ArtistSearchCb cb);
    
    // 获取用户上传审核通过的音乐列表
    using UploadedMusicCb = std::function<void(bool, int, const QList<QVariantMap>&)>;
    void fetchUploadedMusic(UploadedMusicCb cb);
    
    // 修改用户密码
    void changePassword(const QString &oldPassword, const QString &newPassword, std::function<void(bool, const QString&)> cb);

    // ─── 歌单相关 ────────────────────────────────────
    using PlaylistDetailCb = std::function<void(bool, const QVariantMap&)>;
    void fetchPlaylistDetail(int playlistId, PlaylistDetailCb cb);
    
    using UserPlaylistsCb = std::function<void(bool, const QList<QVariantMap>&)>;
    void fetchUserPlaylists(UserPlaylistsCb cb);
    
    using CreatePlaylistCb = std::function<void(bool, const QString&, const QVariantMap&)>;
    void createPlaylist(const QString &name, const QString &description, CreatePlaylistCb cb);
    
    using UpdatePlaylistCb = std::function<void(bool, const QString&, const QVariantMap&)>;
    void updatePlaylist(int playlistId, const QString &name, const QString &description, UpdatePlaylistCb cb);
    
    using DeletePlaylistCb = std::function<void(bool, const QString&)>;
    void deletePlaylist(int playlistId, DeletePlaylistCb cb);
    
    using PlaylistMusicCb = std::function<void(bool, int, const QList<QVariantMap>&)>;
    void fetchPlaylistMusic(int playlistId, PlaylistMusicCb cb);
    
    using AddMusicToPlaylistCb = std::function<void(bool, const QString&)>;
    void addMusicToPlaylist(int playlistId, int musicId, AddMusicToPlaylistCb cb);
    
    using RemoveMusicFromPlaylistCb = std::function<void(bool, const QString&)>;
    void removeMusicFromPlaylist(int playlistId, int musicId, RemoveMusicFromPlaylistCb cb);
    
    // 收藏歌单相关
    void fetchFavoritePlaylists(UserPlaylistsCb cb);
    void favoritePlaylist(int playlistId, AddMusicToPlaylistCb cb);
    void unfavoritePlaylist(int playlistId, RemoveMusicFromPlaylistCb cb);
    void fetchFavoritePlaylistMusic(int playlistId, PlaylistMusicCb cb);

    // ─── 用户认证 ────────────────────────────────────
    using AuthCb = std::function<void(bool success, const QString &message,
                                       const QString &token, const QVariantMap &user)>;
    void login(const QString &username, const QString &password, AuthCb cb);
    void registerUser(const QString &username, const QString &password,
                      const QString &email, const QString &verificationCode, AuthCb cb);
    void sendVerificationCode(const QString &email, std::function<void(bool, const QString&)> cb);

    // ─── 忘记密码 ────────────────────────────────────
    void sendResetCode(const QString &email, std::function<void(bool, const QString&)> cb);
    void resetPassword(const QString &email, const QString &verificationCode,
                       const QString &newPassword, std::function<void(bool, const QString&)> cb);

    // ─── 用户上传 ────────────────────────────────────
    using UploadCb = std::function<void(bool, const QString&)>;
    void uploadMusic(const QString &musicFilePath, const QString &title,
                     const QString &artist, const QString &language, int duration,
                     int uploadUserId, const QString &album = QString(),
                     const QString &tags = QString(),
                     const QString &coverFilePath = QString(),
                     const QString &lyricsFilePath = QString(),
                     UploadCb cb = nullptr);

private:
    QNetworkAccessManager m_nam;
    QString getAuthToken() const;
};
