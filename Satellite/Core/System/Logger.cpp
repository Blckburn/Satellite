/**
 * @file Logger.cpp
 * @brief Реализация системы логирования
 */

#include "Core/System/Logger.h"
#include <iomanip>
#include <ctime>

namespace Satellite {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() 
    : m_logToFile(false), 
      m_consoleLogLevel(LogLevel::INFO), 
      m_fileLogLevel(LogLevel::DEBUG) {
}

Logger::~Logger() {
    shutdown();
}

void Logger::initialize(bool logToFile, const std::string& logFileName,
    LogLevel consoleLogLevel, LogLevel fileLogLevel) {
    m_logToFile = logToFile;
    m_consoleLogLevel = consoleLogLevel;
    m_fileLogLevel = fileLogLevel;

    if (m_logToFile) {
        m_logFile.open(logFileName, std::ios::out | std::ios::trunc);
        if (!m_logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFileName << std::endl;
            m_logToFile = false;
        }
    }
}

void Logger::shutdown() {
    if (m_logToFile && m_logFile.is_open()) {
        m_logFile.close();
    }
}

void Logger::setConsoleLogLevel(LogLevel level) {
    m_consoleLogLevel = level;
}

void Logger::setFileLogLevel(LogLevel level) {
    m_fileLogLevel = level;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    // Блокировка для многопоточной безопасности
    std::lock_guard<std::mutex> lock(m_mutex);

    // Формируем префикс с временем и уровнем логирования
    std::string levelStr;
    switch (level) {
    case LogLevel::DEBUG:
        levelStr = "[DEBUG]";
        break;
    case LogLevel::INFO:
        levelStr = "[INFO]";
        break;
    case LogLevel::WARNING:
        levelStr = "[WARNING]";
        break;
    case LogLevel::ERROR:
        levelStr = "[ERROR]";
        break;
    default:
        levelStr = "[UNKNOWN]";
    }

    // Получаем текущее время для префикса
    auto now = std::time(nullptr);
    std::tm tm_struct;
#ifdef _WIN32
    localtime_s(&tm_struct, &now);
#else
    localtime_r(&now, &tm_struct);
#endif
    auto tm = tm_struct;
    std::ostringstream timeStream;
    timeStream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string timeStr = timeStream.str();

    // Формируем полное сообщение
    std::string fullMessage = timeStr + " " + levelStr + " " + message;

    // Выводим в консоль, если уровень логирования позволяет
    if (m_consoleLogLevel != LogLevel::NONE && level >= m_consoleLogLevel) {
        if (level == LogLevel::ERROR) {
            std::cerr << fullMessage << std::endl;
        }
        else {
            std::cout << fullMessage << std::endl;
        }
    }

    // Записываем в файл, если включено логирование в файл и уровень позволяет
    if (m_logToFile && m_logFile.is_open() && m_fileLogLevel != LogLevel::NONE && level >= m_fileLogLevel) {
        m_logFile << fullMessage << std::endl;
        m_logFile.flush();
    }
}

void Log(LogLevel level, const std::string& message) {
    Logger::getInstance().log(level, message);
}

} // namespace Satellite