#pragma once

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QMap>
#include <QTimer>

class DesktopLrc : public QWidget
{
    Q_OBJECT

public:
    explicit DesktopLrc(QWidget *parent = nullptr);
    ~DesktopLrc();

    void setLyrics(const QString &lyrics);
    void loadLyricsFile(const QString &filePath);
    void updatePosition(qint64 position);
    void setCurrentSong(const QString &title, const QString &artist);

public slots:
    void showWindow();
    void hideWindow();
    void toggleWindow();
    void testLyricScroll();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void parseLyrics(const QString &lyricsText);
    QString getLyricAtTime(qint64 timeMs) const;
    void updateLyricDisplay();

    QString m_currentLyrics;
    bool m_dragging = false;
    QPoint m_dragPosition;
    
    // 歌词数据
    QMap<qint64, QString> m_lyricsMap;  // 时间戳 -> 歌词
    qint64 m_currentPosition = 0;
    QString m_currentSongTitle;
    QString m_currentSongArtist;
    
    // 显示设置
    QFont m_font;
    QColor m_textColor;
    QColor m_backgroundColor;
    bool m_isVisible = true;
    
    QTimer *m_updateTimer;
};