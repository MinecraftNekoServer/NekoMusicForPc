/**
 * @file main.cpp
 * @brief NekoMusic 日系动漫风入口
 */

#include <QApplication>
#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    // Linux: 使用 PulseAudio 后端避免 PipeWire 初始化阻塞
    qputenv("QT_MULTIMEDIA_BACKEND", "pulse");
#endif

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("NekoMusic"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    app.setOrganizationName(QStringLiteral("NekoMusic"));

    QFont font(QStringLiteral("Segoe UI"), 14);
    font.setStyleHint(QFont::SansSerif);
    app.setFont(font);

    MainWindow window;
    window.show();

    return app.exec();
}
