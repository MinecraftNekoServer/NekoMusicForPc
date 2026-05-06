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

static QString svgPathFill(const QColor &color)
{
    if (color.alpha() >= 255)
        return color.name(QColor::HexRgb);
    return QStringLiteral("rgba(%1,%2,%3,%4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alphaF(), 0, 'f', 5);
}

QPixmap render(const char *pathD, int size, const QColor &color, int viewBox)
{
    // 构造最小 SVG（半透明用 rgba，避免 #AARRGGBB 在个别渲染路径下异常）
    QString svg = QString(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %1 %1\" width=\"%2\" height=\"%2\">"
        "<path fill=\"%3\" d=\"%4\"/>"
        "</svg>")
        .arg(viewBox)
        .arg(size)
        .arg(svgPathFill(color))
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
