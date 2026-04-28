#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPropertyAnimation>
#include "../core/playerengine.h"

struct LyricLine {
    qint64 time;
    QString text;
    QString translation;
};

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(PlayerEngine *engine, QWidget *parent = nullptr);
    ~PlayerPage() override;

    void setMusicInfo(int id, const QString &title, const QString &artist,
                      const QString &album, const QString &coverUrl = QString());
    void retranslate();
    void loadLyrics(int musicId);
    void updateLyricHighlight(qint64 positionMs);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

signals:
    void backRequested();

private:
    void setupUi();
    void loadCover(const QString &url);
    void parseLrc(const QString &lrc);
    void rebuildLyricLabels();

    PlayerEngine *m_engine;

    QPushButton *m_backBtn;
    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;
    QLabel *m_albumLabel;
    QScrollArea *m_lyricsScroll;
    QWidget *m_lyricsContainer;
    QVBoxLayout *m_lyricsLayout;

    int m_musicId = 0;
    QString m_coverUrl;
    QVector<LyricLine> m_lyrics;
    int m_currentLyricLine = -1;
    QPropertyAnimation *m_scrollAnim = nullptr;
};
