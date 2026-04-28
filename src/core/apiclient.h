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
};
