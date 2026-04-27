/**
 * @file main.cpp
 * @brief NekoMusic 日系动漫风入口
 */

#include <QApplication>
#include "ui/mainwindow.h"

int main(int argc, char *argv[])
{
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
