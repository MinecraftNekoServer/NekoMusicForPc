#pragma once

/**
 * @file theme.h
 * @brief NekoMusic 日系动漫风主题常量
 *
 * 薰衣草紫 + 樱花粉 + 薄荷绿，温暖的深夜紫黑背景。
 * 所有配色、尺寸、动画参数集中管理。
 */

#include <QColor>

namespace Theme
{

    // ─── 主配色：薰衣草紫 ─────────────────────────────────────
    constexpr const char *kLavender = "#C4A7E7";   // 主色
    constexpr const char *kLavenderLt = "#D4BFF0"; // 浅薰衣草
    constexpr const char *kLavenderDk = "#A88BD4"; // 深薰衣草

    // ─── 辅色：樱花粉 ──────────────────────────────────────────
    constexpr const char *kSakura = "#F2ACB9";   // 樱花粉
    constexpr const char *kSakuraLt = "#F8C8D1"; // 浅粉
    constexpr const char *kSakuraDk = "#E08FA0"; // 深粉

    // ─── 点缀：薄荷绿 ──────────────────────────────────────────
    constexpr const char *kMint = "#7EC8C8";   // 薄荷绿
    constexpr const char *kMintLt = "#9DD8D8"; // 浅薄荷
    constexpr const char *kMintDk = "#5EAEAE"; // 深薄荷

    // ─── 背景色 ────────────────────────────────────────────────
    constexpr const char *kBgDeep = "#1A1625";    // 最深紫黑
    constexpr const char *kBgMid = "#241F31";     // 暗紫灰
    constexpr const char *kBgSurface = "#2D263F"; // 表面紫灰

    // ─── 文字色 ────────────────────────────────────────────────
    constexpr const char *kTextMain = "#F5F0FF";                   // 暖白
    constexpr const char *kTextSub = "rgba(245, 240, 255, 166)";   // 65% 暖灰紫
    constexpr const char *kTextMuted = "rgba(245, 240, 255, 102)"; // 40% 弱紫灰

    // ─── 毛玻璃 ────────────────────────────────────────────────
    // 卡片/面板毛玻璃背景 (65% 不透明度)
    constexpr const char *kGlassBg = "rgba(45, 38, 65, 166)";
    // 侧边栏毛玻璃 (80% 不透明度)
    constexpr const char *kGlassSidebar = "rgba(36, 31, 49, 204)";
    // 播放栏毛玻璃 (85% 不透明度)
    constexpr const char *kGlassPlayer = "rgba(36, 31, 49, 218)";
    // 轮播遮罩毛玻璃
    constexpr const char *kGlassOverlay = "rgba(26, 22, 37, 180)";

    // ─── 边框 ──────────────────────────────────────────────────
    constexpr const char *kBorderGlass = "rgba(196, 167, 231, 38)";  // 薰衣草15%
    constexpr const char *kBorderFocus = "rgba(126, 200, 200, 166)"; // 薄荷65%

    // ─── 渐变（QSS 格式） ─────────────────────────────────────
    constexpr const char *kGradMain = "qlineargradient(x1:0,y1:0,x2:0.3,y2:1,"
                                      "stop:0 #C4A7E7, stop:1 #A88BD4)";
    constexpr const char *kGradSakura = "qlineargradient(x1:0,y1:0,x2:0.3,y2:1,"
                                        "stop:0 #F2ACB9, stop:1 #E08FA0)";
    constexpr const char *kGradMint = "qlineargradient(x1:0,y1:0,x2:0.3,y2:1,"
                                      "stop:0 #7EC8C8, stop:1 #5EAEAE)";
    constexpr const char *kGradBg = "qlineargradient(x1:0,y1:0,x2:0.3,y2:1,"
                                    "stop:0 #1A1625, stop:1 #241F31)";

    // ─── 布局尺寸 ──────────────────────────────────────────────
    constexpr int kSidebarW = 240;   // 侧边栏宽度
    constexpr int kTitleBarH = 56;   // 标题栏高度
    constexpr int kPlayerBarH = 80;  // 播放栏高度
    constexpr int kCoverSmall = 140; // 网格卡片封面尺寸
    constexpr int kCoverRadius = 12; // 封面圆角

    // ─── 圆角 ──────────────────────────────────────────────────
    constexpr int kRSm = 8;
    constexpr int kRMd = 12;
    constexpr int kRLg = 16;
    constexpr int kRXl = 24;

    // ─── 动画 (ms) ─────────────────────────────────────────────
    constexpr int kAnimFast = 150;
    constexpr int kAnimNormal = 250;
    constexpr int kAnimSlow = 400;
    constexpr int kCarouselMs = 5000; // 轮播间隔

    // ─── API ───────────────────────────────────────────────────
    constexpr const char *kApiBase = "https://music.cnmsb.xin";

} // namespace Theme
