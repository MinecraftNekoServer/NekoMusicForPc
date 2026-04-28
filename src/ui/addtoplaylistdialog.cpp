#include "addtoplaylistdialog.h"
#include "core/playlistdb.h"
#include "core/i18n.h"
#include "theme/theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>

AddToPlaylistDialog::AddToPlaylistDialog(const MusicInfo& music, QWidget *parent)
    : QDialog(parent), m_music(music)
{
    setWindowTitle(I18n::instance().tr("addToPlaylist"));
    setFixedSize(360, 420);
    setStyleSheet(
        "QDialog { background-color: rgba(36, 31, 49, 0.98); }"
        "QListWidget { background-color: rgba(45, 38, 65, 100); border: 1px solid rgba(196, 167, 231, 0.2); border-radius: 8px; color: #e0e0e0; font-size: 13px; }"
        "QListWidget::item { padding: 10px; border-radius: 4px; }"
        "QListWidget::item:selected { background-color: rgba(196, 167, 231, 0.3); }"
        "QListWidget::item:hover { background-color: rgba(196, 167, 231, 0.15); }"
        "QPushButton { background-color: rgba(196, 167, 231, 0.2); border: 1px solid rgba(196, 167, 231, 0.3); border-radius: 8px; color: #e0e0e0; font-size: 13px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: rgba(196, 167, 231, 0.4); }"
    );
    setupUi();
    loadPlaylists();
}

void AddToPlaylistDialog::setupUi()
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(20, 20, 20, 20);
    lay->setSpacing(12);

    // 提示文本
    auto *hintLbl = new QLabel(I18n::instance().tr("selectPlaylist"), this);
    hintLbl->setStyleSheet("QLabel { color: #b0a0c0; font-size: 13px; }");
    lay->addWidget(hintLbl);

    // 播放列表列表
    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    lay->addWidget(m_listWidget);

    // 按钮
    auto *btnLay = new QHBoxLayout();
    btnLay->setSpacing(8);

    m_createBtn = new QPushButton(I18n::instance().tr("createPlaylist"), this);
    connect(m_createBtn, &QPushButton::clicked, this, &AddToPlaylistDialog::createNewPlaylist);
    btnLay->addWidget(m_createBtn);

    btnLay->addStretch();

    auto *cancelBtn = new QPushButton(I18n::instance().tr("cancel"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLay->addWidget(cancelBtn);

    m_okBtn = new QPushButton(I18n::instance().tr("ok"), this);
    m_okBtn->setStyleSheet(
        "QPushButton { background-color: rgba(196, 167, 231, 0.4); border: 1px solid rgba(196, 167, 231, 0.5); border-radius: 8px; color: #ffffff; font-size: 13px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: rgba(196, 167, 231, 0.6); }"
    );
    connect(m_okBtn, &QPushButton::clicked, this, [this]() {
        auto *selected = m_listWidget->currentItem();
        if (selected) {
            int playlistId = selected->data(Qt::UserRole).toInt();
            addMusicToPlaylist(playlistId);
        }
    });
    btnLay->addWidget(m_okBtn);

    lay->addLayout(btnLay);
}

void AddToPlaylistDialog::loadPlaylists()
{
    m_listWidget->clear();
    auto playlists = PlaylistDatabase::instance().getAllPlaylists();

    for (const auto &pl : playlists) {
        auto *item = new QListWidgetItem(pl.name, m_listWidget);
        item->setData(Qt::UserRole, pl.localId);
        m_listWidget->addItem(item);
    }
}

void AddToPlaylistDialog::createNewPlaylist()
{
    bool ok = false;
    QString name = QInputDialog::getText(this,
        I18n::instance().tr("createPlaylist"),
        I18n::instance().tr("playlistName"),
        QLineEdit::Normal,
        I18n::instance().tr("newPlaylist"),
        &ok);

    if (ok && !name.isEmpty()) {
        int playlistId = PlaylistDatabase::instance().createPlaylist(name);
        if (playlistId > 0) {
            addMusicToPlaylist(playlistId);
        }
    }
}

void AddToPlaylistDialog::addMusicToPlaylist(int playlistId)
{
    if (PlaylistDatabase::instance().addMusic(playlistId, m_music)) {
        QMessageBox::information(this,
            I18n::instance().tr("success"),
            I18n::instance().tr("musicAdded"));
        accept();
    } else {
        QMessageBox::warning(this,
            I18n::instance().tr("error"),
            I18n::instance().tr("musicAddFailed"));
    }
}
