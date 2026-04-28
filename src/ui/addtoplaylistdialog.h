#ifndef ADDTOPLAYLISTDIALOG_H
#define ADDTOPLAYLISTDIALOG_H

#include <QDialog>
#include "core/musicinfo.h"

class QListWidget;
class QVBoxLayout;
class QPushButton;

class AddToPlaylistDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddToPlaylistDialog(const MusicInfo& music, QWidget *parent = nullptr);

private:
    void setupUi();
    void loadPlaylists();
    void createNewPlaylist();
    void addMusicToPlaylist(int playlistId);

    MusicInfo m_music;
    QListWidget *m_listWidget = nullptr;
    QPushButton *m_createBtn = nullptr;
    QPushButton *m_okBtn = nullptr;
};

#endif // ADDTOPLAYLISTDIALOG_H
