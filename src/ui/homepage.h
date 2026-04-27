#pragma once

/**
 * @file homepage.h
 * @brief 首页 — 日系动漫风
 *
 * 轮播：热门歌曲
 * 推荐歌单：POST /api/playlists/search
 * 热门音乐：GET /api/music/ranking
 * 最新音乐：GET /api/music/latest
 */

#include <QWidget>
#include <QNetworkAccessManager>

class Carousel;
class GlassWidget;
class QGridLayout;

struct PlaylistInfo;
struct MusicInfo;

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

signals:
    void navigateToPlaylist(int id);
    void playMusic(int id);

public slots:
    void refreshData();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    GlassWidget *createSection(const QString &title, QGridLayout *&grid, QWidget *&gridContainer);

    void fetchHotMusic();     // 轮播 + 热门网格
    void fetchPlaylists();    // 推荐歌单网格
    void fetchLatestMusic();  // 最新网格

    void populatePlaylistGrid(QGridLayout *grid, QWidget *container, const QList<PlaylistInfo> &list);
    void populateMusicGrid(QGridLayout *grid, QWidget *container, const QList<MusicInfo> &list);

    Carousel *m_carousel = nullptr;

    // 推荐歌单区
    QGridLayout *m_playlistGrid = nullptr;
    QWidget *m_playlistContainer = nullptr;

    // 热门音乐区
    QGridLayout *m_hotGrid = nullptr;
    QWidget *m_hotContainer = nullptr;

    // 最新音乐区
    QGridLayout *m_latestGrid = nullptr;
    QWidget *m_latestContainer = nullptr;

    QNetworkAccessManager m_nam;
};
