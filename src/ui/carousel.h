#pragma once

/**
 * @file carousel.h
 * @brief 全屏大图轮播组件
 *
 * 占据内容区上方约 55% 高度，自动轮播推荐歌单。
 * 大图背景 + 底部毛玻璃遮罩 + 歌单信息 + 播放按钮。
 * 左右箭头切换 + 底部圆点指示器（薄荷绿高亮）。
 * 淡入淡出切换动画。
 */

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QList>
#include <QTimer>
#include <QGraphicsOpacityEffect>

struct CarouselItem {
    int playlistId = -1;
    QString title;
    QString description;
    QString coverUrl;
};

class Carousel : public QWidget
{
    Q_OBJECT

public:
    explicit Carousel(QWidget *parent = nullptr);
    void setItems(const QList<CarouselItem> &items);

signals:
    void itemClicked(int playlistId);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    void setupUi();
    void goToIndex(int index);
    void updateDisplay();
    QPushButton *createArrowBtn(const QString &objName);
    /** 淡入淡出切换动画 */
    void animateTransition();

    QList<CarouselItem> m_items;
    int m_currentIndex = 0;
    QTimer m_timer;
    bool m_animating = false;   // 动画进行中标记

    QLabel *m_bgLabel      = nullptr;
    QLabel *m_titleLabel   = nullptr;
    QLabel *m_descLabel    = nullptr;
    QPushButton *m_playBtn = nullptr;
    QWidget *m_dotsWidget  = nullptr;
    QGraphicsOpacityEffect *m_opacityEffect = nullptr;
};
