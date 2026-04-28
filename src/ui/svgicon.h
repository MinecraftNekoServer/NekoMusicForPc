#pragma once

/**
 * @file svgicon.h
 * @brief SVG 路径图标渲染器
 *
 * 从 old 项目的 Vue 组件中提取 SVG path 数据，
 * 在运行时渲染为 QPixmap/QIcon，支持指定颜色和尺寸。
 */

#include <QIcon>
#include <QPixmap>
#include <QString>
#include <QColor>

namespace Icons {

// ─── 侧边栏导航图标 ────────────────────────────────
// home
constexpr const char *kHome =
    "M10 20v-6h4v6h5v-8h3L12 3 2 12h3v8z";
// heart (favorites)
constexpr const char *kHeart =
    "M12 21.35l-1.45-1.32C5.4 15.36 2 12.28 2 8.5 2 5.42 4.42 3 7.5 3"
    "c1.74 0 3.41.81 4.5 2.09C13.09 3.81 14.76 3 16.5 3 19.58 3 22 5.42 22 8.5"
    "c0 3.78-3.4 6.86-8.55 11.54L12 21.35z";
// clock (recent)
constexpr const char *kClock =
    "M11.99 2C6.47 2 2 6.48 2 12s4.47 10 9.99 10C17.52 22 22 17.52 22 12"
    "S17.52 2 11.99 2zM12 20c-4.42 0-8-3.58-8-8s3.58-8 8-8 8 3.58 8 8-3.58 8-8 8z"
    "m.5-13H11v6l5.25 3.15.75-1.23-4.5-2.67z";
// upload (cloud upload)
constexpr const char *kUpload =
    "M19.35 10.04C18.67 6.59 15.64 4 12 4 9.11 4 6.6 5.64 5.35 8.04"
    " 2.34 8.36 0 10.91 0 14c0 3.31 2.69 6 6 6h13c2.76 0 5-2.24 5-5"
    " 0-2.64-2.05-4.78-4.65-4.96zM14 13v4h-4v-4H7l5-5 5 5h-3z";

// ─── 播放控制图标 ──────────────────────────────────
// prev
constexpr const char *kPrev =
    "M6 6h2v12H6zm3.5 6l8.5 6V6z";
// play
constexpr const char *kPlay =
    "M8 5v14l11-7z";
// pause
constexpr const char *kPause =
    "M6 19h4V5H6v14zm8-14v14h4V5h-4z";
// next
constexpr const char *kNext =
    "M6 18l8.5-6L6 6v12zM16 6v12h2V6h-2z";
// shuffle
constexpr const char *kShuffle =
    "M10.59 9.17L5.41 4 4 5.41l5.17 5.17 1.42-1.41zM14.5 4l2.04 2.04L4 18.59"
    " 5.41 20 17.54 7.76 19.5 9.7V4h-5zM14.83 13.41l-1.42 1.41 3.18 3.18L14.5 20"
    "H19.5v-5l-1.96 1.96-2.71-2.71z";

// ─── 标题栏图标 ────────────────────────────────────
// search
constexpr const char *kSearch =
    "M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3"
    "S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99"
    "L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5"
    " 11.99 14 9.5 14z";
// settings/gear
constexpr const char *kSettings =
    "M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58"
    "c.18-.14.23-.41.12-.61l-1.92-3.32c-.12-.22-.37-.29-.59-.22l-2.39.96"
    "c-.5-.38-1.03-.7-1.62-.94l-.36-2.54c-.04-.24-.24-.41-.48-.41h-3.84"
    "c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96"
    "c-.22-.08-.47 0-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58"
    "c-.05.3-.09.63-.09.94s.02.64.07.94l-2.03 1.58c-.18.14-.23.41-.12.61"
    "l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54"
    "c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.56"
    " 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61"
    "l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62"
    " 3.6 3.6-1.62 3.6-3.6 3.6z";
// minimize
constexpr const char *kMinimize =
    "M6 13h12v-2H6v2z";
// maximize (rectangle)
constexpr const char *kMaximize =
    "M3 5v14h18V5H3zm16 12H5V7h14v10z";
// close (X)
constexpr const char *kClose =
    "M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59"
    " 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12 19 6.41z";

// ─── 轮播箭头 ──────────────────────────────────────
// left arrow
constexpr const char *kLeft =
    "M15.41 16.59L10.83 12l4.58-4.59L14 6l-6 6 6 6 1.41-1.41z";
// right arrow
constexpr const char *kRight =
    "M8.59 16.59L13.17 12 8.59 7.41 10 6l6 6-6 6-1.41-1.41z";

// ─── 其他 ──────────────────────────────────────────
// music note (封面占位)
constexpr const char *kMusic =
    "M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4"
    "V7h4V3h-6z";
// volume
constexpr const char *kVolume =
    "M3 9v6h4l5 5V4L7 9H3zm13.5 3c0-1.77-1.02-3.29-2.5-4.03v8.05"
    "c1.48-.73 2.5-2.25 2.5-4.02z";

/**
 * @brief 将 SVG path 渲染为 QPixmap
 * @param pathD   SVG path 的 d 属性
 * @param size    图标尺寸 (px)
 * @param color   填充颜色
 * @param viewBox SVG viewBox 尺寸 (默认24)
 */
QPixmap render(const char *pathD, int size, const QColor &color, int viewBox = 24);

/**
 * @brief 将 SVG path 渲染为 QIcon（Normal + Active 状态可选不同颜色）
 */
QIcon icon(const char *pathD, int size, const QColor &normal,
           const QColor &active = QColor(), int viewBox = 24);

}  // namespace Icons
