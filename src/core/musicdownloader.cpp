#include "musicdownloader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDir>

MusicDownloader::MusicDownloader(QObject *parent) : QObject(parent) {}

MusicDownloader::~MusicDownloader()
{
    cancel();
}

void MusicDownloader::download(const QUrl &url)
{
    cancel();

    // Generate cache path from URL hash (no extension - FFmpeg detects format from content)
    QString hash = QCryptographicHash::hash(url.toEncoded(), QCryptographicHash::Md5).toHex();
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/nekomusic-cache";
    QDir().mkpath(cacheDir);
    m_tempPath = cacheDir + "/" + hash;

    // Check if already cached
    if (QFile::exists(m_tempPath)) {
        emit bufferReady(m_tempPath);
        emit downloadFinished(m_tempPath);
        return;
    }

    m_file = new QFile(m_tempPath + ".part");
    if (!m_file->open(QIODevice::WriteOnly)) {
        emit downloadError(QStringLiteral("无法创建临时文件"));
        delete m_file;
        m_file = nullptr;
        return;
    }

    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_nam.get(req);

    m_bufferEmitted = false;

    connect(m_reply, &QNetworkReply::downloadProgress, this, &MusicDownloader::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &MusicDownloader::onReplyFinished);
    connect(m_reply, &QNetworkReply::readyRead, this, [this]() {
        if (m_file && m_reply) {
            m_file->write(m_reply->readAll());
        }
    });
}

void MusicDownloader::cancel()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    if (m_file) {
        m_file->close();
        m_file->remove();
        delete m_file;
        m_file = nullptr;
    }
    m_tempPath.clear();
    m_bufferEmitted = false;
}

void MusicDownloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (m_bufferEmitted) return;

    bool ready = false;
    if (bytesTotal > 0) {
        // Known size: 30% threshold
        int percent = static_cast<int>(bytesReceived * 100 / bytesTotal);
        emit downloadProgress(percent);
        ready = (percent >= 30);
    } else {
        // Unknown size: 500KB threshold
        emit downloadProgress(-1); // unknown
        ready = (bytesReceived >= 500 * 1024);
    }

    if (ready) {
        m_bufferEmitted = true;
        // Close and rename partial file to final path
        if (m_file) {
            m_file->flush();
            m_file->close();
            QFile::rename(m_tempPath + ".part", m_tempPath);
        }
        emit bufferReady(m_tempPath);
        // Reopen file for continued writing
        m_file = new QFile(m_tempPath);
        if (m_file->open(QIODevice::Append)) {
            m_file->seek(m_file->size());
        }
    }
}

void MusicDownloader::onReplyFinished()
{
    if (!m_reply || !m_file) {
        emit downloadError(m_reply ? m_reply->errorString() : QStringLiteral("下载失败"));
        cancel();
        return;
    }

    if (m_reply->error() != QNetworkReply::NoError) {
        emit downloadError(m_reply->errorString());
        cancel();
        return;
    }

    // Write remaining data
    m_file->write(m_reply->readAll());
    m_file->close();

    // Rename if not already done
    if (m_tempPath.endsWith(".part") || QFile::exists(m_tempPath + ".part")) {
        QFile::rename(m_tempPath + ".part", m_tempPath);
    }

    emit downloadFinished(m_tempPath);

    delete m_file;
    m_file = nullptr;
    m_reply->deleteLater();
    m_reply = nullptr;
}
