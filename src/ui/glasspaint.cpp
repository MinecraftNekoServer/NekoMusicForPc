#include "glasspaint.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

namespace {

QPixmap noiseTile()
{
    static QPixmap cache;
    if (!cache.isNull())
        return cache;

    constexpr int n = 96;
    QImage img(n, n, QImage::Format_ARGB32);
    img.fill(Qt::transparent);
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < n; ++x) {
            const qreal t = qSin(qreal(x) * 12.9898 + qreal(y) * 78.233) * 43758.5453;
            int v = int(t) & 255;
            v = (v - 128) / 8;
            const int g = qBound(0, 128 + v, 255);
            img.setPixelColor(x, y, QColor(g, g, g, 22));
        }
    }
    cache = QPixmap::fromImage(img);
    return cache;
}

void drawNoiseOverlay(QPainter &p, const QRect &r, qreal strength = 1.0)
{
    const QPixmap tile = noiseTile();
    if (tile.isNull())
        return;
    p.save();
    p.setOpacity(0.035 * strength);
    p.drawTiledPixmap(r, tile);
    p.restore();
}

} // namespace

namespace GlassPaint {

void paintMainWindowDeepBackdrop(QPainter &p, const QRect &r, bool darkMode)
{
    p.setRenderHint(QPainter::Antialiasing);

    QLinearGradient bg(r.topLeft(), QPoint(int(r.width() * 0.3), r.height()));
    if (darkMode) {
        bg.setColorAt(0.0, QColor(26, 22, 37));
        bg.setColorAt(0.55, QColor(30, 25, 44));
        bg.setColorAt(1.0, QColor(36, 31, 49));
    } else {
        bg.setColorAt(0.0, QColor(248, 249, 250));
        bg.setColorAt(0.5, QColor(241, 243, 246));
        bg.setColorAt(1.0, QColor(233, 236, 239));
    }
    p.fillRect(r, bg);

    // 顶区柔光（类似 Plasma 桌面泛光）
    QRadialGradient topGlow(QPointF(r.center().x(), r.top() + r.height() * 0.08), r.width() * 0.75);
    if (darkMode) {
        topGlow.setColorAt(0.0, QColor(196, 167, 231, 38));
        topGlow.setColorAt(0.45, QColor(126, 200, 200, 12));
        topGlow.setColorAt(1.0, Qt::transparent);
    } else {
        topGlow.setColorAt(0.0, QColor(196, 167, 231, 55));
        topGlow.setColorAt(0.5, QColor(255, 255, 255, 90));
        topGlow.setColorAt(1.0, Qt::transparent);
    }
    p.fillRect(r, topGlow);

    // 底部两侧暗角（增加景深）
    const int vx = int(r.width() * 0.35);
    const int vy = int(r.height() * 0.4);
    QRadialGradient vLeft(r.bottomLeft(), qMax(vx, vy));
    vLeft.setColorAt(0.0, darkMode ? QColor(10, 8, 18, 110) : QColor(108, 117, 125, 28));
    vLeft.setColorAt(1.0, Qt::transparent);
    p.fillRect(r, vLeft);

    QRadialGradient vRight(r.bottomRight(), qMax(vx, vy));
    vRight.setColorAt(0.0, darkMode ? QColor(10, 8, 18, 100) : QColor(108, 117, 125, 22));
    vRight.setColorAt(1.0, Qt::transparent);
    p.fillRect(r, vRight);

    drawNoiseOverlay(p, r, darkMode ? 1.15 : 0.85);
}

void paintBarGlass(QPainter &p, const QRect &r, BarKind kind, bool darkMode)
{
    p.setRenderHint(QPainter::Antialiasing);

    QLinearGradient depth(r.topLeft(), r.bottomLeft());
    if (darkMode) {
        depth.setColorAt(0.0, QColor(44, 38, 62, 236));
        depth.setColorAt(0.55, QColor(34, 29, 48, 228));
        depth.setColorAt(1.0, QColor(26, 22, 37, 218));
    } else {
        depth.setColorAt(0.0, QColor(255, 255, 255, 248));
        depth.setColorAt(0.5, QColor(248, 249, 250, 242));
        depth.setColorAt(1.0, QColor(236, 239, 242, 235));
    }
    p.fillRect(r, depth);

    // 左上高光（KDE 面板常见）
    QRadialGradient hi(QPointF(r.left() + r.width() * 0.08, r.top() + r.height() * 0.12), r.height() * 1.1);
    if (darkMode) {
        hi.setColorAt(0.0, QColor(255, 255, 255, 26));
        hi.setColorAt(0.35, QColor(196, 167, 231, 22));
        hi.setColorAt(1.0, Qt::transparent);
    } else {
        hi.setColorAt(0.0, QColor(255, 255, 255, 200));
        hi.setColorAt(0.4, QColor(196, 167, 231, 45));
        hi.setColorAt(1.0, Qt::transparent);
    }
    p.fillRect(r, hi);

    drawNoiseOverlay(p, r, 0.9);

    QLinearGradient accent;
    switch (kind) {
    case BarKind::TitleBar:
        accent = QLinearGradient(r.topLeft(), r.topRight());
        accent.setColorAt(0.0, QColor(196, 167, 231, 0));
        accent.setColorAt(0.5, QColor(196, 167, 231, darkMode ? 48 : 55));
        accent.setColorAt(1.0, QColor(196, 167, 231, 0));
        p.setPen(QPen(QBrush(accent), 1));
        p.drawLine(r.bottomLeft(), r.bottomRight());
        break;
    case BarKind::Sidebar:
        accent = QLinearGradient(r.topRight(), r.bottomRight());
        accent.setColorAt(0.0, QColor(196, 167, 231, darkMode ? 38 : 50));
        accent.setColorAt(1.0, QColor(196, 167, 231, darkMode ? 12 : 18));
        p.setPen(QPen(QBrush(accent), 1));
        p.drawLine(r.topRight(), r.bottomRight());
        break;
    case BarKind::PlayerBar:
        accent = QLinearGradient(r.topLeft(), r.topRight());
        accent.setColorAt(0.0, QColor(196, 167, 231, 0));
        accent.setColorAt(0.5, QColor(196, 167, 231, darkMode ? 52 : 58));
        accent.setColorAt(1.0, QColor(196, 167, 231, 0));
        p.setPen(QPen(QBrush(accent), 1));
        p.drawLine(r.topLeft(), r.topRight());
        break;
    }
}

void paintRoundedGlassCard(QPainter &p, const QPainterPath &path, const QColor &base, qreal bodyOpacity,
                           const QColor &border, int /*radius*/, bool darkMode)
{
    p.setRenderHint(QPainter::Antialiasing);

    QColor body = base;
    body.setAlphaF(qBound(0.0, bodyOpacity * (darkMode ? 0.92 : 0.88), 1.0));
    p.fillPath(path, body);

    QRectF br = path.boundingRect();
    QLinearGradient spec(br.topLeft(), QPointF(br.center().x(), br.top() + br.height() * 0.42));
    if (darkMode) {
        spec.setColorAt(0.0, QColor(255, 255, 255, 34));
        spec.setColorAt(0.55, QColor(196, 167, 231, 18));
        spec.setColorAt(1.0, Qt::transparent);
    } else {
        spec.setColorAt(0.0, QColor(255, 255, 255, 210));
        spec.setColorAt(0.45, QColor(196, 167, 231, 55));
        spec.setColorAt(1.0, Qt::transparent);
    }
    p.fillPath(path, spec);

    p.save();
    p.setClipPath(path);
    drawNoiseOverlay(p, br.toRect(), darkMode ? 1.08 : 0.82);

    // 底部轻微收光，增强「磨砂层」厚度感（类似 Plasma 卡片景深）
    QLinearGradient foot(br.bottomLeft(), QPointF(br.center().x(), br.top() + br.height() * 0.35));
    if (darkMode) {
        foot.setColorAt(0.0, QColor(0, 0, 0, 52));
        foot.setColorAt(1.0, Qt::transparent);
    } else {
        foot.setColorAt(0.0, QColor(108, 117, 125, 18));
        foot.setColorAt(1.0, Qt::transparent);
    }
    p.fillRect(br.toRect(), foot);
    p.restore();

    p.setPen(QPen(border, 1.0));
    p.drawPath(path);
}

} // namespace GlassPaint
