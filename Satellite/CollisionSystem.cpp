#include "CollisionSystem.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>

// -----------------------------------------------------------------------------
// CollisionShape implementation
// -----------------------------------------------------------------------------

CollisionShape::CollisionShape(float radius)
    : m_type(Type::CIRCLE), m_radius(radius), m_width(0.0f), m_height(0.0f) {
}

CollisionShape::CollisionShape(float width, float height)
    : m_type(Type::RECTANGLE), m_radius(0.0f), m_width(width), m_height(height) {
}

CollisionShape::CollisionShape()
    : m_type(Type::POINT), m_radius(0.0f), m_width(0.0f), m_height(0.0f) {
}

// -----------------------------------------------------------------------------
// CollisionSystem implementation
// -----------------------------------------------------------------------------

CollisionSystem::CollisionSystem(std::shared_ptr<TileMap> tileMap)
    : m_tileMap(tileMap) {
}

CollisionInfo CollisionSystem::checkMapCollision(
    Entity* entity, float nextX, float nextY, const CollisionShape& shape) {

    CollisionInfo info;
    info.entityA = entity;

    // Получаем текущие координаты сущности
    const auto& position = entity->getPosition();
    float currentX = position.x;
    float currentY = position.y;

    // Проверяем коллизию в зависимости от типа формы
    if (shape.getType() == CollisionShape::Type::POINT) {
        // Для точечной формы просто проверяем, проходим ли тайл
        int tileX = static_cast<int>(std::floor(nextX));
        int tileY = static_cast<int>(std::floor(nextY));

        if (!m_tileMap->isValidCoordinate(tileX, tileY) ||
            !m_tileMap->isTileWalkable(tileX, tileY)) {

            info.hasCollision = true;
            info.tileX = tileX;
            info.tileY = tileY;

            // Вычисляем глубину проникновения
            float centerX = tileX + 0.5f;
            float centerY = tileY + 0.5f;
            info.penetrationX = (nextX > centerX) ? (tileX + 1.0f - nextX) : (tileX - nextX);
            info.penetrationY = (nextY > centerY) ? (tileY + 1.0f - nextY) : (tileY - nextY);
        }
    }
    else if (shape.getType() == CollisionShape::Type::RECTANGLE) {
        // Для прямоугольной формы проверяем все углы
        float halfWidth = shape.getWidth() / 2.0f;
        float halfHeight = shape.getHeight() / 2.0f;

        // Координаты углов прямоугольника
        float corners[4][2] = {
            {nextX - halfWidth, nextY - halfHeight}, // Верхний левый
            {nextX + halfWidth, nextY - halfHeight}, // Верхний правый
            {nextX + halfWidth, nextY + halfHeight}, // Нижний правый
            {nextX - halfWidth, nextY + halfHeight}  // Нижний левый
        };

        for (int i = 0; i < 4; i++) {
            float cornerX = corners[i][0];
            float cornerY = corners[i][1];

            int tileX = static_cast<int>(std::floor(cornerX));
            int tileY = static_cast<int>(std::floor(cornerY));

            if (!m_tileMap->isValidCoordinate(tileX, tileY) ||
                !m_tileMap->isTileWalkable(tileX, tileY)) {

                info.hasCollision = true;
                info.tileX = tileX;
                info.tileY = tileY;

                // Вычисляем глубину проникновения
                // Упрощенный расчет для примера, в реальности нужен более сложный алгоритм
                float tileCornerX = (cornerX > tileX + 0.5f) ? tileX + 1.0f : tileX;
                float tileCornerY = (cornerY > tileY + 0.5f) ? tileY + 1.0f : tileY;

                float penX = std::abs(cornerX - tileCornerX);
                float penY = std::abs(cornerY - tileCornerY);

                // Сохраняем наибольшую глубину проникновения
                if (penX > info.penetrationX) info.penetrationX = penX;
                if (penY > info.penetrationY) info.penetrationY = penY;

                // Для упрощения берем только первую коллизию
                break;
            }
        }
    }
    else if (shape.getType() == CollisionShape::Type::CIRCLE) {
        // Для круговой формы проверяем пересечение с тайлами
        float radius = shape.getRadius();

        // Определяем границы тайлов, которые нужно проверить
        int startTileX = static_cast<int>(std::floor(nextX - radius));
        int startTileY = static_cast<int>(std::floor(nextY - radius));
        int endTileX = static_cast<int>(std::floor(nextX + radius));
        int endTileY = static_cast<int>(std::floor(nextY + radius));

        for (int tileY = startTileY; tileY <= endTileY; tileY++) {
            for (int tileX = startTileX; tileX <= endTileX; tileX++) {
                if (!m_tileMap->isValidCoordinate(tileX, tileY) ||
                    !m_tileMap->isTileWalkable(tileX, tileY)) {

                    // Проверяем расстояние от центра круга до ближайшей точки тайла
                    float closestX = std::max(static_cast<float>(tileX),
                        std::min(nextX, static_cast<float>(tileX + 1.0f)));
                    float closestY = std::max(static_cast<float>(tileY),
                        std::min(nextY, static_cast<float>(tileY + 1.0f)));

                    float distanceX = nextX - closestX;
                    float distanceY = nextY - closestY;
                    float distanceSquared = distanceX * distanceX + distanceY * distanceY;

                    if (distanceSquared < radius * radius) {
                        info.hasCollision = true;
                        info.tileX = tileX;
                        info.tileY = tileY;

                        // Вычисляем глубину проникновения
                        float distance = std::sqrt(distanceSquared);
                        float overlapDistance = radius - distance;

                        if (distance > 0) {
                            info.penetrationX = overlapDistance * (distanceX / distance);
                            info.penetrationY = overlapDistance * (distanceY / distance);
                        }
                        else {
                            // В случае, если центр круга находится точно на границе тайла
                            info.penetrationX = radius;
                            info.penetrationY = 0.0f;
                        }

                        // Для упрощения берем только первую коллизию
                        break;
                    }
                }
            }
            if (info.hasCollision) break;
        }
    }

    return info;
}

CollisionInfo CollisionSystem::checkEntityCollision(
    Entity* entityA, const CollisionShape& shapeA,
    Entity* entityB, const CollisionShape& shapeB) {

    CollisionInfo info;
    info.entityA = entityA;
    info.entityB = entityB;

    // Получаем позиции сущностей
    const auto& posA = entityA->getPosition();
    const auto& posB = entityB->getPosition();

    // Проверяем коллизию в зависимости от типов форм
    float penetrationX = 0.0f;
    float penetrationY = 0.0f;

    if (shapeA.getType() == CollisionShape::Type::RECTANGLE &&
        shapeB.getType() == CollisionShape::Type::RECTANGLE) {

        // Прямоугольник-прямоугольник
        info.hasCollision = checkRectRectCollision(
            posA.x, posA.y, shapeA.getWidth(), shapeA.getHeight(),
            posB.x, posB.y, shapeB.getWidth(), shapeB.getHeight(),
            penetrationX, penetrationY
        );
    }
    else if (shapeA.getType() == CollisionShape::Type::CIRCLE &&
        shapeB.getType() == CollisionShape::Type::CIRCLE) {

        // Круг-круг
        info.hasCollision = checkCircleCircleCollision(
            posA.x, posA.y, shapeA.getRadius(),
            posB.x, posB.y, shapeB.getRadius(),
            penetrationX, penetrationY
        );
    }
    else if (shapeA.getType() == CollisionShape::Type::RECTANGLE &&
        shapeB.getType() == CollisionShape::Type::CIRCLE) {

        // Прямоугольник-круг
        info.hasCollision = checkRectCircleCollision(
            posA.x, posA.y, shapeA.getWidth(), shapeA.getHeight(),
            posB.x, posB.y, shapeB.getRadius(),
            penetrationX, penetrationY
        );
    }
    else if (shapeA.getType() == CollisionShape::Type::CIRCLE &&
        shapeB.getType() == CollisionShape::Type::RECTANGLE) {

        // Круг-прямоугольник
        info.hasCollision = checkRectCircleCollision(
            posB.x, posB.y, shapeB.getWidth(), shapeB.getHeight(),
            posA.x, posA.y, shapeA.getRadius(),
            penetrationX, penetrationY
        );

        // Меняем знак проникновения, т.к. мы поменяли порядок аргументов
        penetrationX = -penetrationX;
        penetrationY = -penetrationY;
    }

    info.penetrationX = penetrationX;
    info.penetrationY = penetrationY;

    return info;
}

std::vector<CollisionInfo> CollisionSystem::checkAllCollisions(
    const std::vector<std::shared_ptr<Entity>>& entities) {

    std::vector<CollisionInfo> collisions;

    // Проверяем коллизии между всеми парами сущностей
    // Это наивная реализация с O(n²) сложностью, для оптимизации можно использовать
    // пространственное разбиение (например, сетка или quadtree)
    for (size_t i = 0; i < entities.size(); i++) {
        for (size_t j = i + 1; j < entities.size(); j++) {
            // ToDo: Получить формы коллизий для сущностей
            // В этой упрощенной версии предполагаем, что у сущностей есть метод getCollisionShape()

            // CollisionInfo info = checkEntityCollision(
            //     entities[i].get(), entities[i]->getCollisionShape(),
            //     entities[j].get(), entities[j]->getCollisionShape()
            // );

            // if (info.hasCollision) {
            //     collisions.push_back(info);
            // }
        }
    }

    return collisions;
}

void CollisionSystem::resolveCollision(CollisionInfo& info) {
    if (!info.hasCollision) return;

    // Если это коллизия с тайлом
    if (info.entityB == nullptr) {
        // Смещаем сущность назад для устранения проникновения
        const auto& position = info.entityA->getPosition();

        // Определяем направление отталкивания
        // Для простоты выбираем направление с меньшей глубиной проникновения
        if (std::abs(info.penetrationX) < std::abs(info.penetrationY)) {
            // Отталкивание по X
            float newX = position.x - info.penetrationX;
            info.entityA->setPosition(newX, position.y, position.z);
        }
        else {
            // Отталкивание по Y
            float newY = position.y - info.penetrationY;
            info.entityA->setPosition(position.x, newY, position.z);
        }
    }
    else {
        // Коллизия между двумя сущностями
        const auto& posA = info.entityA->getPosition();
        const auto& posB = info.entityB->getPosition();

        // Вектор от A к B
        float dirX = posB.x - posA.x;
        float dirY = posB.y - posA.y;

        // Нормализуем вектор
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0.0f) {
            dirX /= length;
            dirY /= length;
        }
        else {
            // Если сущности находятся в одной точке, отталкиваем в случайном направлении
            dirX = 1.0f;
            dirY = 0.0f;
        }

        // Вычисляем величину отталкивания
        float pushDistanceA = info.penetrationX * 0.5f;
        float pushDistanceB = info.penetrationX * 0.5f;

        // Отталкиваем обе сущности
        info.entityA->setPosition(posA.x - dirX * pushDistanceA, posA.y - dirY * pushDistanceA, posA.z);
        info.entityB->setPosition(posB.x + dirX * pushDistanceB, posB.y + dirY * pushDistanceB, posB.z);
    }
}

bool CollisionSystem::canMove(
    Entity* entity, const CollisionShape& shape,
    float nextX, float nextY, CollisionInfo* outCollision) {

    CollisionInfo info = checkMapCollision(entity, nextX, nextY, shape);

    if (outCollision) {
        *outCollision = info;
    }

    return !info.hasCollision;
}

CollisionInfo CollisionSystem::tryMove(
    Entity* entity, const CollisionShape& shape,
    float deltaX, float deltaY, bool slideAlongWalls) {

    const auto& position = entity->getPosition();
    float nextX = position.x + deltaX;
    float nextY = position.y + deltaY;

    // Проверяем коллизию при перемещении
    CollisionInfo info = checkMapCollision(entity, nextX, nextY, shape);

    if (!info.hasCollision) {
        // Если нет коллизии, перемещаем сущность
        entity->setPosition(nextX, nextY, position.z);
    }
    else if (slideAlongWalls) {
        // Пытаемся скользить вдоль стены

        // Проверяем движение только по X
        CollisionInfo infoX = checkMapCollision(entity, nextX, position.y, shape);
        if (!infoX.hasCollision) {
            entity->setPosition(nextX, position.y, position.z);
        }

        // Проверяем движение только по Y
        CollisionInfo infoY = checkMapCollision(entity, position.x, nextY, shape);
        if (!infoY.hasCollision) {
            entity->setPosition(position.x, nextY, position.z);
        }
    }

    // Сохраняем информацию о коллизии для отладки
    if (info.hasCollision) {
        m_debugCollisions.push_back(info);

        // Ограничиваем количество сохраненных коллизий
        if (m_debugCollisions.size() > 10) {
            m_debugCollisions.erase(m_debugCollisions.begin());
        }
    }

    return info;
}

bool CollisionSystem::isPointWalkable(float x, float y) {
    int tileX = static_cast<int>(std::floor(x));
    int tileY = static_cast<int>(std::floor(y));

    return m_tileMap->isValidCoordinate(tileX, tileY) &&
        m_tileMap->isTileWalkable(tileX, tileY);
}

void CollisionSystem::renderDebug(SDL_Renderer* renderer, int centerX, int centerY) {
    // Отрисовка отладочной информации о коллизиях
    // Эта функция должна быть реализована с использованием IsometricRenderer
    // для преобразования мировых координат в экранные
}

// -----------------------------------------------------------------------------
// CollisionSystem private methods
// -----------------------------------------------------------------------------

bool CollisionSystem::checkPointRectCollision(
    float pointX, float pointY,
    float rectX, float rectY, float rectWidth, float rectHeight) {

    float halfWidth = rectWidth / 2.0f;
    float halfHeight = rectHeight / 2.0f;

    return (pointX >= rectX - halfWidth && pointX <= rectX + halfWidth &&
        pointY >= rectY - halfHeight && pointY <= rectY + halfHeight);
}

bool CollisionSystem::checkRectRectCollision(
    float rectAX, float rectAY, float rectAWidth, float rectAHeight,
    float rectBX, float rectBY, float rectBWidth, float rectBHeight,
    float& outPenetrationX, float& outPenetrationY) {

    float halfWidthA = rectAWidth / 2.0f;
    float halfHeightA = rectAHeight / 2.0f;
    float halfWidthB = rectBWidth / 2.0f;
    float halfHeightB = rectBHeight / 2.0f;

    // Вычисляем расстояние между центрами прямоугольников
    float distanceX = rectBX - rectAX;
    float distanceY = rectBY - rectAY;

    // Вычисляем минимальное расстояние для отсутствия пересечения
    float minDistanceX = halfWidthA + halfWidthB;
    float minDistanceY = halfHeightA + halfHeightB;

    // Проверяем пересечение
    if (std::abs(distanceX) < minDistanceX && std::abs(distanceY) < minDistanceY) {
        // Вычисляем глубину проникновения
        outPenetrationX = minDistanceX - std::abs(distanceX);
        outPenetrationY = minDistanceY - std::abs(distanceY);

        // Устанавливаем правильный знак для проникновения
        if (distanceX > 0.0f) outPenetrationX = -outPenetrationX;
        if (distanceY > 0.0f) outPenetrationY = -outPenetrationY;

        return true;
    }

    return false;
}

bool CollisionSystem::checkCircleCircleCollision(
    float circleAX, float circleAY, float circleARadius,
    float circleBX, float circleBY, float circleBRadius,
    float& outPenetrationX, float& outPenetrationY) {

    // Вычисляем расстояние между центрами кругов
    float distanceX = circleBX - circleAX;
    float distanceY = circleBY - circleAY;
    float distanceSquared = distanceX * distanceX + distanceY * distanceY;

    // Вычисляем сумму радиусов
    float radiusSum = circleARadius + circleBRadius;

    // Проверяем пересечение
    if (distanceSquared < radiusSum * radiusSum) {
        float distance = std::sqrt(distanceSquared);
        float overlapDistance = radiusSum - distance;

        // Вычисляем глубину проникновения
        if (distance > 0.0f) {
            outPenetrationX = overlapDistance * (distanceX / distance);
            outPenetrationY = overlapDistance * (distanceY / distance);
        }
        else {
            // Если центры кругов совпадают, отталкиваем в произвольном направлении
            outPenetrationX = radiusSum;
            outPenetrationY = 0.0f;
        }

        return true;
    }

    return false;
}

bool CollisionSystem::checkRectCircleCollision(
    float rectX, float rectY, float rectWidth, float rectHeight,
    float circleX, float circleY, float circleRadius,
    float& outPenetrationX, float& outPenetrationY) {

    float halfWidth = rectWidth / 2.0f;
    float halfHeight = rectHeight / 2.0f;

    // Находим ближайшую к центру круга точку прямоугольника
    float closestX = std::max(rectX - halfWidth, std::min(circleX, rectX + halfWidth));
    float closestY = std::max(rectY - halfHeight, std::min(circleY, rectY + halfHeight));

    // Вычисляем расстояние от центра круга до ближайшей точки прямоугольника
    float distanceX = circleX - closestX;
    float distanceY = circleY - closestY;
    float distanceSquared = distanceX * distanceX + distanceY * distanceY;

    // Проверяем пересечение
    if (distanceSquared < circleRadius * circleRadius) {
        float distance = std::sqrt(distanceSquared);
        float overlapDistance = circleRadius - distance;

        // Вычисляем глубину проникновения
        if (distance > 0.0f) {
            outPenetrationX = overlapDistance * (distanceX / distance);
            outPenetrationY = overlapDistance * (distanceY / distance);
        }
        else {
            // Если центр круга находится внутри прямоугольника, отталкиваем в ближайшую сторону
            if (std::abs(circleX - rectX) < halfWidth) {
                outPenetrationX = 0.0f;
                outPenetrationY = (circleY > rectY) ? circleRadius : -circleRadius;
            }
            else {
                outPenetrationX = (circleX > rectX) ? circleRadius : -circleRadius;
                outPenetrationY = 0.0f;
            }
        }

        return true;
    }

    return false;
}