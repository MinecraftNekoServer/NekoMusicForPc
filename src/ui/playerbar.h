#pragma once

/**
 * @file playerbar.h
 * @brief 底部播放控制栏 — 日系动漫风
 *
 * 80px 紧凑，重度毛玻璃 + 薰衣草紫顶线。
 * 进度条薰衣草填充，播放按钮渐变。
 */

#include <QWidget>

class PlayerEngine;
class QSlider;
class QPushButton;
class QLabel;

class PlayerBar : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerBar(PlayerEngine *engine, QWidget *parent = nullptr);
    void retranslate();
    void setSongInfo(const QString &title, const QString &artist);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    void updateState();

    PlayerEngine *m_engine = nullptr;
    QPushButton *m_playBtn = nullptr;
    QSlider *m_progress = nullptr;
    QSlider *m_volume = nullptr;
    QLabel *m_songName = nullptr;
    QLabel *m_artist = nullptr;
    QLabel *m_curTime = nullptr;
    QLabel *m_durTime = nullptr;
};
