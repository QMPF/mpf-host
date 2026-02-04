#pragma once

#include <mpf/interfaces/ilogger.h>
#include <QObject>
#include <QMutex>
#include <QString>
#include <functional>

namespace mpf {

/**
 * @brief Default logger implementation
 * 
 * Routes logs to Qt's message system with formatting.
 * Can be replaced with custom implementation via ServiceRegistry.
 */
class Logger : public QObject, public ILogger
{
    Q_OBJECT

public:
    using LogHandler = std::function<void(Level, const QString&, const QString&)>;

    explicit Logger(QObject* parent = nullptr);
    ~Logger() override;

    // ILogger interface
    void log(Level level, const QString& tag, const QString& message) override;
    void setMinLevel(Level level) override;
    Level minLevel() const override;

    // Custom handler
    void setHandler(LogHandler handler);

    // Formatting
    void setFormat(const QString& format);
    QString format() const;

    // Static convenience
    static Logger* instance();
    static void setInstance(Logger* logger);

private:
    QString formatMessage(Level level, const QString& tag, const QString& message);
    static QString levelToString(Level level);

    Level m_minLevel = Level::Debug;
    QString m_format = "[%level%] [%tag%] %message%";
    LogHandler m_handler;
    mutable QMutex m_mutex;

    static Logger* s_instance;
};

// Convenience macros
#define MPF_LOG_TRACE(tag, msg) \
    if (auto* _logger = mpf::Logger::instance()) _logger->trace(tag, msg)
#define MPF_LOG_DEBUG(tag, msg) \
    if (auto* _logger = mpf::Logger::instance()) _logger->debug(tag, msg)
#define MPF_LOG_INFO(tag, msg) \
    if (auto* _logger = mpf::Logger::instance()) _logger->info(tag, msg)
#define MPF_LOG_WARNING(tag, msg) \
    if (auto* _logger = mpf::Logger::instance()) _logger->warning(tag, msg)
#define MPF_LOG_ERROR(tag, msg) \
    if (auto* _logger = mpf::Logger::instance()) _logger->error(tag, msg)

} // namespace mpf
