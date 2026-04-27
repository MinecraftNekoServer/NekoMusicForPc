#pragma once

/**
 * @file sidebar.h
 * @brief 侧边栏导航 — 日系动漫风
 *
 * 240px 紧凑宽度，重度毛玻璃。
 * 选中项薰衣草紫背景 + 左侧薄荷绿竖条指示器。
 */

#include <QWidget>
#include <QMap>
#include <QIcon>

class QPushButton;

class Sidebar : public QWidget
{
    Q_OBJECT

public:
    explicit Sidebar(QWidget *parent = nullptr);
    void setActiveNav(const QString &key);
    void retranslate();

signals:
    void navigationRequested(const QString &key);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void setupUi();
    QPushButton *createNavItem(const QString &key, const QString &label, const QIcon &icon);

    QMap<QString, QPushButton *> m_navBtns;
    QString m_activeKey;
};
