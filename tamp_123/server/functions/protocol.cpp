#include "protocol.h"

namespace Protocol
{
    QString makeServerBusyMessage()
    {
        return QString("Сервер занят. Пожалуйста, подключитесь позднее.\n");
    }

    QString makeWelcomeAndAskNickMessage()
    {
        return QString("Добро пожаловать на сервер! Пожалуйста, введите ваш ник:\n");
    }

    QString makeUserJoinedMessage(int id, const QString &nick)
    {
        // Строго по ТЗ: "К чату присоединился 'Ник' (ID: [X])."
        return QString("К чату присоединился '%1' (ID: [%2]).").arg(nick).arg(id);
    }

    QString makeUserRenamedMessage(int id, const QString &newNick)
    {
        // Строго по ТЗ: "Клиент [ID] сменил ник на 'НовоеИмя'."
        return QString("Клиент [%1] сменил ник на '%2'.").arg(id).arg(newNick);
    }

    QString makeUserLeftMessage(int id, const QString &nick)
    {
        // Строго по ТЗ: "Отключился 'Ник' (ID: [X])."
        return QString("Отключился '%1' (ID: [%2]).").arg(nick).arg(id);
    }

    QString makeOnlineCountMessage(int count)
    {
        return QString("Текущее количество подключенных клиентов: %1").arg(count);
    }

    QString makeChatMessage(int id, const QString &nick, const QString &message)
    {
        return QString("[%1] %2: %3").arg(id).arg(nick).arg(message);
    }
}
