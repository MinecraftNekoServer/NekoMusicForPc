#pragma once

/**
 * @file myplaylists.h
 * @brief 我的歌单页面 — 显示用户创建的歌单列表，支持创建/编辑/删除
 */

#include <QWidget>
#include <QList>

#include "ui/playlistcard.h"

class QScrollArea;
class QVBoxLayout;
class QLabel;
class QGridLayout;
class ApiClient;

class MyPlaylists : public QWidget
{
    Q_OBJECT

public:
    explicit MyPlaylists(ApiClient *apiClient, QWidget *parent = nullptr);

signals:
    void playlistClicked(int playlistId);
    void backRequested();

public slots:
    void refresh();
    void retranslate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void loadMyPlaylists();
    void createPlaylist();
    void editPlaylist(int playlistId, const QString &currentName, const QString &currentDesc);
    void deletePlaylist(int playlistId, const QString &playlistName);

    ApiClient *m_apiClient = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_container = nullptr;
    QGridLayout *m_gridLayout = nullptr;

    QList<PlaylistInfo> m_playlists;
};
