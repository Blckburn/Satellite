#include "EntityManager.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include "Door.h"

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
    float playerX, float playerY, float directionX, float directionY) {

    // ОТЛАДКА
    LOG_DEBUG("Looking for nearest interactive object at position (" +
        std::to_string(playerX) + ", " + std::to_string(playerY) + ")");

    // НОВЫЙ КОД: Сначала ищем ОТКРЫТЫЕ двери поблизости с приоритетом
    for (auto& obj : m_interactiveObjects) {
        if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
            if (doorObj->isOpen() && doorObj->isActive() && doorObj->isInteractable()) {
                float objX = doorObj->getPosition().x;
                float objY = doorObj->getPosition().y;

                float dx = objX - playerX;
                float dy = objY - playerY;
                float distanceSquared = dx * dx + dy * dy;
                float radius = doorObj->getInteractionRadius();

                if (distanceSquared <= radius * radius) {
                    LOG_DEBUG("Found OPEN door in range: " + doorObj->getName());
                    return doorObj;
                }
            }
        }
    }

    // Стандартный поиск всех интерактивных объектов
    std::shared_ptr<InteractiveObject> nearestObject = nullptr;
    float minDistanceSquared = std::numeric_limits<float>::max();

    int countChecked = 0;
    int countInRange = 0;

    // Проходим по всем интерактивным объектам
    for (auto& obj : m_interactiveObjects) {
        countChecked++;

        if (!obj->isActive() || !obj->isInteractable()) {
            continue;
        }

        float objX = obj->getPosition().x;
        float objY = obj->getPosition().y;

        float dx = objX - playerX;
        float dy = objY - playerY;
        float distanceSquared = dx * dx + dy * dy;

        // Получаем радиус взаимодействия
        float interactionRadius = 1.0f; // Значение по умолчанию
        if (auto interactiveObj = std::dynamic_pointer_cast<InteractiveObject>(obj)) {
            interactionRadius = interactiveObj->getInteractionRadius();
        }

        // Проверяем, находится ли объект в пределах радиуса
        if (distanceSquared <= interactionRadius * interactionRadius) {
            countInRange++;

            // Если это первый найденный объект или он ближе предыдущего ближайшего
            if (!nearestObject || distanceSquared < minDistanceSquared) {
                nearestObject = std::dynamic_pointer_cast<InteractiveObject>(obj);
                minDistanceSquared = distanceSquared;
            }
        }
    }

    // ОТЛАДКА
    LOG_DEBUG("Checked " + std::to_string(countChecked) + " objects, found " +
        std::to_string(countInRange) + " in range, nearest: " +
        (nearestObject ? nearestObject->getName() : "none"));

    return nearestObject;
}

void EntityManager::clear() {
    m_entities.clear();
    m_interactiveObjects.clear();
    LOG_INFO("EntityManager cleared all entities");
}