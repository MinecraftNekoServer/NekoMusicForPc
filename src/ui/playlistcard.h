#pragma once

/**
 * @file playlistcard.h
 * @brief 歌单卡片 — 网格布局版
 *
 * 170x220，圆角封面由 paintEvent 绘制 + 歌单名。
 * 悬停薄荷绿光晕边框 + 平滑上浮动画。
 */

#include <QWidget>
#include <QLabel>
#include <QPixmap>

struct PlaylistInfo {
    int id = -1;
    QString name;
    QString description;
    int musicCount = 0;
    QString coverUrl;
};

class PlaylistCard : public QWidget
{
    Q_OBJECT

public:
    explicit PlaylistCard(const PlaylistInfo &info, QWidget *parent = nullptr);
    int playlistId() const;

signals:
    void clicked(int id);

protected:
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void paintEvent(QPaintEvent *) override;

private:
    void loadCover();
    void animateHover(bool enter);

    PlaylistInfo m_info;
    QLabel *m_name       = nullptr;
    QPixmap m_coverPixmap;
    bool m_hovered       = false;
};
