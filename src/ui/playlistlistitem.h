#ifndef PLAYLISTLISTITEM_H
#define PLAYLISTLISTITEM_H

#include <QWidget>
#include <QLabel>

class PlaylistListItem : public QWidget {
    Q_OBJECT

public:
    explicit PlaylistListItem(int playlistId, const QString& name, int musicCount, const QString& coverUrl, QWidget *parent = nullptr);

    int playlistId() const { return m_playlistId; }
    void setMusicCount(int count);

signals:
    void clicked(int playlistId);
    void renameRequested(int playlistId);
    void deleteRequested(int playlistId);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_playlistId;
    QString m_name;
    int m_musicCount;
    bool m_hovered = false;
    QLabel *m_coverLbl = nullptr;
    QLabel *m_nameLbl = nullptr;
    void setPlaceholderCover();
};

#endif // PLAYLISTLISTITEM_H
