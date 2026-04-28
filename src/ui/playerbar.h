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

signals:
    void coverClicked();
    void playlistClicked();
    void previousClicked();
    void nextClicked();
    void favoriteClicked(int musicId);

public:
    explicit PlayerBar(PlayerEngine *engine, QWidget *parent = nullptr);
    void retranslate();
    void setSongInfo(const QString &title, const QString &artist, const QString &coverUrl = QString());
    void setCoverVisible(bool visible);
    void setCurrentMusicId(int musicId);
    int currentMusicId() const { return m_currentMusicId; }
    void setFavoriteStatus(bool isFavorited);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    void updateState();
    void setCoverPixmap(const QPixmap &pm);
    void loadCoverAsync(const QString &url);

    PlayerEngine *m_engine = nullptr;
    QPushButton *m_playBtn = nullptr;
    QPushButton *m_heartBtn = nullptr;
    QSlider *m_progress = nullptr;
    QSlider *m_volume = nullptr;
    QLabel *m_songName = nullptr;
    QLabel *m_artist = nullptr;
    QLabel *m_curTime = nullptr;
    QLabel *m_durTime = nullptr;
    QPushButton *m_cover = nullptr;
    int m_currentMusicId = 0;
    bool m_isFavorited = false;
};
