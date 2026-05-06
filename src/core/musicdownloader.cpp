#include "musicdownloader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDir>
#include <QCoreApplication>

MusicDownloader& MusicDownloader::instance()
{
    static MusicDownloader instance;
    return instance;
}

MusicDownloader::MusicDownloader(QObject *parent) : QObject(parent) 
{
    // 配置网络管理器，减少连接保持
    m_nam.setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

MusicDownloader::~MusicDownloader()
{
    cancel();
}

void MusicDownloader::cancel()
{
    qDebug() << "[MusicDownloader] cancel called";
    if (m_reply) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        // 强制删除reply，不等待事件循环
        QCoreApplication::processEvents();
        m_reply = nullptr;
    }
    if (m_file) {
        if (m_file->isOpen()) {
            m_file->close();
        }
        delete m_file;
        m_file = nullptr;
    }
    m_bufferEmitted = false;
}

void MusicDownloader::download(const QUrl &url)
{
    cancel();
    m_bufferEmitted = false;

    // Generate cache path from URL hash (no extension - FFmpeg detects format from content)
    QString hash = QCryptographicHash::hash(url.toEncoded(), QCryptographicHash::Md5).toHex();
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/nekomusic-cache";
    QDir().mkpath(cacheDir);
    m_tempPath = cacheDir + "/" + hash;

    // Check if already cached
    if (QFile::exists(m_tempPath)) {
        // 文件已完全下载，直接播放完整文件
        emit downloadFinished(m_tempPath);
        return;
    }

    // Remove old partial file if exists
    QString partPath = m_tempPath + ".part";
    QFile::remove(partPath);

    m_file = new QFile(partPath, this);
    if (!m_file->open(QIODevice::WriteOnly)) {
        emit downloadError(QStringLiteral("无法创建临时文件"));
        return;
    }

    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = m_nam.get(req);
    // 不要设置parent，让reply在完成后自动删除
    // m_reply->setParent(this);

    connect(m_reply, &QNetworkReply::downloadProgress, this, &MusicDownloader::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &MusicDownloader::onReplyFinished);
    connect(m_reply, &QNetworkReply::readyRead, this, &MusicDownloader::onReadyRead);
}

void MusicDownloader::onReadyRead()
{
    if (m_file && m_reply) {
        m_file->write(m_reply->readAll());
    }
}

void MusicDownloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    m_bytesReceived = bytesReceived;
    m_bytesTotal = bytesTotal;
    emit downloadProgress(bytesReceived, bytesTotal);
    
    // 检查是否达到缓冲阈值（30%），如果达到且尚未发射bufferReady信号，则发射
    if (bytesTotal > 0 && !m_bufferEmitted) {
        double progress = static_cast<double>(bytesReceived) / bytesTotal;
        if (progress >= 0.3) {  // 30%缓冲阈值
            QString partPath = m_tempPath + ".part";
            if (QFile::exists(partPath)) {
                m_bufferEmitted = true;
                emit bufferReady(partPath);
            }
        }
    }
}

void MusicDownloader::onReplyFinished()
{
    if (!m_reply) return;

    QNetworkReply *reply = m_reply;
    m_reply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        emit downloadError(reply->errorString());
        reply->deleteLater();
        return;
    }

    // Write remaining data
    if (m_file && m_file->isOpen()) {
        m_file->write(reply->readAll());
        m_file->close();
    }

    // Rename partial file to final path if needed
    QString partPath = m_tempPath + ".part";
    if (QFile::exists(partPath)) {
        QFile::rename(partPath, m_tempPath);
    }

    emit downloadFinished(m_tempPath);

    reply->deleteLater();
    if (m_file) {
        delete m_file;
        m_file = nullptr;
    }
}
