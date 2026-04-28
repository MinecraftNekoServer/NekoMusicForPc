/**
 * @file main.cpp
 * @brief NekoMusic 日系动漫风入口
 */

#include <QApplication>
#include <QSettings>
#include <QLocalSocket>
#include <QLocalServer>
#include "ui/mainwindow.h"
#include "core/i18n.h"
#include "core/playlistdb.h"

// 单实例服务器名称
static const QString kServerName = QStringLiteral("NekoMusicSingleInstance");

int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    // Linux: 使用 PulseAudio 后端避免 PipeWire 初始化阻塞
    qputenv("QT_MULTIMEDIA_BACKEND", "pulse");
#endif

    // 检查是否已有实例在运行
    QLocalSocket socket;
    socket.connectToServer(kServerName);
    if (socket.waitForConnected(500)) {
        // 已有实例在运行，发送消息激活它然后退出
        socket.write("SHOW");
        socket.waitForBytesWritten(1000);
        return 0;
    }

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

    MainWindow *window = new MainWindow;

    // 创建单实例服务器
    QLocalServer::removeServer(kServerName);
    QLocalServer *server = new QLocalServer(&app);
    if (server->listen(kServerName)) {
        QObject::connect(server, &QLocalServer::newConnection, [server, window]() {
            QLocalSocket *clientSocket = server->nextPendingConnection();
            QObject::connect(clientSocket, &QLocalSocket::readyRead, [clientSocket, window]() {
                QByteArray data = clientSocket->readAll();
                if (data == "SHOW") {
                    // 激活主窗口
                    window->show();
                    window->raise();
                    window->activateWindow();
                }
            });
            QObject::connect(clientSocket, &QLocalSocket::disconnected, clientSocket, &QLocalSocket::deleteLater);
        });
    }

    window->show();

    int result = app.exec();

    // 清理数据库
    PlaylistDatabase::instance().close();

    return result;
}
