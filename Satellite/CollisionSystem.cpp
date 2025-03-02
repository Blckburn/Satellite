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

    // Выбираем подход в зависимости от типа формы
    if (shape.getType() == CollisionShape::Type::CIRCLE) {
        float radius = shape.getRadius();

        // Используем сетку тайлов вокруг центра круга
        int startTileX = static_cast<int>(std::floor(nextX - radius));
        int startTileY = static_cast<int>(std::floor(nextY - radius));
        int endTileX = static_cast<int>(std::ceil(nextX + radius));
        int endTileY = static_cast<int>(std::ceil(nextY + radius));

        for (int tileY = startTileY; tileY < endTileY; tileY++) {
            for (int tileX = startTileX; tileX < endTileX; tileX++) {
                // Проверяем, находится ли тайл в пределах карты и проходим ли он
                if (!m_tileMap->isValidCoordinate(tileX, tileY) ||
                    !m_tileMap->isTileWalkable(tileX, tileY)) {

                    // Определяем ближайшую точку тайла к центру круга
                    float closestX = std::max(static_cast<float>(tileX),
                        std::min(nextX, static_cast<float>(tileX + 1.0f)));
                    float closestY = std::max(static_cast<float>(tileY),
                        std::min(nextY, static_cast<float>(tileY + 1.0f)));

                    // Вычисляем расстояние от центра круга до ближайшей точки тайла
                    float distanceX = nextX - closestX;
                    float distanceY = nextY - closestY;
                    float distanceSquared = distanceX * distanceX + distanceY * distanceY;

                    // Коллизия обнаружена, если расстояние меньше радиуса
                    if (distanceSquared < radius * radius) {
                        info.hasCollision = true;
                        info.tileX = tileX;
                        info.tileY = tileY;

                        // Вычисляем глубину проникновения
                        float distance = std::sqrt(distanceSquared);

                        // Избегаем деления на ноль
                        if (distance > 0.001f) {
                            float overlap = radius - distance;
                            info.penetrationX = overlap * (distanceX / distance);
                            info.penetrationY = overlap * (distanceY / distance);
                        }
                        else {
                            // Если центр круга находится прямо на границе тайла
                            info.penetrationX = radius;
                            info.penetrationY = 0.0f;
                        }

                        // Снижаем величину проникновения для более плавного движения
                        info.penetrationX *= 0.8f;
                        info.penetrationY *= 0.8f;

                        return info; // Возвращаем первую обнаруженную коллизию
                    }
                }
            }
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

    // Добавляем небольшой буфер, чтобы разрешить близкое приближение к стенам
    CollisionShape reducedShape(shape.getType() == CollisionShape::Type::CIRCLE ?
        shape.getRadius() * 0.95f : shape.getRadius());

    CollisionInfo info = checkMapCollision(entity, nextX, nextY, reducedShape);

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

    // 1. Сначала проверяем коллизию при полном перемещении
    CollisionInfo info = checkMapCollision(entity, nextX, nextY, shape);

    // 2. Если нет коллизии, просто перемещаем сущность
    if (!info.hasCollision) {
        entity->setPosition(nextX, nextY, position.z);
        return info;
    }

    // 3. Если включено скольжение вдоль стен, пробуем частичное перемещение
    if (slideAlongWalls) {
        // 3.1 Вычисляем направление движения
        float length = std::sqrt(deltaX * deltaX + deltaY * deltaY);
        if (length < 0.0001f) return info; // Предотвращаем деление на очень маленькие числа

        float dirX = deltaX / length;
        float dirY = deltaY / length;

        // 3.2 Постепенно увеличиваем расстояние перемещения до обнаружения коллизии
        float safeDistance = 0.0f;
        float maxDistance = length;
        float testDistance = maxDistance;
        float precision = 0.01f; // Точность поиска безопасного расстояния

        // Используем двоичный поиск для нахождения максимального безопасного расстояния
        while (maxDistance - safeDistance > precision) {
            testDistance = (safeDistance + maxDistance) * 0.5f;
            float testX = position.x + dirX * testDistance;
            float testY = position.y + dirY * testDistance;

            if (!checkMapCollision(entity, testX, testY, shape).hasCollision) {
                safeDistance = testDistance;
            }
            else {
                maxDistance = testDistance;
            }
        }

        // 3.3 Перемещаем на безопасное расстояние
        if (safeDistance > 0.0f) {
            float safeX = position.x + dirX * safeDistance;
            float safeY = position.y + dirY * safeDistance;
            entity->setPosition(safeX, safeY, position.z);

            // 3.4 Пробуем скользить вдоль оси, перпендикулярной направлению столкновения
            // После перемещения на безопасное расстояние, проверяем возможность движения по осям X и Y
            float remainingDeltaX = nextX - safeX;
            float remainingDeltaY = nextY - safeY;

            // Проверяем движение только по X
            CollisionInfo infoX = checkMapCollision(entity, safeX + remainingDeltaX, safeY, shape);
            if (!infoX.hasCollision) {
                entity->setPosition(safeX + remainingDeltaX, safeY, position.z);
            }

            // Обновляем текущую позицию после возможного перемещения по X
            const auto& updatedPos = entity->getPosition();

            // Проверяем движение только по Y
            CollisionInfo infoY = checkMapCollision(entity, updatedPos.x, safeY + remainingDeltaY, shape);
            if (!infoY.hasCollision) {
                entity->setPosition(updatedPos.x, safeY + remainingDeltaY, position.z);
            }

            // Информация о коллизии изменилась, так как мы выполнили частичное перемещение
            info.hasCollision = false;
        }
    }

    // 4. Сохраняем информацию о коллизии для отладки
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