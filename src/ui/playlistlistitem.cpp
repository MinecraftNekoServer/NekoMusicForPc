#include "ui/playlistlistitem.h"

#include <QMouseEvent>
#include <QPainter>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>

PlaylistListItem::PlaylistListItem(int localId, const QString& name, int musicCount, QWidget *parent)
    : QWidget(parent), m_localId(localId), m_name(name), m_musicCount(musicCount)
{
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(40);
}

void PlaylistListItem::setMusicCount(int count) {
    m_musicCount = count;
    update();
}

void PlaylistListItem::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_localId);
    }
    QWidget::mousePressEvent(event);
}

void PlaylistListItem::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background-color: rgba(40, 40, 50, 0.95); border: 1px solid rgba(255, 255, 255, 0.1); border-radius: 8px; padding: 4px; }"
        "QMenu::item { color: #e0e0e0; padding: 8px 24px; border-radius: 4px; }"
        "QMenu::item:selected { background-color: rgba(255, 255, 255, 0.1); }"
    );

    QAction *renameAction = menu.addAction(QStringLiteral("重命名"));
    QAction *deleteAction = menu.addAction(QStringLiteral("删除"));

    QAction *selected = menu.exec(event->globalPos());
    if (selected == renameAction) {
        emit renameRequested(m_localId);
    } else if (selected == deleteAction) {
        emit deleteRequested(m_localId);
    }
}

void PlaylistListItem::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    if (m_hovered) {
        painter.fillRect(rect(), QColor(255, 255, 255, 15));
    }

    // Text
    QFont font = painter.font();
    font.setPointSize(12);
    painter.setFont(font);
    painter.setPen(QColor(0xe0, 0xe0, 0xe0));

    QRect textRect(16, 0, width() - 32, height());
    QString displayText = QString("%1 (%2)").arg(m_name).arg(m_musicCount);
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, displayText);
}

void PlaylistListItem::enterEvent(QEnterEvent *event) {
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void PlaylistListItem::leaveEvent(QEvent *event) {
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}
