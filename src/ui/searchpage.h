#pragma once

/**
 * @file searchpage.h
 * @brief 搜索页面 — 显示音乐和歌手搜索结果
 */

#include <QWidget>
#include <QList>

#include "core/musicinfo.h"

class QScrollArea;
class QVBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class ApiClient;

class SearchPage : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPage(ApiClient *apiClient, QWidget *parent = nullptr);

signals:
    void playMusic(const MusicInfo &info);
    void backRequested();

public slots:
    void search(const QString &query);
    void retranslate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void fetchMusicResults();
    void fetchArtistResults();
    void buildMusicList();
    void buildArtistList();
    void showMusicPage();
    void showArtistPage();

    ApiClient *m_apiClient = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QPushButton *m_searchBtn = nullptr;
    QPushButton *m_musicTab = nullptr;
    QPushButton *m_artistTab = nullptr;
    QLabel *m_statusLabel = nullptr;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_container = nullptr;
    QVBoxLayout *m_listLayout = nullptr;

    QString m_query;
    QList<MusicInfo> m_musicResults;
    QList<QVariantMap> m_artistResults;
    bool m_showingMusic = true;
    int m_page = 1;
    static const int kPageSize = 20;
};
