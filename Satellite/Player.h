#pragma once

#pragma once

#include "Entity.h"
#include "TileMap.h"
#include <SDL.h>

/**
 * @brief Класс игрока
 */
class Player : public Entity {
public:
    /**
     * @brief Направления персонажа
     */
    enum class Direction {
        NORTH,      ///< Север
        NORTHEAST,  ///< Северо-восток
        EAST,       ///< Восток
        SOUTHEAST,  ///< Юго-восток
        SOUTH,      ///< Юг
        SOUTHWEST,  ///< Юго-запад
        WEST,       ///< Запад
        NORTHWEST   ///< Северо-запад
    };

    /**
     * @brief Конструктор
     * @param name Имя сущности
     * @param tileMap Указатель на карту для проверки коллизий
     */
    Player(const std::string& name, TileMap* tileMap);

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
     * @brief Обработка нажатий клавиш для определения направления движения
     */
    void detectKeyInput();

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
     * @brief Проверка возможности диагонального перемещения
     * @param fromX Начальная X координата
     * @param fromY Начальная Y координата
     * @param toX Конечная X координата
     * @param toY Конечная Y координата
     * @return true, если диагональное перемещение возможно, false в противном случае
     */
    bool canMoveDiagonally(int fromX, int fromY, int toX, int toY);

    /**
     * @brief Получение текущего направления игрока
     * @return Текущее направление
     */
    Direction getCurrentDirection() const { return m_currentDirection; }

    /**
     * @brief Получение субкоординаты X внутри тайла
     * @return Субкоордината X (0.0-1.0)
     */
    float getSubX() const { return m_subX; }

    /**
     * @brief Установка субкоординаты X внутри тайла
     * @param subX Субкоордината X (0.0-1.0)
     */
    void setSubX(float subX) { m_subX = subX; }

    /**
     * @brief Получение субкоординаты Y внутри тайла
     * @return Субкоордината Y (0.0-1.0)
     */
    float getSubY() const { return m_subY; }

    /**
     * @brief Установка субкоординаты Y внутри тайла
     * @param subY Субкоордината Y (0.0-1.0)
     */
    void setSubY(float subY) { m_subY = subY; }

    /**
     * @brief Получение полной X координаты (тайл + субпозиция)
     * @return Полная X координата
     */
    float getFullX() const { return m_position.x + m_subX; }

    /**
     * @brief Получение полной Y координаты (тайл + субпозиция)
     * @return Полная Y координата
     */
    float getFullY() const { return m_position.y + m_subY; }

    /**
     * @brief Получение размера коллизии
     * @return Размер коллизии (радиус)
     */
    float getCollisionSize() const { return m_collisionSize; }

    /**
     * @brief Установка размера коллизии
     * @param size Размер коллизии (радиус)
     */
    void setCollisionSize(float size) { m_collisionSize = size; }

    /**
     * @brief Получение цвета игрока
     * @return Цвет игрока
     */
    SDL_Color getColor() const { return m_color; }

    /**
     * @brief Установка цвета игрока
     * @param color Цвет игрока
     */
    void setColor(const SDL_Color& color) { m_color = color; }

    /**
     * @brief Получение текущей скорости движения
     * @return Скорость движения
     */
    float getMoveSpeed() const { return m_moveSpeed; }

    /**
     * @brief Установка скорости движения
     * @param speed Скорость движения
     */
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }

    /**
     * @brief Получение высоты игрока
     * @return Высота игрока
     */
    float getHeight() const { return m_height; }

    /**
     * @brief Установка высоты игрока
     * @param height Высота игрока
     */
    void setHeight(float height) { m_height = height; }

    /**
     * @brief Получение направления движения по X
     * @return Направление движения по X (-1.0, 0.0 или 1.0)
     */
    float getDirectionX() const { return m_dX; }

    /**
     * @brief Получение направления движения по Y
     * @return Направление движения по Y (-1.0, 0.0 или 1.0)
     */
    float getDirectionY() const { return m_dY; }

    /**
     * @brief Проверка движения игрока
     * @return true, если игрок движется, false в противном случае
     */
    bool isMoving() const { return m_dX != 0.0f || m_dY != 0.0f; }

private:
    /**
     * @brief Обновление текущего направления на основе вектора движения
     */
    void updateDirection();

    /**
     * @brief Создает цвета для граней игрока на основе основного цвета
     */
    void updateFaceColors();

private:
    TileMap* m_tileMap;             ///< Указатель на карту для проверки коллизий
    Direction m_currentDirection;   ///< Текущее направление игрока
    float m_subX;                   ///< Позиция внутри тайла по X (0.0-1.0)
    float m_subY;                   ///< Позиция внутри тайла по Y (0.0-1.0)
    float m_moveSpeed;              ///< Скорость движения
    float m_dX;                     ///< Направление движения по X
    float m_dY;                     ///< Направление движения по Y
    float m_collisionSize;          ///< Размер коллизии (радиус)
    float m_height;                 ///< Высота игрока
    SDL_Color m_color;              ///< Основной цвет игрока
    SDL_Color m_leftFaceColor;      ///< Цвет левой грани
    SDL_Color m_rightFaceColor;     ///< Цвет правой грани
};