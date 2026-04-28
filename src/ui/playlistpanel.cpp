#include "ui/playlistpanel.h"
#include "core/playlistmanager.h"
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
#include <QGraphicsDropShadowEffect>

// ─── 播放队列项卡片 ──────────────────────────────────────
class PlaylistItemCard : public QWidget {
public:
    explicit PlaylistItemCard(const MusicInfo &info, int index, bool isCurrent, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(info.id), m_index(index), m_isCurrent(isCurrent)
    {
        setFixedHeight(60);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, false);

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(10, 6, 10, 6);
        lay->setSpacing(10);

        // 序号或播放图标
        m_indexLbl = new QLabel(this);
        m_indexLbl->setFixedSize(24, 24);
        m_indexLbl->setAlignment(Qt::AlignCenter);
        m_indexLbl->setStyleSheet(
            "QLabel { color: " + QString(Theme::kTextMuted) + "; font-size: 12px; }"
        );
        updateIndexDisplay();
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
        infoLay->setSpacing(2);

        m_titleLbl = new QLabel(info.title, infoV);
        m_titleLbl->setStyleSheet(
            "QLabel { font-size: 13px; font-weight: 600; color: " + QString(m_isCurrent ? Theme::kLavender : Theme::kTextMain) + "; }"
        );
        infoLay->addWidget(m_titleLbl);

        m_artistLbl = new QLabel(info.artist, infoV);
        m_artistLbl->setStyleSheet("QLabel { font-size: 11px; color: " + QString(Theme::kTextSub) + "; }");
        infoLay->addWidget(m_artistLbl);

        infoLay->addStretch();
        lay->addWidget(infoV, 1);

        // 移除按钮（悬停显示）
        m_removeBtn = new QPushButton(QStringLiteral("×"), this);
        m_removeBtn->setFixedSize(24, 24);
        m_removeBtn->setCursor(Qt::PointingHandCursor);
        m_removeBtn->setStyleSheet(
            "QPushButton { background: transparent; border: none; color: " + QString(Theme::kTextMuted) + "; font-size: 18px; border-radius: 12px; }"
            "QPushButton:hover { background: rgba(255, 100, 100, 0.3); color: #ff6464; }"
        );
        m_removeBtn->hide();
        connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
            emit removeRequested(m_musicId);
        });
        lay->addWidget(m_removeBtn);
    }

    std::function<void(int)> onClicked;
    std::function<void(int)> removeRequested;

    void updateCurrentState(bool isCurrent) {
        m_isCurrent = isCurrent;
        updateIndexDisplay();
        m_titleLbl->setStyleSheet(
            "QLabel { font-size: 13px; font-weight: 600; color: " + QString(m_isCurrent ? Theme::kLavender : Theme::kTextMain) + "; }"
        );
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QWidget::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 6, 6);
        p.fillPath(path, m_isCurrent ? QColor(196, 167, 231, 40) : QColor(45, 38, 65, 80));
    }

    void enterEvent(QEnterEvent *event) override {
        if (m_removeBtn) m_removeBtn->show();
        QWidget::enterEvent(event);
    }

    void leaveEvent(QEvent *event) override {
        if (m_removeBtn) m_removeBtn->hide();
        QWidget::leaveEvent(event);
    }

    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton && onClicked) {
            onClicked(m_musicId);
        }
        QWidget::mousePressEvent(e);
    }

private:
    void updateIndexDisplay() {
        if (m_isCurrent) {
            auto icon = Icons::render(Icons::kPlay, 14, QColor(196, 167, 231));
            QPixmap pix(24, 24);
            pix.fill(Qt::transparent);
            QPainter p(&pix);
            p.drawPixmap(5, 5, icon);
            m_indexLbl->setPixmap(pix);
        } else {
            m_indexLbl->setText(QString::number(m_index + 1));
        }
    }

    void loadCover() {
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
        CoverCache::instance()->fetchCover(musicId, QString());
    }

    void applyPixmap(const QPixmap &pix) {
        int s = qMin(pix.width(), pix.height());
        QPixmap scaled = pix.copy((pix.width()-s)/2, (pix.height()-s)/2, s, s)
            .scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_coverLbl->setPixmap(scaled);
    }

    int m_musicId;
    int m_index;
    bool m_isCurrent;
    QLabel *m_indexLbl = nullptr;
    QLabel *m_coverLbl = nullptr;
    QLabel *m_titleLbl = nullptr;
    QLabel *m_artistLbl = nullptr;
    QPushButton *m_removeBtn = nullptr;
};

// ─── PlaylistPanel ──────────────────────────────────────

PlaylistPanel::PlaylistPanel(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(360, 480);

    // 阴影效果
    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 100));
    setGraphicsEffect(shadow);

    setupUi();
    refresh();
}

void PlaylistPanel::setupUi() {
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(8);

    // 顶部栏
    auto *header = new QWidget(this);
    auto *headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(0, 0, 0, 0);

    m_titleLabel = new QLabel(I18n::instance().tr("playlist"), header);
    m_titleLabel->setStyleSheet(
        "QLabel { font-size: 16px; font-weight: 700; color: " + QString(Theme::kLavender) + "; }"
    );
    headerLay->addWidget(m_titleLabel);

    m_countLabel = new QLabel(header);
    m_countLabel->setStyleSheet("QLabel { font-size: 12px; color: " + QString(Theme::kTextMuted) + "; }");
    headerLay->addWidget(m_countLabel);

    headerLay->addStretch();

    m_clearBtn = new QPushButton(I18n::instance().tr("clear"), header);
    m_clearBtn->setFixedHeight(28);
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    m_clearBtn->setStyleSheet(
        "QPushButton { background: rgba(255, 100, 100, 0.2); border: 1px solid rgba(255, 100, 100, 0.3); border-radius: 14px; color: #ff6464; font-size: 11px; padding: 0 12px; }"
        "QPushButton:hover { background: rgba(255, 100, 100, 0.4); }"
    );
    connect(m_clearBtn, &QPushButton::clicked, this, []() {
        PlaylistManager::instance().clearPlaylist();
    });
    headerLay->addWidget(m_clearBtn);

    m_closeBtn = new QPushButton(QStringLiteral("×"), header);
    m_closeBtn->setFixedSize(28, 28);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    m_closeBtn->setStyleSheet(
        "QPushButton { background: rgba(245, 240, 255, 0.1); border: none; border-radius: 14px; color: " + QString(Theme::kTextMain) + "; font-size: 18px; }"
        "QPushButton:hover { background: rgba(196, 167, 231, 0.3); color: " + QString(Theme::kLavender) + "; }"
    );
    connect(m_closeBtn, &QPushButton::clicked, this, &PlaylistPanel::hideRequested);
    headerLay->addWidget(m_closeBtn);

    lay->addWidget(header);

    // 分隔线
    auto *divider = new QWidget(this);
    divider->setFixedHeight(1);
    divider->setStyleSheet("background: rgba(196, 167, 231, 0.2);");
    lay->addWidget(divider);

    // 滚动列表
    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);

    m_listContainer = new QWidget(m_scroll);
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(4, 4, 4, 4);
    m_listLayout->setSpacing(4);

    m_scroll->setWidget(m_listContainer);
    lay->addWidget(m_scroll, 1);
}

void PlaylistPanel::refresh() {
    rebuildList();
}

void PlaylistPanel::rebuildList() {
    // 清除现有项
    for (auto *item : m_items) {
        m_listLayout->removeWidget(item);
        item->deleteLater();
    }
    m_items.clear();

    const auto& playlist = PlaylistManager::instance().playlist();
    int currentIndex = PlaylistManager::instance().currentIndex();

    if (playlist.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("emptyPlaylist"), m_listContainer);
        emptyLbl->setAlignment(Qt::AlignCenter);
        emptyLbl->setStyleSheet(
            "QLabel { color: " + QString(Theme::kTextSub) + "; font-size: 13px; padding: 40px; }"
        );
        m_listLayout->addWidget(emptyLbl);
        m_items.append(emptyLbl);
    } else {
        for (int i = 0; i < playlist.size(); ++i) {
            auto *card = new PlaylistItemCard(playlist[i], i, i == currentIndex, m_listContainer);
            card->onClicked = [this, musicId = playlist[i].id](int) {
                emit playRequested(musicId);
            };
            card->removeRequested = [this](int localId) {
                PlaylistManager::instance().removeFromPlaylist(localId);
                refresh();
            };
            m_listLayout->addWidget(card);
            m_items.append(card);
        }
    }

    m_listLayout->addStretch();

    // 更新计数
    if (m_countLabel) {
        m_countLabel->setText(QString::number(playlist.size()));
    }
}

void PlaylistPanel::retranslate() {
    if (m_titleLabel) {
        m_titleLabel->setText(I18n::instance().tr("playlist"));
    }
    if (m_clearBtn) {
        m_clearBtn->setText(I18n::instance().tr("clear"));
    }
}

void PlaylistPanel::showPanel() {
    refresh();
    show();
    raise();
}

void PlaylistPanel::hidePanel() {
    hide();
}

void PlaylistPanel::togglePanel() {
    if (isVisible()) {
        hidePanel();
    } else {
        showPanel();
    }
}

void PlaylistPanel::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 背景
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);
    p.fillPath(path, QColor(36, 31, 49, 245));

    // 边框
    p.setPen(QPen(QColor(196, 167, 231, 40), 1));
    p.drawPath(path);
}
