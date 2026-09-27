#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class Logger
{
public:
    static Logger& instance();

    // Инициализация лог-файла (перезапись при старте)
    bool init(const QString &filePath = "server.log");

    // Запись события в лог-файл и консоль
    void log(const QString &message);

    // Завершение работы с логом (перезапись при выключении по требованию ТЗ)
    void closeAndReset();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    QFile m_logFile;
    QTextStream m_stream;
    QMutex m_mutex;
    QString m_filePath;
    bool m_isInitialized;
};

#endif // LOGGER_H
