#include <QApplication>
#include <QEventLoop>
#include "logindialog.h"
#include "chatwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString host = "127.0.0.1";
    quint16 port = 5555;
    QString nickname = "Клиент_1";

    while (true) {
        LoginDialog loginDlg;
        loginDlg.setValues(host, port, nickname);

        if (loginDlg.exec() != QDialog::Accepted) {
            // Пользователь нажал «Отмена» или закрыл окно входа
            break;
        }

        // Сохраняем значения для удобства повторного использования
        host = loginDlg.host();
        port = loginDlg.port();
        nickname = loginDlg.nickname();

        ChatWindow *chatWin = new ChatWindow(host, port, nickname);
        chatWin->show();

        // Ожидаем события возврата к окну входа или закрытия окна
        QEventLoop loop;
        QObject::connect(chatWin, &ChatWindow::returnToLogin, &loop, &QEventLoop::quit);
        QObject::connect(chatWin, &ChatWindow::windowClosed, &loop, &QEventLoop::quit);
        loop.exec();

        bool exitApp = chatWin->shouldExitApp();
        chatWin->deleteLater();

        if (exitApp) {
            break;
        }
    }

    return 0;
}
