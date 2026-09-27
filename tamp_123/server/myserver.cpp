#include "myserver.h"
#include <QTcpSocket>

MyServer::MyServer(QObject *parent)
    : QTcpServer(parent),
      m_nextClientId(1)
{
}

MyServer::~MyServer()
{
    stopServer();
}

bool MyServer::startServer(quint16 port)
{
    Logger::instance().init("server.log");

    if (!listen(QHostAddress::Any, port)) {
        Logger::instance().log(QString("[ОШИБКА] Не удалось запустить сервер на порту %1: %2")
                               .arg(port).arg(errorString()));
        return false;
    }

    Logger::instance().log(QString("Сервер успешно запущен и слушает порт %1 (Макс. клиентов: %2)")
                           .arg(port).arg(MAX_CLIENTS));
    return true;
}

void MyServer::stopServer()
{
    if (isListening()) {
        close();
    }

    // Отключаем всех клиентов
    for (auto client : m_clients.values()) {
        if (client) {
            client->disconnectClient();
            client->deleteLater();
        }
    }
    m_clients.clear();

    // Завершаем и перезаписываем лог-файл по условию ТЗ
    Logger::instance().closeAndReset();
}

void MyServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *socket = new QTcpSocket(this);
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        delete socket;
        return;
    }

    // Проверка лимита клиентов (максимум 5)
    if (m_clients.size() >= MAX_CLIENTS) {
        Logger::instance().log(QString("Подключение отклонено: достигнут лимит %1 клиентов (IP: %2). Отправка уведомления о занятости.")
                               .arg(MAX_CLIENTS).arg(socket->peerAddress().toString()));
        
        // По ТЗ: сервер пишет лишнему клиенту, что занят и просит подключиться позднее, затем сразу же разрывает соединение
        QString busyMsg = Protocol::makeServerBusyMessage();
        socket->write(busyMsg.toUtf8());
        socket->flush();
        socket->disconnectFromHost();
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        return;
    }

    int clientId = m_nextClientId++;
    ClientHandler *handler = new ClientHandler(clientId, socket, this);
    m_clients.insert(clientId, handler);

    connect(handler, &ClientHandler::nickSubmitted, this, &MyServer::onNickSubmitted);
    connect(handler, &ClientHandler::nickChangeRequested, this, &MyServer::onNickChangeRequested);
    connect(handler, &ClientHandler::messageReceived, this, &MyServer::onMessageReceived);
    connect(handler, &ClientHandler::disconnected, this, &MyServer::onClientDisconnected);

    Logger::instance().log(QString("Новое физическое подключение: Client ID [%1] с адреса %2")
                           .arg(clientId).arg(socket->peerAddress().toString()));

    // Приветствие и запрос ника новому клиенту
    handler->sendMessage(Protocol::makeWelcomeAndAskNickMessage());

    // По ТЗ: При подключении очередного клиента сервер дополнительно отправляет всем
    // подключенным клиентам информацию о текущем количестве подключенных клиентов.
    broadcastOnlineCount();
}

bool MyServer::isNickTaken(const QString &nick, int excludeId) const
{
    QString target = nick.trimmed();
    for (auto handler : m_clients.values()) {
        if (handler && handler->id() != excludeId && handler->isNickRegistered()) {
            if (handler->nickname().compare(target, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }
    }
    return false;
}

void MyServer::onNickSubmitted(int id, const QString &nick)
{
    if (!m_clients.contains(id)) return;
    ClientHandler *handler = m_clients.value(id);
    QString trimmedNick = nick.trimmed();

    if (trimmedNick.isEmpty() || isNickTaken(trimmedNick, id)) {
        Logger::instance().log(QString("Попытка входа с уже занятым ником '%1' (Client ID: [%2]). Вход запрещен.")
                               .arg(trimmedNick).arg(id));
        handler->sendMessage(QString("Ошибка: Ник '%1' уже занят другим пользователем. Вход запрещен.").arg(trimmedNick));
        handler->disconnectClient();
        return;
    }

    handler->setNickRegistered(trimmedNick);

    // По ТЗ: При присоединении нового клиента сервер запрашивает ник, затем в лог-файл и в чат пишет:
    // "К чату присоединился 'Ник' (ID: [X])."
    QString joinMsg = Protocol::makeUserJoinedMessage(id, trimmedNick);
    Logger::instance().log(joinMsg);
    broadcastMessage(joinMsg);

    // Дополнительно обновляем для всех счетчик клиентов
    broadcastOnlineCount();
}

void MyServer::onNickChangeRequested(int id, const QString &newNick)
{
    if (!m_clients.contains(id)) return;
    ClientHandler *handler = m_clients.value(id);
    QString trimmedNick = newNick.trimmed();

    if (trimmedNick.isEmpty() || isNickTaken(trimmedNick, id)) {
        Logger::instance().log(QString("Клиент [%1] попытался сменить ник на уже занятый '%2'. Смена отклонена.")
                               .arg(id).arg(trimmedNick));
        handler->sendMessage(QString("Ошибка: Ник '%1' уже занят другим пользователем. Смена ника отклонена.").arg(trimmedNick));
        return;
    }

    handler->setNickname(trimmedNick);

    // По ТЗ: При смене ника сервер уведомляет чат и лог-файл:
    // "Клиент [ID] сменил ник на 'НовоеИмя'."
    QString renameMsg = Protocol::makeUserRenamedMessage(id, trimmedNick);
    Logger::instance().log(renameMsg);
    broadcastMessage(renameMsg);
}

void MyServer::onMessageReceived(int id, const QString &nick, const QString &text)
{
    // Веерная рассылка сообщений по очереди всем клиентам
    QString chatMsg = Protocol::makeChatMessage(id, nick, text);
    Logger::instance().log(QString("[ЧАТ] %1").arg(chatMsg));
    
    // Отправляем всем остальным клиентам (веерно)
    broadcastMessage(chatMsg, id);
}

void MyServer::onClientDisconnected(int id, const QString &nick)
{
    if (m_clients.contains(id)) {
        ClientHandler *handler = m_clients.take(id);
        handler->deleteLater();
    }

    // Если клиент был зарегистрирован с ником, уведомляем чат и лог
    if (!nick.isEmpty()) {
        QString leaveMsg = Protocol::makeUserLeftMessage(id, nick);
        Logger::instance().log(leaveMsg);
        broadcastMessage(leaveMsg);
    }

    // Рассылка обновленного количества активных клиентов
    broadcastOnlineCount();
}

void MyServer::broadcastMessage(const QString &message, int excludeId)
{
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.key() != excludeId && it.value() && it.value()->isNickRegistered()) {
            it.value()->sendMessage(message);
        }
    }
}

void MyServer::broadcastOnlineCount()
{
    int activeCount = 0;
    for (auto handler : m_clients.values()) {
        if (handler && handler->isNickRegistered()) {
            activeCount++;
        }
    }

    QString onlineMsg = Protocol::makeOnlineCountMessage(activeCount);
    for (auto handler : m_clients.values()) {
        if (handler && handler->isNickRegistered()) {
            handler->sendMessage(onlineMsg);
        }
    }
}
