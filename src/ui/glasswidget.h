#pragma once

/**
 * @file glasswidget.h
 * @brief 毛玻璃效果基础容器
 *
 * 重度沉浸式毛玻璃：半透明背景 + 圆角 + 微光边框。
 * 所有需要毛玻璃的面板（侧边栏/播放栏/卡片容器）均可使用。
 */

#include <QWidget>
#include <QColor>

class GlassWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlassWidget(QWidget *parent = nullptr);

    /** 背景不透明度 0~1 */
    qreal opacity() const;
    void setOpacity(qreal v);

    /** 背景基色，默认 #2D263F */
    QColor baseColor() const;
    void setBaseColor(const QColor &c);

    /** 边框颜色 */
    QColor borderColor() const;
    void setBorderColor(const QColor &c);

    /** 圆角 */
    int borderRadius() const;
    void setBorderRadius(int r);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    qreal m_opacity = 0.65;
    QColor m_base{45, 38, 65};           // #2D263F
    QColor m_border{196, 167, 231, 38};  // 薰衣草15%
    int m_radius = 16;
};
