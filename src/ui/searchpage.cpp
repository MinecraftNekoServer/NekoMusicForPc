/**
 * @file searchpage.cpp
 * @brief 搜索页面实现
 */

#include "searchpage.h"
#include "ui/musiclistpage.h"
#include "core/apiclient.h"
#include "core/i18n.h"
#include "theme/theme.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

SearchPage::SearchPage(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
}

void SearchPage::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(48, 20, 48, 48);
    mainLayout->setSpacing(0);

    // 顶部栏：返回按钮
    auto *topBar = new QHBoxLayout();
    topBar->setContentsMargins(0, 0, 0, 24);

    auto *backBtn = new QPushButton(this);
    backBtn->setFixedSize(44, 44);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setObjectName("searchBackBtn");
    backBtn->setText(QString::fromUtf8("\xe2\x86\x90"));
    connect(backBtn, &QPushButton::clicked, this, [this]() { emit backRequested(); });

    topBar->addWidget(backBtn);
    topBar->addStretch();
    mainLayout->addLayout(topBar);

    // 标题
    m_titleLabel = new QLabel(I18n::instance().tr("searchResults"), this);
    m_titleLabel->setObjectName("searchTitleLabel");
    m_titleLabel->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(m_titleLabel);

    // 结果数量标签
    m_resultsLabel = new QLabel(this);
    m_resultsLabel->setObjectName("searchResultsLabel");
    m_resultsLabel->setAlignment(Qt::AlignLeft);
    mainLayout->addWidget(m_resultsLabel);

    // 滚动区域
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("searchScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    m_contentWidget = new QWidget();
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 16, 0, 0);
    m_contentLayout->setSpacing(16);

    m_scrollArea->setWidget(m_contentWidget);
    mainLayout->addWidget(m_scrollArea, 1);

    // 初始化网络管理器
    m_nam = new QNetworkAccessManager(this);
}

void SearchPage::search(const QString &query)
{
    m_currentQuery = query;
    clearResults();
    loadSearchResults(query);
}

void SearchPage::clearResults()
{
    QLayoutItem *item;
    while ((item = m_contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    m_resultsLabel->setText("");
}

void SearchPage::loadSearchResults(const QString &query)
{
    if (query.isEmpty()) return;

    QString url = QString::fromUtf8("%1/api/music/search?q=%2").arg(Theme::kApiBase).arg(QUrl::toPercentEncoding(query));
    QNetworkReply *reply = m_nam->get(QNetworkRequest(QUrl(url)));
    connect(reply, &QNetworkReply::finished, this, [this, reply, query]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(response);
            QJsonObject obj = doc.object();
            bool success = obj.value("success").toBool();
            if (success) {
                QJsonArray data = obj.value("data").toArray();
                int count = data.size();
                m_resultsLabel->setText(I18n::instance().tr("searchResultsCount").arg(count).arg(query));

                for (const QJsonValue &value : data) {
                    QJsonObject musicObj = value.toObject();
                    MusicInfo info;
                    info.id = musicObj.value("id").toInt();
                    info.title = musicObj.value("title").toString();
                    info.artist = musicObj.value("artist").toString();
                    info.album = musicObj.value("album").toString();
                    info.duration = musicObj.value("duration").toInt();
                    info.coverUrl = musicObj.value("coverUrl").toString();

                    // 创建简单的音乐项
                    auto *itemWidget = new QPushButton(m_contentWidget);
                    itemWidget->setFixedHeight(60);
                    itemWidget->setCursor(Qt::PointingHandCursor);
                    itemWidget->setStyleSheet(
                        "QPushButton { "
                        "  text-align: left; "
                        "  background: transparent; "
                        "  border: none; "
                        "  padding: 8px 12px; "
                        "}"
                        "QPushButton:hover { "
                        "  background: rgba(102, 126, 234, 0.1); "
                        "}"
                    );
                    
                    auto *layout = new QHBoxLayout(itemWidget);
                    layout->setContentsMargins(0, 0, 0, 0);
                    
                    auto *titleLabel = new QLabel(info.title, itemWidget);
                    auto *artistLabel = new QLabel(info.artist, itemWidget);
                    artistLabel->setStyleSheet("color: rgba(255, 255, 255, 0.7);");
                    
                    layout->addWidget(titleLabel);
                    layout->addStretch();
                    layout->addWidget(artistLabel);
                    
                    // 点击播放
                    connect(itemWidget, &QPushButton::clicked, this, [this, info]() {
                        emit playMusic(info);
                    });
                    
                    m_contentLayout->addWidget(itemWidget);
                }
            } else {
                m_resultsLabel->setText(I18n::instance().tr("searchFailed").arg(obj.value("message").toString()));
            }
        } else {
            m_resultsLabel->setText(I18n::instance().tr("networkError").arg(reply->errorString()));
        }
        reply->deleteLater();
    });
}

void SearchPage::retranslate()
{
    if (m_titleLabel) {
        m_titleLabel->setText(I18n::instance().tr("searchResults"));
    }
    if (!m_currentQuery.isEmpty()) {
        search(m_currentQuery);
    }
}