/**
 * @file homepage.cpp
 * @brief 首页实现
 *
 * 轮播：GET /api/music/ranking 热门歌曲
 * 推荐歌单：POST /api/playlists/search
 * 热门音乐：GET /api/music/ranking
 * 最新音乐：GET /api/music/latest
 */

#include "homepage.h"
#include "theme/theme.h"
#include "ui/carousel.h"
#include "ui/playlistcard.h"
#include "ui/musiccard.h"
#include "ui/glasswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QUrlQuery>

HomePage::HomePage(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
    refreshData();

    // 入场淡入
    auto *eff = new QGraphicsOpacityEffect(this);
    eff->setOpacity(0.0);
    setGraphicsEffect(eff);
    auto *anim = new QPropertyAnimation(eff, "opacity");
    anim->setDuration(600);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, this, [this]() {
        setGraphicsEffect(nullptr);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void HomePage::setupUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setObjectName("hpScroll");

    auto *container = new QWidget(scroll);
    container->setObjectName("hpContainer");
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // ─── 轮播 ────────────────────────────────────────
    m_carousel = new Carousel(container);
    lay->addWidget(m_carousel);
    lay->addSpacing(24);

    // ─── 推荐歌单 ────────────────────────────────────
    auto *plSection = createSection(QStringLiteral("推荐歌单"), m_playlistGrid, m_playlistContainer);
    lay->addWidget(plSection);
    lay->addSpacing(20);

    // ─── 热门音乐 ────────────────────────────────────
    auto *hotSection = createSection(QStringLiteral("热门音乐"), m_hotGrid, m_hotContainer);
    lay->addWidget(hotSection);
    lay->addSpacing(20);

    // ─── 最新音乐 ────────────────────────────────────
    auto *latestSection = createSection(QStringLiteral("最新音乐"), m_latestGrid, m_latestContainer);
    lay->addWidget(latestSection);

    lay->addStretch();
    scroll->setWidget(container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}

GlassWidget *HomePage::createSection(const QString &title, QGridLayout *&grid, QWidget *&gridContainer)
{
    auto *card = new GlassWidget(this);
    card->setObjectName("hpRecommend");
    card->setBorderRadius(Theme::kRXl);
    card->setOpacity(0.55);

    auto *recLay = new QVBoxLayout(card);
    recLay->setContentsMargins(28, 24, 28, 24);
    recLay->setSpacing(16);

    auto *titleRow = new QHBoxLayout();
    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("hpSectionTitle");
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    recLay->addLayout(titleRow);

    gridContainer = new QWidget(card);
    grid = new QGridLayout(gridContainer);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(16);
    grid->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto *loading = new QLabel(QStringLiteral("加载中..."), card);
    loading->setObjectName("hpLoading");
    loading->setAlignment(Qt::AlignCenter);
    grid->addWidget(loading, 0, 0);

    recLay->addWidget(gridContainer);
    return card;
}

void HomePage::refreshData()
{
    fetchHotMusic();
    fetchPlaylists();
    fetchLatestMusic();
}

/**
 * @brief GET /api/music/ranking → 轮播 + 热门网格
 */
void HomePage::fetchHotMusic()
{
    QUrl url(QString::fromUtf8("%1/api/music/ranking").arg(Theme::kApiBase));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("limit"), QStringLiteral("8"));
    url.setQuery(q);

    QNetworkReply *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.object().value("success").toBool()) return;

        auto arr = doc.object().value("data").toArray();

        // 轮播：前 5
        QList<CarouselItem> items;
        for (int i = 0; i < qMin(arr.size(), 5); ++i) {
            auto obj = arr[i].toObject();
            CarouselItem ci;
            ci.playlistId = obj.value("id").toInt();
            ci.title = obj.value("title").toString();
            ci.description = obj.value("artist").toString();
            ci.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                              .arg(Theme::kApiBase).arg(obj.value("id").toInt());
            items.append(ci);
        }
        m_carousel->setItems(items);

        // 热门网格：前 8
        QList<MusicInfo> list;
        for (int i = 0; i < qMin(arr.size(), 8); ++i) {
            auto obj = arr[i].toObject();
            MusicInfo info;
            info.id = obj.value("id").toInt();
            info.title = obj.value("title").toString();
            info.artist = obj.value("artist").toString();
            info.album = obj.value("album").toString();
            info.duration = obj.value("duration").toInt();
            info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                .arg(Theme::kApiBase).arg(info.id);
            list.append(info);
        }
        populateMusicGrid(m_hotGrid, m_hotContainer, list);
    });
}

/**
 * @brief POST /api/playlists/search → 推荐歌单网格
 */
void HomePage::fetchPlaylists()
{
    QNetworkRequest req(QUrl(QString::fromUtf8("%1/api/playlists/search").arg(Theme::kApiBase)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_nam.post(req, QByteArray("{\"query\":\"a\"}"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.object().value("success").toBool()) return;

        QList<PlaylistInfo> list;
        auto arr = doc.object().value("results").toArray();
        for (int i = 0; i < qMin(arr.size(), 8); ++i) {
            auto obj = arr[i].toObject();
            PlaylistInfo info;
            info.id = obj.value("id").toInt();
            info.name = obj.value("name").toString();
            info.description = obj.value("description").toString();
            info.musicCount = obj.value("musicCount").toInt();
            int firstId = obj.value("firstMusicId").toInt(0);
            if (firstId > 0) {
                info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                    .arg(Theme::kApiBase).arg(firstId);
            } else {
                info.coverUrl = QString::fromUtf8("%1/api/music/cover/1").arg(Theme::kApiBase);
            }
            list.append(info);
        }
        populatePlaylistGrid(m_playlistGrid, m_playlistContainer, list);
    });
}

/**
 * @brief GET /api/music/latest → 最新网格
 */
void HomePage::fetchLatestMusic()
{
    QUrl url(QString::fromUtf8("%1/api/music/latest").arg(Theme::kApiBase));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("limit"), QStringLiteral("8"));
    url.setQuery(q);

    QNetworkReply *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.object().value("success").toBool()) return;

        QList<MusicInfo> list;
        auto arr = doc.object().value("data").toArray();
        for (int i = 0; i < qMin(arr.size(), 8); ++i) {
            auto obj = arr[i].toObject();
            MusicInfo info;
            info.id = obj.value("id").toInt();
            info.title = obj.value("title").toString();
            info.artist = obj.value("artist").toString();
            info.album = obj.value("album").toString();
            info.duration = obj.value("duration").toInt();
            info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                .arg(Theme::kApiBase).arg(info.id);
            list.append(info);
        }
        populateMusicGrid(m_latestGrid, m_latestContainer, list);
    });
}

void HomePage::populatePlaylistGrid(QGridLayout *grid, QWidget *container, const QList<PlaylistInfo> &list)
{
    QLayoutItem *item;
    while ((item = grid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    if (list.isEmpty()) {
        auto *empty = new QLabel(QStringLiteral("暂无歌单"), container);
        empty->setObjectName("hpLoading");
        empty->setAlignment(Qt::AlignCenter);
        grid->addWidget(empty, 0, 0);
        return;
    }
    int col = 0, row = 0;
    for (const auto &info : list) {
        auto *card = new PlaylistCard(info, container);
        connect(card, &PlaylistCard::clicked, this, &HomePage::navigateToPlaylist);

        auto *eff = new QGraphicsOpacityEffect(card);
        eff->setOpacity(0.0);
        card->setGraphicsEffect(eff);
        auto *anim = new QPropertyAnimation(eff, "opacity");
        anim->setDuration(300);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &QPropertyAnimation::finished, card, [card]() {
            card->setGraphicsEffect(nullptr);
        });
        anim->start(QAbstractAnimation::DeleteWhenStopped);

        grid->addWidget(card, row, col);
        col++;
        if (col >= 4) { col = 0; row++; }
    }
}

void HomePage::populateMusicGrid(QGridLayout *grid, QWidget *container, const QList<MusicInfo> &list)
{
    QLayoutItem *item;
    while ((item = grid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    if (list.isEmpty()) {
        auto *empty = new QLabel(QStringLiteral("暂无数据"), container);
        empty->setObjectName("hpLoading");
        empty->setAlignment(Qt::AlignCenter);
        grid->addWidget(empty, 0, 0);
        return;
    }
    int col = 0, row = 0;
    for (const auto &info : list) {
        auto *card = new MusicCard(info, container);
        connect(card, &MusicCard::clicked, this, &HomePage::playMusic);

        auto *eff = new QGraphicsOpacityEffect(card);
        eff->setOpacity(0.0);
        card->setGraphicsEffect(eff);
        auto *anim = new QPropertyAnimation(eff, "opacity");
        anim->setDuration(300);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, &QPropertyAnimation::finished, card, [card]() {
            card->setGraphicsEffect(nullptr);
        });
        anim->start(QAbstractAnimation::DeleteWhenStopped);

        grid->addWidget(card, row, col);
        col++;
        if (col >= 4) { col = 0; row++; }
    }
}

void HomePage::paintEvent(QPaintEvent *) { /* 透明，由父窗口渐变背景透出 */ }
