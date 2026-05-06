/**
 * @file favoritespage.cpp
 * @brief "我喜欢的"页面 — 显示用户收藏的音乐列表
 */

#include "favoritespage.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "core/covercache.h"
#include "core/usermanager.h"
#include "theme/theme.h"
#include "ui/svgicon.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>

// ─── 单曲封面标签（圆角 6px + 异步加载）─────────────────
class FavCoverLabel : public QLabel
{
public:
    explicit FavCoverLabel(int size, QWidget *parent = nullptr) : QLabel(parent), m_size(size)
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

// ─── 收藏音乐卡片 ────────────────────────
class FavMusicCard : public QWidget
{
public:
    explicit FavMusicCard(const MusicInfo &info, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(info.id), m_info(info)
    {
        setFixedHeight(70);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, false);

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(12, 8, 12, 8);
        lay->setSpacing(14);

        // 封面
        auto *coverLbl = new FavCoverLabel(54, this);
        coverLbl->loadCover(info.coverUrl);
        lay->addWidget(coverLbl);

        // 信息
        auto *infoV = new QWidget(this);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(4);

        auto *titleLbl = new QLabel(info.title, infoV);
        titleLbl->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + QString(Theme::kTextMain) + "; }");
        infoLay->addWidget(titleLbl);

        auto *artistLbl = new QLabel(info.artist, infoV);
        artistLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextSub) + "; }");
        infoLay->addWidget(artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        // 时长
        if (info.duration > 0) {
            int mins = info.duration / 60;
            int secs = info.duration % 60;
            auto *timeLbl = new QLabel(
                QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')), this);
            timeLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
            lay->addWidget(timeLbl);
        }

        // 红心图标（已收藏状态，纯展示）
        auto *heartLbl = new QLabel(this);
        heartLbl->setFixedSize(24, 24);
        heartLbl->setPixmap(QPixmap(":/icons/icon_heart.png"));
        lay->addWidget(heartLbl);
    }

    int musicId() const { return m_musicId; }
    const MusicInfo& info() const { return m_info; }

    std::function<void(int)> onClicked;

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 8, 8);
        p.fillPath(path, QColor(45, 38, 65, 100));
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onClicked) {
            onClicked(m_musicId);
        }
        QWidget::mousePressEvent(e);
    }

private:
    int m_musicId;
    MusicInfo m_info;
};

// ─── FavoritesPage ────────────────────────────────────────

FavoritesPage::FavoritesPage(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), m_apiClient(apiClient)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();

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

void FavoritesPage::retranslate()
{
    // Update title if needed
}

void FavoritesPage::refresh()
{
    loadFavorites();
}

void FavoritesPage::setupUi()
{
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setObjectName("favoritesScroll");

    m_container = new QWidget(m_scroll);
    m_container->setObjectName("favoritesContainer");
    m_mainLay = new QVBoxLayout(m_container);
    m_mainLay->setContentsMargins(24, 24, 24, 24);
    m_mainLay->setSpacing(16);

    // 标题
    auto *titleLabel = new QLabel(I18n::instance().tr("myFavorites"), m_container);
    titleLabel->setObjectName("favoritesTitle");
    m_mainLay->addWidget(titleLabel);

    // 状态标签（加载中/空状态）
    m_statusLabel = new QLabel(I18n::instance().tr("loading"), m_container);
    m_statusLabel->setObjectName("favoritesStatus");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("QLabel { font-size: 14px; color: " + QString(Theme::kTextMuted) + "; padding: 40px 0; }");
    m_mainLay->addWidget(m_statusLabel);

    // 列表区域
    m_listLay = new QVBoxLayout();
    m_listLay->setContentsMargins(0, 0, 0, 0);
    m_listLay->setSpacing(8);
    m_listLay->setAlignment(Qt::AlignTop);
    m_mainLay->addLayout(m_listLay);

    m_mainLay->addStretch();

    m_scroll->setWidget(m_container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(m_scroll);

    // 加载数据
    loadFavorites();
}

void FavoritesPage::loadFavorites()
{
    QLayoutItem *item;
    while ((item = m_listLay->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget())
            w->deleteLater();
        delete item;
    }

    if (!UserManager::instance().isLoggedIn()) {
        m_statusLabel->setText(I18n::instance().tr("pleaseLoginFirst"));
        m_statusLabel->show();
        return;
    }

    m_statusLabel->setText(I18n::instance().tr("loading"));
    m_statusLabel->show();

    m_apiClient->fetchFavorites([this](bool success, const QList<QVariantMap>& favorites) {
        m_statusLabel->hide();

        if (!success || favorites.isEmpty()) {
            m_statusLabel->setText(I18n::instance().tr("emptyFavorites"));
            m_statusLabel->show();
            return;
        }

        for (const auto &fav : favorites) {
            MusicInfo info;
            info.id = fav.value("id").toInt();
            info.title = fav.value("title").toString();
            info.artist = fav.value("artist").toString();
            info.album = fav.value("album").toString();
            info.duration = fav.value("duration").toInt();
            info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                .arg(Theme::kApiBase).arg(info.id);

            auto *card = new FavMusicCard(info, m_container);
            card->onClicked = [this, info](int musicId) {
                Q_UNUSED(musicId);
                emit playRequested(info.id, info.title, info.artist, info.coverUrl);
            };
            m_listLay->addWidget(card);
        }
        m_listLay->addStretch(1);
    });
}

void FavoritesPage::paintEvent(QPaintEvent *)
{
    // 透明，由父窗口渐变背景透出
}
