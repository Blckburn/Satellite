#include "CollisionSystem.h"
#include <cmath>
#include <algorithm>
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

CollisionResult CollisionSystem::handleCollisionWithSliding(
    int currentX, int currentY,
    float subX, float subY,
    float deltaX, float deltaY,
    float collisionSize) {

    CollisionResult result;

    // Расчет будущей позиции
    float nextSubX = subX + deltaX;
    float nextSubY = subY + deltaY;

    // Определяем, выходим ли за пределы текущего тайла
    bool crossingTileBoundaryX = nextSubX >= 1.0f || nextSubX < 0.0f;
    bool crossingTileBoundaryY = nextSubY >= 1.0f || nextSubY < 0.0f;

    // Определяем целочисленные координаты следующего тайла
    int nextTileX = currentX;
    int nextTileY = currentY;

    // Вычисляем координаты следующего тайла, но НЕ меняем субкоординаты!
    if (nextSubX >= 1.0f) {
        nextTileX = currentX + 1;
    }
    else if (nextSubX < 0.0f) {
        nextTileX = currentX - 1;
    }

    if (nextSubY >= 1.0f) {
        nextTileY = currentY + 1;
    }
    else if (nextSubY < 0.0f) {
        nextTileY = currentY - 1;
    }

    // Проверка, есть ли коллизия при движении в новый тайл
    bool collisionX = false;
    bool collisionY = false;

    // Проверяем, можем ли мы двигаться по X
    if (nextTileX != currentX) {
        collisionX = !canMoveTo(currentX, currentY, nextTileX, currentY);
    }

    // Проверяем, можем ли мы двигаться по Y
    if (nextTileY != currentY) {
        collisionY = !canMoveTo(currentX, currentY, currentX, nextTileY);
    }

    // Проверяем диагональное движение
    bool diagonalMovement = (nextTileX != currentX && nextTileY != currentY);
    bool collisionDiagonal = false;

    if (diagonalMovement) {
        // Проверка, можем ли двигаться по диагонали
        collisionDiagonal = !canMoveDiagonally(currentX, currentY, nextTileX, nextTileY);

        // Если есть коллизия при диагональном движении, но нет коллизий по отдельным осям,
        // разрешаем скольжение
        if (collisionDiagonal && !collisionX && !collisionY) {
            // Пробуем определить, по какой оси лучше скользить

            // Вычисляем расстояние до центра тайла по каждой оси
            float centerDistX = std::abs(0.5f - subX);
            float centerDistY = std::abs(0.5f - subY);

            // Если мы ближе к центру по X, то скользим по Y
            if (centerDistX < centerDistY) {
                collisionX = true;
                collisionY = false;
            }
            else {
                collisionX = false;
                collisionY = true;
            }
        }
    }

    // Начальные значения для скорректированной позиции - используем исходные, не изменяя!
    float adjustedSubX = nextSubX;
    float adjustedSubY = nextSubY;
    bool sliding = false;

    // Применяем изменения с учетом коллизий
    if (collisionX) {
        // Если есть коллизия по X, скользим вдоль стены
        // Возвращаемся к исходной субкоординате X
        adjustedSubX = subX;
        sliding = true;
    }

    if (collisionY) {
        // Если есть коллизия по Y, скользим вдоль стены
        // Возвращаемся к исходной субкоординате Y
        adjustedSubY = subY;
        sliding = true;
    }

    // Заполняем результат
    result.collision = collisionX || collisionY || collisionDiagonal;
    result.adjustedX = adjustedSubX;
    result.adjustedY = adjustedSubY;
    result.slidingX = collisionX;
    result.slidingY = collisionY;

    return result;
}

bool CollisionSystem::checkRectangleCollision(float x, float y, float width, float height) {
    // Получаем координаты тайлов, которые может пересекать прямоугольник
    int left = static_cast<int>(std::floor(x - width / 2.0f));
    int right = static_cast<int>(std::ceil(x + width / 2.0f));
    int top = static_cast<int>(std::floor(y - height / 2.0f));
    int bottom = static_cast<int>(std::ceil(y + height / 2.0f));

    // Проверяем проходимость каждого тайла
    for (int tileY = top; tileY < bottom; tileY++) {
        for (int tileX = left; tileX < right; tileX++) {
            if (!m_tileMap->isValidCoordinate(tileX, tileY) ||
                !m_tileMap->isTileWalkable(tileX, tileY)) {
                return true; // Найдена коллизия
            }
        }
    }

    return false; // Коллизий не найдено
}

bool CollisionSystem::checkCircleCollision(
    float x1, float y1, float radius1,
    float x2, float y2, float radius2) {

    // Вычисляем квадрат расстояния между центрами
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distSquared = dx * dx + dy * dy;

    // Вычисляем квадрат суммы радиусов
    float radiusSum = radius1 + radius2;
    float radiusSumSquared = radiusSum * radiusSum;

    // Если квадрат расстояния меньше или равен квадрату суммы радиусов, окружности пересекаются
    return distSquared <= radiusSumSquared;
}

bool CollisionSystem::checkCircleRectCollision(
    float circleX, float circleY, float radius,
    float rectX, float rectY, float rectWidth, float rectHeight) {

    // Вычисляем ближайшую точку прямоугольника к центру окружности
    float closestX = std::max(rectX - rectWidth / 2.0f, std::min(circleX, rectX + rectWidth / 2.0f));
    float closestY = std::max(rectY - rectHeight / 2.0f, std::min(circleY, rectY + rectHeight / 2.0f));

    // Вычисляем расстояние от центра окружности до ближайшей точки прямоугольника
    float dx = closestX - circleX;
    float dy = closestY - circleY;
    float distSquared = dx * dx + dy * dy;

    // Если квадрат расстояния меньше или равен квадрату радиуса, есть пересечение
    return distSquared <= radius * radius;
}