/**
 * @file Entity.h
 * @brief Базовый класс для всех игровых объектов
 */

#pragma once

#include "Core/Base/Types.h"

namespace Satellite {

/**
 * @brief Базовый класс для всех игровых объектов
 */
class Entity {
public:
    /**
     * @brief Структура для хранения позиции сущности
     */
    struct Position {
        float x = 0.0f;  ///< Координата X
        float y = 0.0f;  ///< Координата Y
        float z = 0.0f;  ///< Координата Z (для изометрической проекции и сортировки)
    };

    /**
     * @brief Конструктор
     * @param name Имя сущности
     */
    Entity(const std::string& name);

    /**
     * @brief Виртуальный деструктор
     */
    virtual ~Entity();

    /**
     * @brief Инициализация сущности
     * @return true в случае успеха, false при ошибке
     */
    virtual bool initialize() = 0;

    /**
     * @brief Обработка пользовательского ввода
     * @param event Событие SDL
     */
    virtual void handleEvent(const SDL_Event& event);

    /**
     * @brief Обновление состояния сущности
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Отрисовка сущности
     * @param renderer Указатель на SDL_Renderer
     */
    virtual void render(SDL_Renderer* renderer) = 0;

    /**
     * @brief Установка позиции сущности
     * @param x Координата X
     * @param y Координата Y
     * @param z Координата Z (для сортировки по глубине)
     */
    void setPosition(float x, float y, float z = 0.0f);

    /**
     * @brief Получение текущей позиции сущности
     * @return Структура с координатами
     */
    const Position& getPosition() const { return m_position; }

    /**
     * @brief Получение имени сущности
     * @return Имя сущности
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Проверка, активна ли сущность
     * @return true, если сущность активна, false в противном случае
     */
    bool isActive() const { return m_isActive; }

    /**
     * @brief Установка активности сущности
     * @param active Флаг активности
     */
    void setActive(bool active) { m_isActive = active; }

protected:
    std::string m_name;    ///< Имя сущности
    Position m_position;   ///< Позиция сущности
    bool m_isActive;       ///< Флаг активности сущности
};

} // namespace Satellite