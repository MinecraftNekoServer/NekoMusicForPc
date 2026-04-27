/**
 * @file glasswidget.cpp
 * @brief 毛玻璃效果容器实现
 *
 * 重度沉浸式：高透明度 + 微光边框，
 * 让底层的深色渐变背景穿透，形成层次感。
 */

#include "glasswidget.h"

#include <QPainter>
#include <QPainterPath>

GlassWidget::GlassWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
}

qreal GlassWidget::opacity() const { return m_opacity; }
void GlassWidget::setOpacity(qreal v) { m_opacity = v; update(); }

QColor GlassWidget::baseColor() const { return m_base; }
void GlassWidget::setBaseColor(const QColor &c) { m_base = c; update(); }

QColor GlassWidget::borderColor() const { return m_border; }
void GlassWidget::setBorderColor(const QColor &c) { m_border = c; update(); }

int GlassWidget::borderRadius() const { return m_radius; }
void GlassWidget::setBorderRadius(int r) { m_radius = r; update(); }

void GlassWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(rect(), m_radius, m_radius);

    // 半透明背景
    QColor bg = m_base;
    bg.setAlphaF(m_opacity);
    p.fillPath(path, bg);

    // 微光边框
    p.setPen(QPen(m_border, 1.0));
    p.drawPath(path);
}
