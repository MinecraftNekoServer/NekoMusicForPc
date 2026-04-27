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

    using PlaylistsCb = std::function<void(bool, const QList<QVariantMap>&)>;
    void fetchPlaylists(const QString &query, PlaylistsCb cb);

    using MusicListCb = std::function<void(bool, const QList<QVariantMap>&)>;
    void fetchRanking(MusicListCb cb);
    void fetchLatest(int limit, MusicListCb cb);

private:
    QNetworkAccessManager m_nam;
};
