#include "client_handler.h"
#include <QDebug>

ClientHandler::ClientHandler(int id, QTcpSocket *socket, QObject *parent)
    : QObject(parent),
      m_id(id),
      m_socket(socket),
      m_nickname(QString("Клиент_%1").arg(id)),
      m_isNickRegistered(false)
{
    if (m_socket) {
        m_socket->setParent(this);
        connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
        connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);
    }
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::sendMessage(const QString &message)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QByteArray data = message.toUtf8();
    if (!data.endsWith('\n')) {
        data.append('\n');
    }
    m_socket->write(data);
    m_socket->flush();
}

void ClientHandler::disconnectClient()
{
    if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void ClientHandler::onReadyRead()
{
    if (!m_socket) return;

    m_buffer.append(m_socket->readAll());

    while (m_buffer.contains('\n')) {
        int newlineIndex = m_buffer.indexOf('\n');
        QByteArray rawLine = m_buffer.left(newlineIndex);
        m_buffer.remove(0, newlineIndex + 1);

        QString line = QString::fromUtf8(rawLine).trimmed();
        if (line.isEmpty()) {
            continue;
        }

        // Если ник еще не зарегистрирован, первое пришедшее сообщение считаем запросом ника
        if (!m_isNickRegistered) {
            emit nickSubmitted(m_id, line);
        } else {
            // Проверка на команду смены ника /name <НовоеИмя>
            if (line.startsWith("/name", Qt::CaseInsensitive)) {
                QString newNick = line.section(' ', 1).trimmed();
                if (!newNick.isEmpty()) {
                    emit nickChangeRequested(m_id, newNick);
                } else {
                    sendMessage("Ошибка: укажите никнейм, например: /name Иван");
                }
            } else {
                // Обычное сообщение для чата
                emit messageReceived(m_id, m_nickname, line);
            }
        }
    }
}

void ClientHandler::onDisconnected()
{
    emit disconnected(m_id, m_isNickRegistered ? m_nickname : QString());
}
