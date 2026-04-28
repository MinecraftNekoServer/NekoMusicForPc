#ifndef PLAYLISTLISTITEM_H
#define PLAYLISTLISTITEM_H

#include <QWidget>
#include <QLabel>

class PlaylistListItem : public QWidget {
    Q_OBJECT

public:
    explicit PlaylistListItem(int localId, const QString& name, int musicCount, QWidget *parent = nullptr);

    int localId() const { return m_localId; }
    void setMusicCount(int count);

signals:
    void clicked(int localId);
    void renameRequested(int localId);
    void deleteRequested(int localId);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_localId;
    QString m_name;
    int m_musicCount;
    bool m_hovered = false;
};

#endif // PLAYLISTLISTITEM_H
