#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QString>

namespace Protocol
{
    // Сообщение при отказе подключения лишнему клиенту
    QString makeServerBusyMessage();

    // Сообщение приветствия и запроса ника
    QString makeWelcomeAndAskNickMessage();

    // Системное сообщение о присоединении клиента: "К чату присоединился 'Ник' (ID: [X])."
    QString makeUserJoinedMessage(int id, const QString &nick);

    // Системное сообщение о смене ника: "Клиент [ID] сменил ник на 'НовоеИмя'."
    QString makeUserRenamedMessage(int id, const QString &newNick);

    // Системное сообщение об отключении клиента: "Отключился 'Ник' (ID: [X])."
    QString makeUserLeftMessage(int id, const QString &nick);

    // Сообщение о текущем количестве клиентов: "Текущее количество подключенных клиентов: X"
    QString makeOnlineCountMessage(int count);

    // Форматирование обычного сообщения пользователя для веерной рассылки
    QString makeChatMessage(int id, const QString &nick, const QString &message);
}

#endif // PROTOCOL_H
