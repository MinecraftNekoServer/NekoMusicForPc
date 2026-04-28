/**
 * @file playlistdetailpage.cpp
 * @brief 播放列表详情页实现
 */

#include "playlistdetailpage.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "core/covercache.h"
#include "theme/theme.h"
#include "ui/svgicon.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QMenu>

// ─── 播放列表音乐项卡片 ──────────────────────────────────────
class PlaylistMusicCard : public QWidget
{
public:
    explicit PlaylistMusicCard(const MusicInfo &info, int musicId, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(musicId), m_info(info)
    {
        setFixedHeight(70);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, false);

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(12, 8, 12, 8);
        lay->setSpacing(14);

        // 封面
        m_coverLbl = new QLabel(this);
        m_coverLbl->setFixedSize(54, 54);
        m_coverLbl->setScaledContents(false);
        loadCover();
        lay->addWidget(m_coverLbl);

        // 信息
        auto *infoV = new QWidget(this);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(4);

        m_titleLbl = new QLabel(info.title, infoV);
        m_titleLbl->setStyleSheet("QLabel { font-size: 14px; font-weight: 600; color: " + QString(Theme::kTextMain) + "; }");
        infoLay->addWidget(m_titleLbl);

        m_artistLbl = new QLabel(info.artist + " - " + info.album, infoV);
        m_artistLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextSub) + "; }");
        infoLay->addWidget(m_artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        // 时长
        int mins = info.duration / 60;
        int secs = info.duration % 60;
        auto *timeLbl = new QLabel(
            QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')), this);
        timeLbl->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
        lay->addWidget(timeLbl);
    }

    int musicId() const { return m_musicId; }

    std::function<void(int)> onClicked;
    std::function<void(int)> onRemoveRequested;

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

    void contextMenuEvent(QContextMenuEvent *event) override
    {
        QMenu menu(this);
        menu.setStyleSheet(
            "QMenu { background-color: rgba(40, 40, 50, 0.95); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 8px; padding: 4px; }"
            "QMenu::item { color: #e0e0e0; padding: 8px 24px; border-radius: 4px; }"
            "QMenu::item:selected { background-color: rgba(255, 255, 255, 0.1); }"
        );

        QAction *removeAction = menu.addAction(I18n::instance().tr("removeFromPlaylist"));
        QAction *selected = menu.exec(event->globalPos());
        if (selected == removeAction && onRemoveRequested) {
            onRemoveRequested(m_musicId);
        }
    }

private:
    void loadCover()
    {
        if (m_info.coverUrl.isEmpty()) {
            setPlaceholder();
            return;
        }
        QString musicId = QString::number(m_musicId);
        QPixmap cached = CoverCache::instance()->get(musicId);
        if (!cached.isNull()) {
            applyPixmap(cached);
            return;
        }
        connect(CoverCache::instance(), &CoverCache::coverLoaded, this,
                [this, musicId](const QString &id, const QPixmap &pix) {
            if (id == musicId) applyPixmap(pix);
        });
        CoverCache::instance()->fetchCover(musicId, m_info.coverUrl);
    }

    void setPlaceholder()
    {
        QPixmap pix(54, 54);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        QPainterPath path;
        path.addRoundedRect(0, 0, 54, 54, 6, 6);
        p.fillPath(path, QColor(128, 128, 128, 40));
        p.setClipPath(path);
        auto icon = Icons::render(Icons::kMusic, 24, QColor(255, 255, 255, 100));
        p.drawPixmap(15, 15, icon);
        m_coverLbl->setPixmap(pix);
    }

    void applyPixmap(const QPixmap &pix)
    {
        int s = qMin(pix.width(), pix.height());
        QPixmap scaled = pix.copy((pix.width()-s)/2, (pix.height()-s)/2, s, s)
            .scaled(54, 54, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_coverLbl->setPixmap(scaled);
    }

    int m_musicId;
    MusicInfo m_info;
    QLabel *m_coverLbl = nullptr;
    QLabel *m_titleLbl = nullptr;
    QLabel *m_artistLbl = nullptr;
};

// ─── PlaylistDetailPage ──────────────────────────────────────

PlaylistDetailPage::PlaylistDetailPage(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), m_apiClient(apiClient)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
}

void PlaylistDetailPage::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // 顶部栏：返回按钮 + 标题
    auto *header = new QWidget(this);
    header->setFixedHeight(56);
    header->setStyleSheet("QWidget { background: " + QString(Theme::kGlassSidebar) + "; }");
    auto *headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(20, 0, 20, 0);
    headerLay->setSpacing(12);

    auto *backBtn = new QPushButton(QStringLiteral("←"), header);
    backBtn->setFixedSize(36, 36);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet(
        "QPushButton { background: rgba(245,240,255,15); border: none; border-radius: 18px; "
        "color: " + QString(Theme::kTextMain) + "; font-size: 18px; }"
        "QPushButton:hover { background: rgba(196,167,231,40); color: " + QString(Theme::kLavender) + "; }"
    );
    connect(backBtn, &QPushButton::clicked, this, &PlaylistDetailPage::backRequested);
    headerLay->addWidget(backBtn);

    m_headerLabel = new QLabel(header);
    m_headerLabel->setStyleSheet(
        "QLabel { font-size: 18px; font-weight: 700; color: " + QString(Theme::kLavender) + "; }"
    );
    headerLay->addWidget(m_headerLabel);

    headerLay->addStretch();
    mainLay->addWidget(header);

    // 滚动列表区
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setObjectName("playlistScroll");

    m_listContainer = new QWidget(m_scroll);
    m_listContainer->setObjectName("playlistContainer");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(16, 16, 16, 16);
    m_listLayout->setSpacing(8);

    m_scroll->setWidget(m_listContainer);
    mainLay->addWidget(m_scroll, 1);
}

void PlaylistDetailPage::loadPlaylist(int playlistId)
{
    m_playlistId = playlistId;

    if (!m_apiClient) {
        m_playlistName = QStringLiteral("歌单详情");
        updateHeader();
        m_musicList.clear();
        buildList();
        return;
    }

    // 先获取歌单详情得到名称
    m_apiClient->fetchPlaylistDetail(playlistId, [this](bool success, const QVariantMap &detail) {
        if (success) {
            m_playlistName = detail.value("name").toString();
        } else {
            m_playlistName = QStringLiteral("歌单详情");
        }
        updateHeader();

        // 然后获取音乐列表
        m_apiClient->fetchPlaylistMusic(m_playlistId, [this](bool success, int total, const QList<QVariantMap> &musicList) {
            m_musicList.clear();
            if (success) {
                for (const auto &m : musicList) {
                    MusicInfo info;
                    info.id = m.value("id").toInt();
                    info.title = m.value("title").toString();
                    info.artist = m.value("artist").toString();
                    info.album = m.value("album").toString();
                    info.duration = m.value("duration").toInt();
                    info.coverUrl = m.value("coverPath").toString();
                    m_musicList.append(info);
                }
            }
            buildList();
        });
    });
}

void PlaylistDetailPage::updateHeader()
{
    if (m_headerLabel) {
        m_headerLabel->setText(m_playlistName);
    }
}

void PlaylistDetailPage::buildList()
{
    // 清除现有项
    for (auto *widget : m_musicItems) {
        m_listLayout->removeWidget(widget);
        widget->deleteLater();
    }
    m_musicItems.clear();

    if (m_musicList.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("noMusicInPlaylist"), m_listContainer);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet(
            "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }"
        );
        m_listLayout->addWidget(emptyLbl);
        m_musicItems.append(emptyLbl);
    } else {
        for (const auto &info : m_musicList) {
            auto *card = new PlaylistMusicCard(info, info.id, m_listContainer);
            card->onClicked = [this, info](int) {
                emit playMusic(info);
            };
            card->onRemoveRequested = [this, musicId = info.id](int) {
                if (!m_apiClient) return;
                m_apiClient->removeMusicFromPlaylist(m_playlistId, musicId, [this](bool success, const QString &) {
                    if (success) {
                        loadPlaylist(m_playlistId);
                        emit refreshSidebarPlaylists();
                    }
                });
            };
            m_listLayout->addWidget(card);
            m_musicItems.append(card);
        }
    }

    m_listLayout->addStretch();
}

void PlaylistDetailPage::retranslate()
{
    if (m_headerLabel) {
        m_headerLabel->setText(m_playlistName);
    }
}

void PlaylistDetailPage::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    // 透明背景，由父窗口渐变透出
}
