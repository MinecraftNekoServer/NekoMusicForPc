#include "musicdownloader.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>

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

QString MusicDownloader::cachedAudioFilePath(int musicId)
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
            + QStringLiteral("/nekomusic-cache");
    QDir().mkpath(cacheDir);
    return cacheDir + QLatin1Char('/') + QString::number(musicId);
}

void MusicDownloader::purgeLegacyMd5CacheFiles()
{
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
            + QStringLiteral("/nekomusic-cache");
    if (!QDir(cacheDir).exists())
        return;
    static const QRegularExpression md5Name(QStringLiteral("^[0-9a-f]{32}$"));
    QDirIterator it(cacheDir, QDir::Files);
    while (it.hasNext()) {
        it.next();
        QString name = it.fileInfo().fileName();
        if (name.endsWith(QStringLiteral(".part"), Qt::CaseInsensitive))
            name.chop(5);
        if (!md5Name.match(name).hasMatch())
            continue;
        QFile::remove(it.filePath());
    }
}

void MusicDownloader::cancel()
{
    qDebug() << "[MusicDownloader] cancel called";
    if (m_reply) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
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

void MusicDownloader::download(const QUrl &url, int musicId)
{
    cancel();
    m_bufferEmitted = false;

    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
            + QStringLiteral("/nekomusic-cache");
    QDir().mkpath(cacheDir);
    if (musicId > 0) {
        m_tempPath = cachedAudioFilePath(musicId);
    } else {
        const QString hash = QCryptographicHash::hash(url.toEncoded(), QCryptographicHash::Md5).toHex();
        m_tempPath = cacheDir + QLatin1Char('/') + hash;
    }

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
    if (!m_reply || !m_file)
        return;
    m_file->write(m_reply->readAll());
}

void MusicDownloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (!m_reply || !m_file)
        return;
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

    // 勿对 .part 直接 rename：QMediaPlayer 可能尚未完成异步打开 .part，rename 后路径消失会 ENOENT。
    // 先复制到正式缓存路径再删 .part；缓冲播放结束后由 UI 收到 downloadFinished 再切到正式文件。
    const QString partPath = m_tempPath + QStringLiteral(".part");
    if (QFile::exists(partPath)) {
        if (QFile::exists(m_tempPath))
            QFile::remove(m_tempPath);
        if (!QFile::copy(partPath, m_tempPath)) {
            emit downloadError(QStringLiteral("写入完整缓存失败"));
            reply->deleteLater();
            if (m_file) {
                delete m_file;
                m_file = nullptr;
            }
            return;
        }
        QFile::remove(partPath);
    }

    emit downloadFinished(m_tempPath);

    reply->deleteLater();
    if (m_file) {
        delete m_file;
        m_file = nullptr;
    }
}
