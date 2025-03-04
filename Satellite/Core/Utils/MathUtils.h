/**
 * @file MathUtils.h
 * @brief Утилитные математические функции
 */

#pragma once

#include <cmath>
#include <algorithm>
#include "Core/Math/Vector2.h"

namespace Satellite {
namespace Utils {

/**
 * @brief Математические утилиты
 */
class MathUtils {
public:
    /**
     * @brief Преобразует градусы в радианы
     * @param degrees Угол в градусах
     * @return Угол в радианах
     */
    static float degreesToRadians(float degrees) {
        return degrees * 0.01745329251994329576923690768489f; // PI / 180
    }

    /**
     * @brief Преобразует радианы в градусы
     * @param radians Угол в радианах
     * @return Угол в градусах
     */
    static float radiansToDegrees(float radians) {
        return radians * 57.295779513082320876798154814105f; // 180 / PI
    }

    /**
     * @brief Линейная интерполяция между двумя значениями
     * @param a Начальное значение
     * @param b Конечное значение
     * @param t Параметр интерполяции [0, 1]
     * @return Интерполированное значение
     */
    static float lerp(float a, float b, float t) {
        return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
    }

    /**
     * @brief Линейная интерполяция между двумя векторами
     * @param a Начальный вектор
     * @param b Конечный вектор
     * @param t Параметр интерполяции [0, 1]
     * @return Интерполированный вектор
     */
    static Math::Vector2 lerpVector(const Math::Vector2& a, const Math::Vector2& b, float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return Math::Vector2(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

    /**
     * @brief Ограничивает значение в заданном диапазоне
     * @param value Значение
     * @param min Минимальное значение
     * @param max Максимальное значение
     * @return Ограниченное значение
     */
    static float clamp(float value, float min, float max) {
        return std::max(min, std::min(max, value));
    }

    /**
     * @brief Проверяет, близки ли два значения друг к другу
     * @param a Первое значение
     * @param b Второе значение
     * @param epsilon Максимально допустимая разница (по умолчанию 0.0001f)
     * @return true, если значения близки, false в противном случае
     */
    static bool approximately(float a, float b, float epsilon = 0.0001f) {
        return std::abs(a - b) < epsilon;
    }

    /**
     * @brief Проверяет, близки ли два вектора друг к другу
     * @param a Первый вектор
     * @param b Второй вектор
     * @param epsilon Максимально допустимая разница (по умолчанию 0.0001f)
     * @return true, если векторы близки, false в противном случае
     */
    static bool approximatelyVector(const Math::Vector2& a, const Math::Vector2& b, float epsilon = 0.0001f) {
        return approximately(a.x, b.x, epsilon) && approximately(a.y, b.y, epsilon);
    }

    /**
     * @brief Вычисляет угол между двумя векторами
     * @param from Первый вектор
     * @param to Второй вектор
     * @return Угол в градусах
     */
    static float angleBetween(const Math::Vector2& from, const Math::Vector2& to) {
        float dot = from.x * to.x + from.y * to.y;
        float det = from.x * to.y - from.y * to.x;
        float angle = std::atan2(det, dot);
        return radiansToDegrees(angle);
    }

    /**
     * @brief Определяет направление угла между векторами (по часовой или против)
     * @param from Первый вектор
     * @param to Второй вектор
     * @return true, если угол положительный (против часовой стрелки), иначе false
     */
    static bool isAnglePositive(const Math::Vector2& from, const Math::Vector2& to) {
        return from.x * to.y - from.y * to.x > 0;
    }

    /**
     * @brief Значение PI
     */
    static constexpr float PI = 3.14159265358979323846f;
};

} // namespace Utils
} // namespace Satellite