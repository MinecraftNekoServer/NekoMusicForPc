/**
 * @file uploadpage.cpp
 * @brief 上传音乐页面实现
 */

#include "uploadpage.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "core/usermanager.h"
#include "theme/theme.h"
#include "ui/glasswidget.h"
#include "ui/svgicon.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QFile>

UploadPage::UploadPage(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    m_api = new ApiClient(this);
    setupUi();
}

void UploadPage::setupUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scroll);
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(32, 24, 32, 32);
    lay->setSpacing(20);

    // Header: back + title
    auto *header = new QHBoxLayout();
    header->setSpacing(12);

    m_backBtn = new QPushButton(container);
    m_backBtn->setIcon(Icons::render(Icons::kLeft, 24, QColor(Theme::kLavender)));
    m_backBtn->setCursor(Qt::PointingHandCursor);
    m_backBtn->setFixedSize(32, 32);
    m_backBtn->setObjectName("backBtn");
    m_backBtn->setFlat(true);
    connect(m_backBtn, &QPushButton::clicked, this, [this]() {
        emit backRequested();
    });
    header->addWidget(m_backBtn);

    m_titleLabel = new QLabel(I18n::instance().tr("uploadMusicTitle"), container);
    m_titleLabel->setObjectName("pageTitle");
    header->addWidget(m_titleLabel);
    header->addStretch();
    lay->addLayout(header);

    // Description
    m_descLabel = new QLabel(I18n::instance().tr("uploadMusicDesc"), container);
    m_descLabel->setObjectName("pageDesc");
    lay->addWidget(m_descLabel);

    // Card
    auto *card = new GlassWidget(container);
    card->setBorderRadius(Theme::kRXl);
    card->setOpacity(0.5);

    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(24, 20, 24, 20);
    cardLay->setSpacing(16);

    // ─── File selection section ───
    // Music file
    auto *musicRow = new QHBoxLayout();
    auto *musicIcon = new QLabel(card);
    musicIcon->setPixmap(Icons::render(Icons::kMusic, 20, QColor(Theme::kLavender)));
    musicRow->addWidget(musicIcon);

    auto *musicLabel = new QLabel(I18n::instance().tr("musicFileInfo") + " " + I18n::instance().tr("required"), card);
    musicLabel->setObjectName("settingsLabel");
    musicRow->addWidget(musicLabel);
    musicRow->addStretch();

    m_musicFileBtn = new QPushButton(I18n::instance().tr("selectMusicFile"), card);
    m_musicFileBtn->setObjectName("smallBtn");
    m_musicFileBtn->setFixedWidth(140);
    connect(m_musicFileBtn, &QPushButton::clicked, this, &UploadPage::selectMusicFile);
    musicRow->addWidget(m_musicFileBtn);
    cardLay->addLayout(musicRow);

    m_musicFileLabel = new QLabel(I18n::instance().tr("musicFileFormat"), card);
    m_musicFileLabel->setObjectName("settingsInfo");
    cardLay->addWidget(m_musicFileLabel);

    // Cover file with preview
    auto *coverRow = new QHBoxLayout();
    auto *coverIcon = new QLabel(card);
    coverIcon->setPixmap(Icons::render(Icons::kHeart, 20, QColor(Theme::kSakura)));
    coverRow->addWidget(coverIcon);

    auto *coverLabel = new QLabel(I18n::instance().tr("coverInfo") + " " + I18n::instance().tr("optional"), card);
    coverLabel->setObjectName("settingsLabel");
    coverRow->addWidget(coverLabel);
    coverRow->addStretch();

    m_coverFileBtn = new QPushButton(I18n::instance().tr("selectCoverFile"), card);
    m_coverFileBtn->setObjectName("smallBtn");
    m_coverFileBtn->setFixedWidth(140);
    connect(m_coverFileBtn, &QPushButton::clicked, this, &UploadPage::selectCoverFile);
    coverRow->addWidget(m_coverFileBtn);
    cardLay->addLayout(coverRow);

    // Cover preview
    m_coverPreview = new QLabel(card);
    m_coverPreview->setObjectName("coverPreview");
    m_coverPreview->setFixedSize(120, 120);
    m_coverPreview->setAlignment(Qt::AlignCenter);
    m_coverPreview->setStyleSheet(
        "QLabel#coverPreview { background-color: rgba(45, 38, 65, 100); border-radius: 8px; }"
    );
    // Set default placeholder
    m_coverPreview->setPixmap(Icons::render(Icons::kMusic, 40, QColor(Theme::kTextMuted)));
    cardLay->addWidget(m_coverPreview, 0, Qt::AlignLeft);

    // Lyrics file with preview
    auto *lyricsRow = new QHBoxLayout();
    auto *lyricsIcon = new QLabel(card);
    lyricsIcon->setPixmap(Icons::render(Icons::kSearch, 20, QColor(Theme::kMint)));
    lyricsRow->addWidget(lyricsIcon);

    auto *lyricsLabel = new QLabel(I18n::instance().tr("lyricsInfo") + " " + I18n::instance().tr("optional"), card);
    lyricsLabel->setObjectName("settingsLabel");
    lyricsRow->addWidget(lyricsLabel);
    lyricsRow->addStretch();

    m_lyricsFileBtn = new QPushButton(I18n::instance().tr("selectLyricsFile"), card);
    m_lyricsFileBtn->setObjectName("smallBtn");
    m_lyricsFileBtn->setFixedWidth(140);
    connect(m_lyricsFileBtn, &QPushButton::clicked, this, &UploadPage::selectLyricsFile);
    lyricsRow->addWidget(m_lyricsFileBtn);
    cardLay->addLayout(lyricsRow);

    // Lyrics preview
    m_lyricsPreview = new QTextEdit(card);
    m_lyricsPreview->setObjectName("lyricsPreview");
    m_lyricsPreview->setReadOnly(true);
    m_lyricsPreview->setMaximumHeight(150);
    m_lyricsPreview->setPlaceholderText(I18n::instance().tr("lyricsFileFormat"));
    cardLay->addWidget(m_lyricsPreview);

    // Divider
    auto *line1 = new QFrame(card);
    line1->setFrameShape(QFrame::HLine);
    line1->setObjectName("settingsDivider");
    cardLay->addWidget(line1);

    // ─── Basic info section ───
    auto *basicTitle = new QLabel(I18n::instance().tr("basicInfo"), card);
    basicTitle->setObjectName("settingsLabel");
    cardLay->addWidget(basicTitle);

    // Title
    auto *titleRow = new QHBoxLayout();
    auto *titleLabel = new QLabel(I18n::instance().tr("musicTitle") + " " + I18n::instance().tr("required"), card);
    titleLabel->setObjectName("settingsLabel");
    titleLabel->setFixedWidth(120);
    titleRow->addWidget(titleLabel);

    m_titleEdit = new QLineEdit(card);
    m_titleEdit->setObjectName("settingsInput");
    m_titleEdit->setPlaceholderText(I18n::instance().tr("titleRequired"));
    titleRow->addWidget(m_titleEdit);
    cardLay->addLayout(titleRow);

    // Artist
    auto *artistRow = new QHBoxLayout();
    auto *artistLabel = new QLabel(I18n::instance().tr("artistName") + " " + I18n::instance().tr("required"), card);
    artistLabel->setObjectName("settingsLabel");
    artistLabel->setFixedWidth(120);
    artistRow->addWidget(artistLabel);

    m_artistEdit = new QLineEdit(card);
    m_artistEdit->setObjectName("settingsInput");
    m_artistEdit->setPlaceholderText(I18n::instance().tr("artistRequired"));
    artistRow->addWidget(m_artistEdit);
    cardLay->addLayout(artistRow);

    // Language
    auto *langRow = new QHBoxLayout();
    auto *langLabel = new QLabel(I18n::instance().tr("uploadLanguageLabel") + " " + I18n::instance().tr("required"), card);
    langLabel->setObjectName("settingsLabel");
    langLabel->setFixedWidth(120);
    langRow->addWidget(langLabel);

    m_langCombo = new QComboBox(card);
    m_langCombo->setObjectName("settingsCombo");
    m_langCombo->addItem(I18n::instance().tr("langChinese"), "中文");
    m_langCombo->addItem(I18n::instance().tr("langCantonese"), "粤语");
    m_langCombo->addItem(I18n::instance().tr("langShanghainese"), "上海语");
    m_langCombo->addItem(I18n::instance().tr("langEnglish"), "英文");
    m_langCombo->addItem(I18n::instance().tr("langJapanese"), "日语");
    m_langCombo->addItem(I18n::instance().tr("langKorean"), "韩语");
    m_langCombo->addItem(I18n::instance().tr("langFrench"), "法语");
    m_langCombo->addItem(I18n::instance().tr("langGerman"), "德语");
    m_langCombo->addItem(I18n::instance().tr("langRussian"), "俄语");
    m_langCombo->addItem(I18n::instance().tr("langInstrumental"), "纯音乐");
    langRow->addWidget(m_langCombo);
    cardLay->addLayout(langRow);

    // Album
    auto *albumRow = new QHBoxLayout();
    auto *albumLabel = new QLabel(I18n::instance().tr("albumName"), card);
    albumLabel->setObjectName("settingsLabel");
    albumLabel->setFixedWidth(120);
    albumRow->addWidget(albumLabel);

    m_albumEdit = new QLineEdit(card);
    m_albumEdit->setObjectName("settingsInput");
    albumRow->addWidget(m_albumEdit);
    cardLay->addLayout(albumRow);

    // Tags
    auto *tagsRow = new QHBoxLayout();
    auto *tagsLabel = new QLabel(I18n::instance().tr("tagsLabel"), card);
    tagsLabel->setObjectName("settingsLabel");
    tagsLabel->setFixedWidth(120);
    tagsRow->addWidget(tagsLabel);

    m_tagsEdit = new QLineEdit(card);
    m_tagsEdit->setObjectName("settingsInput");
    tagsRow->addWidget(m_tagsEdit);
    cardLay->addLayout(tagsRow);

    // Duration
    auto *durationRow = new QHBoxLayout();
    auto *durationLabel = new QLabel(I18n::instance().tr("durationLabel") + " " + I18n::instance().tr("required"), card);
    durationLabel->setObjectName("settingsLabel");
    durationLabel->setFixedWidth(120);
    durationRow->addWidget(durationLabel);

    m_durationEdit = new QLineEdit(card);
    m_durationEdit->setObjectName("settingsInput");
    m_durationEdit->setPlaceholderText(I18n::instance().tr("durationRequired"));
    m_durationEdit->setFixedWidth(120);
    durationRow->addWidget(m_durationEdit);
    durationRow->addStretch();
    cardLay->addLayout(durationRow);

    // Upload note
    auto *noteLabel = new QLabel(I18n::instance().tr("uploadNote"), card);
    noteLabel->setObjectName("settingsInfo");
    cardLay->addWidget(noteLabel);

    // Upload button
    m_uploadBtn = new QPushButton(I18n::instance().tr("uploadBtn"), card);
    m_uploadBtn->setObjectName("primaryBtn");
    m_uploadBtn->setFixedHeight(44);
    m_uploadBtn->setCursor(Qt::PointingHandCursor);
    connect(m_uploadBtn, &QPushButton::clicked, this, &UploadPage::doUpload);
    cardLay->addWidget(m_uploadBtn);

    cardLay->addStretch();
    lay->addWidget(card);
    lay->addStretch();

    scroll->setWidget(container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}

void UploadPage::selectMusicFile()
{
    QString path = QFileDialog::getOpenFileName(this,
        I18n::instance().tr("selectMusicFile"), QString(),
        "Audio Files (*.mp3 *.flac *.wav)");
    if (!path.isEmpty()) {
        m_musicFilePath = path;
        QString name = path.mid(path.lastIndexOf('/') + 1);
        m_musicFileLabel->setText(name);
    }
}

void UploadPage::selectCoverFile()
{
    QString path = QFileDialog::getOpenFileName(this,
        I18n::instance().tr("selectCoverFile"), QString(),
        "Image Files (*.jpg *.jpeg *.png *.gif *.webp *.bmp)");
    if (!path.isEmpty()) {
        m_coverFilePath = path;
        updateCoverPreview();
    }
}

void UploadPage::selectLyricsFile()
{
    QString path = QFileDialog::getOpenFileName(this,
        I18n::instance().tr("selectLyricsFile"), QString(),
        "Lyrics Files (*.lrc)");
    if (!path.isEmpty()) {
        m_lyricsFilePath = path;
        updateLyricsPreview();
    }
}

void UploadPage::updateCoverPreview()
{
    if (m_coverFilePath.isEmpty()) {
        m_coverPreview->setPixmap(Icons::render(Icons::kMusic, 40, QColor(Theme::kTextMuted)));
        return;
    }
    QPixmap pix(m_coverFilePath);
    if (!pix.isNull()) {
        QPixmap scaled = pix.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_coverPreview->setPixmap(scaled);
    } else {
        m_coverPreview->setPixmap(Icons::render(Icons::kMusic, 40, QColor(Theme::kTextMuted)));
    }
}

void UploadPage::updateLyricsPreview()
{
    if (m_lyricsFilePath.isEmpty()) {
        m_lyricsPreview->clear();
        m_lyricsPreview->setPlaceholderText(I18n::instance().tr("lyricsFileFormat"));
        return;
    }
    QFile file(m_lyricsFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QString::fromUtf8(file.readAll());
        m_lyricsPreview->setPlaceholderText(QString());
        m_lyricsPreview->setPlainText(content);
        file.close();
    } else {
        m_lyricsPreview->clear();
        m_lyricsPreview->setPlaceholderText(I18n::instance().tr("lyricsFileFormat"));
    }
}

void UploadPage::doUpload()
{
    if (!UserManager::instance().isLoggedIn()) {
        QMessageBox::warning(this, I18n::instance().tr("uploadFailed"),
                             I18n::instance().tr("uploadUnauthorized"));
        return;
    }

    // Validate required fields
    QString title = m_titleEdit->text().trimmed();
    QString artist = m_artistEdit->text().trimmed();
    QString lang = m_langCombo->currentData().toString();
    QString durationStr = m_durationEdit->text().trimmed();

    if (m_musicFilePath.isEmpty()) {
        QMessageBox::warning(this, I18n::instance().tr("uploadFailed"),
                             I18n::instance().tr("musicFileRequired"));
        return;
    }
    if (title.isEmpty()) {
        QMessageBox::warning(this, I18n::instance().tr("uploadFailed"),
                             I18n::instance().tr("titleRequired"));
        return;
    }
    if (artist.isEmpty()) {
        QMessageBox::warning(this, I18n::instance().tr("uploadFailed"),
                             I18n::instance().tr("artistRequired"));
        return;
    }
    if (lang.isEmpty()) {
        QMessageBox::warning(this, I18n::instance().tr("uploadFailed"),
                             I18n::instance().tr("languageRequired"));
        return;
    }

    bool ok = false;
    int duration = durationStr.toInt(&ok);
    if (!ok || duration <= 0) {
        QMessageBox::warning(this, I18n::instance().tr("uploadFailed"),
                             I18n::instance().tr("durationInvalid"));
        return;
    }

    // Disable upload button during upload
    m_uploadBtn->setEnabled(false);
    m_uploadBtn->setText(I18n::instance().tr("uploading"));

    int userId = UserManager::instance().userInfo().value("id").toInt();

    m_api->uploadMusic(m_musicFilePath, title, artist, lang, duration, userId,
                       m_albumEdit->text().trimmed(),
                       m_tagsEdit->text().trimmed(),
                       m_coverFilePath, m_lyricsFilePath,
                       [this](bool success, const QString &message) {
        // Back to main thread
        QMetaObject::invokeMethod(this, "onUploadResult",
                                  Qt::QueuedConnection,
                                  Q_ARG(bool, success),
                                  Q_ARG(QString, message));
    });
}

void UploadPage::onUploadResult(bool success, const QString &message)
{
    m_uploadBtn->setEnabled(true);
    m_uploadBtn->setText(I18n::instance().tr("uploadBtn"));

    if (success) {
        QMessageBox::information(this, I18n::instance().tr("uploadSuccess"),
                                 I18n::instance().tr("uploadSuccess"));
        // Clear form
        m_titleEdit->clear();
        m_artistEdit->clear();
        m_albumEdit->clear();
        m_tagsEdit->clear();
        m_durationEdit->clear();
        m_musicFilePath.clear();
        m_coverFilePath.clear();
        m_lyricsFilePath.clear();
        m_musicFileLabel->setText(I18n::instance().tr("musicFileFormat"));
        m_coverPreview->setPixmap(Icons::render(Icons::kMusic, 40, QColor(Theme::kTextMuted)));
        m_lyricsPreview->clear();
        m_lyricsPreview->setPlaceholderText(I18n::instance().tr("lyricsFileFormat"));
    } else {
        QString msg = message;
        if (message.contains("重复")) {
            msg = I18n::instance().tr("uploadDuplicate");
        }
        QMessageBox::warning(this, I18n::instance().tr("uploadFailed"), msg);
    }
}

void UploadPage::retranslate()
{
    m_titleLabel->setText(I18n::instance().tr("uploadMusicTitle"));
    m_descLabel->setText(I18n::instance().tr("uploadMusicDesc"));
    m_musicFileBtn->setText(I18n::instance().tr("selectMusicFile"));
    m_coverFileBtn->setText(I18n::instance().tr("selectCoverFile"));
    m_lyricsFileBtn->setText(I18n::instance().tr("selectLyricsFile"));
    m_uploadBtn->setText(I18n::instance().tr("uploadBtn"));

    // Update file format hints if no file selected
    if (m_musicFilePath.isEmpty())
        m_musicFileLabel->setText(I18n::instance().tr("musicFileFormat"));
    if (m_coverFilePath.isEmpty()) {
        m_lyricsPreview->setPlaceholderText(I18n::instance().tr("lyricsFileFormat"));
    }

    // Update placeholders
    m_titleEdit->setPlaceholderText(I18n::instance().tr("titleRequired"));
    m_artistEdit->setPlaceholderText(I18n::instance().tr("artistRequired"));
    m_durationEdit->setPlaceholderText(I18n::instance().tr("durationRequired"));

    // Update language combo - need to rebuild items
    m_langCombo->blockSignals(true);
    m_langCombo->clear();
    m_langCombo->addItem(I18n::instance().tr("langChinese"), "中文");
    m_langCombo->addItem(I18n::instance().tr("langCantonese"), "粤语");
    m_langCombo->addItem(I18n::instance().tr("langShanghainese"), "上海语");
    m_langCombo->addItem(I18n::instance().tr("langEnglish"), "英文");
    m_langCombo->addItem(I18n::instance().tr("langJapanese"), "日语");
    m_langCombo->addItem(I18n::instance().tr("langKorean"), "韩语");
    m_langCombo->addItem(I18n::instance().tr("langFrench"), "法语");
    m_langCombo->addItem(I18n::instance().tr("langGerman"), "德语");
    m_langCombo->addItem(I18n::instance().tr("langRussian"), "俄语");
    m_langCombo->addItem(I18n::instance().tr("langInstrumental"), "纯音乐");
    m_langCombo->blockSignals(false);
}

void UploadPage::paintEvent(QPaintEvent *) {}
