/**
 * @file apiclient.cpp
 * @brief API 客户端实现
 */

#include "apiclient.h"
#include "theme/theme.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

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
