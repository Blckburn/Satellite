#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <iomanip>

/**
 * @brief Уровни логирования
 */
enum class LogLevel {
    DEBUG,      ///< Отладочная информация
    INFO,       ///< Информационное сообщение
    WARNING,    ///< Предупреждение
    ERROR,      ///< Ошибка
    NONE        ///< Вывод отключен
};

/**
 * @brief Класс для управления логированием
 */
class Logger {
public:
    /**
     * @brief Получение экземпляра синглтона
     * @return Ссылка на экземпляр логгера
     */
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief Инициализация логгера
     * @param logToFile Флаг логирования в файл
     * @param logFileName Имя файла для логирования
     * @param consoleLogLevel Уровень логирования для консоли
     * @param fileLogLevel Уровень логирования для файла
     */
    void initialize(bool logToFile = false, const std::string& logFileName = "satellite.log",
        LogLevel consoleLogLevel = LogLevel::INFO, LogLevel fileLogLevel = LogLevel::DEBUG) {
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

    /**
     * @brief Завершение работы логгера
     */
    void shutdown() {
        if (m_logToFile && m_logFile.is_open()) {
            m_logFile.close();
        }
    }

    /**
     * @brief Установка уровня логирования для консоли
     * @param level Новый уровень логирования
     */
    void setConsoleLogLevel(LogLevel level) {
        m_consoleLogLevel = level;
    }

    /**
     * @brief Установка уровня логирования для файла
     * @param level Новый уровень логирования
     */
    void setFileLogLevel(LogLevel level) {
        m_fileLogLevel = level;
    }

    /**
     * @brief Логирование на уровне Debug
     * @param message Сообщение для логирования
     */
    void debug(const std::string& message) {
        log(LogLevel::DEBUG, message);
    }

    /**
     * @brief Логирование на уровне Info
     * @param message Сообщение для логирования
     */
    void info(const std::string& message) {
        log(LogLevel::INFO, message);
    }

    /**
     * @brief Логирование на уровне Warning
     * @param message Сообщение для логирования
     */
    void warning(const std::string& message) {
        log(LogLevel::WARNING, message);
    }

    /**
     * @brief Логирование на уровне Error
     * @param message Сообщение для логирования
     */
    void error(const std::string& message) {
        log(LogLevel::ERROR, message);
    }

private:
    /**
     * @brief Приватный конструктор (паттерн Singleton)
     */
    Logger() : m_logToFile(false), m_consoleLogLevel(LogLevel::INFO), m_fileLogLevel(LogLevel::DEBUG) {}

    /**
     * @brief Деструктор
     */
    ~Logger() {
        shutdown();
    }

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
    void log(LogLevel level, const std::string& message) {
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
        auto tm = *std::localtime(&now);
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

private:
    bool m_logToFile;                ///< Флаг логирования в файл
    std::ofstream m_logFile;         ///< Файловый поток для логирования
    LogLevel m_consoleLogLevel;      ///< Уровень логирования для консоли
    LogLevel m_fileLogLevel;         ///< Уровень логирования для файла
    std::mutex m_mutex;              ///< Мьютекс для синхронизации
};

// Макросы для удобного использования
#define LOG_DEBUG(msg) Logger::getInstance().debug(msg)
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) Logger::getInstance().error(msg)

// Функция для отключения логирования в релизной сборке
inline void DisableLoggingForRelease() {
    Logger::getInstance().setConsoleLogLevel(LogLevel::NONE);
    Logger::getInstance().setFileLogLevel(LogLevel::ERROR); // Оставляем только ошибки в файле
}