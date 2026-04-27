/**
 * @file svgicon.cpp
 * @brief SVG 路径图标渲染器实现
 *
 * 通过构造最小 SVG 字符串 → QSvgRenderer → QPixmap 渲染。
 */

#include "svgicon.h"

#include <QSvgRenderer>
#include <QPainter>
#include <QByteArray>

namespace Icons {

QPixmap render(const char *pathD, int size, const QColor &color, int viewBox)
{
    // 构造最小 SVG
    QString svg = QString(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %1 %1\" width=\"%2\" height=\"%2\">"
        "<path fill=\"%3\" d=\"%4\"/>"
        "</svg>")
        .arg(viewBox)
        .arg(size)
        .arg(color.name(QColor::HexArgb))
        .arg(QString::fromUtf8(pathD));

    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    renderer.render(&p);
    return pix;
}

QIcon icon(const char *pathD, int size, const QColor &normal,
           const QColor &active, int viewBox)
{
    QIcon ic;
    ic.addPixmap(render(pathD, size, normal, viewBox), QIcon::Normal);
    if (active.isValid())
        ic.addPixmap(render(pathD, size, active, viewBox), QIcon::Active);
    return ic;
}

}  // namespace Icons
