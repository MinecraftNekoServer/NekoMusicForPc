#pragma once

/**
 * @file sidebar.h
 * @brief 侧边栏导航 — 日系动漫风
 *
 * 240px 紧凑宽度，重度毛玻璃。
 * 选中项薰衣草紫背景 + 左侧薄荷绿竖条指示器。
 */

#include <QWidget>
#include <QMap>
#include <QIcon>

class QPushButton;
class QVBoxLayout;
class QWidget;
class PlaylistListItem;

class Sidebar : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);
    void setActiveNav(const QString &key);
    void retranslate();
    void setUploadVisible(bool visible);
    void refreshPlaylists();

signals:
    void navigationRequested(const QString &key);
    void playlistClicked(int localId);
    void playlistCreateRequested();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    QPushButton *createNavItem(const QString &key, const QString &label, const QIcon &icon);
    void refreshPlaylistList();

    QMap<QString, QPushButton *> m_navBtns;
    QString m_activeKey;
    QPushButton *m_favBtn = nullptr;
    QPushButton *m_recBtn = nullptr;
    QPushButton *m_uploadBtn = nullptr;

    QWidget *m_playlistContainer = nullptr;
    QVBoxLayout *m_playlistLayout = nullptr;
    QList<PlaylistListItem *> m_playlistItems;
    QPushButton *m_createPlaylistBtn = nullptr;
};
