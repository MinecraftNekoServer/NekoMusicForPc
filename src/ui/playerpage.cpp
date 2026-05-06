#include "playerpage.h"
#include "../core/playerengine.h"
#include "../core/i18n.h"
#include "../core/covercache.h"
#include "../theme/theme.h"
#include "../theme/thememanager.h"
#include "glasspaint.h"

#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QFrame>
#include <QShowEvent>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDebug>

PlayerPage::PlayerPage(PlayerEngine *engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();

    connect(&Theme::ThemeManager::instance(), &Theme::ThemeManager::themeChanged, this,
            [this](Theme::ThemeMode) {
                applyPlayerPageStyle();
                update();
                rebuildLyricLabels();
                updateLyricHighlight(m_engine->position());
            });
}

PlayerPage::~PlayerPage() = default;

void PlayerPage::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    GlassPaint::paintMainWindowDeepBackdrop(p, rect(), Theme::ThemeManager::instance().isDarkMode());

    QWidget::paintEvent(event);
}

void PlayerPage::applyPlayerPageStyle()
{
    const bool dark = Theme::ThemeManager::instance().isDarkMode();

    if (m_leftGlass) {
        if (dark) {
            m_leftGlass->setBaseColor(QColor(45, 38, 65));
            m_leftGlass->setBorderColor(QColor(196, 167, 231, 58));
            m_leftGlass->setOpacity(0.54);
        } else {
            m_leftGlass->setBaseColor(QColor(255, 255, 255));
            m_leftGlass->setBorderColor(QColor(111, 66, 193, 70));
            m_leftGlass->setOpacity(0.64);
        }
        m_leftGlass->setBorderRadius(Theme::kRXl);
    }
    if (m_rightGlass) {
        if (dark) {
            m_rightGlass->setBaseColor(QColor(40, 34, 58));
            m_rightGlass->setBorderColor(QColor(196, 167, 231, 48));
            m_rightGlass->setOpacity(0.50);
        } else {
            m_rightGlass->setBaseColor(QColor(255, 255, 255));
            m_rightGlass->setBorderColor(QColor(111, 66, 193, 55));
            m_rightGlass->setOpacity(0.58);
        }
        m_rightGlass->setBorderRadius(Theme::kRLg);
    }

    if (dark) {
        m_clrTitle = QString::fromUtf8(Theme::kLavender);
        m_clrArtist = QString::fromUtf8(Theme::kTextSub);
        m_clrAlbum = QString::fromUtf8(Theme::kTextMuted);
        m_clrLyricDim = QString::fromUtf8(Theme::kTextMuted);
        m_clrLyricHi = QString::fromUtf8(Theme::kLavender);
        m_clrLyricHiTrans = QString::fromUtf8(Theme::kLavenderLt);
        m_clrLyricHiBg = QStringLiteral("rgba(196,167,231,24)");
    } else {
        m_clrTitle = QStringLiteral("#6F42C1");
        m_clrArtist = QStringLiteral("rgba(33,37,41,0.78)");
        m_clrAlbum = QStringLiteral("rgba(33,37,41,0.52)");
        m_clrLyricDim = QStringLiteral("rgba(33,37,41,0.52)");
        m_clrLyricHi = QStringLiteral("#6F42C1");
        m_clrLyricHiTrans = QStringLiteral("#8B6FC4");
        m_clrLyricHiBg = QStringLiteral("rgba(196,167,231,0.38)");
    }

    const QString backFg = dark ? QString::fromUtf8(Theme::kLavender) : QStringLiteral("#6F42C1");
    const QString coverBorder = dark ? QStringLiteral("rgba(196,167,231,32)")
                                     : QStringLiteral("rgba(111,66,193,0.32)");

    const int backBgA = dark ? 18 : 22;
    const int backBdA = dark ? 32 : 40;
    const int backHiA = dark ? 42 : 55;
    const int backHiBdA = dark ? 62 : 75;
    const int sepA = dark ? 40 : 48;
    const int sbA = dark ? 60 : 85;
    const int sbHiA = dark ? 100 : 120;

    setStyleSheet(QString::fromUtf8(
                      "#playerPage { background: transparent; }"

                      "#playerBackBtn { "
                      "  background: rgba(196,167,231,%5); color: %1; font-size: 20px; "
                      "  border: 1px solid rgba(196,167,231,%6); border-radius: 22px; }"
                      "#playerBackBtn:hover { "
                      "  background: rgba(196,167,231,%7); color: %1; border-color: rgba(196,167,231,%8); }"

                      "#playerCoverLabel { "
                      "  background: transparent; "
                      "  border: 2px solid %2; "
                      "  border-radius: 32px; }"

                      "#playerSongTitleLabel { "
                      "  color: %3; font-size: 22px; font-weight: 600; "
                      "  background: transparent; qproperty-alignment: 'AlignCenter'; }"

                      "#playerArtistLabel { "
                      "  color: %4; font-size: 15px; font-weight: 400; "
                      "  background: transparent; qproperty-alignment: 'AlignCenter'; }"

                      "#playerAlbumLabel { "
                      "  color: %9; font-size: 13px; "
                      "  background: transparent; qproperty-alignment: 'AlignCenter'; }"

                      "#lyricsTitleLabel { "
                      "  color: %3; font-size: 18px; font-weight: 600; "
                      "  background: transparent; padding-left: 16px; }"

                      "#lyricsSeparator { "
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
                      "    stop:0 rgba(196,167,231,%10), stop:1 rgba(196,167,231,0)); }"

                      "#lyricsScroll { "
                      "  background: transparent; border: none; }"
                      "#lyricsScroll > QWidget { background: transparent; }"

                      "QScrollBar:vertical { width: 4px; background: transparent; }"
                      "QScrollBar::handle:vertical { "
                      "  background: rgba(196,167,231,%11); border-radius: 2px; min-height: 50px; }"
                      "QScrollBar::handle:vertical:hover { "
                      "  background: rgba(196,167,231,%12); }"
                      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
                      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }")
                      .arg(backFg)
                      .arg(coverBorder)
                      .arg(m_clrTitle)
                      .arg(m_clrArtist)
                      .arg(backBgA)
                      .arg(backBdA)
                      .arg(backHiA)
                      .arg(backHiBdA)
                      .arg(m_clrAlbum)
                      .arg(sepA)
                      .arg(sbA)
                      .arg(sbHiA));
}

void PlayerPage::setupUi()
{
    setObjectName("playerPage");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(48, 20, 48, 48);
    mainLayout->setSpacing(0);

    // ─── 顶部栏：返回按钮 ───
    auto *topBar = new QHBoxLayout();
    topBar->setContentsMargins(0, 0, 0, 24);

    m_backBtn = new QPushButton(this);
    m_backBtn->setFixedSize(44, 44);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setObjectName("playerBackBtn");
    // 使用 SVG 返回图标
    m_backBtn->setText(QString::fromUtf8("\xe2\x86\x90"));
    connect(m_backBtn, &QPushButton::clicked, this, [this]() { emit backRequested(); });

    topBar->addWidget(m_backBtn);
    topBar->addStretch();
    mainLayout->addLayout(topBar);

    // ─── 主内容区 ───
    auto *contentRow = new QHBoxLayout();
    contentRow->setSpacing(56);
    contentRow->setContentsMargins(0, 0, 0, 0);

    m_leftGlass = new GlassWidget(this);
    m_leftGlass->setObjectName(QStringLiteral("playerLeftGlass"));
    m_leftGlass->setAttribute(Qt::WA_StyledBackground, false);
    m_leftGlass->setFixedWidth(320 + 48); // 封面 + 左右内边距
    m_leftGlass->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    auto *coverCol = new QVBoxLayout(m_leftGlass);
    coverCol->setSpacing(20);
    coverCol->setContentsMargins(24, 28, 24, 28);
    coverCol->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_coverLabel = new QLabel(m_leftGlass);
    m_coverLabel->setFixedSize(320, 320);
    m_coverLabel->setScaledContents(false);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setObjectName("playerCoverLabel");

    m_titleLabel = new QLabel(I18n::instance().tr("unknown"), m_leftGlass);
    m_titleLabel->setObjectName("playerSongTitleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumWidth(340);

    m_artistLabel = new QLabel(I18n::instance().tr("unknownArtist"), m_leftGlass);
    m_artistLabel->setObjectName("playerArtistLabel");
    m_artistLabel->setAlignment(Qt::AlignCenter);

    m_albumLabel = new QLabel(m_leftGlass);
    m_albumLabel->setObjectName("playerAlbumLabel");
    m_albumLabel->setAlignment(Qt::AlignCenter);

    coverCol->addSpacing(16);
    coverCol->addWidget(m_coverLabel);
    coverCol->addSpacing(12);
    coverCol->addWidget(m_titleLabel);
    coverCol->addSpacing(6);
    coverCol->addWidget(m_artistLabel);
    coverCol->addSpacing(4);
    coverCol->addWidget(m_albumLabel);
    coverCol->addStretch();

    m_rightGlass = new GlassWidget(this);
    m_rightGlass->setObjectName(QStringLiteral("playerRightGlass"));
    m_rightGlass->setAttribute(Qt::WA_StyledBackground, false);
    m_rightGlass->setMinimumWidth(520);
    m_rightGlass->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *lyricsCol = new QVBoxLayout(m_rightGlass);
    lyricsCol->setContentsMargins(20, 22, 22, 24);
    lyricsCol->setSpacing(0);

    auto *lyricsTitle = new QLabel(I18n::instance().tr("lyrics"), m_rightGlass);
    lyricsTitle->setObjectName("lyricsTitleLabel");
    lyricsTitle->setMaximumWidth(500);

    lyricsCol->addWidget(lyricsTitle);
    lyricsCol->addSpacing(16);

    auto *separator = new QLabel(m_rightGlass);
    separator->setFixedHeight(1);
    separator->setObjectName("lyricsSeparator");
    separator->setMaximumWidth(500);
    lyricsCol->addWidget(separator);
    lyricsCol->addSpacing(16);

    m_lyricsScroll = new QScrollArea(m_rightGlass);
    m_lyricsScroll->setObjectName("lyricsScroll");
    m_lyricsScroll->setWidgetResizable(true);
    m_lyricsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_lyricsScroll->setFrameShape(QFrame::NoFrame);

    m_lyricsContainer = new QWidget();
    m_lyricsContainer->setMinimumWidth(500);
    m_lyricsLayout = new QVBoxLayout(m_lyricsContainer);
    m_lyricsLayout->setAlignment(Qt::AlignTop);
    m_lyricsLayout->setSpacing(12);
    m_lyricsLayout->setContentsMargins(16, 20, 16, 20);

    m_lyricsScroll->setWidget(m_lyricsContainer);
    lyricsCol->addWidget(m_lyricsScroll, 1);

    contentRow->addWidget(m_leftGlass, 0);
    contentRow->addWidget(m_rightGlass, 1);

    mainLayout->addLayout(contentRow, 1);

    applyPlayerPageStyle();
}

void PlayerPage::setMusicInfo(int id, const QString &title, const QString &artist,
                              const QString &album, const QString &coverUrl)
{
    const QString t = title.isEmpty() ? I18n::instance().tr("unknown") : title;
    const QString a = artist.isEmpty() ? I18n::instance().tr("unknownArtist") : artist;
    const int prevId = m_musicId;
    const QString prevCoverUrl = m_coverUrl;
    m_musicId = id;
    m_titleLabel->setText(t);
    m_artistLabel->setText(a);
    m_albumLabel->setText(album);

    if (coverUrl.isEmpty())
        return;
    // 同一首歌且封面 URL 未变：不重复走 CoverCache / 网络
    if (prevId == id && coverUrl == prevCoverUrl)
        return;

    disconnect(m_coverConn);
    m_coverConn = {};
    m_coverUrl = coverUrl;
    loadCover(coverUrl);
}

void PlayerPage::retranslate()
{
    // Update labels only if they show default values
    if (m_titleLabel->text() == I18n::instance().tr("unknown") || m_titleLabel->text().isEmpty()) {
        m_titleLabel->setText(I18n::instance().tr("unknown"));
    }
    if (m_artistLabel->text() == I18n::instance().tr("unknownArtist") || m_artistLabel->text().isEmpty()) {
        m_artistLabel->setText(I18n::instance().tr("unknownArtist"));
    }

    // Update lyrics title - find it by object name
    auto *lyricsTitle = findChild<QLabel *>("lyricsTitleLabel");
    if (lyricsTitle) {
        lyricsTitle->setText(I18n::instance().tr("lyrics"));
    }

    // Update no lyrics message if visible
    auto *noLyricsText = findChild<QLabel *>("noLyricsText");
    if (noLyricsText) {
        noLyricsText->setText(I18n::instance().tr("noLyrics"));
    }
}

void PlayerPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 入场动画：淡入 + 内容上移
    auto *opacity = new QGraphicsOpacityEffect(this);
    opacity->setOpacity(0.0);
    setGraphicsEffect(opacity);

    auto *fadeIn = new QPropertyAnimation(opacity, "opacity");
    fadeIn->setDuration(350);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    connect(fadeIn, &QPropertyAnimation::finished, this, [this]() {
        setGraphicsEffect(nullptr);
    });
}

void PlayerPage::applyCoverPixmap(const QPixmap &sourcePixmap)
{
    if (sourcePixmap.isNull())
        return;
    QPixmap rounded(320, 320);
    rounded.fill(Qt::transparent);
    QPainter p(&rounded);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(0, 0, 320, 320, 32, 32);
    p.setClipPath(path);
    p.drawPixmap(0, 0, sourcePixmap.scaled(320, 320, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_coverLabel->setPixmap(rounded);
}

void PlayerPage::loadCover(const QString &url)
{
    if (url.isEmpty())
        return;
    const QString musicId = CoverCache::musicIdFromCoverUrl(url);
    if (musicId.isEmpty())
        return;

    CoverCache *cc = CoverCache::instance();
    if (QPixmap cached = cc->get(musicId); !cached.isNull()) {
        applyCoverPixmap(cached);
        return;
    }

    m_coverLabel->clear();

    disconnect(m_coverConn);
    m_coverConn = connect(cc, &CoverCache::coverLoaded, this,
                            [this, musicId](const QString &id, const QPixmap &pix) {
                                if (id != musicId)
                                    return;
                                if (QString::number(m_musicId) != musicId)
                                    return;
                                if (pix.isNull())
                                    return;
                                applyCoverPixmap(pix);
                            });
    cc->fetchCover(musicId, url);
}

void PlayerPage::loadLyrics(int musicId)
{
    m_currentLyricLine = -1;

    if (musicId <= 0) {
        m_lyrics.clear();
        rebuildLyricLabels();
        return;
    }

    if (m_lyricsCache.contains(musicId)) {
        m_lyrics = m_lyricsCache.value(musicId);
        rebuildLyricLabels();
        return;
    }

    m_lyrics.clear();
    rebuildLyricLabels();

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QString url = QString::fromUtf8("%1/api/music/lyrics/%2?t=%3")
        .arg(Theme::kApiBase).arg(musicId).arg(QDateTime::currentMSecsSinceEpoch());
    QNetworkReply *reply = nam->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, nam, musicId]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject obj = doc.object();
            bool success = obj.value("success").toBool();
            if (success) {
                QString lrc = obj.value("data").toString();
                if (!lrc.isEmpty()) {
                    parseLrc(lrc);
                    rebuildLyricLabels();
                    if (!m_lyrics.isEmpty()) {
                        constexpr int kMax = 64;
                        if (m_lyricsCache.size() >= kMax && !m_lyricsCache.contains(musicId))
                            m_lyricsCache.remove(m_lyricsCache.constBegin().key());
                        m_lyricsCache.insert(musicId, m_lyrics);
                    }
                } else {
                    qDebug() << "歌词API返回空歌词内容，musicId:" << musicId;
                }
            } else {
                qDebug() << "歌词API返回失败，musicId:" << musicId << "message:" << obj.value("message").toString();
            }
        } else {
            qDebug() << "歌词API请求失败，musicId:" << musicId << "error:" << reply->errorString();
        }
        reply->deleteLater();
        nam->deleteLater();
    });
}

void PlayerPage::parseLrc(const QString &lrc)
{
    m_lyrics.clear();
    const QStringList lines = lrc.split('\n');

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.isEmpty()) continue;

        static const QRegularExpression timeRe(R"(\[(\d{1,2}):(\d{1,2})\.(\d{2,3})\])");
        auto match = timeRe.match(line);
        if (!match.hasMatch()) continue;

        int min = match.captured(1).toInt();
        int sec = match.captured(2).toInt();
        int ms = match.captured(3).toInt();
        if (match.captured(3).length() == 2) ms *= 10;
        qint64 timeMs = (min * 60 + sec) * 1000 + ms;

        QString text = line.replace(match.captured(0), "").trimmed();

        QString translation;
        if (i + 1 < lines.size()) {
            QString next = lines[i + 1].trimmed();
            static const QRegularExpression transRe(R"(^\{["'](.+)["']\}$)");
            auto tMatch = transRe.match(next);
            if (tMatch.hasMatch()) {
                translation = tMatch.captured(1);
            }
        }

        m_lyrics.append({timeMs, text, translation});
    }

    std::sort(m_lyrics.begin(), m_lyrics.end(),
              [](const LyricLine &a, const LyricLine &b) { return a.time < b.time; });
}

void PlayerPage::rebuildLyricLabels()
{
    QLayoutItem *item;
    while ((item = m_lyricsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_lyrics.isEmpty()) {
        auto *noData = new QLabel(QString::fromUtf8("\xe2\x99\xaa"), m_lyricsContainer);
        noData->setAlignment(Qt::AlignCenter);
        noData->setObjectName("noLyricsIcon");
        noData->setStyleSheet(QString::fromUtf8(
            "color: %1; font-size: 56px; background: transparent; margin-top: 60px;"
        ).arg(m_clrLyricDim));
        m_lyricsLayout->addWidget(noData);

        auto *noDataLabel = new QLabel(I18n::instance().tr("noLyrics"), m_lyricsContainer);
        noDataLabel->setAlignment(Qt::AlignCenter);
        noDataLabel->setObjectName("noLyricsText");
        noDataLabel->setStyleSheet(QString::fromUtf8(
            "color: %1; font-size: 14px; background: transparent;"
        ).arg(m_clrLyricDim));
        m_lyricsLayout->addWidget(noDataLabel);
        m_lyricsLayout->addStretch();
        return;
    }

    for (int i = 0; i < m_lyrics.size(); ++i) {
        auto *lineWidget = new QWidget(m_lyricsContainer);
        lineWidget->setObjectName(QString("lyricWidget_%1").arg(i));
        auto *lineLayout = new QVBoxLayout(lineWidget);
        lineLayout->setContentsMargins(12, 8, 12, 8);
        lineLayout->setSpacing(4);

        auto *textLabel = new QLabel(m_lyrics[i].text, lineWidget);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setObjectName("lyricText");
        textLabel->setProperty("lyricIndex", i);
        textLabel->setWordWrap(true);
        textLabel->setStyleSheet(QString::fromUtf8(
            "color: %1; font-size: 15px; background: transparent; "
            "border-radius: 8px; padding: 6px 12px;"
        ).arg(m_clrLyricDim));
        lineLayout->addWidget(textLabel);

        if (!m_lyrics[i].translation.isEmpty()) {
            auto *transLabel = new QLabel(m_lyrics[i].translation, lineWidget);
            transLabel->setAlignment(Qt::AlignCenter);
            transLabel->setObjectName("lyricTranslation");
            transLabel->setProperty("lyricIndex", i);
            transLabel->setWordWrap(true);
            transLabel->setStyleSheet(QString::fromUtf8(
                "color: %1; font-size: 12px; background: transparent; "
                "border-radius: 6px; padding: 4px 10px;"
            ).arg(m_clrLyricDim));
            lineLayout->addWidget(transLabel);
        }

        m_lyricsLayout->addWidget(lineWidget);
    }
    m_lyricsLayout->addStretch();
}

void PlayerPage::updateLyricHighlight(qint64 positionMs)
{
    if (m_lyrics.isEmpty()) return;

    int line = -1;
    for (int i = m_lyrics.size() - 1; i >= 0; --i) {
        if (positionMs >= m_lyrics.at(i).time) {
            line = i;
            break;
        }
    }

    if (line == m_currentLyricLine) return;
    m_currentLyricLine = line;

    for (int i = 0; i < m_lyricsLayout->count(); ++i) {
        QLayoutItem *layoutItem = m_lyricsLayout->itemAt(i);
        if (!layoutItem) continue;
        auto *widget = qobject_cast<QWidget *>(layoutItem->widget());
        if (!widget) continue;

        auto *textLabel = widget->findChild<QLabel *>("lyricText");
        auto *transLabel = widget->findChild<QLabel *>("lyricTranslation");
        if (!textLabel) continue;

        int idx = textLabel->property("lyricIndex").toInt();
        bool isCurrent = (idx == line);

        textLabel->setStyleSheet(QString::fromUtf8(
            "color: %1; font-size: %2; font-weight: %3; "
            "background: %4; border-radius: 8px; padding: 6px 12px;"
        ).arg(isCurrent ? m_clrLyricHi : m_clrLyricDim)
         .arg(isCurrent ? 17 : 15)
         .arg(isCurrent ? "bold" : "normal")
         .arg(isCurrent ? m_clrLyricHiBg : QStringLiteral("transparent")));

        if (transLabel) {
            transLabel->setStyleSheet(QString::fromUtf8(
                "color: %1; font-size: %2; "
                "background: transparent; border-radius: 6px; padding: 4px 10px;"
            ).arg(isCurrent ? m_clrLyricHiTrans : m_clrLyricDim)
             .arg(isCurrent ? 13 : 12));
        }
    }

    // Auto-scroll with animation
    if (line >= 0) {
        QLayoutItem *layoutItem = m_lyricsLayout->itemAt(line);
        if (layoutItem && layoutItem->widget()) {
            int y = layoutItem->widget()->y();
            int target = y - m_lyricsScroll->height() / 3;
            auto *scrollBar = m_lyricsScroll->verticalScrollBar();

            // Cancel previous animation
            if (m_scrollAnim) {
                m_scrollAnim->stop();
                delete m_scrollAnim;
            }

            m_scrollAnim = new QPropertyAnimation(scrollBar, "value");
            m_scrollAnim->setDuration(300);
            m_scrollAnim->setStartValue(scrollBar->value());
            m_scrollAnim->setEndValue(target);
            m_scrollAnim->setEasingCurve(QEasingCurve::OutCubic);
            m_scrollAnim->start(QAbstractAnimation::DeleteWhenStopped);
            connect(m_scrollAnim, &QPropertyAnimation::finished, this, [this]() {
                m_scrollAnim = nullptr;
            });
        }
    }
}
