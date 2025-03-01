#pragma once

#include <SDL.h>
#include <string>
#include <vector>
#include <memory>

class Entity;

/**
 * @brief Базовый класс для игровых сцен
 */
class Scene {
public:
    /**
     * @brief Конструктор
     * @param name Имя сцены
     */
    Scene(const std::string& name);

    /**
     * @brief Виртуальный деструктор
     */
    virtual ~Scene();

    /**
     * @brief Инициализация сцены
     * @return true в случае успеха, false при ошибке
     */
    virtual bool initialize() = 0;

    /**
     * @brief Обработка пользовательского ввода
     * @param event Событие SDL
     */
    virtual void handleEvent(const SDL_Event& event);

    /**
     * @brief Обновление сцены
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    virtual void update(float deltaTime);

    /**
     * @brief Отрисовка сцены
     * @param renderer Указатель на SDL_Renderer
     */
    virtual void render(SDL_Renderer* renderer);

    /**
     * @brief Добавление сущности на сцену
     * @param entity Указатель на сущность
     */
    virtual void addEntity(std::shared_ptr<Entity> entity);

    /**
     * @brief Удаление сущности со сцены
     * @param entity Указатель на сущность
     */
    virtual void removeEntity(std::shared_ptr<Entity> entity);

    /**
     * @brief Получение имени сцены
     * @return Имя сцены
     */
    const std::string& getName() const { return m_name; }

protected:
    std::string m_name;                         ///< Имя сцены
    std::vector<std::shared_ptr<Entity>> m_entities;  ///< Список сущностей на сцене
};