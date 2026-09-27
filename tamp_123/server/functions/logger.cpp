#include "logger.h"
#include <QDebug>
#include <iostream>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

Logger& Logger::instance()
{
    static Logger inst;
    return inst;
}

Logger::Logger()
    : m_filePath("server.log"), m_isInitialized(false)
{
}

Logger::~Logger()
{
    closeAndReset();
}

bool Logger::init(const QString &filePath)
{
    QMutexLocker locker(&m_mutex);
    m_filePath = filePath;

    if (m_logFile.isOpen()) {
        m_logFile.close();
    }

    m_logFile.setFileName(m_filePath);
    // При старте сервера лог-файл перезаписывается (Truncate)
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "[ERROR] Не удалось открыть лог-файл:" << m_filePath;
        m_isInitialized = false;
        return false;
    }

    m_stream.setDevice(&m_logFile);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_stream.setEncoding(QStringConverter::Utf8);
#else
    m_stream.setCodec("UTF-8");
#endif
    m_isInitialized = true;

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString header = QString("[%1] [СИСТЕМА] Сервер запущен. Лог инициализирован.").arg(timestamp);
    m_stream << header << "\n";
    m_stream.flush();

    qDebug().noquote() << header;

    return true;
}

void Logger::log(const QString &message)
{
    QMutexLocker locker(&m_mutex);
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString formatted = QString("[%1] %2").arg(timestamp, message);

    qDebug().noquote() << formatted;

    if (m_isInitialized && m_logFile.isOpen()) {
        m_stream << formatted << "\n";
        m_stream.flush();
    }
}

void Logger::closeAndReset()
{
    QMutexLocker locker(&m_mutex);
    if (m_isInitialized && m_logFile.isOpen()) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString footer = QString("[%1] [СИСТЕМА] Сервер остановлен. Перезапись лог-файла.").arg(timestamp);
        qDebug().noquote() << footer;
        
        m_logFile.close();

        // По требованию ТЗ: "При выключении сервера лог-файл перезаписывается"
        if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream resetStream(&m_logFile);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            resetStream.setEncoding(QStringConverter::Utf8);
#else
            resetStream.setCodec("UTF-8");
#endif
            resetStream << QString("[%1] [СИСТЕМА] Сервер выключен. Лог очищен и перезаписан.\n").arg(timestamp);
            resetStream.flush();
            m_logFile.close();
        }
        m_isInitialized = false;
    }
}
