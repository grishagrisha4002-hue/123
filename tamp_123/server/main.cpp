#include <QCoreApplication>
#include <QDebug>
#include <csignal>
#include "myserver.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static MyServer *g_server = nullptr;

void signalHandler(int sig)
{
    qDebug().noquote() << QString("\nПолучен сигнал остановки (%1). Завершение работы сервера...").arg(sig);
    if (g_server) {
        g_server->stopServer();
    }
    QCoreApplication::quit();
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // Установка UTF-8 для вывода в консоль Windows
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    QCoreApplication a(argc, argv);

    quint16 port = 5555;
    if (argc > 1) {
        port = static_cast<quint16>(QString(argv[1]).toUShort());
    }

    MyServer server;
    g_server = &server;

    // Перехват сигналов завершения для перезаписи лог-файла при остановке
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    if (!server.startServer(port)) {
        return 1;
    }

    int ret = a.exec();

    // Перезапись лога при выходе из event loop
    server.stopServer();

    return ret;
}
