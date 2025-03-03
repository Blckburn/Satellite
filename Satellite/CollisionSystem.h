#pragma once

#include "Entity.h"
#include "TileMap.h"
#include <vector>
#include <memory>

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

private:
    TileMap* m_tileMap;                     ///< Указатель на карту
};