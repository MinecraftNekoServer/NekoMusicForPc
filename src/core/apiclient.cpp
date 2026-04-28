/**
 * @file apiclient.cpp
 * @brief API 客户端实现
 */

#include "apiclient.h"
#include "theme/theme.h"
#include "core/usermanager.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFile>

ApiClient::ApiClient(QObject *parent) : QObject(parent) {}

void ApiClient::fetchPlaylists(const QString &query, PlaylistsCb cb)
{
    QUrl url(QString::fromUtf8("%1/api/playlists/search").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["query"] = query;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) { cb(false, {}); return; }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QList<QVariantMap> res;
        if (ok) for (const auto &v : doc.object().value("results").toArray())
            res.append(v.toObject().toVariantMap());
        cb(ok, res);
    });
}

void ApiClient::fetchRanking(MusicListCb cb)
{
    QUrl url(QString::fromUtf8("%1/api/music/ranking?t=%2")
                 .arg(Theme::kApiBase).arg(QDateTime::currentMSecsSinceEpoch()));
    auto *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) { cb(false, {}); return; }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QList<QVariantMap> res;
        if (ok) for (const auto &v : doc.object().value("data").toArray())
            res.append(v.toObject().toVariantMap());
        cb(ok, res);
    });
}

void ApiClient::fetchLatest(int limit, MusicListCb cb)
{
    QUrl url(QString::fromUtf8("%1/api/music/latest?limit=%2&t=%3")
                 .arg(Theme::kApiBase).arg(limit).arg(QDateTime::currentMSecsSinceEpoch()));
    auto *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) { cb(false, {}); return; }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QList<QVariantMap> res;
        if (ok) for (const auto &v : doc.object().value("data").toArray())
            res.append(v.toObject().toVariantMap());
        cb(ok, res);
    });
}

// ─── 用户认证 API ────────────────────────────────────

void ApiClient::login(const QString &username, const QString &password, AuthCb cb)
{
    QUrl url(QString::fromUtf8("%1/api/user/login").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            cb(false, reply->errorString(), {}, {});
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QString message = doc.object().value("message").toString();
        QString token;
        QVariantMap user;
        if (ok) {
            auto data = doc.object().value("data").toObject();
            token = data.value("token").toString();
            user = data.value("user").toObject().toVariantMap();
        }
        cb(ok, message, token, user);
    });
}

void ApiClient::registerUser(const QString &username, const QString &password,
                              const QString &email, const QString &verificationCode, AuthCb cb)
{
    QUrl url(QString::fromUtf8("%1/api/user/register").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;
    body["email"] = email;
    body["verificationCode"] = verificationCode;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            cb(false, reply->errorString(), {}, {});
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QString message = doc.object().value("message").toString();
        QString token;
        QVariantMap user;
        if (ok) {
            auto data = doc.object().value("data").toObject();
            token = data.value("token").toString();
            user = data.value("user").toObject().toVariantMap();
        }
        cb(ok, message, token, user);
    });
}

void ApiClient::sendVerificationCode(const QString &email, std::function<void(bool, const QString&)> cb)
{
    QUrl url(QString::fromUtf8("%1/api/user/send-verification").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["email"] = email;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            cb(false, reply->errorString());
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QString message = doc.object().value("message").toString();
        cb(ok, message);
    });
}

// ─── 忘记密码 API ────────────────────────────────────

void ApiClient::sendResetCode(const QString &email, std::function<void(bool, const QString&)> cb)
{
    QUrl url(QString::fromUtf8("%1/api/user/send-reset-code").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["email"] = email;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            cb(false, reply->errorString());
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QString message = doc.object().value("message").toString();
        cb(ok, message);
    });
}

void ApiClient::resetPassword(const QString &email, const QString &verificationCode,
                               const QString &newPassword, std::function<void(bool, const QString&)> cb)
{
    QUrl url(QString::fromUtf8("%1/api/user/reset-password").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["email"] = email;
    body["verificationCode"] = verificationCode;
    body["newPassword"] = newPassword;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            cb(false, reply->errorString());
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QString message = doc.object().value("message").toString();
        cb(ok, message);
    });
}

// ─── 用户上传 API ────────────────────────────────────

void ApiClient::uploadMusic(const QString &musicFilePath, const QString &title,
                             const QString &artist, const QString &language, int duration,
                             int uploadUserId, const QString &album,
                             const QString &tags, const QString &coverFilePath,
                             const QString &lyricsFilePath, UploadCb cb)
{
    QUrl url(QString::fromUtf8("%1/api/user/upload").arg(Theme::kApiBase));
    QNetworkRequest req(url);

    // Add auth token
    if (UserManager::instance().isLoggedIn()) {
        req.setRawHeader("Authorization", UserManager::instance().token().toUtf8());
    }

    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // Helper lambda to add text part
    auto addTextPart = [multiPart](const QString &name, const QString &value) {
        QHttpPart textPart;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QString("form-data; name=\"%1\"").arg(name));
        textPart.setBody(value.toUtf8());
        multiPart->append(textPart);
    };

    addTextPart("title", title);
    addTextPart("artist", artist);
    addTextPart("language", language);
    addTextPart("duration", QString::number(duration));
    addTextPart("uploadUserId", QString::number(uploadUserId));
    if (!album.isEmpty()) addTextPart("album", album);
    if (!tags.isEmpty()) addTextPart("tags", tags);

    // Music file (required)
    QFile *musicFile = new QFile(musicFilePath);
    if (musicFile->open(QIODevice::ReadOnly)) {
        QHttpPart musicPart;
        QString ext = musicFilePath.mid(musicFilePath.lastIndexOf('.') + 1).toLower();
        musicPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/mpeg");
        musicPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QString("form-data; name=\"musicFile\"; filename=\"%1\"")
                                .arg(musicFilePath.mid(musicFilePath.lastIndexOf('/') + 1)));
        musicPart.setBodyDevice(musicFile);
        musicFile->setParent(multiPart);
        multiPart->append(musicPart);
    } else {
        delete musicFile;
        if (cb) cb(false, "无法打开音乐文件");
        multiPart->deleteLater();
        return;
    }

    // Cover file (optional)
    if (!coverFilePath.isEmpty()) {
        QFile *coverFile = new QFile(coverFilePath);
        if (coverFile->open(QIODevice::ReadOnly)) {
            QHttpPart coverPart;
            coverPart.setHeader(QNetworkRequest::ContentTypeHeader, "image/jpeg");
            coverPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                QString("form-data; name=\"coverFile\"; filename=\"%1\"")
                                    .arg(coverFilePath.mid(coverFilePath.lastIndexOf('/') + 1)));
            coverPart.setBodyDevice(coverFile);
            coverFile->setParent(multiPart);
            multiPart->append(coverPart);
        } else {
            delete coverFile;
        }
    }

    // Lyrics file (optional)
    if (!lyricsFilePath.isEmpty()) {
        QFile *lyricsFile = new QFile(lyricsFilePath);
        if (lyricsFile->open(QIODevice::ReadOnly)) {
            QHttpPart lyricsPart;
            lyricsPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                 QString("form-data; name=\"lyricsFile\"; filename=\"%1\"")
                                     .arg(lyricsFilePath.mid(lyricsFilePath.lastIndexOf('/') + 1)));
            lyricsPart.setBodyDevice(lyricsFile);
            lyricsFile->setParent(multiPart);
            multiPart->append(lyricsPart);
        } else {
            delete lyricsFile;
        }
    }

    auto *reply = m_nam.post(req, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (cb) cb(false, reply->errorString());
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QString message = doc.object().value("message").toString();
        if (cb) cb(ok, message);
    });
}
