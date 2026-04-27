#pragma once

#include <QWidget>

class QLabel;

class RecentPage : public QWidget
{
    Q_OBJECT

public:
    explicit RecentPage(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    QLabel *m_titleLabel = nullptr;
};
