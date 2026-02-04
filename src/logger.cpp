#include "logger.h"
#include <QDebug>
#include <QDateTime>

namespace mpf {

Logger* Logger::s_instance = nullptr;

Logger::Logger(QObject* parent)
    : QObject(parent)
{
    if (!s_instance) {
        s_instance = this;
    }
}

Logger::~Logger()
{
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void Logger::log(Level level, const QString& tag, const QString& message)
{
    if (level < m_minLevel) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    
    if (m_handler) {
        m_handler(level, tag, message);
        return;
    }
    
    QString formatted = formatMessage(level, tag, message);
    
    switch (level) {
    case Level::Trace:
    case Level::Debug:
        qDebug().noquote() << formatted;
        break;
    case Level::Info:
        qInfo().noquote() << formatted;
        break;
    case Level::Warning:
        qWarning().noquote() << formatted;
        break;
    case Level::Error:
        qCritical().noquote() << formatted;
        break;
    }
}

void Logger::setMinLevel(Level level)
{
    QMutexLocker locker(&m_mutex);
    m_minLevel = level;
}

ILogger::Level Logger::minLevel() const
{
    QMutexLocker locker(&m_mutex);
    return m_minLevel;
}

void Logger::setHandler(LogHandler handler)
{
    QMutexLocker locker(&m_mutex);
    m_handler = std::move(handler);
}

void Logger::setFormat(const QString& format)
{
    QMutexLocker locker(&m_mutex);
    m_format = format;
}

QString Logger::format() const
{
    QMutexLocker locker(&m_mutex);
    return m_format;
}

Logger* Logger::instance()
{
    return s_instance;
}

void Logger::setInstance(Logger* logger)
{
    s_instance = logger;
}

QString Logger::formatMessage(Level level, const QString& tag, const QString& message)
{
    QString result = m_format;
    result.replace("%level%", levelToString(level));
    result.replace("%tag%", tag);
    result.replace("%message%", message);
    result.replace("%time%", QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
    result.replace("%date%", QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    return result;
}

QString Logger::levelToString(Level level)
{
    switch (level) {
    case Level::Trace:   return "TRACE";
    case Level::Debug:   return "DEBUG";
    case Level::Info:    return "INFO ";
    case Level::Warning: return "WARN ";
    case Level::Error:   return "ERROR";
    }
    return "?????";
}

} // namespace mpf
