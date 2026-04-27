#pragma once

/**
 * @file homepage.h
 * @brief 首页 — 日系动漫风
 *
 * 上部：全屏大图轮播（Carousel）— 热门歌曲
 * 下部：毛玻璃容器包裹的 4 列推荐网格 — 歌单
 * 整体纵向可滚动，入场淡入动画。
 */

#include <QWidget>
#include <QNetworkAccessManager>

class Carousel;
class QGridLayout;

struct PlaylistInfo;

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

signals:
    void navigateToPlaylist(int id);

public slots:
    void refreshData();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    void fetchHotMusic();     // 轮播：GET /api/music/ranking
    void fetchPlaylists();    // 网格：POST /api/playlists/search
    void populateGrid(const QList<PlaylistInfo> &list);

    Carousel *m_carousel = nullptr;
    QGridLayout *m_gridLayout = nullptr;
    QWidget *m_gridContainer = nullptr;
    QNetworkAccessManager m_nam;

    QList<PlaylistInfo> m_playlists;
};
