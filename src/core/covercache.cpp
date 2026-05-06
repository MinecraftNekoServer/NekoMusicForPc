/**
 * @file covercache.cpp
 * @brief 封面图片磁盘缓存实现
 *
 * 缓存目录跨平台：
 *   Linux/macOS: /tmp/nekomusic-cache/covers
 *   Windows:     %TEMP%/nekomusic-cache/covers
 */

#include "covercache.h"
#include "theme/theme.h"

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

QString CoverCache::musicIdFromCoverUrl(const QString &coverUrl)
{
    const int slash = coverUrl.lastIndexOf(QLatin1Char('/'));
    if (slash < 0)
        return {};
    QString id = coverUrl.mid(slash + 1);
    const int q = id.indexOf(QLatin1Char('?'));
    if (q >= 0)
        id.truncate(q);
    return id;
}

QString CoverCache::resolveCoverUrl(const QString &rawUrl)
{
    const QString u = rawUrl.trimmed();
    if (u.isEmpty())
        return {};

    if (u.startsWith(QLatin1String("http://"), Qt::CaseInsensitive)
        || u.startsWith(QLatin1String("https://"), Qt::CaseInsensitive)) {
        return u;
    }
    if (u.startsWith(QLatin1String("//"))) {
        return QStringLiteral("https:") + u;
    }

    QString base = QString::fromUtf8(Theme::kApiBase);
    while (base.endsWith(QLatin1Char('/')))
        base.chop(1);
    if (u.startsWith(QLatin1Char('/')))
        return base + u;
    return base + QLatin1Char('/') + u;
}

QString CoverCache::cacheDir() const
{
    if (!m_cacheDirInitialized) {
        QString base = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        m_cacheDir = base + QStringLiteral("/nekomusic-cache/covers");
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
    if (musicId.isEmpty())
        return;
    const QString absolute = resolveCoverUrl(coverUrl);
    if (absolute.isEmpty())
        return;

    QPixmap cached = get(musicId);
    if (!cached.isNull()) {
        emit coverLoaded(musicId, cached);
        return;
    }

    if (m_inFlight.contains(musicId))
        return;

    QNetworkRequest req;
    req.setUrl(QUrl(absolute));
    req.setTransferTimeout(10000);
    QNetworkReply *reply = m_nam.get(req);
    m_inFlight.insert(musicId, reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply, musicId]() {
        m_inFlight.remove(musicId);
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
    const QList<QNetworkReply *> reps = m_inFlight.values();
    m_inFlight.clear();
    for (QNetworkReply *reply : reps) {
        if (reply) {
            reply->disconnect();
            reply->abort();
            reply->deleteLater();
        }
    }

    ensureCacheDir();
    QDir dir(cacheDir());
    const auto entries = dir.entryInfoList({QStringLiteral("*.jpg")}, QDir::Files);
    for (const auto &entry : entries) {
        QFile::remove(entry.absoluteFilePath());
    }
}
