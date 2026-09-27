#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include <QMap>
#include "functions/client_handler.h"
#include "functions/logger.h"
#include "functions/protocol.h"

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = nullptr);
    virtual ~MyServer();

    bool startServer(quint16 port = 5555);
    void stopServer();

    // Веерная рассылка сообщения всем подключенным клиентам (или всем, кроме senderId)
    void broadcastMessage(const QString &message, int excludeId = -1);

    // Рассылка текущего количества клиентов
    void broadcastOnlineCount();

    // Проверка занятости ника
    bool isNickTaken(const QString &nick, int excludeId = -1) const;

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onNickSubmitted(int id, const QString &nick);
    void onNickChangeRequested(int id, const QString &newNick);
    void onMessageReceived(int id, const QString &nick, const QString &text);
    void onClientDisconnected(int id, const QString &nick);

private:
    static const int MAX_CLIENTS = 5;
    int m_nextClientId;
    QMap<int, ClientHandler*> m_clients;
};

#endif // MYSERVER_H
