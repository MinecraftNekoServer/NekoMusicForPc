/**
 * @file homepage.cpp
 * @brief 首页实现 — 单一推荐歌单板块
 *
 * 推荐歌单：POST /api/playlists/search
 * 热门音乐：GET /api/music/ranking
 * 最新音乐：GET /api/music/latest
 */

#include "homepage.h"
#include "theme/theme.h"
#include "ui/playlistcard.h"
#include "ui/glasswidget.h"
#include "ui/svgicon.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QNetworkAccessManager>

// ─── 单曲封面标签（圆角 6px + 异步加载）─────────────────
class CoverLabel : public QLabel
{
public:
    explicit CoverLabel(int size, QWidget *parent = nullptr) : QLabel(parent), m_size(size)
    {
        setFixedSize(size, size);
        setScaledContents(false);
        setPlaceholder();
    }

    void setPlaceholder()
    {
        m_pixmap = QPixmap(m_size, m_size);
        m_pixmap.fill(Qt::transparent);
        QPainter p(&m_pixmap);
        QPainterPath pp;
        pp.addRoundedRect(0, 0, m_size, m_size, 6, 6);
        p.fillPath(pp, QColor(128, 128, 128, 40));
        p.setClipPath(pp);
        auto iconPx = Icons::render(Icons::kMusic, 28, QColor(255, 255, 255, 100));
        p.drawPixmap((m_size - 28) / 2, (m_size - 28) / 2, iconPx);
        update();
    }

    void loadCover(const QString &url)
    {
        if (url.isEmpty()) { setPlaceholder(); return; }
        static QNetworkAccessManager nam;
        QNetworkReply *reply = nam.get(QNetworkRequest(QUrl(url)));
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) { setPlaceholder(); return; }
            QPixmap pix;
            if (pix.loadFromData(reply->readAll())) {
                int s = qMin(pix.width(), pix.height());
                m_pixmap = pix.copy((pix.width()-s)/2, (pix.height()-s)/2, s, s)
                    .scaled(m_size, m_size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                update();
            } else {
                setPlaceholder();
            }
        });
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath pp;
        pp.addRoundedRect(0, 0, m_size, m_size, 6, 6);
        p.setClipPath(pp);
        p.drawPixmap(0, 0, m_pixmap);
    }

private:
    QPixmap m_pixmap;
    int m_size;
};

// ─── 聚合卡片（热门/最新音乐）───────────────────────
/**
 * 204px 宽，内部包含 2x2 封面马赛克 + 标题 + 数量。
 * 点击发出 playMusic 信号。
 */
class MusicAggregateCard : public QWidget
{
public:
    enum Type { Hot, Latest };

    MusicAggregateCard(Type type, int firstId, int totalCount, QWidget *parent = nullptr)
        : QWidget(parent), m_firstId(firstId), m_type(type)
    {
        setFixedSize(204, 260);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, false);

        auto *glass = new GlassWidget(this);
        glass->setBorderRadius(Theme::kRXl);
        glass->setOpacity(0.55);
        glass->setObjectName(type == Hot ? "hpHotMusicCard" : "hpLatestMusicCard");

        auto *vlay = new QVBoxLayout(glass);
        vlay->setContentsMargins(12, 12, 12, 12);
        vlay->setSpacing(8);

        // 2x2 封面马赛克（占位，由外部设置封面）
        auto *grid = new QGridLayout();
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setSpacing(3);
        int coverSize = 88;
        for (int i = 0; i < 4; ++i) {
            auto *lbl = new CoverLabel(coverSize, this);
            lbl->setPlaceholder();
            m_covers.append(lbl);
            grid->addWidget(lbl, i / 2, i % 2);
        }
        vlay->addLayout(grid);

        // 标题 & 数量
        auto *titleLbl = new QLabel(
            type == Hot ? QStringLiteral("热门音乐") : QStringLiteral("最新音乐"), glass);
        titleLbl->setObjectName(type == Hot ? "hpHotTitle" : "hpLatestTitle");
        vlay->addWidget(titleLbl);

        auto *countLbl = new QLabel(
            QString::number(totalCount) + QStringLiteral("首"), glass);
        countLbl->setObjectName(type == Hot ? "hpHotCount" : "hpLatestCount");
        vlay->addWidget(countLbl);

        vlay->addStretch();

        auto *outer = new QVBoxLayout(this);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->addWidget(glass);
    }

    void setCover(int index, const QString &url)
    {
        if (index >= 0 && index < m_covers.size())
            m_covers[index]->loadCover(url);
    }

    int firstMusicId() const { return m_firstId; }

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton) emitClicked();
        QWidget::mousePressEvent(e);
    }

private:
    void emitClicked()
    {
        // 直接通过父对象树找到 HomePage 并转发信号
        if (auto *hp = findParentHomePage()) {
            emit hp->playMusic(m_firstId);
        }
    }

    HomePage *findParentHomePage() const
    {
        QObject *p = parent();
        while (p) {
            if (auto *hp = qobject_cast<HomePage *>(p)) return hp;
            p = p->parent();
        }
        return nullptr;
    }

    Type m_type;
    int m_firstId;
    QList<CoverLabel *> m_covers;
};

// ─── HomePage ────────────────────────────────────────

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
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setObjectName("hpScroll");

    auto *container = new QWidget(m_scroll);
    container->setObjectName("hpContainer");
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // ─── 推荐歌单标题 ────────────────────────────────
    auto *titleRow = new QHBoxLayout();
    auto *titleLabel = new QLabel(QStringLiteral("推荐歌单"), container);
    titleLabel->setObjectName("hpSectionTitle");
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();
    lay->addLayout(titleRow);
    lay->addSpacing(12);

    // ─── 卡片横向滚动区 ──────────────────────────────
    m_cardContainer = new QWidget(container);
    m_cardContainer->setObjectName("hpCardContainer");
    m_cardLayout = new QHBoxLayout(m_cardContainer);
    m_cardLayout->setContentsMargins(0, 0, 0, 0);
    m_cardLayout->setSpacing(20);
    m_cardLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // 初始占位
    auto *loading = new QLabel(QStringLiteral("加载中..."), m_cardContainer);
    loading->setObjectName("hpLoading");
    loading->setAlignment(Qt::AlignCenter);
    m_cardLayout->addWidget(loading);

    lay->addWidget(m_cardContainer);
    lay->addStretch();

    m_scroll->setWidget(container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_scroll);
}

void HomePage::refreshData()
{
    m_hotReady = false;
    m_playlistReady = false;
    m_latestReady = false;
    m_hotMusic.clear();
    m_playlists.clear();
    m_latestMusic.clear();

    fetchHotMusic();
    fetchPlaylists();
    fetchLatestMusic();
}

/**
 * @brief GET /api/music/ranking → 热门音乐
 */
void HomePage::fetchHotMusic()
{
    QUrl url(QString::fromUtf8("%1/api/music/ranking").arg(Theme::kApiBase));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("limit"), QStringLiteral("300"));
    url.setQuery(q);

    QNetworkReply *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_hotReady = true;
            rebuildRecommendSection();
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.object().value("success").toBool()) {
            m_hotReady = true;
            rebuildRecommendSection();
            return;
        }

        auto arr = doc.object().value("data").toArray();
        m_hotMusic.clear();
        for (int i = 0; i < arr.size(); ++i) {
            auto obj = arr[i].toObject();
            MusicInfo info;
            info.id = obj.value("id").toInt();
            info.title = obj.value("title").toString();
            info.artist = obj.value("artist").toString();
            info.album = obj.value("album").toString();
            info.duration = obj.value("duration").toInt();
            info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                .arg(Theme::kApiBase).arg(info.id);
            m_hotMusic.append(info);
        }
        m_hotReady = true;
        rebuildRecommendSection();
    });
}

/**
 * @brief POST /api/playlists/search → 推荐歌单
 */
void HomePage::fetchPlaylists()
{
    QNetworkRequest req(QUrl(QString::fromUtf8("%1/api/playlists/search").arg(Theme::kApiBase)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = m_nam.post(req, QByteArray("{\"query\":\"\"}"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_playlistReady = true;
            rebuildRecommendSection();
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.object().value("success").toBool()) {
            m_playlistReady = true;
            rebuildRecommendSection();
            return;
        }

        m_playlists.clear();
        auto arr = doc.object().value("results").toArray();
        for (int i = 0; i < arr.size(); ++i) {
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
            m_playlists.append(info);
        }
        m_playlistReady = true;
        rebuildRecommendSection();
    });
}

/**
 * @brief GET /api/music/latest → 最新音乐
 */
void HomePage::fetchLatestMusic()
{
    QUrl url(QString::fromUtf8("%1/api/music/latest").arg(Theme::kApiBase));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("limit"), QStringLiteral("300"));
    url.setQuery(q);

    QNetworkReply *reply = m_nam.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            m_latestReady = true;
            rebuildRecommendSection();
            return;
        }
        auto doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.object().value("success").toBool()) {
            m_latestReady = true;
            rebuildRecommendSection();
            return;
        }

        m_latestMusic.clear();
        auto arr = doc.object().value("data").toArray();
        for (int i = 0; i < arr.size(); ++i) {
            auto obj = arr[i].toObject();
            MusicInfo info;
            info.id = obj.value("id").toInt();
            info.title = obj.value("title").toString();
            info.artist = obj.value("artist").toString();
            info.album = obj.value("album").toString();
            info.duration = obj.value("duration").toInt();
            info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                .arg(Theme::kApiBase).arg(info.id);
            m_latestMusic.append(info);
        }
        m_latestReady = true;
        rebuildRecommendSection();
    });
}

/**
 * @brief 当三个请求都返回后重建卡片区域
 *
 * 顺序：热门音乐卡片 → 最新音乐卡片 → 推荐歌单卡片
 */
void HomePage::rebuildRecommendSection()
{
    if (!m_hotReady || !m_playlistReady || !m_latestReady) return;

    // 清除旧卡片
    QLayoutItem *item;
    while ((item = m_cardLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // ── 热门音乐聚合卡片 ──
    {
        int firstId = m_hotMusic.isEmpty() ? 0 : m_hotMusic.first().id;
        auto *card = new MusicAggregateCard(MusicAggregateCard::Hot, firstId, m_hotMusic.size(), m_cardContainer);
        for (int i = 0; i < qMin(m_hotMusic.size(), 4); ++i) {
            card->setCover(i, m_hotMusic[i].coverUrl);
        }
        m_cardLayout->addWidget(card);
    }

    // ── 最新音乐聚合卡片 ──
    {
        int firstId = m_latestMusic.isEmpty() ? 0 : m_latestMusic.first().id;
        auto *card = new MusicAggregateCard(MusicAggregateCard::Latest, firstId, m_latestMusic.size(), m_cardContainer);
        for (int i = 0; i < qMin(m_latestMusic.size(), 4); ++i) {
            card->setCover(i, m_latestMusic[i].coverUrl);
        }
        m_cardLayout->addWidget(card);
    }

    // ── 推荐歌单卡片 ──
    for (const auto &info : m_playlists) {
        auto *card = new PlaylistCard(info, m_cardContainer);
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

        m_cardLayout->addWidget(card);
    }
}

void HomePage::paintEvent(QPaintEvent *) { /* 透明，由父窗口渐变背景透出 */ }
