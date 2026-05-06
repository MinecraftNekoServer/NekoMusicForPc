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
#include <QDebug>

namespace Icons {

/** 路径着色：勿用 fill="rgba(...)"，部分 QtSVG / 平台下会落成纯黑；改用实色 + fill-opacity。 */
static QString svgPathFillAttrs(const QColor &color)
{
    QString attrs = QStringLiteral("fill=\"%1\"").arg(color.name(QColor::HexRgb));
    if (color.alpha() < 255) {
        attrs += QStringLiteral(" fill-opacity=\"%1\"")
                     .arg(QString::number(color.alphaF(), 'f', 5));
    }
    return attrs;
}

QPixmap render(const char *pathD, int size, const QColor &color, int viewBox)
{
    QString svg = QString(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %1 %1\" width=\"%2\" height=\"%2\">"
        "<path %3 d=\"%4\"/>"
        "</svg>")
        .arg(viewBox)
        .arg(size)
        .arg(svgPathFillAttrs(color))
        .arg(QString::fromUtf8(pathD));

    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    if (!renderer.isValid()) {
        qWarning() << "Icons::render: invalid SVG (size" << size << ")";
        QPainter p(&pix);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(252, 248, 255));
        p.drawEllipse(0, 0, size, size);
        return pix;
    }
    QPainter p(&pix);
    renderer.render(&p);
    return pix;
}

QIcon icon(const char *pathD, int size, const QColor &normal,
           const QColor &active, int viewBox)
{
    const QPixmap n = render(pathD, size, normal, viewBox);
    const QPixmap a = active.isValid() ? render(pathD, size, active, viewBox) : n;

    QIcon ic;
    const QIcon::Mode modes[] = {QIcon::Normal, QIcon::Disabled, QIcon::Active, QIcon::Selected};
    for (QIcon::Mode m : modes) {
        const QPixmap &pm = (m == QIcon::Active && active.isValid()) ? a : n;
        ic.addPixmap(pm, m, QIcon::Off);
        ic.addPixmap(pm, m, QIcon::On);
    }
    // 避免被当作「模板图标」：KDE/GNOME 等下仅用 alpha 再涂 ButtonText → 纯黑
    ic.setIsMask(false);
    return ic;
}

}  // namespace Icons
