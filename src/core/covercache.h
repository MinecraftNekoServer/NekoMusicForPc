#pragma once

/**
 * @file covercache.h
 * @brief 封面图片磁盘缓存
 *
 * 单例类，负责封面图片的磁盘缓存与网络回源。
 * 缓存路径：<Temp>/nekomusic-cache/covers/
 * 缓存文件名：<music_id>.jpg
 */

#include <QObject>
#include <QPixmap>
#include <QHash>
#include <QNetworkAccessManager>

class QNetworkReply;

class CoverCache : public QObject
{
    Q_OBJECT

public:
    static CoverCache *instance();

    /** 从封面 URL 解析 musicId（路径最后一段，去掉查询串）。 */
    static QString musicIdFromCoverUrl(const QString &coverUrl);

    /** 将相对路径、协议相对 URL 补全为可请求的绝对地址（基于 Theme::kApiBase）。 */
    static QString resolveCoverUrl(const QString &rawUrl);

    /** 获取封面：先查缓存，未命中则从网络下载 */
    void fetchCover(const QString &musicId, const QString &coverUrl);

    /** 从磁盘加载缓存图片，失败返回空 */
    QPixmap get(const QString &musicId) const;

    /** 保存图片到磁盘缓存 */
    void set(const QString &musicId, const QPixmap &pixmap);

    /** 清理全部缓存 */
    void clear();

signals:
    void coverLoaded(const QString &musicId, const QPixmap &pixmap);

private:
    explicit CoverCache(QObject *parent = nullptr);
    QString cacheDir() const;
    void ensureCacheDir() const;

    mutable QString m_cacheDir;
    mutable bool m_cacheDirInitialized = false;
    QNetworkAccessManager m_nam;
    /** 同一 musicId 只保留一个进行中的请求，避免播放栏与播放页重复拉取。 */
    QHash<QString, QNetworkReply *> m_inFlight;
};
