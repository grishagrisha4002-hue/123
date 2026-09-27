#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCloseEvent>
#include "logviewer.h"

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWindow(const QString &host, quint16 port, const QString &nickname, QWidget *parent = nullptr);
    virtual ~ChatWindow();

    bool shouldExitApp() const { return m_exitApp; }

signals:
    void returnToLogin();
    void windowClosed();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onSendMessageClicked();
    void onChangeNickClicked();
    void onOpenLogViewerClicked();
    void onDisconnectClicked();

private:
    void appendFormattedMessage(const QString &rawMessage);
    void sendRawMessage(const QString &msg);

    QString m_host;
    quint16 m_port;
    QString m_nickname;
    bool m_nickHandshakeDone;
    bool m_exitApp;
    bool m_isReturningToLogin;

    QTcpSocket *m_socket;
    QByteArray m_buffer;

    QLabel *m_statusLabel;
    QPushButton *m_disconnectButton;
    QLabel *m_onlineLabel;
    QLabel *m_nickLabel;
    QPushButton *m_changeNickButton;
    QPushButton *m_logViewerButton;

    QTextEdit *m_chatArea;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;

    LogViewer *m_logViewer;
};

#endif // CHATWINDOW_H
