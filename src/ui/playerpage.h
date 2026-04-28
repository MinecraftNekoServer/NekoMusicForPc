#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "../core/playerengine.h"

class PlayerPage : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerPage(PlayerEngine *engine, QWidget *parent = nullptr);
    ~PlayerPage() override;

    void setMusicInfo(int id, const QString &title, const QString &artist,
                      const QString &album, const QString &coverUrl = QString());
    void retranslate();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void backRequested();

private:
    void setupUi();
    void loadCover(const QString &url);

    PlayerEngine *m_engine;

    // UI
    QPushButton *m_backBtn;
    QLabel *m_coverLabel;
    QLabel *m_titleLabel;
    QLabel *m_artistLabel;
    QLabel *m_albumLabel;

    // State
    int m_musicId = 0;
    QString m_coverUrl;
};
