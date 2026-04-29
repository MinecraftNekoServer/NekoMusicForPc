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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFrame>
#include <QStyle>
#include <QDebug>
#include <QInputDialog>
#include <QLineEdit>

// ─── 播放列表音乐项卡片 ──────────────────────────────────────
class PlaylistMusicCard : public QWidget
{
public:
    explicit PlaylistMusicCard(int index, const MusicInfo &info, int musicId, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(musicId), m_index(index), m_info(info)
    {
        setFixedHeight(72);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet(
            "PlaylistMusicCard { "
            "  background: transparent; "
            "  border-bottom: 1px solid rgba(255, 255, 255, 0.1); "
            "}"
            "PlaylistMusicCard:hover { "
            "  background: rgba(102, 126, 234, 0.1); "
            "}"
            "PlaylistMusicCard.playing { "
            "  background: rgba(102, 126, 234, 0.15); "
            "}"
        );

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(20, 12, 20, 12);
        lay->setSpacing(16);

        // 序号
        m_indexLbl = new QLabel(QString::number(index + 1), this);
        m_indexLbl->setFixedSize(32, 72);
        m_indexLbl->setAlignment(Qt::AlignCenter);
        m_indexLbl->setStyleSheet("QLabel { font-size: 14px; color: rgba(255, 255, 255, 0.5); }");
        lay->addWidget(m_indexLbl);

        // 封面
        m_coverLbl = new QLabel(this);
        m_coverLbl->setFixedSize(48, 48);
        m_coverLbl->setScaledContents(false);
        loadCover();
        lay->addWidget(m_coverLbl);

        // 信息
        auto *infoV = new QWidget(this);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(4);

        m_titleLbl = new QLabel(info.title, infoV);
        m_titleLbl->setObjectName("musicTitle");
        m_titleLbl->setStyleSheet(
            "QLabel#musicTitle { "
            "  font-size: 14px; font-weight: 500; color: white; "
            "  padding: 0; margin: 0; "
            "}"
        );
        infoLay->addWidget(m_titleLbl);

        m_artistLbl = new QLabel(info.artist, infoV);
        m_artistLbl->setObjectName("musicArtist");
        m_artistLbl->setStyleSheet(
            "QLabel#musicArtist { "
            "  font-size: 12px; color: rgba(255, 255, 255, 0.6); "
            "  padding: 0; margin: 0; "
            "}"
        );
        infoLay->addWidget(m_artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        // 时长
        int mins = info.duration / 60;
        int secs = info.duration % 60;
        auto *timeLbl = new QLabel(
            QString("%1:%2").arg(mins, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0')), this);
        timeLbl->setFixedWidth(60);
        timeLbl->setAlignment(Qt::AlignCenter);
        timeLbl->setStyleSheet("QLabel { font-size: 13px; color: rgba(255, 255, 255, 0.6); }");
        lay->addWidget(timeLbl);
    }

    int musicId() const { return m_musicId; }
    void setPlaying(bool playing) {
        m_isPlaying = playing;
        setProperty("playing", playing);
        style()->unpolish(this);
        style()->polish(this);
        if (playing) {
            m_indexLbl->setText(QStringLiteral("▶"));
            m_indexLbl->setStyleSheet("QLabel { font-size: 14px; color: #667eea; }");
        } else {
            m_indexLbl->setText(QString::number(m_index + 1));
            m_indexLbl->setStyleSheet("QLabel { font-size: 14px; color: rgba(255, 255, 255, 0.5); }");
        }
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
            "QMenu::item:selected { background-color: rgba(102, 126, 234, 0.15); }"
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
        p.fillPath(path, QColor(102, 126, 234, 180));
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

    // 滚动区域
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setObjectName("playlistScroll");
    m_scroll->setStyleSheet(
        "QScrollArea#playlistScroll { border: none; background: transparent; }"
    );

    m_contentWidget = new QWidget(m_scroll);
    m_contentWidget->setObjectName("playlistContent");
    m_contentWidget->setStyleSheet("QWidget#playlistContent { background: transparent; }");

    auto *contentLay = new QVBoxLayout(m_contentWidget);
    contentLay->setContentsMargins(24, 24, 24, 24);
    contentLay->setSpacing(24);

    // 歌单头部：封面 + 信息
    m_headerWidget = new QWidget(m_contentWidget);
    m_headerWidget->setObjectName("playlistHeader");
    m_headerWidget->setStyleSheet(
        "QWidget#playlistHeader { "
        "  background: rgba(30, 30, 50, 0.9); "
        "  border: 1px solid rgba(255, 255, 255, 0.1); "
        "  border-radius: 12px; "
        "}"
    );
    auto *headerLay = new QHBoxLayout(m_headerWidget);
    headerLay->setContentsMargins(24, 24, 24, 24);
    headerLay->setSpacing(24);

    // 封面 200x200
    m_coverLbl = new QLabel(m_headerWidget);
    m_coverLbl->setFixedSize(200, 200);
    m_coverLbl->setScaledContents(false);
    m_coverLbl->setStyleSheet(
        "QLabel { border-radius: 8px; }"
    );
    headerLay->addWidget(m_coverLbl);

    // 信息区
    auto *infoWidget = new QWidget(m_headerWidget);
    auto *infoLay = new QVBoxLayout(infoWidget);
    infoLay->setContentsMargins(0, 0, 0, 0);
    infoLay->setSpacing(12);

    m_typeLbl = new QLabel(I18n::instance().tr("playlists"), infoWidget);
    m_typeLbl->setObjectName("playlistType");
    m_typeLbl->setStyleSheet(
        "QLabel#playlistType { "
        "  font-size: 12px; color: rgba(255, 255, 255, 0.6); "
        "  letter-spacing: 1px; text-transform: uppercase; "
        "}"
    );
    infoLay->addWidget(m_typeLbl);

    m_nameLbl = new QLabel(infoWidget);
    m_nameLbl->setObjectName("playlistName");
    m_nameLbl->setStyleSheet(
        "QLabel#playlistName { "
        "  font-size: 28px; font-weight: 700; color: white; "
        "  line-height: 1.3; "
        "}"
    );
    m_nameLbl->setWordWrap(true);
    infoLay->addWidget(m_nameLbl);

    m_descLbl = new QLabel(I18n::instance().tr("description"), infoWidget);
    m_descLbl->setObjectName("playlistDesc");
    m_descLbl->setStyleSheet(
        "QLabel#playlistDesc { "
        "  font-size: 14px; color: rgba(255, 255, 255, 0.7); "
        "  line-height: 1.6; "
        "  padding: 4px; "
        "  border-radius: 4px; "
        "}"
        "QLabel#playlistDesc:hover { "
        "  background-color: rgba(255, 255, 255, 0.1); "
        "}"
    );
    m_descLbl->setWordWrap(true);
    m_descLbl->setCursor(Qt::PointingHandCursor);
    m_descLbl->installEventFilter(this);
    infoLay->addWidget(m_descLbl);

    // 创建者信息：头像 + 昵称
    auto *creatorWidget = new QWidget(infoWidget);
    auto *creatorLay = new QHBoxLayout(creatorWidget);
    creatorLay->setContentsMargins(0, 0, 0, 0);
    creatorLay->setSpacing(8);

    m_creatorAvatarLbl = new QLabel(creatorWidget);
    m_creatorAvatarLbl->setFixedSize(20, 20);
    m_creatorAvatarLbl->setScaledContents(true);
    m_creatorAvatarLbl->setStyleSheet(
        "QLabel { border-radius: 10px; background: rgba(102, 126, 234, 0.3); }"
    );
    creatorLay->addWidget(m_creatorAvatarLbl);

    m_creatorNameLbl = new QLabel(creatorWidget);
    m_creatorNameLbl->setObjectName("creatorName");
    m_creatorNameLbl->setStyleSheet(
        "QLabel#creatorName { "
        "  font-size: 12px; color: rgba(255, 255, 255, 0.8); "
        "  font-weight: 500; "
        "}"
    );
    creatorLay->addWidget(m_creatorNameLbl);
    creatorLay->addStretch();

    infoLay->addWidget(creatorWidget);

    m_countLbl = new QLabel(infoWidget);
    m_countLbl->setObjectName("playlistCount");
    m_countLbl->setStyleSheet(
        "QLabel#playlistCount { "
        "  font-size: 13px; color: rgba(255, 255, 255, 0.6); "
        "}"
    );
    infoLay->addWidget(m_countLbl);

    infoLay->addStretch();
    headerLay->addWidget(infoWidget, 1);

    contentLay->addWidget(m_headerWidget);

    // 音乐列表头部
    m_listHeaderWidget = new QWidget(m_contentWidget);
    m_listHeaderWidget->setObjectName("listHeader");
    m_listHeaderWidget->setStyleSheet(
        "QWidget#listHeader { "
        "  background: rgba(40, 40, 65, 0.9); "
        "  border-bottom: 1px solid rgba(255, 255, 255, 0.1); "
        "}"
    );
    auto *listHeaderLay = new QHBoxLayout(m_listHeaderWidget);
    listHeaderLay->setContentsMargins(20, 16, 20, 16);

    m_listTitleLbl = new QLabel(I18n::instance().tr("songList"), m_listHeaderWidget);
    m_listTitleLbl->setObjectName("listTitle");
    m_listTitleLbl->setStyleSheet(
        "QLabel#listTitle { "
        "  font-size: 15px; font-weight: 600; color: white; "
        "}"
    );
    listHeaderLay->addWidget(m_listTitleLbl);

    listHeaderLay->addStretch();

    m_listCountLbl = new QLabel(m_listHeaderWidget);
    m_listCountLbl->setObjectName("listCount");
    m_listCountLbl->setStyleSheet(
        "QLabel#listCount { "
        "  font-size: 13px; color: rgba(255, 255, 255, 0.6); "
        "}"
    );
    listHeaderLay->addWidget(m_listCountLbl);

    contentLay->addWidget(m_listHeaderWidget);

    // 音乐列表容器
    m_listContainer = new QWidget(m_contentWidget);
    m_listContainer->setObjectName("playlistContainer");
    m_listContainer->setStyleSheet("QWidget#playlistContainer { background: transparent; }");
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(0, 0, 0, 0);
    m_listLayout->setSpacing(0);

    contentLay->addWidget(m_listContainer);
    contentLay->addStretch();

    m_scroll->setWidget(m_contentWidget);
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
    p.fillPath(path, QColor(102, 126, 234, 180));
    p.setClipPath(path);
    auto icon = Icons::render(Icons::kMusic, 60, QColor(255, 255, 255, 200));
    p.drawPixmap(70, 70, icon);
    m_coverLbl->setPixmap(pix);
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
            "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 60px 20px; }"
        );
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
    QWidget::paintEvent(event);
    // 透明背景，由父窗口渐变透出
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
