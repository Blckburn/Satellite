/**
 * @file StringUtils.h
 * @brief Утилитные функции для работы со строками
 */

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Satellite {
namespace Utils {

/**
 * @brief Утилиты для работы со строками
 */
class StringUtils {
public:
    /**
     * @brief Разделяет строку на подстроки по разделителю
     * @param str Исходная строка
     * @param delimiter Разделитель
     * @return Вектор подстрок
     */
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        
        return tokens;
    }
    
    /**
     * @brief Объединяет вектор строк в одну строку с разделителем
     * @param strings Вектор строк
     * @param delimiter Разделитель
     * @return Объединенная строка
     */
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter) {
        std::string result;
        bool first = true;
        
        for (const auto& str : strings) {
            if (!first) {
                result += delimiter;
            }
            result += str;
            first = false;
        }
        
        return result;
    }
    
    /**
     * @brief Удаляет пробельные символы в начале строки
     * @param str Исходная строка
     * @return Строка без пробелов в начале
     */
    static std::string trimLeft(const std::string& str) {
        auto it = std::find_if(str.begin(), str.end(), [](char ch) {
            return !std::isspace(static_cast<unsigned char>(ch));
        });
        return it == str.end() ? "" : std::string(it, str.end());
    }
    
    /**
     * @brief Удаляет пробельные символы в конце строки
     * @param str Исходная строка
     * @return Строка без пробелов в конце
     */
    static std::string trimRight(const std::string& str) {
        auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) {
            return !std::isspace(static_cast<unsigned char>(ch));
        });
        return it == str.rend() ? "" : std::string(str.begin(), it.base());
    }
    
    /**
     * @brief Удаляет пробельные символы в начале и конце строки
     * @param str Исходная строка
     * @return Строка без пробелов в начале и конце
     */
    static std::string trim(const std::string& str) {
        return trimLeft(trimRight(str));
    }
    
    /**
     * @brief Преобразует строку к нижнему регистру
     * @param str Исходная строка
     * @return Строка в нижнем регистре
     */
    static std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
            return std::tolower(c);
        });
        return result;
    }
    
    /**
     * @brief Преобразует строку к верхнему регистру
     * @param str Исходная строка
     * @return Строка в верхнем регистре
     */
    static std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
            return std::toupper(c);
        });
        return result;
    }
    
    /**
     * @brief Проверяет, начинается ли строка с указанного префикса
     * @param str Строка для проверки
     * @param prefix Префикс
     * @return true, если строка начинается с префикса, иначе false
     */
    static bool startsWith(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }
    
    /**
     * @brief Проверяет, заканчивается ли строка указанным суффиксом
     * @param str Строка для проверки
     * @param suffix Суффикс
     * @return true, если строка заканчивается суффиксом, иначе false
     */
    static bool endsWith(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() && 
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
    
    /**
     * @brief Заменяет все вхождения подстроки в строке
     * @param str Исходная строка
     * @param from Подстрока для замены
     * @param to Строка, на которую заменяется подстрока
     * @return Строка с замененными подстроками
     */
    static std::string replace(const std::string& str, const std::string& from, const std::string& to) {
        std::string result = str;
        size_t startPos = 0;
        
        while ((startPos = result.find(from, startPos)) != std::string::npos) {
            result.replace(startPos, from.length(), to);
            startPos += to.length();
        }
        
        return result;
    }
    
    /**
     * @brief Преобразует строку в целое число
     * @param str Строка для преобразования
     * @param defaultValue Значение по умолчанию, если преобразование не удалось
     * @return Целое число или значение по умолчанию
     */
    static int toInt(const std::string& str, int defaultValue = 0) {
        try {
            return std::stoi(str);
        } catch (...) {
            return defaultValue;
        }
    }
    
    /**
     * @brief Преобразует строку в число с плавающей точкой
     * @param str Строка для преобразования
     * @param defaultValue Значение по умолчанию, если преобразование не удалось
     * @return Число с плавающей точкой или значение по умолчанию
     */
    static float toFloat(const std::string& str, float defaultValue = 0.0f) {
        try {
            return std::stof(str);
        } catch (...) {
            return defaultValue;
        }
    }
    
    /**
     * @brief Преобразует строку в логическое значение
     * @param str Строка для преобразования
     * @return true, если строка равна "true", "yes", "1", иначе false
     */
    static bool toBool(const std::string& str) {
        std::string lower = toLower(trim(str));
        return lower == "true" || lower == "yes" || lower == "1";
    }
    
    /**
     * @brief Преобразует число в строку с указанной точностью
     * @param value Значение для преобразования
     * @param precision Количество знаков после запятой
     * @return Строковое представление числа
     */
    static std::string toString(float value, int precision = 2) {
        std::ostringstream stream;
        stream.precision(precision);
        stream << std::fixed << value;
        return stream.str();
    }
};

} // namespace Utils
} // namespace Satellite