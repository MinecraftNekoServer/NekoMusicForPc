#pragma once

#include <QWidget>
#include <QList>

#include "core/musicinfo.h"

class QVBoxLayout;
class QScrollArea;

class RecentPage : public QWidget
{
    Q_OBJECT

public:
    explicit RecentPage(QWidget *parent = nullptr);

    void retranslate();
    void refresh();

signals:
    void playRequested(int musicId, const QString& title, const QString& artist, const QString& coverUrl);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    void loadRecentPlays();

    QVBoxLayout *m_mainLay = nullptr;
    QScrollArea *m_scroll = nullptr;
    QWidget *m_container = nullptr;
    QVBoxLayout *m_listLay = nullptr;
};
