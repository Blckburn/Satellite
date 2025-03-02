#pragma once

#include "Entity.h"
#include "TileMap.h"
#include <memory>

/**
 * @brief Класс игрока, управляемый пользователем
 */
class Player : public Entity {
public:
    /**
     * @brief Конструктор
     * @param name Имя игрока
     * @param tileMap Указатель на карту тайлов
     */
    Player(const std::string& name, std::shared_ptr<TileMap> tileMap);

    /**
     * @brief Деструктор
     */
    ~Player();

    /**
     * @brief Инициализация игрока
     * @return true в случае успеха, false при ошибке
     */
    bool initialize() override;

    /**
     * @brief Обработка пользовательского ввода
     * @param event Событие SDL
     */
    void handleEvent(const SDL_Event& event) override;

    /**
     * @brief Обновление состояния игрока
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void update(float deltaTime) override;

    /**
     * @brief Отрисовка игрока
     * @param renderer Указатель на SDL_Renderer
     */
    void render(SDL_Renderer* renderer) override;

    /**
     * @brief Установка позиции игрока
     * @param x Координата X
     * @param y Координата Y
     * @param subX Позиция внутри тайла по X (0.0-1.0)
     * @param subY Позиция внутри тайла по Y (0.0-1.0)
     */
    void setPlayerPosition(float x, float y, float subX = 0.5f, float subY = 0.5f);

    /**
     * @brief Получение X координаты игрока
     * @return X координата
     */
    float getPlayerX() const { return m_position.x; }

    /**
     * @brief Получение Y координаты игрока
     * @return Y координата
     */
    float getPlayerY() const { return m_position.y; }

    /**
     * @brief Получение X субкоординаты игрока внутри тайла
     * @return X субкоордината (0.0-1.0)
     */
    float getPlayerSubX() const { return m_subX; }

    /**
     * @brief Получение Y субкоординаты игрока внутри тайла
     * @return Y субкоордината (0.0-1.0)
     */
    float getPlayerSubY() const { return m_subY; }

    /**
     * @brief Проверка возможности диагонального перемещения
     * @param fromX Начальная X координата
     * @param fromY Начальная Y координата
     * @param toX Конечная X координата
     * @param toY Конечная Y координата
     * @return true, если диагональное перемещение возможно, false в противном случае
     */
    bool canMoveDiagonally(int fromX, int fromY, int toX, int toY);

    /**
     * @brief Получение размера коллизии
     * @return Размер коллизии
     */
    float getCollisionSize() const { return m_collisionSize; }

    /**
     * @brief Установка размера коллизии
     * @param size Размер коллизии
     */
    void setCollisionSize(float size) { m_collisionSize = size; }

    /**
     * @brief Получение скорости движения
     * @return Скорость движения
     */
    float getMoveSpeed() const { return m_moveSpeed; }

    /**
     * @brief Установка скорости движения
     * @param speed Скорость движения
     */
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }

    /**
     * @brief Получение направления движения по X
     * @return Направление движения по X (-1, 0, 1)
     */
    float getMoveDirectionX() const { return m_dX; }

    /**
     * @brief Получение направления движения по Y
     * @return Направление движения по Y (-1, 0, 1)
     */
    float getMoveDirectionY() const { return m_dY; }

    /**
     * @brief Расчет приоритета визуального порядка для изометрической проекции
     * @return Значение приоритета для сортировки
     */
    float calculateZOrderPriority();

private:
    /**
     * @brief Обнаружение нажатий клавиш для движения
     */
    void detectKeyInput();

private:
    std::shared_ptr<TileMap> m_tileMap;  ///< Указатель на карту тайлов
    float m_subX;         ///< Позиция внутри тайла по X (0.0-1.0)
    float m_subY;         ///< Позиция внутри тайла по Y (0.0-1.0)
    float m_moveSpeed;    ///< Скорость движения персонажа
    float m_dX;           ///< Направление движения по X (с плавающей точкой)
    float m_dY;           ///< Направление движения по Y (с плавающей точкой)
    float m_collisionSize; ///< Размер коллизии персонажа

    // Визуальные параметры
    SDL_Color m_color;    ///< Цвет игрока
};