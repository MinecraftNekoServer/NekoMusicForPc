#include "playerpage.h"
#include "../core/playerengine.h"
#include "../theme/theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QNetworkReply>
#include <QResizeEvent>

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
    p.fillRect(rect(), QColor(26, 22, 37)); // Theme::kBgDeep fully opaque
    QWidget::paintEvent(event);
}

void PlayerPage::setupUi()
{
    setAutoFillBackground(true);
    setObjectName("playerPage");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 12, 40, 40);
    mainLayout->setSpacing(20);

    // Back button
    m_backBtn = new QPushButton(QString::fromUtf8("\xe2\x86\x90"), this);
    m_backBtn->setFixedSize(40, 40);
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setObjectName("playerBackBtn");
    mainLayout->addWidget(m_backBtn, 0, Qt::AlignLeft);
    connect(m_backBtn, &QPushButton::clicked, this, [this]() { emit backRequested(); });

    // Center content
    auto *centerLayout = new QVBoxLayout();
    centerLayout->setSpacing(16);
    centerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Large cover
    m_coverLabel = new QLabel(this);
    m_coverLabel->setFixedSize(280, 280);
    m_coverLabel->setScaledContents(false);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setObjectName("playerCoverLabel");

    // Song info
    m_titleLabel = new QLabel(QStringLiteral("Unknown"), this);
    m_titleLabel->setObjectName("playerSongTitleLabel");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setMaximumWidth(500);
    m_titleLabel->setWordWrap(true);

    m_artistLabel = new QLabel(QStringLiteral("Unknown Artist"), this);
    m_artistLabel->setObjectName("playerArtistLabel");
    m_artistLabel->setAlignment(Qt::AlignCenter);

    m_albumLabel = new QLabel();
    m_albumLabel->setObjectName("playerAlbumLabel");
    m_albumLabel->setAlignment(Qt::AlignCenter);

    // Assemble
    centerLayout->addWidget(m_coverLabel);
    centerLayout->addWidget(m_titleLabel);
    centerLayout->addWidget(m_artistLabel);
    centerLayout->addWidget(m_albumLabel);

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();

    // Style
    setStyleSheet(QString::fromUtf8(
        "#playerPage { background: %1; }"
        "#playerBackBtn { "
        "  background: transparent; color: %2; font-size: 22px; "
        "  border: none; border-radius: 20px; }"
        "#playerBackBtn:hover { background: rgba(196,167,231,50); }"
        "#playerCoverLabel { background: transparent; border-radius: 28px; }"
        "#playerSongTitleLabel { "
        "  color: %2; font-size: 26px; font-weight: bold; background: transparent; }"
        "#playerArtistLabel { "
        "  color: %3; font-size: 17px; background: transparent; }"
        "#playerAlbumLabel { "
        "  color: %4; font-size: 14px; background: transparent; }"
    ).arg(Theme::kBgDeep, Theme::kLavender, Theme::kTextSub, Theme::kTextMuted));
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
