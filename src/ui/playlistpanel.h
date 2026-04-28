#pragma once

/**
 * @file playlistpanel.h
 * @brief 播放列表面板 — 浮动在播放器右侧
 *
 * 显示当前播放队列，支持点击播放、移除、清空操作。
 */

#include <QWidget>
#include <QList>
#include "core/musicinfo.h"

class QScrollArea;
class QVBoxLayout;
class QLabel;
class QPushButton;

class PlaylistPanel : public QWidget {
    Q_OBJECT

public:
    explicit PlaylistPanel(QWidget *parent = nullptr);

signals:
    void playRequested(int musicId);
    void hideRequested();

public slots:
    void refresh();
    void retranslate();
    void showPanel();
    void hidePanel();
    void togglePanel();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void rebuildList();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_countLabel = nullptr;
    QPushButton *m_clearBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QScrollArea *m_scroll = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QWidget *m_listContainer = nullptr;
    QList<QWidget *> m_items;
};
