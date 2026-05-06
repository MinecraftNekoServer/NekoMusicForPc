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
class QTimer;
class QGraphicsOpacityEffect;
class QPropertyAnimation;

class PlayerBar : public QWidget
{
    Q_OBJECT

signals:
    void coverClicked();
    void playlistClicked();
    void desktopLyricsToggled(bool enabled);
    void previousClicked();
    void nextClicked();
    void favoriteClicked(int musicId);
    void playModeClicked();

public:
    explicit PlayerBar(PlayerEngine *engine, QWidget *parent = nullptr);
    ~PlayerBar() override;
    void retranslate();
    void setSongInfo(const QString &title, const QString &artist, const QString &coverUrl = QString());
    void setCoverVisible(bool visible);
    void setCurrentMusicId(int musicId);
    int currentMusicId() const { return m_currentMusicId; }
    void setFavoriteStatus(bool isFavorited);
    void setLoading(bool loading);
    void updatePlayModeBtn(const QString &mode);

protected:
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUi();
    void updateState();
    void setCoverPixmap(const QPixmap &pm);
    void updateVolumeIcon(int value);
    void showVolumePanelAnimated();
    void hideVolumePanelAnimated();
    QRect volumePanelHotRectGlobal() const;
    void installVolumePanelAppFilter();
    void removeVolumePanelAppFilter();

    PlayerEngine *m_engine = nullptr;
    QPushButton *m_playBtn = nullptr;
    QPushButton *m_playModeBtn = nullptr;
    QPushButton *m_heartBtn = nullptr;
    QSlider *m_progress = nullptr;
    
    QWidget *m_volumePanel = nullptr;
    QSlider *m_volumeSlider = nullptr;
    QPushButton *m_volumeBtn = nullptr;
    QPushButton *m_desktopLrcBtn = nullptr;
    QLabel *m_volumeLabel = nullptr;
    QGraphicsOpacityEffect *m_volumeOpacityFx = nullptr;
    QPropertyAnimation *m_volumeOpAnim = nullptr;
    QPropertyAnimation *m_volumePosAnim = nullptr;
    bool m_volumePanelClosing = false;
    QTimer *m_volumeLeaveTimer = nullptr;
    bool m_volumeAppFilterInstalled = false;

    QLabel *m_songName = nullptr;
    QLabel *m_artist = nullptr;
    QLabel *m_curTime = nullptr;
    QLabel *m_durTime = nullptr;
    QPushButton *m_cover = nullptr;
    QMetaObject::Connection m_coverConn;
    int m_currentMusicId = 0;
    bool m_isFavorited = false;
    bool m_isLoading = false;
    int m_loadingAngle = 0;
    QTimer *m_loadingTimer = nullptr;
};
