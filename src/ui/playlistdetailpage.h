#pragma once

/**
 * @file playlistdetailpage.h
 * @brief 播放列表详情页 — 显示歌单内的歌曲列表
 *
 * 从 API 加载播放列表的音乐，以列表形式展示。
 * 支持点击播放、右键移除歌曲等操作。
 */

#include <QWidget>
#include <QList>
#include "core/musicinfo.h"

class QScrollArea;
class QVBoxLayout;
class QLabel;
class ApiClient;

class PlaylistDetailPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistDetailPage(ApiClient *apiClient, QWidget *parent = nullptr);

signals:
    void playMusic(const MusicInfo &info);
    void backRequested();
    void refreshSidebarPlaylists();

public slots:
    void loadPlaylist(int playlistId);
    void retranslate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void buildList();
    void updateHeader();

    ApiClient *m_apiClient = nullptr;
    int m_playlistId = 0;
    QString m_playlistName;
    QScrollArea *m_scroll = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QWidget *m_listContainer = nullptr;
    QLabel *m_headerLabel = nullptr;
    QLabel *m_descLabel = nullptr;
    QList<MusicInfo> m_musicList;
    QList<QWidget *> m_musicItems;
};
