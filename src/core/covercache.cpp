/**
 * @file covercache.cpp
 * @brief 封面图片磁盘缓存实现
 *
 * 缓存目录跨平台：
 *   Linux/macOS: /tmp/NekoMusic/covers/
 *   Windows:     %TEMP%/NekoMusic/covers/
 */

#include "covercache.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

CoverCache *CoverCache::instance()
{
    static CoverCache inst;
    return &inst;
}

CoverCache::CoverCache(QObject *parent) : QObject(parent)
{
    m_nam.setTransferTimeout(10000);
}

QString CoverCache::cacheDir() const
{
    if (!m_cacheDirInitialized) {
        QString base = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        m_cacheDir = base + QStringLiteral("/NekoMusic/covers");
        m_cacheDirInitialized = true;
    }
    return m_cacheDir;
}

void CoverCache::ensureCacheDir() const
{
    QDir dir;
    dir.mkpath(cacheDir());
}

QPixmap CoverCache::get(const QString &musicId) const
{
    if (musicId.isEmpty()) return QPixmap();
    QString path = cacheDir() + QLatin1Char('/') + musicId + QStringLiteral(".jpg");
    if (QFile::exists(path)) {
        QPixmap pix;
        if (pix.load(path)) return pix;
        QFile::remove(path); // corrupted
    }
    return QPixmap();
}

void CoverCache::set(const QString &musicId, const QPixmap &pixmap)
{
    if (musicId.isEmpty() || pixmap.isNull()) return;
    ensureCacheDir();
    QString path = cacheDir() + QLatin1Char('/') + musicId + QStringLiteral(".jpg");
    pixmap.save(path, "JPEG", 85);
}

void CoverCache::fetchCover(const QString &musicId, const QString &coverUrl)
{
    if (musicId.isEmpty() || coverUrl.isEmpty()) return;

    QPixmap cached = get(musicId);
    if (!cached.isNull()) {
        emit coverLoaded(musicId, cached);
        return;
    }

    QNetworkRequest req;
    req.setUrl(QUrl(coverUrl));
    req.setTransferTimeout(10000);
    QNetworkReply *reply = m_nam.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, musicId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;

        QPixmap pix;
        if (pix.loadFromData(reply->readAll())) {
            set(musicId, pix);
            emit coverLoaded(musicId, pix);
        }
    });
}

void CoverCache::clear()
{
    ensureCacheDir();
    QDir dir(cacheDir());
    const auto entries = dir.entryInfoList({QStringLiteral("*.jpg")}, QDir::Files);
    for (const auto &entry : entries) {
        QFile::remove(entry.absoluteFilePath());
    }
}
