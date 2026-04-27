/**
 * @file homepage.cpp
 * @brief 首页实现
 *
 * 上部轮播：GET /api/music/ranking 热门歌曲
 * 下部网格：POST /api/playlists/search 精选歌单
 * 封面统一使用 /api/music/cover/{id}
 */

#include "homepage.h"
#include "theme/theme.h"
#include "ui/carousel.h"
#include "ui/playlistcard.h"
#include "ui/glasswidget.h"

#include <QVBoxLayout>
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

    // 入场淡入（动画完成后移除 effect，避免与子控件 effect 嵌套冲突）
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

    // 间距
    lay->addSpacing(24);

    // ─── 推荐区域 ────────────────────────────────────
    auto *recCard = new GlassWidget(container);
    recCard->setObjectName("hpRecommend");
    recCard->setBorderRadius(Theme::kRXl);
    recCard->setOpacity(0.55);

    auto *recLay = new QVBoxLayout(recCard);
    recLay->setContentsMargins(28, 24, 28, 24);
    recLay->setSpacing(16);

    // 标题行
    auto *titleRow = new QHBoxLayout();
    auto *title = new QLabel(QStringLiteral("为你推荐"), recCard);
    title->setObjectName("hpSectionTitle");
    titleRow->addWidget(title);
    titleRow->addStretch();
    recLay->addLayout(titleRow);

    // 4 列网格
    m_gridContainer = new QWidget(recCard);
    m_gridLayout = new QGridLayout(m_gridContainer);
    m_gridLayout->setContentsMargins(0, 0, 0, 0);
    m_gridLayout->setSpacing(16);
    m_gridLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // 加载占位
    auto *loading = new QLabel(QStringLiteral("加载中..."), recCard);
    loading->setObjectName("hpLoading");
    loading->setAlignment(Qt::AlignCenter);
    m_gridLayout->addWidget(loading, 0, 0);

    recLay->addWidget(m_gridContainer);

    lay->addWidget(recCard);
    lay->addStretch();

    scroll->setWidget(container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}

void HomePage::refreshData()
{
    fetchHotMusic();
    fetchPlaylists();
}

/**
 * @brief 获取热门歌曲 → 轮播
 *
 * GET /api/music/ranking?limit=5
 * 封面用 /api/music/cover/{id}
 */
void HomePage::fetchHotMusic()
{
    QUrl url(QString::fromUtf8("%1/api/music/ranking").arg(Theme::kApiBase));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("limit"), QStringLiteral("5"));
    url.setQuery(q);

    QNetworkReply *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.object().value("success").toBool()) return;

        QList<CarouselItem> items;
        auto arr = doc.object().value("data").toArray();
        for (int i = 0; i < qMin(arr.size(), 5); ++i) {
            auto obj = arr[i].toObject();
            CarouselItem ci;
            ci.playlistId = obj.value("id").toInt();
            ci.title = obj.value("title").toString();
            ci.description = obj.value("artist").toString();
            // 封面：/api/music/cover/{id}
            ci.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                              .arg(Theme::kApiBase)
                              .arg(obj.value("id").toInt());
            items.append(ci);
        }
        m_carousel->setItems(items);
    });
}

/**
 * @brief 获取歌单 → 网格
 *
 * POST /api/playlists/search  {"query":"a"}
 * 封面用 /api/music/cover/{firstMusicId}
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
            // 封面：使用 /api/music/cover/{firstMusicId}
            int firstId = obj.value("firstMusicId").toInt(0);
            if (firstId > 0) {
                info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                    .arg(Theme::kApiBase)
                                    .arg(firstId);
            } else {
                // 歌单无歌曲时用默认图标
                info.coverUrl = QString::fromUtf8("%1/api/music/cover/1").arg(Theme::kApiBase);
            }
            list.append(info);
        }
        m_playlists = list;
        populateGrid(list);
    });
}

void HomePage::populateGrid(const QList<PlaylistInfo> &list)
{
    // 清空旧内容
    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (list.isEmpty()) {
        auto *empty = new QLabel(QStringLiteral("暂无歌单"), m_gridContainer);
        empty->setObjectName("hpLoading");
        empty->setAlignment(Qt::AlignCenter);
        m_gridLayout->addWidget(empty, 0, 0);
        return;
    }

    // 4 列网格排列 + 逐个淡入动画
    int col = 0, row = 0;
    for (const auto &info : list) {
        auto *card = new PlaylistCard(info, m_gridContainer);
        connect(card, &PlaylistCard::clicked, this, &HomePage::navigateToPlaylist);

        // 逐个卡片淡入动画（完成后移除 effect）
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

        m_gridLayout->addWidget(card, row, col);
        col++;
        if (col >= 4) { col = 0; row++; }
    }
}

void HomePage::paintEvent(QPaintEvent *) { /* 透明，由父窗口渐变背景透出 */ }
