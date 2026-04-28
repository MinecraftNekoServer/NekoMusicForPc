/**
 * @file toast.cpp
 * @brief Toast 消息提示组件实现
 */

#include "toast.h"
#include "theme/theme.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVariant>

Toast::Toast(QWidget *parent, const QString &message, Type type, int durationMs)
    : QWidget(parent), m_durationMs(durationMs)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    initUi(message, type);
}

void Toast::show(QWidget *parent, const QString &message, Type type, int durationMs)
{
    qDebug() << "[Toast] 创建Toast:" << message << ", type =" << type << ", duration =" << durationMs;
    qDebug() << "[Toast] parent =" << parent << ", parent->width() =" << parent->width();
    auto *toast = new Toast(parent, message, type, durationMs);

    // 确保窗口大小已计算
    toast->adjustSize();
    qDebug() << "[Toast] toast->size() =" << toast->size();

    // 定位到右上角（相对于父窗口）
    QPoint pos = parent->mapToGlobal(QPoint(parent->width() - toast->width() - 24, 80));
    toast->move(pos);
    qDebug() << "[Toast] 位置 =" << pos;

    // 淡入动画
    auto *effect = new QGraphicsOpacityEffect(toast);
    effect->setOpacity(0.0);
    toast->setGraphicsEffect(effect);

    auto *fadeIn = new QPropertyAnimation(effect, "opacity", toast);
    fadeIn->setDuration(300);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::OutCubic);

    // 定时自动消失
    auto *fadeOut = new QPropertyAnimation(effect, "opacity", toast);
    fadeOut->setDuration(300);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::InCubic);

    QObject::connect(fadeOut, &QPropertyAnimation::finished, toast, [toast]() {
        qDebug() << "[Toast] 动画结束，销毁Toast";
        toast->deleteLater();
    });

    auto *timer = new QTimer(toast);
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, toast, [fadeOut, toast]() {
        qDebug() << "[Toast] 定时器触发，开始淡出";
        fadeOut->start();
    });

    fadeIn->start();
    static_cast<QWidget *>(toast)->show();
    qDebug() << "[Toast] Toast已show";
    timer->start(durationMs);
}

void Toast::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 背景
    QPainterPath bgPath;
    bgPath.addRoundedRect(rect(), 12, 12);
    p.fillPath(bgPath, QColor(45, 38, 65, 230));

    // 左侧彩色条
    QColor borderColor = property("borderColor").value<QColor>();
    QPainterPath barPath;
    barPath.addRoundedRect(0, 2, 4, height() - 4, 2, 2);
    p.fillPath(barPath, borderColor);
}

void Toast::initUi(const QString &message, Type type)
{
    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(20, 16, 20, 16);
    lay->setSpacing(12);

    // 图标
    auto *iconLbl = new QLabel(this);
    iconLbl->setFixedSize(20, 20);
    QColor iconColor;
    if (type == Success) iconColor = QColor(74, 222, 128);
    else if (type == Error) iconColor = QColor(239, 68, 68);
    else iconColor = QColor(59, 130, 246);

    // 使用 PNG 图标
    const char *iconRes = ":/icons/icon_heart.png";  // 默认
    if (type == Success) iconRes = ":/icons/icon_play.png";  // 用 play 作为 success 图标
    else if (type == Error) iconRes = ":/icons/icon_close.png";

    iconLbl->setPixmap(QPixmap(iconRes));
    lay->addWidget(iconLbl);

    // 消息文本
    auto *msgLbl = new QLabel(message, this);
    msgLbl->setWordWrap(false);
    msgLbl->setStyleSheet(
        "QLabel { font-size: 14px; font-weight: 500; color: " + QString(Theme::kTextMain) + "; }"
    );
    lay->addWidget(msgLbl);

    // 设置窗口大小
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(message);
    int minWidth = qMax(300, textWidth + 100);
    setFixedSize(minWidth, 52);

    // 更新左侧条的颜色
    QColor borderColor;
    if (type == Success) borderColor = QColor(74, 222, 128);
    else if (type == Error) borderColor = QColor(239, 68, 68);
    else borderColor = QColor(59, 130, 246);

    // 通过 stylesheet 实现左侧彩色条
    setStyleSheet(
        "Toast { border-radius: 12px; }"
    );

    // 存储颜色供 paintEvent 使用
    setProperty("borderColor", borderColor);
}

QColor Toast::getBorderColor(Type type) const
{
    if (type == Success) return QColor(74, 222, 128);
    if (type == Error) return QColor(239, 68, 68);
    return QColor(59, 130, 246);
}

QColor Toast::getIconColor(Type type) const
{
    if (type == Success) return QColor(74, 222, 128);
    if (type == Error) return QColor(239, 68, 68);
    return QColor(59, 130, 246);
}

QString Toast::getIconPath(Type type) const
{
    return QString();
}
