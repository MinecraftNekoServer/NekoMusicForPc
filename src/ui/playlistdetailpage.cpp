/**
 * @file playlistdetailpage.cpp
 * @brief 播放列表详情页实现
 */

#include "playlistdetailpage.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "core/covercache.h"
#include "theme/theme.h"
#include "theme/thememanager.h"
#include "glasspaint.h"
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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFrame>
#include <QStyle>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>
#include <QColor>

// ─── 播放列表音乐项卡片 ──────────────────────────────────────
class PlaylistMusicCard : public QWidget
{
public:
    explicit PlaylistMusicCard(int index, const MusicInfo &info, int musicId, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(musicId), m_index(index), m_info(info)
    {
        setObjectName(QStringLiteral("PlaylistMusicCard"));
        setFixedHeight(72);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, true);

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(20, 12, 20, 12);
        lay->setSpacing(16);

        m_indexLbl = new QLabel(QString::number(index + 1), this);
        m_indexLbl->setFixedSize(32, 72);
        m_indexLbl->setAlignment(Qt::AlignCenter);
        lay->addWidget(m_indexLbl);

        m_coverLbl = new QLabel(this);
        m_coverLbl->setFixedSize(48, 48);
        m_coverLbl->setScaledContents(false);
        loadCover();
        lay->addWidget(m_coverLbl);

        auto *infoV = new QWidget(this);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(4);

        m_titleLbl = new QLabel(info.title, infoV);
        m_titleLbl->setObjectName("musicTitle");
        infoLay->addWidget(m_titleLbl);

        m_artistLbl = new QLabel(info.artist, infoV);
        m_artistLbl->setObjectName("musicArtist");
        infoLay->addWidget(m_artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        int mins = info.duration / 60;
        int secs = info.duration % 60;
        m_timeLbl = new QLabel(
            QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')), this);
        m_timeLbl->setFixedWidth(60);
        m_timeLbl->setAlignment(Qt::AlignCenter);
        lay->addWidget(m_timeLbl);

        applyPalette();
    }

    void applyPalette()
    {
        const bool dark = Theme::ThemeManager::instance().isDarkMode();
        const QString border = dark ? QStringLiteral("rgba(255,255,255,0.10)")
                                    : QStringLiteral("rgba(33,37,41,0.10)");
        const QString hover = dark ? QStringLiteral("rgba(196,167,231,0.10)")
                                   : QStringLiteral("rgba(196,167,231,0.14)");
        const QString playingBg = dark ? QStringLiteral("rgba(196,167,231,0.16)")
                                       : QStringLiteral("rgba(196,167,231,0.22)");
        setStyleSheet(QStringLiteral(
                          "PlaylistMusicCard { background: transparent; border-bottom: 1px solid %1; }"
                          "PlaylistMusicCard:hover { background: %2; }"
                          "PlaylistMusicCard[playing=\"true\"] { background: %3; }")
                          .arg(border, hover, playingBg));

        const QString titleFg = dark ? QStringLiteral("white")
                                     : QStringLiteral("#212529");
        const QString subFg = dark ? QStringLiteral("rgba(255,255,255,0.62)")
                                   : QStringLiteral("rgba(33,37,41,0.62)");
        const QString idxIdle = dark ? QStringLiteral("rgba(255,255,255,0.48)")
                                     : QStringLiteral("rgba(33,37,41,0.45)");
        const QString playAccent = QString::fromUtf8(Theme::kLavender);

        m_titleLbl->setStyleSheet(QStringLiteral(
            "QLabel#musicTitle { font-size: 14px; font-weight: 500; color: %1; padding: 0; margin: 0; }")
                                      .arg(titleFg));
        m_artistLbl->setStyleSheet(QStringLiteral(
            "QLabel#musicArtist { font-size: 12px; color: %1; padding: 0; margin: 0; }")
                                       .arg(subFg));
        m_timeLbl->setStyleSheet(QStringLiteral(
            "QLabel { font-size: 13px; color: %1; }")
                                     .arg(subFg));

        if (m_isPlaying) {
            m_indexLbl->setText(QStringLiteral("▶"));
            m_indexLbl->setStyleSheet(QStringLiteral(
                "QLabel { font-size: 14px; color: %1; }")
                                          .arg(playAccent));
        } else {
            m_indexLbl->setText(QString::number(m_index + 1));
            m_indexLbl->setStyleSheet(QStringLiteral(
                "QLabel { font-size: 14px; color: %1; }")
                                          .arg(idxIdle));
        }
    }

    int musicId() const { return m_musicId; }
    void setPlaying(bool playing) {
        m_isPlaying = playing;
        setProperty("playing", playing);
        style()->unpolish(this);
        style()->polish(this);
        applyPalette();
    }

    std::function<void(int)> onClicked;
    std::function<void(int)> onRemoveRequested;

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QWidget::paintEvent(event);
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
            "QMenu { background-color: rgba(30, 30, 50, 0.98); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 8px; padding: 8px 0; min-width: 180px; }"
            "QMenu::item { color: rgba(255, 255, 255, 0.9); padding: 10px 16px; margin: 0; border-radius: 0; }"
            "QMenu::item:selected { background-color: rgba(196, 167, 231, 0.18); }"
            "QMenu::item:disabled { color: rgba(255, 255, 255, 0.4); }"
        );

        QAction *removeAction = menu.addAction(I18n::instance().tr("removeFromPlaylist"));
        removeAction->setEnabled(true);
        QAction *selected = menu.exec(event->globalPos());
        if (selected == removeAction && onRemoveRequested) {
            onRemoveRequested(m_musicId);
        }
    }

private:
    void loadCover()
    {
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
        QString url;
        if (!m_info.coverUrl.isEmpty()) {
            url = m_info.coverUrl;
        } else {
            url = QString::fromUtf8("%1/api/music/cover/%2").arg(Theme::kApiBase).arg(m_musicId);
        }
        CoverCache::instance()->fetchCover(musicId, url);
    }

    void setPlaceholder()
    {
        QPixmap pix(48, 48);
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(0, 0, 48, 48, 4, 4);
        p.fillPath(path, QColor(196, 167, 231, 200));
        p.setClipPath(path);
        auto icon = Icons::render(Icons::kMusic, 20, QColor(255, 255, 255, 200));
        p.drawPixmap(14, 14, icon);
        m_coverLbl->setPixmap(pix);
    }

    void applyPixmap(const QPixmap &pix)
    {
        if (pix.isNull()) {
            setPlaceholder();
            return;
        }
        QPixmap scaled = pix.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_coverLbl->setPixmap(scaled);
    }

    int m_musicId;
    int m_index;
    bool m_isPlaying = false;
    MusicInfo m_info;
    QLabel *m_indexLbl = nullptr;
    QLabel *m_coverLbl = nullptr;
    QLabel *m_titleLbl = nullptr;
    QLabel *m_artistLbl = nullptr;
    QLabel *m_timeLbl = nullptr;
};

// ─── PlaylistDetailPage ──────────────────────────────────────

PlaylistDetailPage::PlaylistDetailPage(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), m_apiClient(apiClient)
{
    setObjectName(QStringLiteral("playlistDetailPage"));
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();

    connect(&Theme::ThemeManager::instance(), &Theme::ThemeManager::themeChanged, this,
            [this](Theme::ThemeMode) {
                applyPlaylistDetailStyle();
                update();
            });
}

void PlaylistDetailPage::applyPlaylistDetailStyle()
{
    const bool dark = Theme::ThemeManager::instance().isDarkMode();

    if (m_headerGlass) {
        if (dark) {
            m_headerGlass->setBaseColor(QColor(45, 38, 65));
            m_headerGlass->setBorderColor(QColor(196, 167, 231, 58));
            m_headerGlass->setOpacity(0.54);
        } else {
            m_headerGlass->setBaseColor(QColor(255, 255, 255));
            m_headerGlass->setBorderColor(QColor(111, 66, 193, 72));
            m_headerGlass->setOpacity(0.64);
        }
        m_headerGlass->setBorderRadius(Theme::kRXl);
    }
    if (m_listGlass) {
        if (dark) {
            m_listGlass->setBaseColor(QColor(38, 33, 54));
            m_listGlass->setBorderColor(QColor(196, 167, 231, 45));
            m_listGlass->setOpacity(0.50);
        } else {
            m_listGlass->setBaseColor(QColor(255, 255, 255));
            m_listGlass->setBorderColor(QColor(111, 66, 193, 55));
            m_listGlass->setOpacity(0.58);
        }
        m_listGlass->setBorderRadius(Theme::kRLg);
    }

    if (m_scroll) {
        m_scroll->setStyleSheet(QStringLiteral(
            "QScrollArea#playlistScroll { border: none; background: transparent; }"
            "QScrollBar:vertical { width: 6px; background: transparent; }"
            "QScrollBar::handle:vertical { background: rgba(196,167,231,%1); border-radius: 3px; min-height: 48px; }"
            "QScrollBar::handle:vertical:hover { background: rgba(196,167,231,%2); }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }")
                                    .arg(dark ? 68 : 82)
                                    .arg(dark ? 105 : 125));
    }

    setStyleSheet(QStringLiteral("#playlistDetailPage { background: transparent; }"));

    const QString typeCol = dark ? QStringLiteral("rgba(255,255,255,0.58)")
                                 : QStringLiteral("rgba(33,37,41,0.55)");
    const QString nameCol = dark ? QStringLiteral("white") : QStringLiteral("#212529");
    const QString descCol = dark ? QStringLiteral("rgba(255,255,255,0.72)")
                                 : QStringLiteral("rgba(33,37,41,0.72)");
    const QString descHoverBg = dark ? QStringLiteral("rgba(255,255,255,0.08)")
                                     : QStringLiteral("rgba(33,37,41,0.06)");
    const QString creatorCol = dark ? QStringLiteral("rgba(255,255,255,0.82)")
                                    : QStringLiteral("rgba(33,37,41,0.78)");
    const QString countCol = dark ? QStringLiteral("rgba(255,255,255,0.55)")
                                  : QStringLiteral("rgba(33,37,41,0.52)");
    const QString listHeadSep = dark ? QStringLiteral("rgba(255,255,255,0.10)")
                                     : QStringLiteral("rgba(33,37,41,0.10)");

    if (m_typeLbl) {
        m_typeLbl->setStyleSheet(QStringLiteral(
            "QLabel#playlistType { font-size: 12px; color: %1; letter-spacing: 1px; text-transform: uppercase; }")
                                     .arg(typeCol));
    }
    if (m_nameLbl) {
        m_nameLbl->setStyleSheet(QStringLiteral(
            "QLabel#playlistName { font-size: 28px; font-weight: 700; color: %1; line-height: 1.3; }")
                                     .arg(nameCol));
    }
    if (m_descLbl) {
        m_descLbl->setStyleSheet(QStringLiteral(
            "QLabel#playlistDesc { font-size: 14px; color: %1; line-height: 1.6; padding: 4px; border-radius: 4px; }"
            "QLabel#playlistDesc:hover { background-color: %2; }")
                                     .arg(descCol, descHoverBg));
    }
    if (m_creatorAvatarLbl) {
        m_creatorAvatarLbl->setStyleSheet(QStringLiteral(
            "QLabel { border-radius: 10px; background: rgba(196,167,231,%1); }")
                                              .arg(dark ? 42 : 28));
    }
    if (m_creatorNameLbl) {
        m_creatorNameLbl->setStyleSheet(QStringLiteral(
            "QLabel#creatorName { font-size: 12px; color: %1; font-weight: 500; }")
                                            .arg(creatorCol));
    }
    if (m_countLbl) {
        m_countLbl->setStyleSheet(QStringLiteral(
            "QLabel#playlistCount { font-size: 13px; color: %1; }")
                                      .arg(countCol));
    }
    if (m_coverLbl) {
        m_coverLbl->setStyleSheet(QStringLiteral(
            "QLabel { border-radius: 12px; border: 1px solid rgba(196,167,231,%1); }")
                                      .arg(dark ? 38 : 48));
    }
    if (m_listHeaderWidget) {
        m_listHeaderWidget->setStyleSheet(QStringLiteral(
            "QWidget#listHeader { background: transparent; border-bottom: 1px solid %1; }")
                                              .arg(listHeadSep));
    }
    if (m_listTitleLbl) {
        m_listTitleLbl->setStyleSheet(QStringLiteral(
            "QLabel#listTitle { font-size: 15px; font-weight: 600; color: %1; }")
                                          .arg(nameCol));
    }
    if (m_listCountLbl) {
        m_listCountLbl->setStyleSheet(QStringLiteral(
            "QLabel#listCount { font-size: 13px; color: %1; }")
                                          .arg(countCol));
    }

    for (QWidget *w : m_musicItems) {
        if (auto *emptyHint = qobject_cast<QLabel *>(w)) {
            if (emptyHint->objectName() == QLatin1String("playlistEmptyHint")) {
                const QString emptyCol = dark ? QString::fromUtf8(Theme::kTextSub)
                                              : QStringLiteral("rgba(33,37,41,0.55)");
                emptyHint->setStyleSheet(QStringLiteral(
                    "QLabel { color: %1; font-size: 14px; padding: 60px 20px; }")
                                             .arg(emptyCol));
                continue;
            }
        }
        if (auto *card = dynamic_cast<PlaylistMusicCard *>(w))
            card->applyPalette();
    }
}

void PlaylistDetailPage::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setObjectName("playlistScroll");

    m_contentWidget = new QWidget(m_scroll);
    m_contentWidget->setObjectName("playlistContent");
    m_contentWidget->setStyleSheet(QStringLiteral("QWidget#playlistContent { background: transparent; }"));

    auto *contentLay = new QVBoxLayout(m_contentWidget);
    contentLay->setContentsMargins(24, 24, 24, 24);
    contentLay->setSpacing(24);

    m_headerGlass = new GlassWidget(m_contentWidget);
    m_headerGlass->setAttribute(Qt::WA_StyledBackground, false);
    auto *headerLay = new QHBoxLayout(m_headerGlass);
    headerLay->setContentsMargins(24, 24, 24, 24);
    headerLay->setSpacing(24);

    m_coverLbl = new QLabel(m_headerGlass);
    m_coverLbl->setFixedSize(200, 200);
    m_coverLbl->setScaledContents(false);
    headerLay->addWidget(m_coverLbl);

    auto *infoWidget = new QWidget(m_headerGlass);
    auto *infoLay = new QVBoxLayout(infoWidget);
    infoLay->setContentsMargins(0, 0, 0, 0);
    infoLay->setSpacing(12);

    m_typeLbl = new QLabel(I18n::instance().tr("playlists"), infoWidget);
    m_typeLbl->setObjectName("playlistType");
    infoLay->addWidget(m_typeLbl);

    m_nameLbl = new QLabel(infoWidget);
    m_nameLbl->setObjectName("playlistName");
    m_nameLbl->setWordWrap(true);
    infoLay->addWidget(m_nameLbl);

    m_descLbl = new QLabel(I18n::instance().tr("description"), infoWidget);
    m_descLbl->setObjectName("playlistDesc");
    m_descLbl->setWordWrap(true);
    m_descLbl->setCursor(Qt::PointingHandCursor);
    m_descLbl->installEventFilter(this);
    infoLay->addWidget(m_descLbl);

    auto *creatorWidget = new QWidget(infoWidget);
    auto *creatorLay = new QHBoxLayout(creatorWidget);
    creatorLay->setContentsMargins(0, 0, 0, 0);
    creatorLay->setSpacing(8);

    m_creatorAvatarLbl = new QLabel(creatorWidget);
    m_creatorAvatarLbl->setFixedSize(20, 20);
    m_creatorAvatarLbl->setScaledContents(true);
    creatorLay->addWidget(m_creatorAvatarLbl);

    m_creatorNameLbl = new QLabel(creatorWidget);
    m_creatorNameLbl->setObjectName("creatorName");
    creatorLay->addWidget(m_creatorNameLbl);
    creatorLay->addStretch();

    infoLay->addWidget(creatorWidget);

    m_countLbl = new QLabel(infoWidget);
    m_countLbl->setObjectName("playlistCount");
    infoLay->addWidget(m_countLbl);

    infoLay->addStretch();
    headerLay->addWidget(infoWidget, 1);

    contentLay->addWidget(m_headerGlass);

    m_listGlass = new GlassWidget(m_contentWidget);
    m_listGlass->setAttribute(Qt::WA_StyledBackground, false);
    auto *listOuter = new QVBoxLayout(m_listGlass);
    listOuter->setContentsMargins(0, 0, 0, 0);
    listOuter->setSpacing(0);

    m_listHeaderWidget = new QWidget(m_listGlass);
    m_listHeaderWidget->setObjectName("listHeader");
    auto *listHeaderLay = new QHBoxLayout(m_listHeaderWidget);
    listHeaderLay->setContentsMargins(20, 16, 20, 16);

    m_listTitleLbl = new QLabel(I18n::instance().tr("songList"), m_listHeaderWidget);
    m_listTitleLbl->setObjectName("listTitle");
    listHeaderLay->addWidget(m_listTitleLbl);

    listHeaderLay->addStretch();

    m_listCountLbl = new QLabel(m_listHeaderWidget);
    m_listCountLbl->setObjectName("listCount");
    listHeaderLay->addWidget(m_listCountLbl);

    listOuter->addWidget(m_listHeaderWidget);

    m_listContainer = new QWidget(m_listGlass);
    m_listContainer->setObjectName("playlistContainer");
    m_listContainer->setStyleSheet(QStringLiteral("QWidget#playlistContainer { background: transparent; }"));
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(0);
    m_listLayout->setAlignment(Qt::AlignTop);

    listOuter->addWidget(m_listContainer, 1);

    contentLay->addWidget(m_listGlass, 1);
    contentLay->addStretch();

    m_scroll->setWidget(m_contentWidget);
    mainLay->addWidget(m_scroll, 1);

    applyPlaylistDetailStyle();
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
        qDebug() << "[PlaylistDetailPage] fetchPlaylistDetail success:" << success << "detail:" << detail;
        if (success) {
            m_playlistName = detail.value("name").toString();
            m_playlistDesc = detail.value("description").toString();
            // 获取firstMusicId用于封面
            m_firstMusicId = detail.value("firstMusicId").toInt();
            // 获取创建者信息
            auto creatorObj = detail.value("creator").toMap();
            m_creatorId = creatorObj.value("id").toInt();
            m_creatorUsername = creatorObj.value("username").toString();
            qDebug() << "[PlaylistDetailPage] name:" << m_playlistName << "firstMusicId:" << m_firstMusicId
                     << "creatorId:" << m_creatorId << "creatorUsername:" << m_creatorUsername;
        } else {
            m_playlistName = QStringLiteral("歌单详情");
            m_playlistDesc = QString();
            m_firstMusicId = 0;
            m_creatorId = 0;
            m_creatorUsername = QString();
        }

        // 然后获取音乐列表
        m_apiClient->fetchPlaylistMusic(m_playlistId, [this](bool success, int total, const QList<QVariantMap> &musicList) {
            qDebug() << "[PlaylistDetailPage] fetchPlaylistMusic success:" << success << "total:" << total << "list size:" << musicList.size();
            m_musicList.clear();
            if (success) {
                for (const auto &m : musicList) {
                    qDebug() << "[PlaylistDetailPage] music item:" << m;
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
            // 音乐列表加载完成后再更新头部（此时才有正确的歌曲数量）
            updateHeader();
            buildList();
        });
    });
}

void PlaylistDetailPage::updateHeader()
{
    if (m_nameLbl) {
        m_nameLbl->setText(m_playlistName);
    }
    if (m_descLbl) {
        m_descLbl->setText(m_playlistDesc.isEmpty() ? I18n::instance().tr("description") : m_playlistDesc);
    }
    if (m_countLbl) {
        m_countLbl->setText(QString("%1 %2").arg(m_musicList.size()).arg(I18n::instance().tr("songs")));
    }
    if (m_listCountLbl) {
        m_listCountLbl->setText(QString("%1 %2").arg(m_musicList.size()).arg(I18n::instance().tr("songs")));
    }
    // 创建者信息
    if (m_creatorNameLbl) {
        m_creatorNameLbl->setText(m_creatorUsername);
    }
    if (m_creatorAvatarLbl && m_creatorId > 0) {
        QString avatarUrl = QString::fromUtf8("%1/api/user/avatar/%2").arg(Theme::kApiBase).arg(m_creatorId);
        QUrl url(avatarUrl);
        auto *nam = new QNetworkAccessManager(this);
        auto *reply = nam->get(QNetworkRequest(url));
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, nam]() {
            reply->deleteLater();
            nam->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QPixmap pix;
                pix.loadFromData(reply->readAll());
                if (!pix.isNull()) {
                    // 绘制圆形裁剪的头像
                    QPixmap circularPix(20, 20);
                    circularPix.fill(Qt::transparent);
                    QPainter p(&circularPix);
                    p.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addEllipse(0, 0, 20, 20);
                    p.setClipPath(path);
                    p.drawPixmap(0, 0, pix.scaled(20, 20, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                    m_creatorAvatarLbl->setPixmap(circularPix);
                }
            }
        });
    }
    // 加载封面
    if (m_coverLbl) {
        int coverId = m_firstMusicId > 0 ? m_firstMusicId : (m_musicList.isEmpty() ? 0 : m_musicList.first().id);
        QString coverUrl = QString::fromUtf8("%1/api/music/cover/%2").arg(Theme::kApiBase).arg(coverId);
        QUrl url(coverUrl);
        auto *nam = new QNetworkAccessManager(this);
        auto *reply = nam->get(QNetworkRequest(url));
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, nam]() {
            reply->deleteLater();
            nam->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QPixmap pix;
                pix.loadFromData(reply->readAll());
                if (!pix.isNull()) {
                    pix = pix.scaled(200, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    m_coverLbl->setPixmap(pix);
                } else {
                    setPlaceholderCover();
                }
            } else {
                setPlaceholderCover();
            }
        });
    }
}

void PlaylistDetailPage::setPlaceholderCover()
{
    if (!m_coverLbl) return;
    QPixmap pix(200, 200);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(0, 0, 200, 200, 8, 8);
    p.fillPath(path, QColor(196, 167, 231, 200));
    p.setClipPath(path);
    auto icon = Icons::render(Icons::kMusic, 60, QColor(255, 255, 255, 200));
    p.drawPixmap(70, 70, icon);
    m_coverLbl->setPixmap(pix);
}

void PlaylistDetailPage::buildList()
{
    QLayoutItem *lit;
    while ((lit = m_listLayout->takeAt(0)) != nullptr) {
        if (QWidget *w = lit->widget())
            w->deleteLater();
        delete lit;
    }
    m_musicItems.clear();

    if (m_musicList.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("noMusicInPlaylist"), m_listContainer);
        emptyLbl->setObjectName(QStringLiteral("playlistEmptyHint"));
        emptyLbl->setAlignment(Qt::AlignCenter);
        const bool dark = Theme::ThemeManager::instance().isDarkMode();
        const QString emptyCol = dark ? QString::fromUtf8(Theme::kTextSub)
                                      : QStringLiteral("rgba(33,37,41,0.55)");
        emptyLbl->setStyleSheet(QStringLiteral("QLabel { color: %1; font-size: 14px; padding: 60px 20px; }")
                                    .arg(emptyCol));
        m_listLayout->addWidget(emptyLbl);
        m_musicItems.append(emptyLbl);
    } else {
        for (int i = 0; i < m_musicList.size(); ++i) {
            const auto &info = m_musicList.at(i);
            auto *card = new PlaylistMusicCard(i, info, info.id, m_listContainer);
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
    m_listLayout->addStretch(1);
}

void PlaylistDetailPage::retranslate()
{
    if (m_typeLbl) {
        m_typeLbl->setText(I18n::instance().tr("playlists"));
    }
    if (m_nameLbl) {
        m_nameLbl->setText(m_playlistName);
    }
    if (m_descLbl) {
        m_descLbl->setText(m_playlistDesc.isEmpty() ? I18n::instance().tr("description") : m_playlistDesc);
    }
    if (m_listTitleLbl) {
        m_listTitleLbl->setText(I18n::instance().tr("songList"));
    }
}

void PlaylistDetailPage::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    GlassPaint::paintMainWindowDeepBackdrop(p, rect(), Theme::ThemeManager::instance().isDarkMode());
    QWidget::paintEvent(event);
}

bool PlaylistDetailPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_descLbl && event->type() == QEvent::MouseButtonDblClick) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            // 弹出输入对话框修改描述
            bool ok;
            QString newDesc = QInputDialog::getText(this, 
                I18n::instance().tr("editDescription"),
                I18n::instance().tr("enterNewDescription"),
                QLineEdit::Normal, 
                m_playlistDesc.isEmpty() ? "" : m_playlistDesc, 
                &ok);
            
            if (ok && !newDesc.isEmpty() && newDesc != m_playlistDesc) {
                // 调用API更新歌单（名称不变，只更新描述）
                m_apiClient->updatePlaylist(m_playlistId, m_playlistName, newDesc, [this, newDesc](bool success, const QString &message, const QVariantMap &data) {
                    if (success) {
                        m_playlistDesc = newDesc;
                        if (m_descLbl) {
                            m_descLbl->setText(newDesc);
                        }
                        // 刷新侧边栏歌单列表
                        emit refreshSidebarPlaylists();
                    } else {
                        qDebug() << "Failed to update playlist description:" << message;
                    }
                });
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
