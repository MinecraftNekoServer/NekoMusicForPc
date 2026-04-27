#pragma once

#include <QWidget>

class QLabel;

class FavoritesPage : public QWidget
{
    Q_OBJECT

public:
    explicit FavoritesPage(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    QLabel *m_titleLabel = nullptr;
};
