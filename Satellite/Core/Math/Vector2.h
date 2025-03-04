/**
 * @file Vector2.h
 * @brief Класс двумерного вектора для математических операций
 */

#pragma once

#include <cmath>

namespace Satellite {
namespace Math {

/**
 * @brief Класс двумерного вектора
 */
class Vector2 {
public:
    float x;  ///< X компонента
    float y;  ///< Y компонента

    /**
     * @brief Конструктор по умолчанию
     */
    Vector2() : x(0.0f), y(0.0f) {}

    /**
     * @brief Конструктор с инициализацией
     * @param x X компонента
     * @param y Y компонента
     */
    Vector2(float x, float y) : x(x), y(y) {}

    /**
     * @brief Оператор сложения
     * @param other Другой вектор
     * @return Результат сложения
     */
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    /**
     * @brief Оператор вычитания
     * @param other Другой вектор
     * @return Результат вычитания
     */
    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    /**
     * @brief Оператор умножения на скаляр
     * @param scalar Скаляр
     * @return Результат умножения
     */
    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    /**
     * @brief Оператор деления на скаляр
     * @param scalar Скаляр
     * @return Результат деления
     */
    Vector2 operator/(float scalar) const {
        return Vector2(x / scalar, y / scalar);
    }

    /**
     * @brief Оператор сложения с присваиванием
     * @param other Другой вектор
     * @return Ссылка на этот вектор
     */
    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    /**
     * @brief Оператор вычитания с присваиванием
     * @param other Другой вектор
     * @return Ссылка на этот вектор
     */
    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    /**
     * @brief Оператор умножения на скаляр с присваиванием
     * @param scalar Скаляр
     * @return Ссылка на этот вектор
     */
    Vector2& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    /**
     * @brief Оператор деления на скаляр с присваиванием
     * @param scalar Скаляр
     * @return Ссылка на этот вектор
     */
    Vector2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    /**
     * @brief Оператор сравнения
     * @param other Другой вектор
     * @return true, если векторы равны, false в противном случае
     */
    bool operator==(const Vector2& other) const {
        const float epsilon = 0.0001f;
        return std::abs(x - other.x) < epsilon && std::abs(y - other.y) < epsilon;
    }

    /**
     * @brief Оператор неравенства
     * @param other Другой вектор
     * @return true, если векторы не равны, false в противном случае
     */
    bool operator!=(const Vector2& other) const {
        return !(*this == other);
    }

    /**
     * @brief Вычисление длины вектора
     * @return Длина вектора
     */
    float length() const {
        return std::sqrt(x * x + y * y);
    }

    /**
     * @brief Вычисление квадрата длины вектора
     * @return Квадрат длины вектора
     */
    float lengthSquared() const {
        return x * x + y * y;
    }

    /**
     * @brief Нормализация вектора
     * @return Нормализованный вектор
     */
    Vector2 normalized() const {
        float len = length();
        if (len > 0.0f) {
            return Vector2(x / len, y / len);
        }
        return *this;
    }

    /**
     * @brief Нормализация текущего вектора
     * @return Ссылка на этот вектор
     */
    Vector2& normalize() {
        float len = length();
        if (len > 0.0f) {
            x /= len;
            y /= len;
        }
        return *this;
    }

    /**
     * @brief Скалярное произведение двух векторов
     * @param other Другой вектор
     * @return Результат скалярного произведения
     */
    float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    /**
     * @brief Расстояние между двумя векторами
     * @param other Другой вектор
     * @return Расстояние
     */
    float distance(const Vector2& other) const {
        return (*this - other).length();
    }

    /**
     * @brief Квадрат расстояния между двумя векторами
     * @param other Другой вектор
     * @return Квадрат расстояния
     */
    float distanceSquared(const Vector2& other) const {
        return (*this - other).lengthSquared();
    }

    /**
     * @brief Нулевой вектор
     */
    static const Vector2 zero;

    /**
     * @brief Единичный вектор по X
     */
    static const Vector2 unitX;

    /**
     * @brief Единичный вектор по Y
     */
    static const Vector2 unitY;
};

// Определение статических константных членов
inline const Vector2 Vector2::zero(0.0f, 0.0f);
inline const Vector2 Vector2::unitX(1.0f, 0.0f);
inline const Vector2 Vector2::unitY(0.0f, 1.0f);

} // namespace Math
} // namespace Satellite