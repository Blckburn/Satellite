#pragma once

#include "Entity.h"
#include "TileMap.h"
#include "InteractiveObject.h"
#include "Player.h"
#include <memory>
#include <vector>

/**
 * @brief Класс для управления сущностями в игровом мире
 */
class EntityManager {
public:
    /**
     * @brief Конструктор
     * @param tileMap Указатель на карту тайлов
     */
    EntityManager(std::shared_ptr<TileMap> tileMap);

    /**
     * @brief Деструктор
     */
    ~EntityManager() = default;

    /**
     * @brief Добавление сущности
     * @param entity Указатель на сущность
     */
    void addEntity(std::shared_ptr<Entity> entity);

    /**
     * @brief Удаление сущности
     * @param entity Указатель на сущность
     */
    void removeEntity(std::shared_ptr<Entity> entity);

    /**
     * @brief Обновление всех сущностей
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void update(float deltaTime);

    /**
     * @brief Добавление интерактивного объекта
     * @param object Указатель на интерактивный объект
     */
    void addInteractiveObject(std::shared_ptr<InteractiveObject> object);

    /**
     * @brief Удаление интерактивного объекта
     * @param object Указатель на интерактивный объект
     */
    void removeInteractiveObject(std::shared_ptr<InteractiveObject> object);

    /**
     * @brief Поиск ближайшего интерактивного объекта
     * @param playerX X-координата игрока
     * @param playerY Y-координата игрока
     * @param playerDirX X-компонента направления игрока (опционально)
     * @param playerDirY Y-компонента направления игрока (опционально)
     * @return Указатель на ближайший интерактивный объект или nullptr
     */
    std::shared_ptr<InteractiveObject> findNearestInteractiveObject(
        float playerX, float playerY,
        float playerDirX = 0.0f, float playerDirY = 0.0f);

    /**
     * @brief Получение списка всех сущностей
     * @return Ссылка на вектор сущностей
     */
    const std::vector<std::shared_ptr<Entity>>& getEntities() const { return m_entities; }

    /**
     * @brief Получение списка интерактивных объектов
     * @return Ссылка на вектор интерактивных объектов
     */
    const std::vector<std::shared_ptr<InteractiveObject>>& getInteractiveObjects() const {
        return m_interactiveObjects;
    }

    /**
     * @brief Очистка всех списков сущностей
     */
    void clear();

private:
    std::shared_ptr<TileMap> m_tileMap;                        ///< Указатель на карту тайлов
    std::vector<std::shared_ptr<Entity>> m_entities;            ///< Список всех сущностей
    std::vector<std::shared_ptr<InteractiveObject>> m_interactiveObjects; ///< Список интерактивных объектов
};