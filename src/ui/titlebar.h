#pragma once

/**
 * @file titlebar.h
 * @brief 自定义标题栏 — 日系动漫风
 *
 * 56px 紧凑高度，毛玻璃背景。
 * 搜索框聚焦薄荷绿发光，用户头像樱花粉边框。
 */

#include <QWidget>

class QLineEdit;
class QLabel;

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    void retranslate();

signals:
    void searchRequested(const QString &query);
    void settingsClicked();

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void paintEvent(QPaintEvent *) override;

private:
    QLineEdit *m_search = nullptr;
    QLabel *m_logo = nullptr;
    QLabel *m_name = nullptr;

    bool m_dragging = false;
    QPoint m_dragPos;
};
