/**
 * @file searchpage.cpp
 * @brief 搜索页面实现
 */

#include "searchpage.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "core/covercache.h"
#include "core/playlistmanager.h"
#include "theme/theme.h"
#include "ui/svgicon.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QMouseEvent>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStyle>

// ─── 音乐结果卡片 ──────────────────────────────────────
class SearchResultCard : public QWidget
{
public:
    explicit SearchResultCard(const MusicInfo &info, int index, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(info.id), m_index(index), m_info(info)
    {
        setFixedHeight(70);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet(
            "SearchResultCard { background: transparent; border-radius: 8px; }"
            "SearchResultCard:hover { background: rgba(102, 126, 234, 0.1); }"
            "SearchResultCard.playing { background: rgba(102, 126, 234, 0.15); }"
        );

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(12, 8, 12, 8);
        lay->setSpacing(14);

        // 序号/播放图标
        m_indexLbl = new QLabel(QString::number(index + 1), this);
        m_indexLbl->setFixedSize(24, 70);
        m_indexLbl->setAlignment(Qt::AlignCenter);
        m_indexLbl->setStyleSheet("QLabel { font-size: 13px; color: rgba(255, 255, 255, 0.4); }");
        lay->addWidget(m_indexLbl);

        // 封面
        m_coverLbl = new QLabel(this);
        m_coverLbl->setFixedSize(54, 54);
        m_coverLbl->setScaledContents(false);
        loadCover();
        lay->addWidget(m_coverLbl);

        auto *infoV = new QWidget(this);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(4);

        m_titleLbl = new QLabel(info.title, infoV);
        m_titleLbl->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + QString(Theme::kTextMain) + "; }");
        infoLay->addWidget(m_titleLbl);

        QString meta = info.artist;
        if (!info.album.isEmpty()) meta += " - " + info.album;
        m_artistLbl = new QLabel(meta, infoV);
        m_artistLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextSub) + "; }");
        infoLay->addWidget(m_artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        if (info.duration > 0) {
            int mins = info.duration / 60;
            int secs = info.duration % 60;
            auto *timeLbl = new QLabel(
                QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')), this);
            timeLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
            lay->addWidget(timeLbl);
        }
    }

    void setPlaying(bool playing) {
        m_isPlaying = playing;
        setProperty("playing", playing);
        style()->unpolish(this);
        style()->polish(this);
        if (playing) {
            m_indexLbl->setText(QString::fromUtf8("▶"));
            m_indexLbl->setStyleSheet("QLabel { font-size: 13px; color: " + QString(Theme::kMint) + "; }");
        } else {
            m_indexLbl->setText(QString::number(m_index + 1));
            m_indexLbl->setStyleSheet("QLabel { font-size: 13px; color: rgba(255, 255, 255, 0.4); }");
        }
    }

    std::function<void(int)> onClicked;
    std::function<void()> onAddToPlaylist;
    std::function<void()> onAddToQueue;

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onClicked) {
            onClicked(m_musicId);
        }
        QWidget::mousePressEvent(e);
    }

    void contextMenuEvent(QContextMenuEvent *event) override
    {
        QMenu menu(this);
        menu.setStyleSheet(
            "QMenu { background-color: rgba(40, 40, 50, 0.95); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 8px; padding: 4px; }"
            "QMenu::item { color: #e0e0e0; padding: 8px 24px; border-radius: 4px; }"
            "QMenu::item:selected { background-color: rgba(255, 255, 255, 0.1); }"
        );
        QAction *addAction = menu.addAction(I18n::instance().tr("addToPlaylist"));
        QAction *queueAction = menu.addAction(I18n::instance().tr("addToQueue"));
        QAction *selected = menu.exec(event->globalPos());
        if (selected == addAction && onAddToPlaylist) {
            onAddToPlaylist();
        } else if (selected == queueAction && onAddToQueue) {
            onAddToQueue();
        }
    }

private:
    void loadCover()
    {
        if (m_info.coverUrl.isEmpty()) { setPlaceholder(); return; }
        QString musicId = QString::number(m_musicId);
        QPixmap cached = CoverCache::instance()->get(musicId);
        if (!cached.isNull()) { disconnect(m_coverConn); m_coverConn = {}; applyPixmap(cached); return; }
        disconnect(m_coverConn);
        m_coverConn = connect(CoverCache::instance(), &CoverCache::coverLoaded, this,
                [this, musicId](const QString &id, const QPixmap &pix) { if (id == musicId) applyPixmap(pix); });
        CoverCache::instance()->fetchCover(musicId, m_info.coverUrl);
    }

    void setPlaceholder()
    {
        QPixmap pix(54, 54); pix.fill(Qt::transparent);
        QPainter p(&pix); QPainterPath path; path.addRoundedRect(0, 0, 54, 54, 6, 6);
        p.fillPath(path, QColor(128, 128, 128, 40)); p.setClipPath(path);
        auto icon = Icons::render(Icons::kMusic, 24, QColor(255, 255, 255, 100));
        p.drawPixmap(15, 15, icon); m_coverLbl->setPixmap(pix);
    }

    void applyPixmap(const QPixmap &pix)
    {
        disconnect(m_coverConn); m_coverConn = {};
        int s = qMin(pix.width(), pix.height());
        QPixmap scaled = pix.copy((pix.width()-s)/2, (pix.height()-s)/2, s, s)
            .scaled(54, 54, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
        m_coverLbl->setPixmap(scaled);
    }

    int m_musicId;
    int m_index;
    bool m_isPlaying = false;
    MusicInfo m_info;
    QLabel *m_coverLbl = nullptr;
    QLabel *m_titleLbl = nullptr;
    QLabel *m_artistLbl = nullptr;
    QLabel *m_indexLbl = nullptr;
    QMetaObject::Connection m_coverConn;
};

// ─── 歌单结果卡片 ──────────────────────────────────────
class PlaylistResultCard : public QWidget
{
public:
    explicit PlaylistResultCard(const QVariantMap &info, QWidget *parent = nullptr)
        : QWidget(parent), m_playlistInfo(info)
    {
        setFixedHeight(80);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet(
            "PlaylistResultCard { background: rgba(45, 38, 65, 100); border-radius: 8px; }"
            "PlaylistResultCard:hover { background: rgba(102, 126, 234, 0.1); }"
        );

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(12, 10, 12, 10);
        lay->setSpacing(14);

        m_coverLbl = new QLabel(this);
        m_coverLbl->setFixedSize(60, 60);
        m_coverLbl->setStyleSheet("QLabel { border-radius: 6px; }");
        m_coverLbl->setScaledContents(false);
        loadCover();
        lay->addWidget(m_coverLbl);

        auto *infoV = new QWidget(this);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(4);

        auto *nameLbl = new QLabel(info.value("name").toString(), infoV);
        nameLbl->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + QString(Theme::kTextMain) + "; }");
        infoLay->addWidget(nameLbl);

        QString desc = info.value("description").toString();
        if (desc.length() > 50) desc = desc.left(50) + "...";
        auto *descLbl = new QLabel(desc.isEmpty() ? I18n::instance().tr("description") : desc, infoV);
        descLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextSub) + "; }");
        descLbl->setWordWrap(true);
        infoLay->addWidget(descLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        int musicCount = info.value("musicCount").toInt();
        auto *countLbl = new QLabel(QString("%1 %2").arg(musicCount).arg(I18n::instance().tr("songs")), this);
        countLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
        lay->addWidget(countLbl);
    }

    std::function<void(int)> onClicked;

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onClicked) {
            onClicked(m_playlistInfo.value("id").toInt());
        }
        QWidget::mousePressEvent(e);
    }

private:
    void loadCover()
    {
        // 从 firstMusicCover 获取封面
        QString firstMusicCover = m_playlistInfo.value("firstMusicCover").toString();
        if (firstMusicCover.isEmpty()) {
            setPlaceholder();
            return;
        }
        // 从路径中提取音乐ID，例如 /music/covers/372.jpg -> 372
        int musicId = 0;
        int lastSlash = firstMusicCover.lastIndexOf('/');
        int dot = firstMusicCover.lastIndexOf('.');
        if (lastSlash >= 0 && dot > lastSlash) {
            musicId = firstMusicCover.mid(lastSlash + 1, dot - lastSlash - 1).toInt();
        }
        if (musicId <= 0) { setPlaceholder(); return; }

        QString coverUrl = QString::fromUtf8("%1/api/music/cover/%2").arg(Theme::kApiBase).arg(musicId);
        QString id = QString::number(musicId);
        QPixmap cached = CoverCache::instance()->get(id);
        if (!cached.isNull()) { applyPixmap(cached); return; }
        connect(CoverCache::instance(), &CoverCache::coverLoaded, this,
                [this, id](const QString &cid, const QPixmap &pix) { if (cid == id) applyPixmap(pix); });
        CoverCache::instance()->fetchCover(id, coverUrl);
    }

    void setPlaceholder()
    {
        QPixmap pix(60, 60); pix.fill(Qt::transparent);
        QPainter p(&pix); QPainterPath path; path.addRoundedRect(0, 0, 60, 60, 6, 6);
        p.fillPath(path, QColor(128, 128, 128, 40)); p.setClipPath(path);
        auto icon = Icons::render(Icons::kMusic, 28, QColor(255, 255, 255, 100));
        p.drawPixmap(16, 16, icon); m_coverLbl->setPixmap(pix);
    }

    void applyPixmap(const QPixmap &pix)
    {
        if (pix.isNull()) { setPlaceholder(); return; }
        QPixmap scaled = pix.scaled(60, 60, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_coverLbl->setPixmap(scaled);
    }

    QVariantMap m_playlistInfo;
    QLabel *m_coverLbl = nullptr;
};

// ─── 歌手音乐项 ──────────────────────────────────────
class ArtistMusicItem : public QWidget
{
public:
    explicit ArtistMusicItem(const MusicInfo &info, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(info.id), m_info(info)
    {
        setFixedHeight(56);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet(
            "ArtistMusicItem { background: transparent; }"
            "ArtistMusicItem:hover { background: rgba(102, 126, 234, 0.08); border-radius: 6px; }"
        );

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(20, 4, 20, 4);
        lay->setSpacing(12);

        m_coverLbl = new QLabel(this);
        m_coverLbl->setFixedSize(40, 40);
        m_coverLbl->setScaledContents(false);
        loadCover();
        lay->addWidget(m_coverLbl);

        auto *infoV = new QWidget(this);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(2);

        auto *titleLbl = new QLabel(info.title, infoV);
        titleLbl->setStyleSheet("QLabel { font-size: 13px; font-weight: 500; color: " + QString(Theme::kTextMain) + "; }");
        infoLay->addWidget(titleLbl);

        auto *artistLbl = new QLabel(info.artist, infoV);
        artistLbl->setStyleSheet("QLabel { font-size: 11px; color: " + QString(Theme::kTextSub) + "; }");
        infoLay->addWidget(artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        if (info.duration > 0) {
            int mins = info.duration / 60;
            int secs = info.duration % 60;
            auto *timeLbl = new QLabel(
                QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')), this);
            timeLbl->setStyleSheet("QLabel { font-size: 11px; color: " + QString(Theme::kTextMuted) + "; }");
            lay->addWidget(timeLbl);
        }
    }

    std::function<void(int)> onClicked;

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onClicked) {
            onClicked(m_musicId);
        }
        QWidget::mousePressEvent(e);
    }

private:
    void loadCover()
    {
        if (m_info.coverUrl.isEmpty()) { setPlaceholder(); return; }
        QString musicId = QString::number(m_musicId);
        QPixmap cached = CoverCache::instance()->get(musicId);
        if (!cached.isNull()) { disconnect(m_coverConn); m_coverConn = {}; applyPixmap(cached); return; }
        disconnect(m_coverConn);
        m_coverConn = connect(CoverCache::instance(), &CoverCache::coverLoaded, this,
                [this, musicId](const QString &id, const QPixmap &pix) { if (id == musicId) applyPixmap(pix); });
        CoverCache::instance()->fetchCover(musicId, m_info.coverUrl);
    }

    void setPlaceholder()
    {
        QPixmap pix(40, 40); pix.fill(Qt::transparent);
        QPainter p(&pix); QPainterPath path; path.addRoundedRect(0, 0, 40, 40, 4, 4);
        p.fillPath(path, QColor(128, 128, 128, 40)); p.setClipPath(path);
        auto icon = Icons::render(Icons::kMusic, 18, QColor(255, 255, 255, 100));
        p.drawPixmap(11, 11, icon); m_coverLbl->setPixmap(pix);
    }

    void applyPixmap(const QPixmap &pix)
    {
        disconnect(m_coverConn); m_coverConn = {};
        int s = qMin(pix.width(), pix.height());
        QPixmap scaled = pix.copy((pix.width()-s)/2, (pix.height()-s)/2, s, s)
            .scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
        m_coverLbl->setPixmap(scaled);
    }

    int m_musicId;
    MusicInfo m_info;
    QLabel *m_coverLbl = nullptr;
    QMetaObject::Connection m_coverConn;
};

// ─── 歌手结果卡片 ──────────────────────────────────────
class ArtistResultCard : public QWidget
{
public:
    explicit ArtistResultCard(const QVariantMap &info, QWidget *parent = nullptr)
        : QWidget(parent), m_artistInfo(info)
    {
        auto *mainLay = new QVBoxLayout(this);
        mainLay->setContentsMargins(0, 0, 0, 0);
        mainLay->setSpacing(0);

        // 歌手头部
        auto *headerWidget = new QWidget(this);
        headerWidget->setFixedHeight(60);
        headerWidget->setCursor(Qt::PointingHandCursor);
        headerWidget->setAttribute(Qt::WA_StyledBackground, true);
        headerWidget->setStyleSheet(
            "QWidget { background: rgba(45, 38, 65, 100); border-radius: 8px; }"
            "QWidget:hover { background: rgba(102, 126, 234, 0.1); }"
        );
        auto *headerLay = new QHBoxLayout(headerWidget);
        headerLay->setContentsMargins(12, 8, 12, 8);
        headerLay->setSpacing(14);

        m_avatarLbl = new QLabel(headerWidget);
        m_avatarLbl->setFixedSize(44, 44);
        m_avatarLbl->setScaledContents(false);
        QPixmap pix(44, 44); pix.fill(Qt::transparent);
        QPainter p(&pix); QPainterPath path; path.addEllipse(0, 0, 44, 44);
        p.fillPath(path, QColor(128, 128, 128, 40)); p.setClipPath(path);
        auto icon = Icons::render(Icons::kMusic, 24, QColor(255, 255, 255, 100));
        p.drawPixmap(10, 10, icon); m_avatarLbl->setPixmap(pix);
        headerLay->addWidget(m_avatarLbl);

        auto *nameLbl = new QLabel(info.value("name").toString(), headerWidget);
        nameLbl->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + QString(Theme::kTextMain) + "; }");
        headerLay->addWidget(nameLbl, 1);

        int musicCount = info.value("musicCount").toInt();
        auto *countLbl = new QLabel(QString("%1 %2").arg(musicCount).arg(I18n::instance().tr("musicCount")), headerWidget);
        countLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
        headerLay->addWidget(countLbl);

        mainLay->addWidget(headerWidget);

        // 展开/收起按钮
        m_expandBtn = new QPushButton(QString::fromUtf8("▼"), this);
        m_expandBtn->setFixedHeight(32);
        m_expandBtn->setCursor(Qt::PointingHandCursor);
        m_expandBtn->setStyleSheet(
            "QPushButton { background: transparent; border: none; color: " + QString(Theme::kTextMuted) + "; font-size: 12px; }"
            "QPushButton:hover { color: " + QString(Theme::kTextMain) + "; }"
        );
        mainLay->addWidget(m_expandBtn);

        // 歌手音乐列表容器
        m_musicContainer = new QWidget(this);
        m_musicLayout = new QVBoxLayout(m_musicContainer);
        m_musicLayout->setContentsMargins(0, 0, 0, 0);
        m_musicLayout->setSpacing(4);
        m_musicContainer->setVisible(false);
        mainLay->addWidget(m_musicContainer);

        connect(m_expandBtn, &QPushButton::clicked, this, [this]() {
            bool visible = m_musicContainer->isVisible();
            m_musicContainer->setVisible(!visible);
            m_expandBtn->setText(visible ? QString::fromUtf8("▼") : QString::fromUtf8("▲"));
        });

        connect(headerWidget, &QWidget::destroyed, this, [this]() { m_headerDestroyed = true; });
    }

    void setMusicList(const QList<MusicInfo> &musicList)
    {
        QLayoutItem *item;
        while ((item = m_musicLayout->takeAt(0)) != nullptr) { delete item->widget(); delete item; }

        for (int i = 0; i < musicList.size(); ++i) {
            const auto &info = musicList.at(i);
            auto *card = new ArtistMusicItem(info, m_musicContainer);
            card->onClicked = [this, info](int) {
                if (m_headerDestroyed) return;
                if (onPlayMusic) onPlayMusic(info);
            };
            m_musicLayout->addWidget(card);
        }
    }

    std::function<void()> onHeaderClicked;
    std::function<void(const MusicInfo&)> onPlayMusic;

private:
    void mousePressEvent(QMouseEvent *e) override
    {
        // Only trigger on header area (first 60px)
        if (e->button() == Qt::LeftButton && e->pos().y() < 60 && onHeaderClicked) {
            onHeaderClicked();
        }
        QWidget::mousePressEvent(e);
    }

private:
    QVariantMap m_artistInfo;
    QLabel *m_avatarLbl = nullptr;
    QPushButton *m_expandBtn = nullptr;
    QWidget *m_musicContainer = nullptr;
    QVBoxLayout *m_musicLayout = nullptr;
    bool m_headerDestroyed = false;
};

SearchPage::SearchPage(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), m_apiClient(apiClient)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
}

void SearchPage::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // Header
    auto *header = new QWidget(this);
    header->setFixedHeight(56);
    header->setStyleSheet("QWidget { background: " + QString(Theme::kGlassSidebar) + "; }");
    auto *headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(20, 0, 20, 0);
    headerLay->setSpacing(12);

    auto *backBtn = new QPushButton(QString::fromUtf8("←"), header);
    backBtn->setFixedSize(36, 36);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: rgba(245,240,255,15); border: none; border-radius: 18px; "
        "color: " + QString(Theme::kTextMain) + "; font-size: 18px; }"
        "QPushButton:hover { background: rgba(196,167,231,40); color: " + QString(Theme::kLavender) + "; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &SearchPage::backRequested);
    headerLay->addWidget(backBtn);

    // Tabs
    m_musicTab = new QPushButton(I18n::instance().tr("searchMusic"), header);
    m_musicTab->setFixedHeight(36);
    m_musicTab->setCursor(Qt::PointingHandCursor);
    m_musicTab->setCheckable(true);
    m_musicTab->setChecked(true);
    m_musicTab->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: " + QString(Theme::kTextSub) + "; "
        "font-size: 13px; font-weight: 600; padding: 0 16px; }"
        "QPushButton:checked { color: " + QString(Theme::kMint) + "; border-bottom: 2px solid " + QString(Theme::kMint) + "; }"
        "QPushButton:hover { color: " + QString(Theme::kTextMain) + "; }"
    );
    connect(m_musicTab, &QPushButton::clicked, this, &SearchPage::showMusicPage);
    headerLay->addWidget(m_musicTab);

    m_playlistTab = new QPushButton(I18n::instance().tr("searchPlaylist"), header);
    m_playlistTab->setFixedHeight(36);
    m_playlistTab->setCursor(Qt::PointingHandCursor);
    m_playlistTab->setCheckable(true);
    m_playlistTab->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: " + QString(Theme::kTextSub) + "; "
        "font-size: 13px; font-weight: 600; padding: 0 16px; }"
        "QPushButton:checked { color: " + QString(Theme::kMint) + "; border-bottom: 2px solid " + QString(Theme::kMint) + "; }"
        "QPushButton:hover { color: " + QString(Theme::kTextMain) + "; }"
    );
    connect(m_playlistTab, &QPushButton::clicked, this, &SearchPage::showPlaylistPage);
    headerLay->addWidget(m_playlistTab);

    m_artistTab = new QPushButton(I18n::instance().tr("artist"), header);
    m_artistTab->setFixedHeight(36);
    m_artistTab->setCursor(Qt::PointingHandCursor);
    m_artistTab->setCheckable(true);
    m_artistTab->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: " + QString(Theme::kTextSub) + "; "
        "font-size: 13px; font-weight: 600; padding: 0 16px; }"
        "QPushButton:checked { color: " + QString(Theme::kMint) + "; border-bottom: 2px solid " + QString(Theme::kMint) + "; }"
        "QPushButton:hover { color: " + QString(Theme::kTextMain) + "; }"
    );
    connect(m_artistTab, &QPushButton::clicked, this, &SearchPage::showArtistPage);
    headerLay->addWidget(m_artistTab);

    headerLay->addStretch();
    mainLay->addWidget(header);

    // Status label
    m_statusLabel = new QLabel(I18n::instance().tr("inputKeywordToSearch"), this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }"
    );
    mainLay->addWidget(m_statusLabel);

    // Scroll area
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setVisible(false);

    m_container = new QWidget(m_scroll);
    m_listLayout = new QVBoxLayout(m_container);
    m_listLayout->setContentsMargins(16, 16, 16, 16);
    m_listLayout->setSpacing(8);
    m_listLayout->setAlignment(Qt::AlignTop);

    m_scroll->setWidget(m_container);
    mainLay->addWidget(m_scroll, 1);
}

void SearchPage::search(const QString &query)
{
    m_query = query;
    m_page = 1;
    m_musicResults.clear();
    m_playlistResults.clear();
    m_artistResults.clear();

    showLoading();
    fetchMusicResults();
    fetchPlaylistResults();
    fetchArtistResults();
}

void SearchPage::showLoading()
{
    m_statusLabel->setText(I18n::instance().tr("loading"));
    m_statusLabel->show();
    m_scroll->setVisible(false);
}

void SearchPage::hideLoading()
{
    m_statusLabel->hide();
    m_scroll->setVisible(true);
}

void SearchPage::fetchMusicResults()
{
    if (!m_apiClient) return;
    m_apiClient->searchMusic(m_query, m_page, kPageSize, [this](bool success, int total, int page, int pageSize, const QList<QVariantMap> &results) {
        QTimer::singleShot(0, this, [this, success, results]() {
            if (success) {
                m_musicResults.clear();
                for (const auto &item : results) {
                    MusicInfo info;
                    info.id = item.value("id").toInt();
                    info.title = item.value("title").toString();
                    info.artist = item.value("artist").toString();
                    info.album = item.value("album").toString();
                    info.duration = item.value("duration").toInt();
                    info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2").arg(Theme::kApiBase).arg(info.id);
                    m_musicResults.append(info);
                }
            }
            if (m_activeTab == 0) buildMusicList();
        });
    });
}

void SearchPage::fetchPlaylistResults()
{
    if (!m_apiClient) return;
    m_apiClient->fetchPlaylists(m_query, [this](bool success, const QList<QVariantMap> &results) {
        QTimer::singleShot(0, this, [this, success, results]() {
            if (success) {
                m_playlistResults = results;
            }
            if (m_activeTab == 1) buildPlaylistList();
        });
    });
}

void SearchPage::fetchArtistResults()
{
    if (!m_apiClient) return;
    m_apiClient->searchArtists(m_query, [this](bool success, const QVariantMap &result) {
        QTimer::singleShot(0, this, [this, success, result]() {
            if (success) {
                m_artistResults.clear();
                m_artistResults.append(result);
            }
            if (m_activeTab == 2) buildArtistList();
        });
    });
}

void SearchPage::buildMusicList()
{
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) { delete item->widget(); delete item; }

    if (m_musicResults.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("noResults"), m_container);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet("QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }");
        m_listLayout->addWidget(emptyLbl);
    } else {
        // Play all button + result count header
        auto *headerWidget = new QWidget(m_container);
        auto *headerLay = new QHBoxLayout(headerWidget);
        headerLay->setContentsMargins(0, 0, 0, 8);

        m_playAllBtn = new QPushButton(I18n::instance().tr("playAll"), headerWidget);
        m_playAllBtn->setFixedHeight(36);
        m_playAllBtn->setCursor(Qt::PointingHandCursor);
        m_playAllBtn->setStyleSheet(
            "QPushButton { background: " + QString(Theme::kMint) + "; border: none; border-radius: 18px; "
            "color: " + QString(Theme::kBgMid) + "; font-size: 13px; font-weight: 600; padding: 0 20px; }"
            "QPushButton:hover { background: " + QString(Theme::kMintLt) + "; }"
        );
        connect(m_playAllBtn, &QPushButton::clicked, this, [this]() {
            emit playAllRequested(m_musicResults);
        });
        headerLay->addWidget(m_playAllBtn);

        m_resultCountLbl = new QLabel(QString("%1 %2").arg(m_musicResults.size()).arg(I18n::instance().tr("songs")), headerWidget);
        m_resultCountLbl->setStyleSheet("QLabel { font-size: 13px; color: " + QString(Theme::kTextMuted) + "; }");
        headerLay->addWidget(m_resultCountLbl);
        headerLay->addStretch();

        m_listLayout->addWidget(headerWidget);

        // Music cards
        for (int i = 0; i < m_musicResults.size(); ++i) {
            const auto &info = m_musicResults.at(i);
            auto *card = new SearchResultCard(info, i, m_container);
            card->onClicked = [this, info](int) { emit playMusic(info); };
            card->onAddToPlaylist = [this, info]() {
                // Will be connected by MainWindow
            };
            card->onAddToQueue = [this, info]() {
                PlaylistManager::instance().addToPlaylist(info);
            };
            m_listLayout->addWidget(card);
        }
    }
    m_listLayout->addStretch(1);
    hideLoading();
}

void SearchPage::buildPlaylistList()
{
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) { delete item->widget(); delete item; }

    if (m_playlistResults.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("noResults"), m_container);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet("QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }");
        m_listLayout->addWidget(emptyLbl);
    } else {
        for (const auto &info : m_playlistResults) {
            auto *card = new PlaylistResultCard(info, m_container);
            int playlistId = info.value("id").toInt();
            card->onClicked = [this](int id) { emit openPlaylist(id); };
            m_listLayout->addWidget(card);
        }
    }
    m_listLayout->addStretch(1);
    hideLoading();
}

void SearchPage::buildArtistList()
{
    QLayoutItem *item;
    while ((item = m_listLayout->takeAt(0)) != nullptr) { delete item->widget(); delete item; }

    if (m_artistResults.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("noResults"), m_container);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet("QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }");
        m_listLayout->addWidget(emptyLbl);
    } else {
        for (const auto &info : m_artistResults) {
            auto *card = new ArtistResultCard(info, m_container);
            card->onHeaderClicked = [this]() {};
            card->onPlayMusic = [this](const MusicInfo &info) {
                emit playMusic(info);
            };

            // 解析歌手音乐列表
            QVariantMap artistObj = info.value("artist").toMap();
            if (artistObj.isEmpty()) artistObj = info;
            QVariantList musicList = artistObj.value("musicList").toList();
            if (musicList.isEmpty()) musicList = info.value("musicList").toList();

            QList<MusicInfo> artistMusicList;
            for (const auto &m : musicList) {
                MusicInfo mi;
                mi.id = m.toMap().value("id").toInt();
                mi.title = m.toMap().value("title").toString();
                mi.artist = m.toMap().value("artist").toString();
                mi.album = m.toMap().value("album").toString();
                mi.duration = m.toMap().value("duration").toInt();
                mi.coverUrl = QString::fromUtf8("%1/api/music/cover/%2").arg(Theme::kApiBase).arg(mi.id);
                artistMusicList.append(mi);
            }
            card->setMusicList(artistMusicList);

            m_listLayout->addWidget(card);
        }
    }
    m_listLayout->addStretch(1);
    hideLoading();
}

void SearchPage::showMusicPage()
{
    m_activeTab = 0;
    m_musicTab->setChecked(true);
    m_playlistTab->setChecked(false);
    m_artistTab->setChecked(false);
    if (!m_musicResults.isEmpty()) buildMusicList();
    else if (!m_query.isEmpty()) showLoading();
}

void SearchPage::showPlaylistPage()
{
    m_activeTab = 1;
    m_playlistTab->setChecked(true);
    m_musicTab->setChecked(false);
    m_artistTab->setChecked(false);
    if (!m_playlistResults.isEmpty()) buildPlaylistList();
    else if (!m_query.isEmpty()) showLoading();
}

void SearchPage::showArtistPage()
{
    m_activeTab = 2;
    m_artistTab->setChecked(true);
    m_musicTab->setChecked(false);
    m_playlistTab->setChecked(false);
    if (!m_artistResults.isEmpty()) buildArtistList();
    else if (!m_query.isEmpty()) showLoading();
}

void SearchPage::retranslate()
{
    m_musicTab->setText(I18n::instance().tr("searchMusic"));
    m_playlistTab->setText(I18n::instance().tr("searchPlaylist"));
    m_artistTab->setText(I18n::instance().tr("artist"));
    if (m_playAllBtn) m_playAllBtn->setText(I18n::instance().tr("playAll"));
    if (m_resultCountLbl && !m_musicResults.isEmpty()) {
        m_resultCountLbl->setText(QString("%1 %2").arg(m_musicResults.size()).arg(I18n::instance().tr("songs")));
    }
}

void SearchPage::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}
