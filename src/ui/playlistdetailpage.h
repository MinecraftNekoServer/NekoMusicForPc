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
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUi();
    void buildList();
    void updateHeader();
    void setPlaceholderCover();

    ApiClient *m_apiClient = nullptr;
    int m_playlistId = 0;
    int m_firstMusicId = 0;
    QString m_playlistName;
    QString m_playlistDesc;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_contentWidget = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QWidget *m_listContainer = nullptr;
    
    // Header elements
    QWidget *m_headerWidget = nullptr;
    QLabel *m_coverLbl = nullptr;
    QLabel *m_typeLbl = nullptr;
    QLabel *m_nameLbl = nullptr;
    QLabel *m_descLbl = nullptr;
    QLabel *m_creatorAvatarLbl = nullptr;
    QLabel *m_creatorNameLbl = nullptr;
    QLabel *m_countLbl = nullptr;
    
    // List header
    QWidget *m_listHeaderWidget = nullptr;
    QLabel *m_listTitleLbl = nullptr;
    QLabel *m_listCountLbl = nullptr;
    
    int m_creatorId = 0;
    QString m_creatorUsername;
    
    QList<MusicInfo> m_musicList;
    QList<QWidget *> m_musicItems;
};
