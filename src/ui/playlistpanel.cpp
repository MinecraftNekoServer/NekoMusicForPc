#include "ui/playlistpanel.h"
#include "core/playlistmanager.h"
#include "core/i18n.h"
#include "core/covercache.h"
#include "theme/theme.h"
#include "theme/thememanager.h"
#include "glasspaint.h"
#include "ui/svgicon.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QFrame>
#include <QLinearGradient>

// ─── 播放队列项卡片 ──────────────────────────────────────
class PlaylistItemCard : public QWidget {
public:
    explicit PlaylistItemCard(const MusicInfo &info, int index, bool isCurrent, QWidget *parent = nullptr)
        : QWidget(parent), m_musicId(info.id), m_index(index), m_isCurrent(isCurrent)
    {
        setObjectName(QStringLiteral("PlaylistItemCard"));
        setFixedHeight(64);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_StyledBackground, false);

        const bool dark = Theme::ThemeManager::instance().isDarkMode();

        auto *lay = new QHBoxLayout(this);
        lay->setContentsMargins(10, 8, 8, 8);
        lay->setSpacing(12);

        m_indexLbl = new QLabel(this);
        m_indexLbl->setFixedSize(26, 26);
        m_indexLbl->setAlignment(Qt::AlignCenter);
        applyIndexLabelStyle(dark);
        updateIndexDisplay();
        lay->addWidget(m_indexLbl);

        m_coverLbl = new QLabel(this);
        m_coverLbl->setFixedSize(48, 48);
        m_coverLbl->setScaledContents(false);
        loadCover();
        lay->addWidget(m_coverLbl);

        auto *infoV = new QWidget(this);
        infoV->setAttribute(Qt::WA_TranslucentBackground);
        auto *infoLay = new QVBoxLayout(infoV);
        infoLay->setContentsMargins(0, 0, 0, 0);
        infoLay->setSpacing(3);

        m_titleLbl = new QLabel(info.title, infoV);
        applyTitleStyle(dark);
        infoLay->addWidget(m_titleLbl);

        m_artistLbl = new QLabel(info.artist, infoV);
        applyArtistStyle(dark);
        infoLay->addWidget(m_artistLbl);

        lay->addWidget(infoV, 1);

        m_removeBtn = new QPushButton(QStringLiteral("×"), this);
        m_removeBtn->setFixedSize(28, 28);
        m_removeBtn->setCursor(Qt::PointingHandCursor);
        applyRemoveBtnStyle(dark);
        m_removeBtn->hide();
        connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
            if (removeRequested)
                removeRequested(m_musicId);
        });
        lay->addWidget(m_removeBtn);
    }

    std::function<void(int)> onClicked;
    std::function<void(int)> removeRequested;

    void updateCurrentState(bool isCurrent)
    {
        m_isCurrent = isCurrent;
        const bool dark = Theme::ThemeManager::instance().isDarkMode();
        applyIndexLabelStyle(dark);
        updateIndexDisplay();
        applyTitleStyle(dark);
        applyArtistStyle(dark);
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const bool dark = Theme::ThemeManager::instance().isDarkMode();

        QPainterPath path;
        path.addRoundedRect(rect().adjusted(2, 2, -2, -2), 10, 10);

        if (dark) {
            if (m_isCurrent) {
                QLinearGradient g(rect().topLeft(), rect().bottomLeft());
                g.setColorAt(0.0, QColor(196, 167, 231, 38));
                g.setColorAt(1.0, QColor(126, 200, 200, 14));
                p.fillPath(path, g);
                p.strokePath(path, QPen(QColor(196, 167, 231, 95), 1.0));
            } else if (m_hover) {
                p.fillPath(path, QColor(196, 167, 231, 18));
                p.strokePath(path, QPen(QColor(196, 167, 231, 55), 1.0));
            } else {
                p.fillPath(path, QColor(42, 36, 58, 200));
                p.strokePath(path, QPen(QColor(196, 167, 231, 32), 1.0));
            }
        } else {
            if (m_isCurrent) {
                p.fillPath(path, QColor(196, 167, 231, 42));
                p.strokePath(path, QPen(QColor(111, 66, 193, 85), 1.0));
            } else if (m_hover) {
                p.fillPath(path, QColor(196, 167, 231, 22));
                p.strokePath(path, QPen(QColor(111, 66, 193, 45), 1.0));
            } else {
                p.fillPath(path, QColor(255, 255, 255, 220));
                p.strokePath(path, QPen(QColor(111, 66, 193, 35), 1.0));
            }
        }
    }

    void enterEvent(QEnterEvent *event) override
    {
        m_hover = true;
        if (m_removeBtn)
            m_removeBtn->show();
        update();
        QWidget::enterEvent(event);
    }

    void leaveEvent(QEvent *event) override
    {
        m_hover = false;
        if (m_removeBtn)
            m_removeBtn->hide();
        update();
        QWidget::leaveEvent(event);
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && onClicked)
            onClicked(m_musicId);
        QWidget::mousePressEvent(e);
    }

private:
    void applyIndexLabelStyle(bool dark)
    {
        const QString c = dark ? QString::fromUtf8(Theme::kTextMuted) : QStringLiteral("rgba(33,37,41,0.45)");
        m_indexLbl->setStyleSheet(QStringLiteral("QLabel { color: %1; font-size: 12px; font-weight: 600; }").arg(c));
    }

    void applyTitleStyle(bool dark)
    {
        const QString col = m_isCurrent
            ? QString::fromUtf8(Theme::kLavender)
            : (dark ? QString::fromUtf8(Theme::kTextMain) : QStringLiteral("#212529"));
        m_titleLbl->setStyleSheet(QStringLiteral(
            "QLabel { font-size: 13px; font-weight: 600; color: %1; }").arg(col));
    }

    void applyArtistStyle(bool dark)
    {
        const QString col = dark ? QString::fromUtf8(Theme::kTextSub) : QStringLiteral("rgba(33,37,41,0.62)");
        m_artistLbl->setStyleSheet(QStringLiteral("QLabel { font-size: 11px; color: %1; }").arg(col));
    }

    void applyRemoveBtnStyle(bool dark)
    {
        const QString fg = dark ? QString::fromUtf8(Theme::kTextMuted) : QStringLiteral("rgba(33,37,41,0.45)");
        m_removeBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: transparent; border: none; color: %1; font-size: 18px; border-radius: 14px; }"
            "QPushButton:hover { background: rgba(255,100,100,0.28); color: #ff6464; }")
                                       .arg(fg));
    }

    void updateIndexDisplay()
    {
        if (m_isCurrent) {
            const QColor ac = Theme::ThemeManager::instance().isDarkMode() ? QColor(196, 167, 231)
                                                                            : QColor(111, 66, 193);
            auto icon = Icons::render(Icons::kPlay, 14, ac);
            QPixmap pix(26, 26);
            pix.fill(Qt::transparent);
            QPainter p(&pix);
            p.drawPixmap(6, 6, icon);
            m_indexLbl->setPixmap(pix);
        } else {
            m_indexLbl->clear();
            m_indexLbl->setText(QString::number(m_index + 1));
        }
    }

    void loadCover()
    {
        const QString musicId = QString::number(m_musicId);
        CoverCache *cc = CoverCache::instance();
        if (QPixmap cached = cc->get(musicId); !cached.isNull()) {
            applyPixmap(cached);
            return;
        }
        const QString url = QString::fromUtf8("%1/api/music/cover/%2").arg(Theme::kApiBase).arg(m_musicId);
        connect(cc, &CoverCache::coverLoaded, this, [this, musicId](const QString &id, const QPixmap &pix) {
            if (id == musicId)
                applyPixmap(pix);
        });
        cc->fetchCover(musicId, url);
    }

    void applyPixmap(const QPixmap &pix)
    {
        if (pix.isNull())
            return;
        int s = qMin(pix.width(), pix.height());
        QPixmap scaled = pix.copy((pix.width() - s) / 2, (pix.height() - s) / 2, s, s)
                           .scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap rounded(48, 48);
        rounded.fill(Qt::transparent);
        QPainter p(&rounded);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath cp;
        cp.addRoundedRect(0, 0, 48, 48, 10, 10);
        p.setClipPath(cp);
        p.drawPixmap(0, 0, scaled);
        m_coverLbl->setPixmap(rounded);
    }

    int m_musicId;
    int m_index;
    bool m_isCurrent = false;
    bool m_hover = false;
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
    setFixedSize(400, 540);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(28);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0, 0, 0, 110));
    setGraphicsEffect(shadow);

    setupUi();
    applyPanelChrome();

    connect(&Theme::ThemeManager::instance(), &Theme::ThemeManager::themeChanged, this,
            [this](Theme::ThemeMode) {
                applyPanelChrome();
                refresh();
            });

    refresh();
}

void PlaylistPanel::applyPanelChrome()
{
    const bool dark = Theme::ThemeManager::instance().isDarkMode();

    if (m_titleLabel) {
        m_titleLabel->setStyleSheet(QStringLiteral(
            "QLabel { font-size: 17px; font-weight: 700; color: %1; letter-spacing: 0.35px; }")
                                        .arg(dark ? QString::fromUtf8(Theme::kLavender) : QStringLiteral("#6F42C1")));
    }
    if (m_countLabel) {
        m_countLabel->setStyleSheet(QStringLiteral(
            "QLabel { font-size: 12px; color: %1; padding-left: 8px; }")
                                        .arg(dark ? QString::fromUtf8(Theme::kTextMuted)
                                                  : QStringLiteral("rgba(33,37,41,0.52)")));
    }
    if (m_clearBtn) {
        m_clearBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: rgba(255,100,100,%1); border: 1px solid rgba(255,100,100,%2); "
            "border-radius: 14px; color: #ff6464; font-size: 11px; font-weight: 600; padding: 0 14px; }"
            "QPushButton:hover { background: rgba(255,100,100,%3); }")
                                      .arg(dark ? 22 : 18)
                                      .arg(dark ? 42 : 38)
                                      .arg(dark ? 40 : 34));
    }
    if (m_closeBtn) {
        m_closeBtn->setStyleSheet(QStringLiteral(
            "QPushButton { background: rgba(196,167,231,%2); border: none; border-radius: 14px; color: %1; font-size: 18px; }"
            "QPushButton:hover { background: rgba(196,167,231,%3); color: %4; }")
                                      .arg(dark ? QString::fromUtf8(Theme::kTextMain) : QStringLiteral("#212529"))
                                      .arg(dark ? 14 : 20)
                                      .arg(dark ? 32 : 40)
                                      .arg(dark ? QString::fromUtf8(Theme::kLavender) : QStringLiteral("#6F42C1")));
    }
    if (m_divider) {
        m_divider->setStyleSheet(QStringLiteral(
            "QWidget#ppDivider { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
            "stop:0 transparent, stop:0.15 rgba(196,167,231,%1), stop:0.5 rgba(196,167,231,%2), stop:0.85 rgba(196,167,231,%1), stop:1 transparent); }")
                                     .arg(dark ? 55 : 65)
                                     .arg(dark ? 28 : 38));
    }
    if (m_scroll) {
        m_scroll->setStyleSheet(QStringLiteral(
            "QScrollArea#ppScroll { border: none; background: transparent; }"
            "QScrollBar:vertical { width: 5px; background: transparent; margin: 2px 0 4px 0; }"
            "QScrollBar::handle:vertical { background: rgba(196,167,231,%1); border-radius: 3px; min-height: 40px; }"
            "QScrollBar::handle:vertical:hover { background: rgba(196,167,231,%2); }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }")
                                    .arg(dark ? 70 : 82)
                                    .arg(dark ? 108 : 125));
    }
}

void PlaylistPanel::setupUi()
{
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(14, 14, 14, 14);
    lay->setSpacing(10);

    auto *header = new QWidget(this);
    header->setObjectName(QStringLiteral("ppHeader"));
    auto *headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(4, 2, 4, 6);

    m_titleLabel = new QLabel(I18n::instance().tr("playlist"), header);
    headerLay->addWidget(m_titleLabel);

    m_countLabel = new QLabel(header);
    headerLay->addWidget(m_countLabel);

    headerLay->addStretch();

    m_clearBtn = new QPushButton(I18n::instance().tr("clear"), header);
    m_clearBtn->setFixedHeight(30);
    m_clearBtn->setCursor(Qt::PointingHandCursor);
    connect(m_clearBtn, &QPushButton::clicked, this, [this]() {
        auto& manager = PlaylistManager::instance();
        int currentIndex = manager.currentIndex();
        MusicInfo currentMusic;
        bool hasCurrent = (currentIndex >= 0 && currentIndex < manager.count());
        if (hasCurrent) {
            currentMusic = manager.playlist()[currentIndex];
        }

        // 清空后保留当前音乐
        if (hasCurrent) {
            manager.clearPlaylist();
            manager.addToPlaylist(currentMusic);
            manager.setCurrentIndex(0);
        } else {
            manager.clearPlaylist();
        }
        refresh();
    });
    headerLay->addWidget(m_clearBtn);

    m_closeBtn = new QPushButton(QStringLiteral("×"), header);
    m_closeBtn->setFixedSize(30, 30);
    m_closeBtn->setCursor(Qt::PointingHandCursor);
    connect(m_closeBtn, &QPushButton::clicked, this, &PlaylistPanel::hideRequested);
    headerLay->addWidget(m_closeBtn);

    lay->addWidget(header);

    m_divider = new QWidget(this);
    m_divider->setObjectName(QStringLiteral("ppDivider"));
    m_divider->setFixedHeight(1);
    lay->addWidget(m_divider);

    m_scroll = new QScrollArea(this);
    m_scroll->setObjectName(QStringLiteral("ppScroll"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll->setFrameShape(QFrame::NoFrame);

    m_listContainer = new QWidget(m_scroll);
    m_listLayout = new QVBoxLayout(m_listContainer);
    m_listLayout->setContentsMargins(2, 6, 6, 8);
    m_listLayout->setSpacing(6);
    m_listLayout->setAlignment(Qt::AlignTop);

    m_scroll->setWidget(m_listContainer);
    lay->addWidget(m_scroll, 1);
}

void PlaylistPanel::refresh() {
    // 保存当前滚动位置
    int scrollPos = 0;
    if (m_scroll) {
        scrollPos = m_scroll->verticalScrollBar()->value();
    }

    rebuildList();

    // 恢复滚动位置
    if (m_scroll) {
        m_scroll->verticalScrollBar()->setValue(scrollPos);
    }
}

void PlaylistPanel::rebuildList() {
    QLayoutItem *lit;
    while ((lit = m_listLayout->takeAt(0)) != nullptr) {
        if (QWidget *w = lit->widget())
            w->deleteLater();
        delete lit;
    }
    m_items.clear();

    const auto& playlist = PlaylistManager::instance().playlist();
    int currentIndex = PlaylistManager::instance().currentIndex();

    if (playlist.isEmpty()) {
        auto *emptyLbl = new QLabel(I18n::instance().tr("emptyPlaylist"), m_listContainer);
        emptyLbl->setAlignment(Qt::AlignCenter);
        const bool dark = Theme::ThemeManager::instance().isDarkMode();
        const QString c = dark ? QString::fromUtf8(Theme::kTextSub) : QStringLiteral("rgba(33,37,41,0.55)");
        emptyLbl->setStyleSheet(QStringLiteral(
            "QLabel { color: %1; font-size: 13px; padding: 48px 24px; line-height: 1.5; }")
                                    .arg(c));
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

    m_listLayout->addStretch(1);

    if (m_countLabel)
        m_countLabel->setText(QStringLiteral("· %1").arg(playlist.size()));
}

void PlaylistPanel::retranslate()
{
    if (m_titleLabel)
        m_titleLabel->setText(I18n::instance().tr("playlist"));
    if (m_clearBtn)
        m_clearBtn->setText(I18n::instance().tr("clear"));
    applyPanelChrome();
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

void PlaylistPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const bool dark = Theme::ThemeManager::instance().isDarkMode();

    QPainterPath clip;
    clip.addRoundedRect(rect().adjusted(1, 1, -1, -1), 14, 14);
    p.setClipPath(clip);
    GlassPaint::paintMainWindowDeepBackdrop(p, rect(), dark);
    p.setClipping(false);

    p.setPen(QPen(dark ? QColor(196, 167, 231, 58) : QColor(111, 66, 193, 72), 1.0));
    p.drawPath(clip);
}
