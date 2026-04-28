#include "ui/playlistlistitem.h"

#include <QMouseEvent>
#include <QPainter>
#include <QMenu>
#include <QHBoxLayout>
#include <QPainterPath>
#include <QNetworkAccessManager>
#include <QNetworkReply>

PlaylistListItem::PlaylistListItem(int playlistId, const QString& name, int musicCount, const QString& coverUrl, QWidget *parent)
    : QWidget(parent), m_playlistId(playlistId), m_name(name), m_musicCount(musicCount)
{
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(56);  // padding 10+36+10 = 56

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(16, 10, 16, 10);
    lay->setSpacing(10);

    // Cover
    m_coverLbl = new QLabel(this);
    m_coverLbl->setFixedSize(36, 36);
    m_coverLbl->setScaledContents(false);
    lay->addWidget(m_coverLbl);

    if (!coverUrl.isEmpty()) {
        QUrl url(coverUrl.startsWith("http") ? coverUrl : QString::fromUtf8("https://music.cnmsb.xin%1").arg(coverUrl));
        auto *nam = new QNetworkAccessManager(this);
        auto *reply = nam->get(QNetworkRequest(url));
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, nam]() {
            reply->deleteLater();
            nam->deleteLater();
            if (reply->error() == QNetworkReply::NoError) {
                QPixmap pix;
                pix.loadFromData(reply->readAll());
                if (!pix.isNull()) {
                    pix = pix.scaled(36, 36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                    m_coverLbl->setPixmap(pix);
                } else {
                    setPlaceholderCover();
                }
            } else {
                setPlaceholderCover();
            }
        });
    } else {
        setPlaceholderCover();
    }

    // Name
    m_nameLbl = new QLabel(this);
    m_nameLbl->setText(m_name);
    m_nameLbl->setStyleSheet("QLabel { font-size: 13px; color: rgba(255, 255, 255, 0.7); }");
    m_nameLbl->setAlignment(Qt::AlignVCenter);
    m_nameLbl->setWordWrap(false);
    lay->addWidget(m_nameLbl, 1);
}

void PlaylistListItem::setMusicCount(int count) {
    m_musicCount = count;
}

void PlaylistListItem::setPlaceholderCover() {
    QPixmap pix(36, 36);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(0, 0, 36, 36, 6, 6);
    p.fillPath(path, QColor(128, 128, 128, 60));
    m_coverLbl->setPixmap(pix);
}

void PlaylistListItem::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_playlistId);
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
        emit renameRequested(m_playlistId);
    } else if (selected == deleteAction) {
        emit deleteRequested(m_playlistId);
    }
}

void PlaylistListItem::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background + border-left
    QRect r = rect();
    if (m_hovered) {
        QPainterPath path;
        path.addRoundedRect(r.adjusted(2, 2, -2, -2), 8, 8);
        painter.fillPath(path, QColor(255, 255, 255, 20));  // rgba(255,255,255,0.08) ~ 20/255
    }
    // border-left 3px transparent (visible on hover/active in future)
    painter.fillRect(0, r.height() / 2 - 10, 3, 20, QColor(0, 0, 0, 1));
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
