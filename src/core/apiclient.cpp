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

// 现有函数
void ApiClient::fetchRanking(MusicListCb cb) {
    QUrl url(QString::fromUtf8("%1/api/music/ranking?t=%2").arg(Theme::kApiBase).arg(QDateTime::currentMSecsSinceEpoch()));
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

void ApiClient::fetchLatest(int limit, MusicListCb cb) {
    QUrl url(QString::fromUtf8("%1/api/music/latest?limit=%2&t=%3").arg(Theme::kApiBase).arg(limit).arg(QDateTime::currentMSecsSinceEpoch()));
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

void ApiClient::fetchFavorites(MusicListCb cb) {
    QUrl url(QString::fromUtf8("%1/api/user/favorites").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    if (UserManager::instance().isLoggedIn()) {
        req.setRawHeader("Authorization", UserManager::instance().token().toUtf8());
    }
    auto *reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) { cb(false, {}); return; }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QList<QVariantMap> res;
        if (ok) for (const auto &v : doc.object().value("favorites").toArray())
            res.append(v.toObject().toVariantMap());
        cb(ok, res);
    });
}

void ApiClient::login(const QString &username, const QString &password, AuthCb cb) {
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
                              const QString &email, const QString &verificationCode, AuthCb cb) {
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

void ApiClient::sendVerificationCode(const QString &email, std::function<void(bool, const QString&)> cb) {
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

void ApiClient::sendResetCode(const QString &email, std::function<void(bool, const QString&)> cb) {
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
                               const QString &newPassword, std::function<void(bool, const QString&)> cb) {
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

// 新实现的API
void ApiClient::searchMusic(const QString &query, int page, int pageSize, MusicSearchCb cb) {
    QUrl url(QString::fromUtf8("%1/api/music/search").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["query"] = query;
    body["page"] = page;
    body["pageSize"] = pageSize;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb, query]() {
        reply->deleteLater();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "[搜索API]搜索:" << query << "，HTTP状态码:" << statusCode << "，错误:" << reply->errorString();
            if (cb) cb(false, 0, 0, 0, {});
            return;
        }
        auto response = reply->readAll();
        auto doc = QJsonDocument::fromJson(response);
        bool ok = doc.object().value("success").toBool();
        QList<QVariantMap> results;
        int total = 0, currentPage = 0, currentPageSize = 0;
        if (ok) {
            auto data = doc.object().value("data").toObject();
            total = data.value("total").toInt();
            currentPage = data.value("page").toInt();
            currentPageSize = data.value("pageSize").toInt();
            for (const auto &v : data.value("results").toArray()) {
                results.append(v.toObject().toVariantMap());
            }
            qDebug() << "[搜索API]搜索:" << query << "，HTTP状态码:" << statusCode << "，成功，找到" << total << "个结果";
        } else {
            QString message = doc.object().value("message").toString();
            qDebug() << "[搜索API]搜索:" << query << "，HTTP状态码:" << statusCode << "，失败，消息:" << message;
        }
        if (cb) cb(ok, total, currentPage, currentPageSize, results);
    });
}

void ApiClient::fetchMusicInfo(int musicId, MusicInfoCb cb) {
    QUrl url(QString::fromUtf8("%1/api/music/info/%2").arg(Theme::kApiBase).arg(musicId));
    auto *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (cb) cb(false, {});
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QVariantMap musicInfo;
        if (ok) {
            auto data = doc.object().value("data").toObject();
            musicInfo = data.toVariantMap();
        }
        if (cb) cb(ok, musicInfo);
    });
}

void ApiClient::fetchLyrics(int musicId, LyricsCb cb) {
    QUrl url(QString::fromUtf8("%1/api/music/lyrics/%2?t=%3").arg(Theme::kApiBase).arg(musicId).arg(QDateTime::currentMSecsSinceEpoch()));
    auto *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (cb) cb(false, "");
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QString lyrics = "";
        if (ok) {
            lyrics = doc.object().value("data").toString();
        }
        if (cb) cb(ok, lyrics);
    });
}

void ApiClient::searchArtists(const QString &query, ArtistSearchCb cb) {
    QUrl url(QString::fromUtf8("%1/api/artists/search").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["query"] = query;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (cb) cb(false, {});
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QVariantMap artistInfo;
        if (ok) {
            artistInfo = doc.object().value("artist").toObject().toVariantMap();
        }
        if (cb) cb(ok, artistInfo);
    });
}

void ApiClient::fetchUploadedMusic(UploadedMusicCb cb) {
    QUrl url(QString::fromUtf8("%1/api/user/uploaded-music").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    if (UserManager::instance().isLoggedIn()) {
        req.setRawHeader("Authorization", UserManager::instance().token().toUtf8());
    }
    auto *reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (cb) cb(false, 0, {});
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QList<QVariantMap> musicList;
        int total = 0;
        if (ok) {
            total = doc.object().value("total").toInt();
            for (const auto &v : doc.object().value("musicList").toArray()) {
                musicList.append(v.toObject().toVariantMap());
            }
        }
        if (cb) cb(ok, total, musicList);
    });
}

void ApiClient::changePassword(const QString &oldPassword, const QString &newPassword, std::function<void(bool, const QString&)> cb) {
    QUrl url(QString::fromUtf8("%1/api/user/password/change").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    if (UserManager::instance().isLoggedIn()) {
        req.setRawHeader("Authorization", QString("Bearer %1").arg(UserManager::instance().token()).toUtf8());
    }
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body["oldPassword"] = oldPassword;
    body["newPassword"] = newPassword;
    auto *reply = m_nam.post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, cb]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            if (cb) cb(false, reply->errorString());
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        bool ok = doc.object().value("success").toBool();
        QString message = ok ? doc.object().value("message").toString() : doc.object().value("error").toString();
        if (cb) cb(ok, message);
    });
}

void ApiClient::uploadMusic(const QString &musicFilePath, const QString &title,
                             const QString &artist, const QString &language, int duration,
                             int uploadUserId, const QString &album,
                             const QString &tags, const QString &coverFilePath,
                             const QString &lyricsFilePath, UploadCb cb) {
    QUrl url(QString::fromUtf8("%1/api/user/upload").arg(Theme::kApiBase));
    QNetworkRequest req(url);
    if (UserManager::instance().isLoggedIn()) {
        req.setRawHeader("Authorization", UserManager::instance().token().toUtf8());
    }
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    
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
    
    QFile *musicFile = new QFile(musicFilePath);
    if (musicFile->open(QIODevice::ReadOnly)) {
        QHttpPart musicPart;
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