#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QFile>

class MusicDownloader : public QObject
{
    Q_OBJECT
public:
    explicit MusicDownloader(QObject *parent = nullptr);
    ~MusicDownloader() override;

    void download(const QUrl &url);
    void cancel();

signals:
    void bufferReady(const QString &localPath);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished(const QString &localPath);
    void downloadError(const QString &error);

private:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onReplyFinished();
    void onReadyRead();

    QNetworkAccessManager m_nam;
    QNetworkReply *m_reply = nullptr;
    QFile *m_file = nullptr;
    QString m_tempPath;
    bool m_bufferEmitted = false;
    qint64 m_bytesReceived = 0;
    qint64 m_bytesTotal = 0;
};
