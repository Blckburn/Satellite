#include "EntityManager.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>

EntityManager::EntityManager(std::shared_ptr<TileMap> tileMap)
    : m_tileMap(tileMap) {
    LOG_INFO("EntityManager initialized");
}

void EntityManager::addEntity(std::shared_ptr<Entity> entity) {
    if (!entity) {
        LOG_WARNING("Attempted to add null entity");
        return;
    }

    m_entities.push_back(entity);

    // Если это интерактивный объект, добавляем его также в список интерактивных объектов
    auto interactiveObj = std::dynamic_pointer_cast<InteractiveObject>(entity);
    if (interactiveObj) {
        addInteractiveObject(interactiveObj);
    }

    LOG_INFO("Added entity: " + entity->getName());
}

void EntityManager::removeEntity(std::shared_ptr<Entity> entity) {
    if (!entity) return;

    // Удаляем из списка всех сущностей
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it != m_entities.end()) {
        m_entities.erase(it);
        LOG_INFO("Removed entity: " + entity->getName());
    }

    // Если это интерактивный объект, удаляем его также из списка интерактивных объектов
    auto interactiveObj = std::dynamic_pointer_cast<InteractiveObject>(entity);
    if (interactiveObj) {
        removeInteractiveObject(interactiveObj);
    }
}

void EntityManager::update(float deltaTime) {
    // Обновляем все сущности и удаляем неактивные
    {
        auto it = m_entities.begin();
        while (it != m_entities.end()) {
            if ((*it)->isActive()) {
                (*it)->update(deltaTime);
                ++it;
            }
            else {
                // Удаляем неактивные сущности
                LOG_INFO("Entity became inactive: " + (*it)->getName());

                // Если сущность это интерактивный объект, то также нужно удалить его из m_interactiveObjects,
                // но не здесь, а во второй части метода
                it = m_entities.erase(it);
            }
        }
    }

    // Отдельно обновляем и очищаем список интерактивных объектов
    {
        auto it = m_interactiveObjects.begin();
        while (it != m_interactiveObjects.end()) {
            if ((*it)->isActive()) {
                // Здесь можно не вызывать update, так как интерактивные объекты
                // также присутствуют в m_entities и уже были обновлены выше
                ++it;
            }
            else {
                // Удаляем неактивные объекты
                LOG_INFO("Interactive object became inactive: " + (*it)->getName());
                it = m_interactiveObjects.erase(it);
            }
        }
    }
}

void EntityManager::addInteractiveObject(std::shared_ptr<InteractiveObject> object) {
    if (!object) {
        LOG_WARNING("Attempted to add null interactive object");
        return;
    }

    // Проверяем, не добавлен ли уже этот объект
    auto it = std::find(m_interactiveObjects.begin(), m_interactiveObjects.end(), object);
    if (it != m_interactiveObjects.end()) {
        LOG_WARNING("Interactive object already exists: " + object->getName());
        return;
    }

    m_interactiveObjects.push_back(object);
    LOG_INFO("Added interactive object: " + object->getName());
}

void EntityManager::removeInteractiveObject(std::shared_ptr<InteractiveObject> object) {
    if (!object) return;

    auto it = std::find(m_interactiveObjects.begin(), m_interactiveObjects.end(), object);
    if (it != m_interactiveObjects.end()) {
        m_interactiveObjects.erase(it);
        LOG_INFO("Removed interactive object: " + object->getName());
    }
}

std::shared_ptr<InteractiveObject> EntityManager::findNearestInteractiveObject(
    float playerX, float playerY, float playerDirX, float playerDirY) {

    std::shared_ptr<InteractiveObject> nearest = nullptr;
    float minDistanceSquared = std::numeric_limits<float>::max();

    // Если направление не определено (игрок стоит на месте), будет учитываться любое направление
    bool hasDirection = (std::abs(playerDirX) > 0.01f || std::abs(playerDirY) > 0.01f);

    for (auto& object : m_interactiveObjects) {
        if (!object->isActive() || !object->isInteractable()) continue;

        // Координаты объекта
        float objectX = object->getPosition().x;
        float objectY = object->getPosition().y;

        // Вычисление вектора от игрока к объекту
        float dx = objectX - playerX;
        float dy = objectY - playerY;
        float distanceSquared = dx * dx + dy * dy;

        // Получение радиуса взаимодействия объекта
        float interactionRadius = object->getInteractionRadius();
        float radiusSquared = interactionRadius * interactionRadius;

        // Если объект находится в радиусе взаимодействия
        if (distanceSquared <= radiusSquared) {
            // Проверяем угол между направлением игрока и направлением к объекту,
            // если у игрока есть направление движения
            bool objectInFront = true; // По умолчанию считаем, что объект перед игроком

            if (hasDirection) {
                // Нормализуем вектор направления к объекту
                float length = std::sqrt(distanceSquared);
                if (length > 0.0001f) { // Избегаем деления на ноль
                    float normDx = dx / length;
                    float normDy = dy / length;

                    // Вычисляем скалярное произведение (dot product) между направлением игрока
                    // и нормализованным вектором направления к объекту
                    float dotProduct = playerDirX * normDx + playerDirY * normDy;

                    // Используем более узкий сектор для точности выбора объекта
                    objectInFront = dotProduct > 0.5f; // Примерно 60 градусов в каждую сторону
                }
            }

            // Если объект находится перед игроком и ближе предыдущего ближайшего
            if (objectInFront && distanceSquared < minDistanceSquared) {
                minDistanceSquared = distanceSquared;
                nearest = object;
            }
        }
    }

    return nearest;
}

void EntityManager::clear() {
    m_entities.clear();
    m_interactiveObjects.clear();
    LOG_INFO("EntityManager cleared all entities");
}