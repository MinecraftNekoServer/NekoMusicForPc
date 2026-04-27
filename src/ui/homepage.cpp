/**
 * @file homepage.cpp
 * @brief 首页实现 — 单一推荐歌单板块
 *
 * 推荐歌单：POST /api/playlists/search
 * 热门音乐：GET /api/music/ranking
 * 最新音乐：GET /api/music/latest
 */

#include "homepage.h"
#include "core/i18n.h"
#include "core/covercache.h"
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
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QUrlQuery>

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
        QString musicId = url.mid(url.lastIndexOf(QLatin1Char('/')) + 1);

        QPixmap cached = CoverCache::instance()->get(musicId);
        if (!cached.isNull()) {
            applyPixmap(cached);
            return;
        }

        connect(CoverCache::instance(), &CoverCache::coverLoaded, this,
                [this, musicId](const QString &id, const QPixmap &pix) {
            if (id == musicId) applyPixmap(pix);
        });
        CoverCache::instance()->fetchCover(musicId, url);
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
    void applyPixmap(const QPixmap &pix)
    {
        int s = qMin(pix.width(), pix.height());
        m_pixmap = pix.copy((pix.width()-s)/2, (pix.height()-s)/2, s, s)
            .scaled(m_size, m_size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        update();
    }

    QPixmap m_pixmap;
    int m_size;
};

// ─── 聚合卡片（热门/最新音乐）───────────────────────
class MusicAggregateCard : public QWidget
{
public:
    enum Type { Hot, Latest };

    MusicAggregateCard(Type type, int firstId, int totalCount, QWidget *parent = nullptr)
        : QWidget(parent), m_firstId(firstId), m_type(type), m_totalCount(totalCount)
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

        // 2x2 封面马赛克
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
            type == Hot ? I18n::instance().tr("hot_music") : I18n::instance().tr("latest_music"), glass);
        titleLbl->setObjectName(type == Hot ? "hpHotTitle" : "hpLatestTitle");
        vlay->addWidget(titleLbl);

        auto *countLbl = new QLabel(
            QString::number(totalCount) + I18n::instance().tr("songs"), glass);
        countLbl->setObjectName(type == Hot ? "hpHotCount" : "hpLatestCount");
        vlay->addWidget(countLbl);

        vlay->addStretch();

        auto *outer = new QVBoxLayout(this);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->addWidget(glass);
    }

    void retranslate() {
        auto *titleLbl = findChild<QLabel *>(m_type == Hot ? "hpHotTitle" : "hpLatestTitle");
        if (titleLbl) titleLbl->setText(m_type == Hot ? I18n::instance().tr("hot_music") : I18n::instance().tr("latest_music"));
        auto *countLbl = findChild<QLabel *>(m_type == Hot ? "hpHotCount" : "hpLatestCount");
        if (countLbl) countLbl->setText(QString::number(m_totalCount) + I18n::instance().tr("songs"));
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

    int m_type;
    int m_firstId;
    int m_totalCount;
    QList<CoverLabel *> m_covers;
};

// Store for retranslate
static QList<MusicAggregateCard *> m_aggCards;

// ─── HomePage ────────────────────────────────────────

HomePage::HomePage(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();

    // 延迟加载数据，先显示UI
    QTimer::singleShot(100, this, &HomePage::refreshData);

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
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(16);

    // ─── 推荐歌单标题 ─────────────────────────────────
    auto *titleLabel = new QLabel(I18n::instance().tr("recommend_playlists"), container);
    titleLabel->setObjectName("hpSectionTitle");
    lay->addWidget(titleLabel);
    lay->addSpacing(8);

    // ─── 卡片横向滚动区 ───────────────────────────────
    m_cardContainer = new QWidget(container);
    m_cardContainer->setObjectName("hpCardContainer");
    m_cardLayout = new QHBoxLayout(m_cardContainer);
    m_cardLayout->setContentsMargins(0, 0, 0, 0);
    m_cardLayout->setSpacing(20);
    m_cardLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    auto *loadingLabel = new QLabel(I18n::instance().tr("loading"), m_cardContainer);
    loadingLabel->setObjectName("hpLoading");
    loadingLabel->setAlignment(Qt::AlignCenter);
    m_cardLayout->addWidget(loadingLabel);

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

void HomePage::retranslate()
{
    auto *tl = findChild<QLabel *>("hpSectionTitle");
    if (tl) tl->setText(I18n::instance().tr("recommend_playlists"));
    auto *ll = findChild<QLabel *>("hpLoading");
    if (ll) ll->setText(I18n::instance().tr("loading"));

    // Update all MusicAggregateCard instances
    for (auto *card : m_aggCards) {
        card->retranslate();
    }
}

void HomePage::fetchHotMusic()
{
    QUrl url(QString::fromUtf8("%1/api/music/ranking").arg(Theme::kApiBase));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("limit"), QStringLiteral("300"));
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setTransferTimeout(5000); // 5s timeout
    QNetworkReply *reply = m_nam.get(req);
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

void HomePage::fetchPlaylists()
{
    QNetworkRequest req(QUrl(QString::fromUtf8("%1/api/playlists/search").arg(Theme::kApiBase)));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setTransferTimeout(5000); // 5s timeout
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

void HomePage::fetchLatestMusic()
{
    QUrl url(QString::fromUtf8("%1/api/music/latest").arg(Theme::kApiBase));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("limit"), QStringLiteral("300"));
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setTransferTimeout(5000); // 5s timeout
    QNetworkReply *reply = m_nam.get(req);
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

void HomePage::rebuildRecommendSection()
{
    if (!m_hotReady || !m_playlistReady || !m_latestReady) return;

    QLayoutItem *item;
    while ((item = m_cardLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    m_aggCards.clear();

    // ── 热门音乐聚合卡片 ──
    {
        int firstId = m_hotMusic.isEmpty() ? 0 : m_hotMusic.first().id;
        auto *card = new MusicAggregateCard(MusicAggregateCard::Hot, firstId, m_hotMusic.size(), m_cardContainer);
        for (int i = 0; i < qMin(m_hotMusic.size(), 4); ++i) {
            card->setCover(i, m_hotMusic[i].coverUrl);
        }
        m_aggCards.append(card);
        m_cardLayout->addWidget(card);
    }

    // ── 最新音乐聚合卡片 ──
    {
        int firstId = m_latestMusic.isEmpty() ? 0 : m_latestMusic.first().id;
        auto *card = new MusicAggregateCard(MusicAggregateCard::Latest, firstId, m_latestMusic.size(), m_cardContainer);
        for (int i = 0; i < qMin(m_latestMusic.size(), 4); ++i) {
            card->setCover(i, m_latestMusic[i].coverUrl);
        }
        m_aggCards.append(card);
        m_cardLayout->addWidget(card);
    }

    // ── 推荐歌单卡片（原有样式不动）──
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
