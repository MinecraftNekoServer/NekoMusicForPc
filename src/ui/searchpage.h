#pragma once

/**
 * @file searchpage.h
 * @brief 搜索页面
 */

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include "core/musicinfo.h"

class MusicListPage;

class SearchPage : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPage(QWidget *parent = nullptr);
    void search(const QString &query);
    void retranslate();

signals:
    void backRequested();
    void playMusic(const MusicInfo &info);

private:
    void setupUi();
    void clearResults();
    void loadSearchResults(const QString &query);

    QNetworkAccessManager *m_nam = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_resultsLabel = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QWidget *m_contentWidget = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QString m_currentQuery;
};