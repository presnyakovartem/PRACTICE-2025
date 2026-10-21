#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <string>

#include <spdlog/logger.h>

// debug  : DEBUG, INFO, WARNING, ERROR
// release: INFO, WARNING, ERROR
// Также поддерживаются явные уровни info/warning/error.
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

class Logger {
public:
    Logger(const std::string& logFilePath, LogLevel minLevel);

    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    static LogLevel fromString(const std::string& value);

private:
    std::shared_ptr<spdlog::logger> logger;

    static spdlog::level::level_enum toSpdlogLevel(LogLevel level);
};

#endif // LOGGER_H
