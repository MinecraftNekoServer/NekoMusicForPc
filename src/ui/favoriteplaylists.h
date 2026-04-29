#pragma once

/**
 * @file favoriteplaylists.h
 * @brief 收藏歌单页面 — 显示用户收藏的歌单列表
 */

#include <QWidget>
#include <QList>

#include "ui/playlistcard.h"

class QScrollArea;
class QVBoxLayout;
class QLabel;
class QGridLayout;
class ApiClient;

class FavoritePlaylists : public QWidget
{
    Q_OBJECT

public:
    explicit FavoritePlaylists(ApiClient *apiClient, QWidget *parent = nullptr);

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
    void loadFavoritePlaylists();

    ApiClient *m_apiClient = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_container = nullptr;
    QGridLayout *m_gridLayout = nullptr;

    QList<PlaylistInfo> m_playlists;
};
