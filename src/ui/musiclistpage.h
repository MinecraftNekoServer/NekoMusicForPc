#pragma once

/**
 * @file musiclistpage.h
 * @brief 音乐列表页面 — 显示热门音乐或最新音乐
 *
 * 可复用的列表页面,支持两种模式:热门音乐(ranking)和最新音乐(latest)。
 * 从 API 加载数据,以列表形式展示,支持点击播放。
 */

#include <QWidget>
#include <QList>

class QScrollArea;
class QVBoxLayout;
class QLabel;
class ApiClient;

class MusicListPage : public QWidget
{
    Q_OBJECT

public:
    enum Type { Hot, Latest };

    struct MusicInfo {
        int id = -1;
        QString title;
        QString artist;
        QString album;
        int duration = 0;
        QString coverUrl;
    };

    explicit MusicListPage(Type type, QWidget *parent = nullptr);

signals:
    void playMusic(const MusicInfo &info);
    void addToQueue(const MusicInfo &info);
    void addToPlaylist(const MusicInfo &info);
    void backRequested();

public slots:
    void refresh();
    void retranslate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void fetchData();
    void buildList();
    void buildListBatch();

    Type m_type;
    QScrollArea *m_scroll = nullptr;
    QVBoxLayout *m_listLayout = nullptr;
    QWidget *m_listContainer = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_loadingLabel = nullptr;
    ApiClient *m_api = nullptr;

    QList<MusicInfo> m_musicList;
    bool m_loaded = false;
    bool m_dataFetched = false;  // 标记是否已经加载过数据
    int m_buildIndex = 0;        // 分批创建索引
    bool m_buildingList = false; // 分批创建 guard
};
