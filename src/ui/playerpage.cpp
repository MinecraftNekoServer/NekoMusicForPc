#include "playerpage.h"
#include "../core/playerengine.h"
#include "../core/i18n.h"
#include "../theme/theme.h"

#include <QPainter>
#include <QPainterPath>
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
}

PlayerPage::~PlayerPage() = default;

void PlayerPage::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 深色渐变背景
    QLinearGradient grad(rect().topLeft(), rect().bottomRight());
    grad.setColorAt(0.0, QColor(20, 16, 30));
    grad.setColorAt(1.0, QColor(30, 24, 45));
    p.fillRect(rect(), grad);

    QWidget::paintEvent(event);
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

    // 左侧封面列
    auto *coverCol = new QVBoxLayout();
    coverCol->setSpacing(20);
    coverCol->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    // 封面容器（带阴影效果）
    m_coverLabel = new QLabel(this);
    m_coverLabel->setFixedSize(320, 320);
    m_coverLabel->setScaledContents(false);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setObjectName("playerCoverLabel");

    m_titleLabel = new QLabel(I18n::instance().tr("unknown"), this);
    m_titleLabel->setObjectName("playerSongTitleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setMaximumWidth(340);

    m_artistLabel = new QLabel(I18n::instance().tr("unknownArtist"), this);
    m_artistLabel->setObjectName("playerArtistLabel");
    m_artistLabel->setAlignment(Qt::AlignCenter);

    m_albumLabel = new QLabel();
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

    // 右侧歌词区
    auto *lyricsCol = new QVBoxLayout();
    lyricsCol->setContentsMargins(0, 0, 0, 0);
    lyricsCol->setSpacing(0);

    // 歌词标题
    auto *lyricsTitle = new QLabel(I18n::instance().tr("lyrics"), this);
    lyricsTitle->setObjectName("lyricsTitleLabel");
    lyricsTitle->setMaximumWidth(500);

    lyricsCol->addWidget(lyricsTitle);
    lyricsCol->addSpacing(16);

    // 歌词分隔线
    auto *separator = new QLabel(this);
    separator->setFixedHeight(1);
    separator->setObjectName("lyricsSeparator");
    separator->setMaximumWidth(500);
    lyricsCol->addWidget(separator);
    lyricsCol->addSpacing(16);

    m_lyricsScroll = new QScrollArea(this);
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

    contentRow->addLayout(coverCol, 0);
    contentRow->addLayout(lyricsCol, 1);

    mainLayout->addLayout(contentRow, 1);

    // ─── 样式 ───
    setStyleSheet(QString::fromUtf8(
        "#playerPage { background: transparent; }"

        // 返回按钮
        "#playerBackBtn { "
        "  background: rgba(196,167,231,15); color: %1; font-size: 20px; "
        "  border: 1px solid rgba(196,167,231,30); border-radius: 22px; }"
        "#playerBackBtn:hover { "
        "  background: rgba(196,167,231,40); color: %1; border-color: rgba(196,167,231,60); }"

        // 封面
        "#playerCoverLabel { "
        "  background: transparent; "
        "  border: 2px solid rgba(196,167,231,25); "
        "  border-radius: 32px; }"

        // 歌曲标题
        "#playerSongTitleLabel { "
        "  color: %1; font-size: 22px; font-weight: 600; "
        "  background: transparent; qproperty-alignment: 'AlignCenter'; }"

        // 艺术家
        "#playerArtistLabel { "
        "  color: %2; font-size: 15px; font-weight: 400; "
        "  background: transparent; qproperty-alignment: 'AlignCenter'; }"

        // 专辑
        "#playerAlbumLabel { "
        "  color: %3; font-size: 13px; "
        "  background: transparent; qproperty-alignment: 'AlignCenter'; }"

        // 歌词标题
        "#lyricsTitleLabel { "
        "  color: %1; font-size: 18px; font-weight: 600; "
        "  background: transparent; padding-left: 16px; }"

        // 分隔线
        "#lyricsSeparator { "
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0, "
        "    stop:0 rgba(196,167,231,40), stop:1 rgba(196,167,231,0)); }"

        // 歌词滚动区
        "#lyricsScroll { "
        "  background: transparent; border: none; }"
        "#lyricsScroll > QWidget { background: transparent; }"

        // 滚动条
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { "
        "  background: rgba(196,167,231,60); border-radius: 2px; min-height: 50px; }"
        "QScrollBar::handle:vertical:hover { "
        "  background: rgba(196,167,231,100); }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
    ).arg(Theme::kLavender, Theme::kTextSub, Theme::kTextMuted));
}

void PlayerPage::setMusicInfo(int id, const QString &title, const QString &artist,
                              const QString &album, const QString &coverUrl)
{
    m_musicId = id;
    m_titleLabel->setText(title.isEmpty() ? I18n::instance().tr("unknown") : title);
    m_artistLabel->setText(artist.isEmpty() ? I18n::instance().tr("unknownArtist") : artist);
    m_albumLabel->setText(album);

    if (!coverUrl.isEmpty()) {
        m_coverUrl = coverUrl;
        loadCover(coverUrl);
    }
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

void PlayerPage::loadCover(const QString &url)
{
    if (url.isEmpty()) return;
    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    QNetworkReply *reply = nam->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, nam]() {
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pm;
            if (pm.loadFromData(reply->readAll())) {
                QPixmap rounded(320, 320);
                rounded.fill(Qt::transparent);
                QPainter p(&rounded);
                p.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addRoundedRect(0, 0, 320, 320, 32, 32);
                p.setClipPath(path);
                p.drawPixmap(0, 0, pm.scaled(320, 320, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_coverLabel->setPixmap(rounded);
            }
        }
        reply->deleteLater();
        nam->deleteLater();
    });
}

void PlayerPage::loadLyrics(int musicId)
{
    m_lyrics.clear();
    m_currentLyricLine = -1;
    rebuildLyricLabels();

    if (musicId <= 0) return;

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
        ).arg(Theme::kTextMuted));
        m_lyricsLayout->addWidget(noData);

        auto *noDataLabel = new QLabel(I18n::instance().tr("noLyrics"), m_lyricsContainer);
        noDataLabel->setAlignment(Qt::AlignCenter);
        noDataLabel->setObjectName("noLyricsText");
        noDataLabel->setStyleSheet(QString::fromUtf8(
            "color: %1; font-size: 14px; background: transparent;"
        ).arg(Theme::kTextMuted));
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
        ).arg(Theme::kTextMuted));
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
            ).arg(Theme::kTextMuted));
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
        ).arg(isCurrent ? Theme::kLavender : Theme::kTextMuted)
         .arg(isCurrent ? 17 : 15)
         .arg(isCurrent ? "bold" : "normal")
         .arg(isCurrent ? "rgba(196,167,231,20)" : "transparent"));

        if (transLabel) {
            transLabel->setStyleSheet(QString::fromUtf8(
                "color: %1; font-size: %2; "
                "background: transparent; border-radius: 6px; padding: 4px 10px;"
            ).arg(isCurrent ? Theme::kLavenderLt : Theme::kTextMuted)
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
