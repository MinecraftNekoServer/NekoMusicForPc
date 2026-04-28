#include "playerpage.h"
#include "../core/playerengine.h"
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
    p.fillRect(rect(), QColor(26, 22, 37));
    QWidget::paintEvent(event);
}

void PlayerPage::setupUi()
{
    setObjectName("playerPage");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 12, 40, 40);
    mainLayout->setSpacing(16);

    // Back button
    m_backBtn = new QPushButton(QString::fromUtf8("\xe2\x86\x90"), this);
    m_backBtn->setFixedSize(40, 40);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setObjectName("playerBackBtn");
    mainLayout->addWidget(m_backBtn, 0, Qt::AlignLeft);
    connect(m_backBtn, &QPushButton::clicked, this, [this]() { emit backRequested(); });

    // Top row: cover (left) + info & lyrics (right)
    auto *topRow = new QHBoxLayout();
    topRow->setSpacing(40);

    // Left: cover
    auto *coverCol = new QVBoxLayout();
    coverCol->setSpacing(16);
    coverCol->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_coverLabel = new QLabel(this);
    m_coverLabel->setFixedSize(280, 280);
    m_coverLabel->setScaledContents(false);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setObjectName("playerCoverLabel");

    m_titleLabel = new QLabel(QStringLiteral("Unknown"), this);
    m_titleLabel->setObjectName("playerSongTitleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setWordWrap(true);

    m_artistLabel = new QLabel(QStringLiteral("Unknown Artist"), this);
    m_artistLabel->setObjectName("playerArtistLabel");
    m_artistLabel->setAlignment(Qt::AlignCenter);

    m_albumLabel = new QLabel();
    m_albumLabel->setObjectName("playerAlbumLabel");
    m_albumLabel->setAlignment(Qt::AlignCenter);

    coverCol->addWidget(m_coverLabel);
    coverCol->addWidget(m_titleLabel);
    coverCol->addWidget(m_artistLabel);
    coverCol->addWidget(m_albumLabel);
    coverCol->addStretch();

    // Right: lyrics
    m_lyricsScroll = new QScrollArea(this);
    m_lyricsScroll->setObjectName("lyricsScroll");
    m_lyricsScroll->setWidgetResizable(true);
    m_lyricsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_lyricsScroll->setMinimumWidth(300);

    m_lyricsContainer = new QWidget();
    m_lyricsLayout = new QVBoxLayout(m_lyricsContainer);
    m_lyricsLayout->setAlignment(Qt::AlignTop);
    m_lyricsLayout->setSpacing(10);
    m_lyricsLayout->setContentsMargins(20, 30, 20, 30);

    m_lyricsScroll->setWidget(m_lyricsContainer);

    topRow->addLayout(coverCol, 0);
    topRow->addWidget(m_lyricsScroll, 1);

    mainLayout->addLayout(topRow, 1);

    // Style
    setStyleSheet(QString::fromUtf8(
        "#playerPage { background: transparent; }"
        "#playerBackBtn { "
        "  background: transparent; color: %1; font-size: 22px; "
        "  border: none; border-radius: 20px; }"
        "#playerBackBtn:hover { background: rgba(196,167,231,50); }"
        "#playerCoverLabel { background: transparent; border-radius: 28px; }"
        "#playerSongTitleLabel { "
        "  color: %1; font-size: 24px; font-weight: bold; background: transparent; }"
        "#playerArtistLabel { "
        "  color: %2; font-size: 16px; background: transparent; }"
        "#playerAlbumLabel { "
        "  color: %3; font-size: 13px; background: transparent; }"
        "#lyricsScroll { "
        "  background: transparent; border: none; }"
        "#lyricsScroll > QWidget { background: transparent; }"
        "QScrollBar:vertical { width: 4px; background: transparent; }"
        "QScrollBar::handle:vertical { "
        "  background: rgba(196,167,231,80); border-radius: 2px; min-height: 40px; }"
    ).arg(Theme::kLavender, Theme::kTextSub, Theme::kTextMuted));
}

void PlayerPage::setMusicInfo(int id, const QString &title, const QString &artist,
                              const QString &album, const QString &coverUrl)
{
    m_musicId = id;
    m_titleLabel->setText(title.isEmpty() ? QStringLiteral("Unknown") : title);
    m_artistLabel->setText(artist.isEmpty() ? QStringLiteral("Unknown Artist") : artist);
    m_albumLabel->setText(album);

    if (!coverUrl.isEmpty()) {
        m_coverUrl = coverUrl;
        loadCover(coverUrl);
    }
}

void PlayerPage::retranslate()
{
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
                QPixmap rounded(280, 280);
                rounded.fill(Qt::transparent);
                QPainter p(&rounded);
                p.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addRoundedRect(0, 0, 280, 280, 28, 28);
                p.setClipPath(path);
                p.drawPixmap(0, 0, pm.scaled(280, 280, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
    connect(reply, &QNetworkReply::finished, this, [this, reply, nam]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = doc.object();
            if (obj.value("success").toBool()) {
                QString lrc = obj.value("data").toString();
                if (!lrc.isEmpty()) {
                    parseLrc(lrc);
                    rebuildLyricLabels();
                }
            }
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

        // Match [mm:ss.xx] or [mm:ss.xxx]
        static const QRegularExpression timeRe(R"(\[(\d{1,2}):(\d{1,2})\.(\d{2,3})\])");
        auto match = timeRe.match(line);
        if (!match.hasMatch()) continue;

        int min = match.captured(1).toInt();
        int sec = match.captured(2).toInt();
        int ms = match.captured(3).toInt();
        // 2-digit ms: divide by 100, 3-digit: divide by 1000
        if (match.captured(3).length() == 2) ms *= 10;
        qint64 timeMs = (min * 60 + sec) * 1000 + ms;

        QString text = line.replace(match.captured(0), "").trimmed();

        // Check next line for translation {"..."}
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

    // Sort by time
    std::sort(m_lyrics.begin(), m_lyrics.end(),
              [](const LyricLine &a, const LyricLine &b) { return a.time < b.time; });
}

void PlayerPage::rebuildLyricLabels()
{
    // Clear existing
    QLayoutItem *item;
    while ((item = m_lyricsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (m_lyrics.isEmpty()) {
        auto *noData = new QLabel(QString::fromUtf8("\xf0\x9f\x8e\xb5"), m_lyricsContainer);
        noData->setAlignment(Qt::AlignCenter);
        noData->setObjectName("noLyricsIcon");
        noData->setStyleSheet(QString::fromUtf8(
            "color: %1; font-size: 48px; background: transparent;"
        ).arg(Theme::kTextMuted));
        m_lyricsLayout->addWidget(noData);

        auto *noDataLabel = new QLabel(tr("No Lyrics"), m_lyricsContainer);
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
        auto *lineLayout = new QVBoxLayout(lineWidget);
        lineLayout->setContentsMargins(0, 0, 0, 0);
        lineLayout->setSpacing(2);

        auto *textLabel = new QLabel(m_lyrics[i].text, lineWidget);
        textLabel->setAlignment(Qt::AlignCenter);
        textLabel->setObjectName("lyricText");
        textLabel->setProperty("lyricIndex", i);
        textLabel->setWordWrap(true);
        textLabel->setStyleSheet(QString::fromUtf8(
            "color: %1; font-size: 15px; background: transparent; padding: 2px 0;"
        ).arg(Theme::kTextMuted));
        lineLayout->addWidget(textLabel);

        if (!m_lyrics[i].translation.isEmpty()) {
            auto *transLabel = new QLabel(m_lyrics[i].translation, lineWidget);
            transLabel->setAlignment(Qt::AlignCenter);
            transLabel->setObjectName("lyricTranslation");
            transLabel->setProperty("lyricIndex", i);
            transLabel->setWordWrap(true);
            transLabel->setStyleSheet(QString::fromUtf8(
                "color: %1; font-size: 12px; background: transparent; padding: 0 0 6px 0;"
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

    // Update all labels
    for (int i = 0; i < m_lyricsLayout->count(); ++i) {
        QLayoutItem *layoutItem = m_lyricsLayout->itemAt(i);
        if (!layoutItem) continue;
        auto *widget = qobject_cast<QWidget *>(layoutItem->widget());
        if (!widget) {
            // Could be a stretch, skip
            continue;
        }
        auto *textLabel = widget->findChild<QLabel *>("lyricText");
        auto *transLabel = widget->findChild<QLabel *>("lyricTranslation");
        if (!textLabel) continue;

        int idx = textLabel->property("lyricIndex").toInt();
        bool isCurrent = (idx == line);

        textLabel->setStyleSheet(QString::fromUtf8(
            "color: %1; font-size: %2; font-weight: %3; background: transparent; padding: 2px 0;"
        ).arg(isCurrent ? Theme::kLavender : Theme::kTextMuted)
         .arg(isCurrent ? 18 : 15)
         .arg(isCurrent ? "bold" : "normal"));

        if (transLabel) {
            transLabel->setStyleSheet(QString::fromUtf8(
                "color: %1; font-size: %2; background: transparent; padding: 0 0 6px 0;"
            ).arg(isCurrent ? Theme::kLavenderLt : Theme::kTextMuted)
             .arg(isCurrent ? 14 : 12));
        }
    }

    // Auto-scroll to current line
    if (line >= 0) {
        QLayoutItem *layoutItem = m_lyricsLayout->itemAt(line);
        if (layoutItem && layoutItem->widget()) {
            int y = layoutItem->widget()->y();
            auto *scrollBar = m_lyricsScroll->verticalScrollBar();
            scrollBar->setValue(y - m_lyricsScroll->height() / 3);
        }
    }
}
