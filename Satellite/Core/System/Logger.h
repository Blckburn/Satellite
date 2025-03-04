/**
 * @file Logger.h
 * @brief Система логирования для движка Satellite
 */

#pragma once

#include "Core/Base/Types.h"
#include <fstream>
#include <mutex>
#include <iostream>

namespace Satellite {

/**
 * @brief Класс для управления логированием
 */
class Logger {
public:
    /**
     * @brief Получение экземпляра синглтона
     * @return Ссылка на экземпляр логгера
     */
    static Logger& getInstance();

    /**
     * @brief Инициализация логгера
     * @param logToFile Флаг логирования в файл
     * @param logFileName Имя файла для логирования
     * @param consoleLogLevel Уровень логирования для консоли
     * @param fileLogLevel Уровень логирования для файла
     */
    void initialize(bool logToFile = false, const std::string& logFileName = "satellite.log",
        LogLevel consoleLogLevel = LogLevel::INFO, LogLevel fileLogLevel = LogLevel::DEBUG);

    /**
     * @brief Завершение работы логгера
     */
    void shutdown();

    /**
     * @brief Установка уровня логирования для консоли
     * @param level Новый уровень логирования
     */
    void setConsoleLogLevel(LogLevel level);

    /**
     * @brief Установка уровня логирования для файла
     * @param level Новый уровень логирования
     */
    void setFileLogLevel(LogLevel level);

    /**
     * @brief Логирование на уровне Debug
     * @param message Сообщение для логирования
     */
    void debug(const std::string& message);

    /**
     * @brief Логирование на уровне Info
     * @param message Сообщение для логирования
     */
    void info(const std::string& message);

    /**
     * @brief Логирование на уровне Warning
     * @param message Сообщение для логирования
     */
    void warning(const std::string& message);

    /**
     * @brief Логирование на уровне Error
     * @param message Сообщение для логирования
     */
    void error(const std::string& message);

private:
    /**
     * @brief Приватный конструктор (паттерн Singleton)
     */
    Logger();

    /**
     * @brief Деструктор
     */
    ~Logger();

    /**
     * @brief Запрет копирования
     */
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Внутренний метод логирования
     * @param level Уровень логирования
     * @param message Сообщение для логирования
     */
    void log(LogLevel level, const std::string& message);

private:
    bool m_logToFile;                ///< Флаг логирования в файл
    std::ofstream m_logFile;         ///< Файловый поток для логирования
    LogLevel m_consoleLogLevel;      ///< Уровень логирования для консоли
    LogLevel m_fileLogLevel;         ///< Уровень логирования для файла
    std::mutex m_mutex;              ///< Мьютекс для синхронизации
};

/**
 * @brief Запись в лог с указанным уровнем
 * @param level Уровень логирования
 * @param message Сообщение для лога
 */
void Log(LogLevel level, const std::string& message);

/**
 * @brief Запись отладочного сообщения в лог
 * @param message Сообщение для лога
 */
inline void LogDebug(const std::string& message) { Log(LogLevel::DEBUG, message); }

/**
 * @brief Запись информационного сообщения в лог
 * @param message Сообщение для лога
 */
inline void LogInfo(const std::string& message) { Log(LogLevel::INFO, message); }

/**
 * @brief Запись предупреждения в лог
 * @param message Сообщение для лога
 */
inline void LogWarning(const std::string& message) { Log(LogLevel::WARNING, message); }

/**
 * @brief Запись ошибки в лог
 * @param message Сообщение для лога
 */
inline void LogError(const std::string& message) { Log(LogLevel::ERROR, message); }

} // namespace Satellite

// Определения макросов для удобного логирования
#define LOG_DEBUG(msg) Satellite::LogDebug(msg)
#define LOG_INFO(msg) Satellite::LogInfo(msg)
#define LOG_WARNING(msg) Satellite::LogWarning(msg)
#define LOG_ERROR(msg) Satellite::LogError(msg)