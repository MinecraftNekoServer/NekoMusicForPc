#pragma once

#include <QRect>

class QPainter;
class QPainterPath;
class QColor;

/**
 * KDE Plasma 风格的「软件毛玻璃」绘制：多层半透明渐变 + 顶光 + 细微噪点纹理。
 * 不依赖 KWindowSystem（无真窗口背景模糊），但在视觉上接近 Plasma 面板的层次与通透感。
 */
namespace GlassPaint {

enum class BarKind {
    TitleBar,
    Sidebar,
    PlayerBar,
};

/** 主窗口大背景：渐变 + 氛围光 + 暗角 + 噪点 */
void paintMainWindowDeepBackdrop(QPainter &p, const QRect &r, bool darkMode);

/** 标题栏 / 侧边栏 / 播放栏 条带玻璃 */
void paintBarGlass(QPainter &p, const QRect &r, BarKind kind, bool darkMode);

/** 圆角卡片（GlassWidget 等） */
void paintRoundedGlassCard(QPainter &p, const QPainterPath &path, const QColor &base, qreal bodyOpacity,
                           const QColor &border, int radius, bool darkMode);

} // namespace GlassPaint
