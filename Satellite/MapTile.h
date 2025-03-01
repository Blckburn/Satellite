#pragma once

#include "TileType.h"
#include <string>
#include <SDL.h>

/**
 * @brief Класс для представления тайла на карте
 */
class MapTile {
public:
    /**
     * @brief Конструктор по умолчанию (создает пустой тайл)
     */
    MapTile();

    /**
     * @brief Конструктор с указанием типа
     * @param type Тип тайла
     */
    MapTile(TileType type);

    /**
     * @brief Конструктор с полной инициализацией
     * @param type Тип тайла
     * @param walkable Признак проходимости
     * @param transparent Признак прозрачности
     * @param height Высота тайла
     */
    MapTile(TileType type, bool walkable, bool transparent, float height);

    /**
     * @brief Деструктор
     */
    ~MapTile();

    /**
     * @brief Получение типа тайла
     * @return Тип тайла
     */
    TileType getType() const { return m_type; }

    /**
     * @brief Установка типа тайла
     * @param type Тип тайла
     */
    void setType(TileType type);

    /**
     * @brief Проверка проходимости тайла
     * @return true, если тайл проходим, false в противном случае
     */
    bool isWalkable() const { return m_walkable; }

    /**
     * @brief Установка признака проходимости
     * @param walkable Признак проходимости
     */
    void setWalkable(bool walkable) { m_walkable = walkable; }

    /**
     * @brief Проверка прозрачности тайла
     * @return true, если тайл прозрачный, false в противном случае
     */
    bool isTransparent() const { return m_transparent; }

    /**
     * @brief Установка признака прозрачности
     * @param transparent Признак прозрачности
     */
    void setTransparent(bool transparent) { m_transparent = transparent; }

    /**
     * @brief Получение высоты тайла
     * @return Высота тайла
     */
    float getHeight() const { return m_height; }

    /**
     * @brief Установка высоты тайла
     * @param height Высота тайла
     */
    void setHeight(float height) { m_height = height; }

    /**
     * @brief Получение цвета для рендеринга тайла
     * @return Цвет тайла (SDL_Color)
     */
    SDL_Color getColor() const;

    /**
     * @brief Установка цвета для рендеринга тайла
     * @param color Цвет тайла
     */
    void setColor(const SDL_Color& color) { m_color = color; }

    /**
     * @brief Получение строкового представления тайла
     * @return Строковое представление тайла
     */
    std::string toString() const;

private:
    TileType m_type;       ///< Тип тайла
    bool m_walkable;       ///< Признак проходимости
    bool m_transparent;    ///< Признак прозрачности
    float m_height;        ///< Высота тайла
    SDL_Color m_color;     ///< Цвет для рендеринга

    // Дополнительные свойства, которые могут быть добавлены позже:
    // bool m_destructible;  ///< Может ли тайл быть разрушен
    // bool m_flammable;     ///< Может ли тайл гореть
    // float m_friction;     ///< Трение поверхности
    // int m_hardness;       ///< Прочность тайла
};