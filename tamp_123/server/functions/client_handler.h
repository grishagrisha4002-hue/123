#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>

class ClientHandler : public QObject
{
    Q_OBJECT
public:
    explicit ClientHandler(int id, QTcpSocket *socket, QObject *parent = nullptr);
    virtual ~ClientHandler();

    int id() const { return m_id; }
    QString nickname() const { return m_nickname; }
    bool isNickRegistered() const { return m_isNickRegistered; }

    void setNickRegistered(const QString &nick) {
        m_nickname = nick;
        m_isNickRegistered = true;
    }

    void setNickname(const QString &newNick) {
        m_nickname = newNick;
    }

    void sendMessage(const QString &message);
    void disconnectClient();

signals:
    void nickSubmitted(int id, const QString &nick);
    void nickChangeRequested(int id, const QString &newNick);
    void messageReceived(int id, const QString &nick, const QString &text);
    void disconnected(int id, const QString &nick);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    int m_id;
    QTcpSocket *m_socket;
    QString m_nickname;
    bool m_isNickRegistered;
    QByteArray m_buffer;
};

#endif // CLIENT_HANDLER_H
