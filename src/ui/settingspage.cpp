/**
 * @file settingspage.cpp
 * @brief 设置页面实现
 */

#include "settingspage.h"
#include "core/i18n.h"
#include "theme/theme.h"
#include "ui/glasswidget.h"
#include "version.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QSettings>
#include <QCheckBox>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setupUi();
}

void SettingsPage::setupUi()
{
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scroll);
    auto *lay = new QVBoxLayout(container);
    lay->setContentsMargins(32, 32, 32, 32);
    lay->setSpacing(16);

    auto *card = new GlassWidget(container);
    card->setBorderRadius(Theme::kRXl);
    card->setOpacity(0.5);

    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(24, 20, 24, 20);
    cardLay->setSpacing(16);

    // 语言设置
    auto *langRow = new QHBoxLayout();
    m_langLabel = new QLabel(I18n::instance().languageLabel(), card);
    m_langLabel->setObjectName("settingsLabel");
    langRow->addWidget(m_langLabel);
    langRow->addStretch();

    m_langCombo = new QComboBox(card);
    m_langCombo->setObjectName("settingsCombo");
    m_langCombo->blockSignals(true);
    m_langCombo->addItem(I18n::instance().languageChinese(), I18n::ZhCN);
    m_langCombo->addItem(I18n::instance().languageNya(), I18n::NyaCN);
    m_langCombo->addItem(I18n::instance().languageEnglish(), I18n::EnUS);

    // 恢复保存的语言设置
    QSettings settings;
    I18n::Language savedLang = static_cast<I18n::Language>(
        settings.value("language", static_cast<int>(I18n::ZhCN)).toInt());
    I18n::instance().setLanguage(savedLang);
    int idx = (savedLang == I18n::ZhCN) ? 0 : (savedLang == I18n::NyaCN) ? 1 : 2;
    m_langCombo->setCurrentIndex(idx);

    m_langCombo->blockSignals(false);
    connect(m_langCombo, QOverload<int>::of(&QComboBox::activated), this, [this](int index) {
        auto lang = static_cast<I18n::Language>(m_langCombo->itemData(index).toInt());
        I18n::instance().setLanguage(lang);
        // 保存设置
        QSettings settings;
        settings.setValue("language", static_cast<int>(lang));
        retranslate();
        emit languageChanged(lang);
    });
    langRow->addWidget(m_langCombo);
    cardLay->addLayout(langRow);

    // 分隔线
    auto *line = new QFrame(card);
    line->setFrameShape(QFrame::HLine);
    line->setObjectName("settingsDivider");
    cardLay->addWidget(line);

    // 桌面歌词设置
    auto *lyricsRow = new QHBoxLayout();
    QLabel *lyricsLabel = new QLabel(I18n::instance().tr("desktopLyrics"), card);
    lyricsLabel->setObjectName("settingsLabel");
    lyricsRow->addWidget(lyricsLabel);
    lyricsRow->addStretch();

    m_desktopLyricsCheck = new QCheckBox(card);
    m_desktopLyricsCheck->setObjectName("settingsCheck");
    
    // 恢复保存的桌面歌词设置
    bool lyricsEnabled = settings.value("desktopLyrics", true).toBool();
    m_desktopLyricsCheck->setChecked(lyricsEnabled);
    
    connect(m_desktopLyricsCheck, &QCheckBox::toggled, this, [this](bool checked) {
        QSettings settings;
        settings.setValue("desktopLyrics", checked);
        emit desktopLyricsChanged(checked);
    });
    
    lyricsRow->addWidget(m_desktopLyricsCheck);
    cardLay->addLayout(lyricsRow);

    // 分隔线
    auto *line2 = new QFrame(card);
    line2->setFrameShape(QFrame::HLine);
    line2->setObjectName("settingsDivider");
    cardLay->addWidget(line2);

    // 自动恢复播放设置
    auto *autoPlayRow = new QHBoxLayout();
    QLabel *autoPlayLabel = new QLabel(I18n::instance().tr("autoResumePlayback"), card);
    autoPlayLabel->setObjectName("settingsLabel");
    autoPlayRow->addWidget(autoPlayLabel);
    autoPlayRow->addStretch();

    m_autoResumeCheck = new QCheckBox(card);
    m_autoResumeCheck->setObjectName("settingsCheck");
    
    // 恢复保存的自动恢复播放设置（默认关闭）
    bool autoResumeEnabled = settings.value("autoResumePlayback", false).toBool();
    m_autoResumeCheck->setChecked(autoResumeEnabled);
    
    connect(m_autoResumeCheck, &QCheckBox::toggled, this, [this](bool checked) {
        QSettings settings;
        settings.setValue("autoResumePlayback", checked);
    });
    
    autoPlayRow->addWidget(m_autoResumeCheck);
    cardLay->addLayout(autoPlayRow);

    // 分隔线
    auto *line3 = new QFrame(card);
    line3->setFrameShape(QFrame::HLine);
    line3->setObjectName("settingsDivider");
    cardLay->addWidget(line3);

    // 版本 & 系统
    m_versionLabel = new QLabel(QString("%1: %2").arg(I18n::instance().version(), QString::fromUtf8(APP_VERSION)), card);
    m_versionLabel->setObjectName("settingsInfo");
    cardLay->addWidget(m_versionLabel);

    m_systemLabel = new QLabel(QString("%1: %2").arg(I18n::instance().system()).arg(QSysInfo::prettyProductName()), card);
    m_systemLabel->setObjectName("settingsInfo");
    cardLay->addWidget(m_systemLabel);

    // 检查更新按钮
    m_checkUpdateBtn = new QPushButton(I18n::instance().tr("checkForUpdates"), card);
    m_checkUpdateBtn->setObjectName("checkUpdateBtn");
    m_checkUpdateBtn->setFixedHeight(40);
    m_checkUpdateBtn->setCursor(Qt::PointingHandCursor);
    m_checkUpdateBtn->setStyleSheet(
        "QPushButton#checkUpdateBtn { "
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #667eea, stop:1 #764ba2); "
        "  color: white; "
        "  border: none; "
        "  border-radius: 8px; "
        "  font-size: 14px; "
        "  font-weight: 600; "
        "}"
        "QPushButton#checkUpdateBtn:hover { opacity: 0.9; }"
        "QPushButton#checkUpdateBtn:pressed { opacity: 0.8; }"
        "QPushButton#checkUpdateBtn:disabled { opacity: 0.6; }"
    );
    connect(m_checkUpdateBtn, &QPushButton::clicked, this, &SettingsPage::checkForUpdatesRequested);
    cardLay->addWidget(m_checkUpdateBtn);

    cardLay->addStretch();
    lay->addWidget(card);
    lay->addStretch();

    scroll->setWidget(container);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(scroll);
}

void SettingsPage::retranslate()
{
    m_langLabel->setText(I18n::instance().languageLabel());
    m_langCombo->setItemText(0, I18n::instance().languageChinese());
    m_langCombo->setItemText(1, I18n::instance().languageNya());
    m_langCombo->setItemText(2, I18n::instance().languageEnglish());
    m_versionLabel->setText(QString("%1: %2").arg(I18n::instance().version(), QString::fromUtf8(APP_VERSION)));
    m_systemLabel->setText(QString("%1: %2").arg(I18n::instance().system()).arg(QSysInfo::prettyProductName()));
}

void SettingsPage::paintEvent(QPaintEvent *) {}
