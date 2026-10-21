#include "logging/Logger.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <filesystem>
#include <stdexcept>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace {
    std::atomic<int> loggerCounter{0};

    std::string toLower(std::string value) {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        );

        return value;
    }
}

Logger::Logger(const std::string& logFilePath, LogLevel minLevel) {
    if (logFilePath.empty()) {
        throw std::invalid_argument("Log file path is empty");
    }

    std::filesystem::path path(logFilePath);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }

    std::string loggerName =
        "dentobase_logger_" + std::to_string(loggerCounter.fetch_add(1));

    logger = spdlog::basic_logger_mt(loggerName, logFilePath, true);
    logger->set_level(toSpdlogLevel(minLevel));
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    logger->flush_on(spdlog::level::debug);
}

void Logger::debug(const std::string& message) {
    logger->debug(message);
}

void Logger::info(const std::string& message) {
    logger->info(message);
}

void Logger::warning(const std::string& message) {
    logger->warn(message);
}

void Logger::error(const std::string& message) {
    logger->error(message);
}

LogLevel Logger::fromString(const std::string& value) {
    std::string lower = toLower(value);

    if (lower == "debug") {
        return LogLevel::DEBUG;
    }

    if (lower == "release") {
        return LogLevel::INFO;
    }

    if (lower == "info") {
        return LogLevel::INFO;
    }

    if (lower == "warning" || lower == "warn") {
        return LogLevel::WARNING;
    }

    if (lower == "error") {
        return LogLevel::ERROR;
    }

    throw std::invalid_argument("Unknown log level: " + value);
}

spdlog::level::level_enum Logger::toSpdlogLevel(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return spdlog::level::debug;
        case LogLevel::INFO:
            return spdlog::level::info;
        case LogLevel::WARNING:
            return spdlog::level::warn;
        case LogLevel::ERROR:
            return spdlog::level::err;
        default:
            return spdlog::level::info;
    }
}
