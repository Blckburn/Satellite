#pragma once

#include "TileType.h"
#include <string>
#include <vector>
#include <sstream>
#include <SDL.h>

/**
 * @brief Класс для представления тайла на карте
 */
class MapTile {
public:
    /**
     * @brief Структура для хранения декоративного элемента
     */
    struct Decoration {
        int id;            // Идентификатор декорации
        std::string name;  // Название
        float scale;       // Масштаб (1.0 = нормальный размер)
        bool animated;     // Имеет ли анимацию

        Decoration(int _id = 0, const std::string& _name = "", float _scale = 1.0f, bool _animated = false)
            : id(_id), name(_name), scale(_scale), animated(_animated) {
        }
    };

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
     * @brief Конструктор с полной инициализацией базовых свойств
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

    // Новые методы для планетарных характеристик

    /**
     * @brief Проверка, является ли тайл водой
     * @return true, если тайл является водой, false в противном случае
     */
    bool isWater() const { return IsWater(m_type); }

    /**
     * @brief Получение идентификатора биома
     * @return Идентификатор биома
     */
    int getBiomeId() const { return m_biomeId; }

    /**
     * @brief Установка идентификатора биома
     * @param biomeId Идентификатор биома
     */
    void setBiomeId(int biomeId) { m_biomeId = biomeId; }

    /**
     * @brief Получение температуры
     * @return Температура тайла
     */
    float getTemperature() const { return m_temperature; }

    /**
     * @brief Установка температуры
     * @param temperature Температура тайла
     */
    void setTemperature(float temperature) { m_temperature = temperature; }

    /**
     * @brief Получение влажности
     * @return Влажность тайла
     */
    float getHumidity() const { return m_humidity; }

    /**
     * @brief Установка влажности
     * @param humidity Влажность тайла
     */
    void setHumidity(float humidity) { m_humidity = humidity; }

    /**
     * @brief Получение высоты рельефа
     * @return Высота рельефа (0.0 - 1.0)
     */
    float getElevation() const { return m_elevation; }

    /**
     * @brief Установка высоты рельефа
     * @param elevation Высота рельефа (0.0 - 1.0)
     */
    void setElevation(float elevation) { m_elevation = elevation; }

    /**
     * @brief Получение уровня радиации
     * @return Уровень радиации (0.0 - 1.0)
     */
    float getRadiationLevel() const { return m_radiationLevel; }

    /**
     * @brief Установка уровня радиации
     * @param level Уровень радиации (0.0 - 1.0)
     */
    void setRadiationLevel(float level) { m_radiationLevel = level; }

    /**
     * @brief Добавление декоративного элемента
     * @param decoration Декоративный элемент
     */
    void addDecoration(const Decoration& decoration) { m_decorations.push_back(decoration); }

    /**
     * @brief Удаление декоративного элемента по ID
     * @param decorationId ID декоративного элемента
     * @return true если элемент был найден и удален, false в противном случае
     */
    bool removeDecoration(int decorationId);

    /**
     * @brief Получение всех декоративных элементов
     * @return Вектор декоративных элементов
     */
    const std::vector<Decoration>& getDecorations() const { return m_decorations; }

    /**
     * @brief Очистка всех декоративных элементов
     */
    void clearDecorations() { m_decorations.clear(); }

    /**
     * @brief Проверка наличия опасности
     * @return true, если тайл опасен, false в противном случае
     */
    bool isHazardous() const { return IsHazardous(m_type) || m_radiationLevel > 0.3f; }

    /**
     * @brief Получение плотности ресурсов
     * @return Плотность ресурсов (0.0 - 1.0)
     */
    float getResourceDensity() const { return m_resourceDensity; }

    /**
     * @brief Установка плотности ресурсов
     * @param density Плотность ресурсов (0.0 - 1.0)
     */
    void setResourceDensity(float density) { m_resourceDensity = density; }

private:
    TileType m_type;           ///< Тип тайла
    bool m_walkable;           ///< Признак проходимости
    bool m_transparent;        ///< Признак прозрачности
    float m_height;            ///< Высота тайла (для 3D отображения)
    SDL_Color m_color;         ///< Цвет для рендеринга

    // Новые свойства для планетарных поверхностей
    int m_biomeId = 0;                       ///< Идентификатор биома
    float m_temperature = 20.0f;             ///< Температура в градусах Цельсия
    float m_humidity = 0.5f;                 ///< Влажность (0.0 - 1.0)
    float m_elevation = 0.0f;                ///< Высота рельефа (0.0 - 1.0)
    float m_radiationLevel = 0.0f;           ///< Уровень радиации (0.0 - 1.0)
    float m_resourceDensity = 0.0f;          ///< Плотность ресурсов (0.0 - 1.0)
    std::vector<Decoration> m_decorations;   ///< Декоративные элементы на тайле
};