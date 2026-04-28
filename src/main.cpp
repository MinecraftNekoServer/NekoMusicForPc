/**
 * @file main.cpp
 * @brief NekoMusic 日系动漫风入口
 */

#include <QApplication>
#include <QSettings>
#include "ui/mainwindow.h"
#include "core/i18n.h"
#include "core/playlistdb.h"

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
    app.setOrganizationDomain(QStringLiteral("nekomusic.local"));

    // 加载用户设置
    QSettings settings;
    int lang = settings.value("language", static_cast<int>(I18n::ZhCN)).toInt();
    I18n::instance().setLanguage(static_cast<I18n::Language>(lang));

    // 初始化播放列表数据库
    PlaylistDatabase::instance().init();

    QFont font(QStringLiteral("Segoe UI"), 14);
    font.setStyleHint(QFont::SansSerif);
    app.setFont(font);
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app.png")));

    QApplication::setQuitOnLastWindowClosed(false);

    MainWindow window;
    window.show();

    int result = app.exec();

    // 清理数据库
    PlaylistDatabase::instance().close();

    return result;
}
