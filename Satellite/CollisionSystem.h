#pragma once

#include "Entity_old.h"
#include "TileMap.h"
#include <vector>
#include <memory>

/**
 * @brief Структура, содержащая результат проверки коллизии
 */
struct CollisionResult {
    bool collision;       ///< Было ли столкновение
    float adjustedX;      ///< Скорректированная X-координата
    float adjustedY;      ///< Скорректированная Y-координата
    bool slidingX;        ///< Происходит ли скольжение по оси X
    bool slidingY;        ///< Происходит ли скольжение по оси Y

    /**
     * @brief Конструктор по умолчанию
     */
    CollisionResult() :
        collision(false),
        adjustedX(0.0f),
        adjustedY(0.0f),
        slidingX(false),
        slidingY(false) {
    }
};

/**
 * @brief Система для обработки коллизий между сущностями и картой
 */
class CollisionSystem {
public:
    /**
     * @brief Конструктор
     * @param tileMap Указатель на карту для проверки коллизий с тайлами
     */
    CollisionSystem(TileMap* tileMap);

    /**
     * @brief Деструктор
     */
    ~CollisionSystem();

    /**
     * @brief Проверка возможности перемещения
     * @param fromX Начальная X координата
     * @param fromY Начальная Y координата
     * @param toX Конечная X координата
     * @param toY Конечная Y координата
     * @return true, если перемещение возможно, false в противном случае
     */
    bool canMoveTo(int fromX, int fromY, int toX, int toY);

    /**
     * @brief Проверка возможности диагонального перемещения
     * @param fromX Начальная X координата
     * @param fromY Начальная Y координата
     * @param toX Конечная X координата
     * @param toY Конечная Y координата
     * @return true, если диагональное перемещение возможно, false в противном случае
     */
    bool canMoveDiagonally(int fromX, int fromY, int toX, int toY);

    /**
     * @brief Обработка коллизий с учетом скольжения вдоль стен
     * @param currentX Текущая целочисленная X координата
     * @param currentY Текущая целочисленная Y координата
     * @param subX Текущая суб-координата X (0.0-1.0)
     * @param subY Текущая суб-координата Y (0.0-1.0)
     * @param deltaX Изменение по X
     * @param deltaY Изменение по Y
     * @param collisionSize Размер объекта для коллизий
     * @return Результат обработки коллизии
     */
    CollisionResult handleCollisionWithSliding(
        int currentX, int currentY,
        float subX, float subY,
        float deltaX, float deltaY,
        float collisionSize);

    /**
     * @brief Проверка пересечения прямоугольной области с картой
     * @param x X-координата центра области
     * @param y Y-координата центра области
     * @param width Ширина области
     * @param height Высота области
     * @return true, если есть коллизия с непроходимым тайлом
     */
    bool checkRectangleCollision(float x, float y, float width, float height);

    /**
     * @brief Проверка коллизии между двумя окружностями
     * @param x1 X-координата центра первой окружности
     * @param y1 Y-координата центра первой окружности
     * @param radius1 Радиус первой окружности
     * @param x2 X-координата центра второй окружности
     * @param y2 Y-координата центра второй окружности
     * @param radius2 Радиус второй окружности
     * @return true, если окружности пересекаются
     */
    static bool checkCircleCollision(
        float x1, float y1, float radius1,
        float x2, float y2, float radius2);

    /**
     * @brief Проверка коллизии между окружностью и прямоугольником
     * @param circleX X-координата центра окружности
     * @param circleY Y-координата центра окружности
     * @param radius Радиус окружности
     * @param rectX X-координата центра прямоугольника
     * @param rectY Y-координата центра прямоугольника
     * @param rectWidth Ширина прямоугольника
     * @param rectHeight Высота прямоугольника
     * @return true, если есть пересечение
     */
    static bool checkCircleRectCollision(
        float circleX, float circleY, float radius,
        float rectX, float rectY, float rectWidth, float rectHeight);

private:
    TileMap* m_tileMap;                     ///< Указатель на карту
};