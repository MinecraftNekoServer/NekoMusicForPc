/**
 * @file sidebar.cpp
 * @brief 侧边栏实现
 *
 * 240px 重度毛玻璃，深紫黑半透 + 薰衣草微光。
 * 选中态：薰衣草背景 + 左侧薄荷绿竖条。
 */

#include "sidebar.h"
#include "theme/theme.h"
#include "ui/playlistlistitem.h"
#include "core/i18n.h"
#include "core/usermanager.h"
#include "core/apiclient.h"

#include <QInputDialog>
#include <QLineEdit>

#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QStyle>

namespace {
QString navIconName(const QString &key) {
    if (key == "favorites") return "heart";
    if (key == "recent") return "clock";
    if (key == "upload") return "upload";
    if (key == "settings") return "settings";
    return key; // home, etc.
}
QString navIconPath(const QString &key, bool active) {
    QString name = navIconName(key);
    return active ? QString(":/icons/nav_%1_active.png").arg(name) : QString(":/icons/nav_%1.png").arg(name);
}
}

Sidebar::Sidebar(ApiClient *apiClient, QWidget *parent) : QWidget(parent), m_apiClient(apiClient)
{
    setFixedWidth(Theme::kSidebarW);
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
    setActiveNav("home");
    // Upload nav only visible when logged in
    setUploadVisible(UserManager::instance().isLoggedIn());
    loadPlaylists();
}

void Sidebar::setupUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setObjectName("sbScroll");

    auto *container = new QWidget(scroll);
    container->setObjectName("sbContainer");
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(10, 12, 10, 12);
    lay->setSpacing(8);

    // 主导航（带 PNG 图标）
    lay->addWidget(createNavItem("home", I18n::instance().tr("home"),
                                 QIcon(":/icons/nav_home_active.png")));

    // 我喜欢的（可点击导航）
    m_favBtn = createNavItem("favorites", I18n::instance().tr("favorites"),
                             QIcon(":/icons/nav_heart.png"));
    lay->addWidget(m_favBtn);

    // 最近播放（可点击导航）
    m_recBtn = createNavItem("recent", I18n::instance().tr("recentPlay"),
                             QIcon(":/icons/nav_clock.png"));
    lay->addWidget(m_recBtn);

    // 上传音乐（可点击导航）
    m_uploadBtn = createNavItem("upload", I18n::instance().tr("uploadMusic"),
                                QIcon(":/icons/nav_upload.png"));
    lay->addWidget(m_uploadBtn);

    // 分隔线
    auto *div = new QWidget(container);
    div->setObjectName("sbDivider");
    div->setFixedHeight(1);
    lay->addWidget(div);

    // 歌单区域标题
    auto *plHeader = new QLabel(I18n::instance().tr("myPlaylistsTitle"), container);
    plHeader->setObjectName("sbPlaylistTitle");
    lay->addWidget(plHeader);

    // 播放列表容器
    m_playlistContainer = new QWidget(container);
    m_playlistLayout = new QVBoxLayout(m_playlistContainer);
    m_playlistLayout->setContentsMargins(4, 0, 4, 0);
    m_playlistLayout->setSpacing(2);
    lay->addWidget(m_playlistContainer);

    // 创建歌单按钮
    m_createPlaylistBtn = new QPushButton(I18n::instance().tr("createPlaylist"), container);
    m_createPlaylistBtn->setObjectName("sbCreatePlaylist");
    m_createPlaylistBtn->setFixedHeight(36);
    m_createPlaylistBtn->setCursor(Qt::PointingHandCursor);
    connect(m_createPlaylistBtn, &QPushButton::clicked, this, [this]() {
        emit playlistCreateRequested();
    });
    lay->addWidget(m_createPlaylistBtn);

    lay->addStretch();

    scroll->setWidget(container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}

void Sidebar::refreshPlaylists()
{
    loadPlaylists();
}

void Sidebar::loadPlaylists()
{
    if (!m_apiClient || !UserManager::instance().isLoggedIn()) {
        // Not logged in or no API client, clear the list
        m_apiPlaylists.clear();
        refreshPlaylistList();
        return;
    }

    m_apiClient->fetchUserPlaylists([this](bool success, const QList<QVariantMap> &playlists) {
        if (success) {
            m_apiPlaylists.clear();
            for (const auto &pl : playlists) {
                ApiPlaylistInfo info;
                info.id = pl.value("id").toInt();
                info.name = pl.value("name").toString();
                info.description = pl.value("description").toString();
                info.musicCount = pl.value("musicCount").toInt();
                m_apiPlaylists.append(info);
            }
            qDebug() << "[歌单] 共加载" << m_apiPlaylists.size() << "个歌单";

            // 为每个歌单获取封面（通过获取歌单内第一首音乐的封面，空歌单用musicId=0）
            for (int i = 0; i < m_apiPlaylists.size(); ++i) {
                int playlistId = m_apiPlaylists[i].id;
                qDebug() << "[歌单封面] 开始请求歌单封面, playlistId =" << playlistId;
                m_apiClient->fetchPlaylistMusic(playlistId, [this, playlistId](bool ok, int total, const QList<QVariantMap> &musicList) {
                    qDebug() << "[歌单封面] 歌单音乐回调, playlistId =" << playlistId << ", ok =" << ok << ", total =" << total << ", size =" << musicList.size();
                    int firstMusicId = 0;
                    if (ok && !musicList.isEmpty()) {
                        firstMusicId = musicList.first().value("id").toInt();
                    }
                    QString coverUrl = QString::fromUtf8("%1/api/music/cover/%2").arg(Theme::kApiBase).arg(firstMusicId);
                    // 在缓存中找到对应的歌单并更新封面
                    for (auto &info : m_apiPlaylists) {
                        if (info.id == playlistId) {
                            info.coverUrl = coverUrl;
                            qDebug() << "[歌单封面] id =" << playlistId << ", musicId =" << firstMusicId << ", coverUrl =" << coverUrl;
                            break;
                        }
                    }
                    refreshPlaylistList();
                });
            }
        } else {
            m_apiPlaylists.clear();
            qDebug() << "[歌单] 加载失败";
        }
        refreshPlaylistList();
    });
}

void Sidebar::refreshPlaylistList()
{
    // 清除现有项
    for (auto *item : m_playlistItems) {
        if (item) {
            m_playlistLayout->removeWidget(item);
            item->deleteLater();
        }
    }
    m_playlistItems.clear();

    if (m_apiPlaylists.isEmpty()) {
        auto *empty = new QLabel(I18n::instance().tr("noPlaylists"), m_playlistContainer);
        empty->setObjectName("sbEmptyPlaylist");
        empty->setAlignment(Qt::AlignCenter);
        empty->setWordWrap(true);
        m_playlistLayout->addWidget(empty);
    } else {
        for (const auto &pl : m_apiPlaylists) {
            auto *item = new PlaylistListItem(pl.id, pl.name, pl.musicCount, pl.coverUrl, m_playlistContainer);
            connect(item, &PlaylistListItem::clicked, this, [this, playlistId = pl.id]() {
                emit playlistClicked(playlistId);
            });
            connect(item, &PlaylistListItem::renameRequested, this, [this, playlistId = pl.id]() {
                if (!m_apiClient) return;
                // 找到当前歌单信息
                for (const auto &pl : m_apiPlaylists) {
                    if (pl.id == playlistId) {
                        // 弹出重命名对话框
                        bool ok;
                        QString newName = QInputDialog::getText(this,
                            I18n::instance().tr("renamePlaylist"),
                            I18n::instance().tr("inputNewPlaylistName"),
                            QLineEdit::Normal,
                            pl.name,
                            &ok);
                        if (ok && !newName.isEmpty() && newName != pl.name) {
                            // 调用API更新歌单名称
                            m_apiClient->updatePlaylist(playlistId, newName, pl.description, [this, playlistId, newName](bool success, const QString &, const QVariantMap &) {
                                if (success) {
                                    // 更新缓存
                                    for (auto &p : m_apiPlaylists) {
                                        if (p.id == playlistId) {
                                            p.name = newName;
                                            break;
                                        }
                                    }
                                    refreshPlaylistList();
                                }
                            });
                        }
                        break;
                    }
                }
            });
            connect(item, &PlaylistListItem::editDescriptionRequested, this, [this, playlistId = pl.id]() {
                if (!m_apiClient) return;
                // 找到当前歌单信息
                for (const auto &pl : m_apiPlaylists) {
                    if (pl.id == playlistId) {
                        // 弹出修改描述对话框
                        bool ok;
                        QString newDesc = QInputDialog::getText(this,
                            I18n::instance().tr("modifyPlaylistDesc"),
                            I18n::instance().tr("inputPlaylistDesc"),
                            QLineEdit::Normal,
                            pl.description,
                            &ok);
                        if (ok && newDesc != pl.description) {
                            // 调用API更新歌单描述
                            m_apiClient->updatePlaylist(playlistId, pl.name, newDesc, [this, playlistId, newDesc](bool success, const QString &, const QVariantMap &) {
                                if (success) {
                                    // 更新缓存
                                    for (auto &p : m_apiPlaylists) {
                                        if (p.id == playlistId) {
                                            p.description = newDesc;
                                            break;
                                        }
                                    }
                                    refreshPlaylistList();
                                }
                            });
                        }
                        break;
                    }
                }
            });
            connect(item, &PlaylistListItem::deleteRequested, this, [this, playlistId = pl.id]() {
                if (!m_apiClient) return;
                m_apiClient->deletePlaylist(playlistId, [this, playlistId](bool success, const QString &) {
                    if (success) {
                        // 从缓存中移除
                        for (int i = 0; i < m_apiPlaylists.size(); ++i) {
                            if (m_apiPlaylists[i].id == playlistId) {
                                m_apiPlaylists.removeAt(i);
                                break;
                            }
                        }
                        refreshPlaylistList();
                    }
                });
            });
            m_playlistLayout->addWidget(item);
            m_playlistItems.append(item);
        }
    }
}

QPushButton *Sidebar::createNavItem(const QString &key, const QString &label, const QIcon &icon)
{
    auto *btn = new QPushButton(label, this);
    btn->setObjectName("sbNavItem");
    btn->setFixedHeight(42);
    btn->setIcon(icon);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setProperty("navKey", key);
    connect(btn, &QPushButton::clicked, this, [this, key]() {
        setActiveNav(key);
        emit navigationRequested(key);
    });
    m_navBtns[key] = btn;
    return btn;
}

void Sidebar::setActiveNav(const QString &key)
{
    m_activeKey = key;
    for (auto it = m_navBtns.constBegin(); it != m_navBtns.constEnd(); ++it) {
        bool active = (it.key() == key);
        it.value()->setProperty("active", active);
        it.value()->style()->unpolish(it.value());
        it.value()->style()->polish(it.value());
        // 更新图标
        it.value()->setIcon(QIcon(navIconPath(it.key(), active)));
    }
}

void Sidebar::retranslate()
{
    auto *homeBtn = m_navBtns.value("home");
    if (homeBtn) homeBtn->setText(I18n::instance().tr("home"));

    if (m_favBtn) m_favBtn->setText(I18n::instance().tr("favorites"));
    if (m_recBtn) m_recBtn->setText(I18n::instance().tr("recentPlay"));
    if (m_uploadBtn) m_uploadBtn->setText(I18n::instance().tr("uploadMusic"));

    auto *plHeader = findChild<QLabel *>("sbPlaylistTitle");
    if (plHeader) plHeader->setText(I18n::instance().tr("myPlaylistsTitle"));

    if (m_createPlaylistBtn) m_createPlaylistBtn->setText(I18n::instance().tr("createPlaylist"));
}

void Sidebar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 重度毛玻璃背景
    QColor bg(36, 31, 49, 204);  // 80% 不透明
    p.fillRect(rect(), bg);

    // 薰衣草微光叠加（从左上角辐射）
    QRadialGradient glow(QPointF(0, 0), width() * 1.2);
    glow.setColorAt(0.0, QColor(196, 167, 231, 18));  // 7%
    glow.setColorAt(1.0, QColor(196, 167, 231, 0));
    p.fillRect(rect(), glow);

    // 右侧边线
    p.setPen(QPen(QColor(196, 167, 231, 25), 1));
    p.drawLine(rect().topRight(), rect().bottomRight());
}

void Sidebar::setUploadVisible(bool visible)
{
    if (m_uploadBtn) {
        m_uploadBtn->setVisible(visible);
    }
}
