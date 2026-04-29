/**
 * @file myplaylists.cpp
 * @brief 我的歌单页面实现
 */

#include "myplaylists.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "core/usermanager.h"
#include "theme/theme.h"
#include "ui/svgicon.h"
#include "ui/playlistcard.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QMenu>
#include <QCursor>
#include <QPainter>
#include <QPainterPath>

// ─── MyPlaylists ────────────────────────────────────────

MyPlaylists::MyPlaylists(ApiClient *apiClient, QWidget *parent)
    : QWidget(parent), m_apiClient(apiClient)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
}

void MyPlaylists::setupUi()
{
    auto *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // 顶部栏
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
    connect(backBtn, &QPushButton::clicked, this, &MyPlaylists::backRequested);
    headerLay->addWidget(backBtn);

    m_titleLabel = new QLabel(I18n::instance().tr("myPlaylistsTitle"), header);
    m_titleLabel->setStyleSheet(
        "QLabel { font-size: 18px; font-weight: 700; color: " + QString(Theme::kLavender) + "; }"
    );
    headerLay->addWidget(m_titleLabel);

    headerLay->addStretch();
    mainLay->addWidget(header);

    // 状态标签
    m_statusLabel = new QLabel(I18n::instance().tr("loading"), this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(
        "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 14px; padding: 40px; }"
    );
    mainLay->addWidget(m_statusLabel);

    // 网格滚动区
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setVisible(false);

    m_container = new QWidget(m_scroll);
    m_gridLayout = new QGridLayout(m_container);
    m_gridLayout->setContentsMargins(20, 20, 20, 20);
    m_gridLayout->setSpacing(16);
    m_gridLayout->setColumnStretch(0, 1);
    m_gridLayout->setColumnStretch(1, 1);
    m_gridLayout->setColumnStretch(2, 1);
    m_gridLayout->setColumnStretch(3, 1);

    m_scroll->setWidget(m_container);
    mainLay->addWidget(m_scroll, 1);

    loadMyPlaylists();
}

void MyPlaylists::refresh()
{
    loadMyPlaylists();
}

void MyPlaylists::loadMyPlaylists()
{
    // 清除现有卡片
    while (QLayoutItem *item = m_gridLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }

    if (!UserManager::instance().isLoggedIn()) {
        m_statusLabel->setText(I18n::instance().tr("pleaseLoginFirst"));
        m_statusLabel->show();
        m_scroll->setVisible(false);
        return;
    }

    m_statusLabel->setText(I18n::instance().tr("loading"));
    m_statusLabel->show();

    m_apiClient->fetchUserPlaylists([this](bool success, const QList<QVariantMap> &playlists) {
        m_statusLabel->hide();

        if (!success || playlists.isEmpty()) {
            m_statusLabel->setText(I18n::instance().tr("noPlaylists"));
            m_statusLabel->show();
            m_scroll->setVisible(false);
            return;
        }

        m_playlists.clear();
        int row = 0, col = 0;
        const int cols = 4;

        for (const auto &pl : playlists) {
            PlaylistInfo info;
            info.id = pl.value("id").toInt();
            info.name = pl.value("name").toString();
            info.description = pl.value("description").toString();
            info.musicCount = pl.value("musicCount").toInt();
            info.coverUrl = QString::fromUtf8("%1/api/music/cover/%2")
                                .arg(Theme::kApiBase).arg(info.id);
            m_playlists.append(info);

            auto *card = new PlaylistCard(info, m_container);
            connect(card, &PlaylistCard::clicked, this, [this, info]() {
                emit playlistClicked(info.id);
            });
            // 右键菜单：用事件过滤器
            card->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(card, &QWidget::customContextMenuRequested, this, [this, info, card]() {
                QMenu menu(this);
                menu.setStyleSheet(
                    "QMenu { background-color: rgba(40, 40, 50, 0.95); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 8px; padding: 4px; }"
                    "QMenu::item { color: #e0e0e0; padding: 8px 24px; border-radius: 4px; }"
                    "QMenu::item:selected { background-color: rgba(255, 255, 255, 0.1); }"
                );
                QAction *editAction = menu.addAction(I18n::instance().tr("edit"));
                QAction *deleteAction = menu.addAction(I18n::instance().tr("delete"));
                QAction *selected = menu.exec(QCursor::pos());
                if (selected == editAction) {
                    editPlaylist(info.id, info.name, info.description);
                } else if (selected == deleteAction) {
                    deletePlaylist(info.id, info.name);
                }
            });
            m_gridLayout->addWidget(card, row, col);

            col++;
            if (col >= cols) {
                col = 0;
                row++;
            }
        }

        m_gridLayout->setRowStretch(row + 1, 1);
        m_scroll->setVisible(true);
    });
}

void MyPlaylists::createPlaylist()
{
    bool ok;
    QString name = QInputDialog::getText(this,
        I18n::instance().tr("createPlaylist"),
        I18n::instance().tr("inputPlaylistName"),
        QLineEdit::Normal,
        QString(),
        &ok);

    if (!ok || name.trimmed().isEmpty()) return;

    QString desc = QInputDialog::getText(this,
        I18n::instance().tr("createPlaylist"),
        I18n::instance().tr("playlistDescPlaceholder"),
        QLineEdit::Normal,
        QString(),
        &ok);

    if (!ok) return;

    m_apiClient->createPlaylist(name.trimmed(), desc.trimmed(), [this](bool success, const QString &message, const QVariantMap &data) {
        if (success) {
            refresh();
        }
    });
}

void MyPlaylists::editPlaylist(int playlistId, const QString &currentName, const QString &currentDesc)
{
    bool ok;
    QString name = QInputDialog::getText(this,
        I18n::instance().tr("editPlaylist"),
        I18n::instance().tr("playlistNamePlaceholder"),
        QLineEdit::Normal,
        currentName,
        &ok);

    if (!ok || name.trimmed().isEmpty()) return;

    QString desc = QInputDialog::getText(this,
        I18n::instance().tr("editPlaylist"),
        I18n::instance().tr("playlistDescPlaceholder"),
        QLineEdit::Normal,
        currentDesc,
        &ok);

    if (!ok) return;

    m_apiClient->updatePlaylist(playlistId, name.trimmed(), desc.trimmed(), [this](bool success, const QString &, const QVariantMap &) {
        if (success) {
            refresh();
        }
    });
}

void MyPlaylists::deletePlaylist(int playlistId, const QString &playlistName)
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        I18n::instance().tr("deletePlaylist"),
        I18n::instance().tr("deleteConfirmWithPlaylistName").replace("{name}", playlistName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    m_apiClient->deletePlaylist(playlistId, [this](bool success, const QString &) {
        if (success) {
            refresh();
        }
    });
}

void MyPlaylists::retranslate()
{
    m_titleLabel->setText(I18n::instance().tr("myPlaylistsTitle"));
}

void MyPlaylists::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    // 透明背景
}
