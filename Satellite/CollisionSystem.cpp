#include "CollisionSystem.h"
#include <cmath>
#include "Logger.h"

CollisionSystem::CollisionSystem(TileMap* tileMap)
    : m_tileMap(tileMap) {
    LOG_INFO("CollisionSystem initialized");
}

CollisionSystem::~CollisionSystem() {
    // Освобождение ресурсов, если они были выделены
    LOG_INFO("CollisionSystem destroyed");
}

bool CollisionSystem::canMoveTo(int fromX, int fromY, int toX, int toY) {
    // Проверяем, что клетка назначения находится в пределах карты
    if (!m_tileMap->isValidCoordinate(toX, toY)) {
        return false;
    }

    // Проверяем, что клетка назначения проходима
    return m_tileMap->isTileWalkable(toX, toY);
}

bool CollisionSystem::canMoveDiagonally(int fromX, int fromY, int toX, int toY) {
    // Убедимся, что это действительно диагональное перемещение
    int dx = toX - fromX;
    int dy = toY - fromY;

    if (abs(dx) != 1 || abs(dy) != 1) {
        // Не диагональное перемещение, используем обычную проверку
        return canMoveTo(fromX, fromY, toX, toY);
    }

    // Проверяем целевой тайл
    if (!canMoveTo(fromX, fromY, toX, toY)) {
        return false;
    }

    // Проверка обоих промежуточных тайлов для избежания "срезания углов"
    // Оба тайла должны быть проходимыми
    bool canPassX = canMoveTo(fromX, fromY, toX, fromY);
    bool canPassY = canMoveTo(fromX, fromY, fromX, toY);

    return canPassX && canPassY;
}