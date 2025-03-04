/**
 * @file Types.h
 * @brief Основные типы и перечисления для движка Satellite
 */

#pragma once

// Стандартные включения
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <cmath>

// Включения SDL
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Color;
struct SDL_Window;
struct SDL_Event;
struct SDL_Point;
struct SDL_Rect;
typedef struct _TTF_Font TTF_Font;

namespace Satellite {

// Константы движка
namespace Constants {
    // Размеры тайлов по умолчанию
    constexpr int DEFAULT_TILE_WIDTH = 64;
    constexpr int DEFAULT_TILE_HEIGHT = 32;

    // Ограничения карты
    constexpr int MAX_MAP_SIZE = 256;

    // Параметры рендеринга
    constexpr float HEIGHT_SCALE = 30.0f;  // Масштаб для высоты в изометрии

    // Параметры игрока
    constexpr float PLAYER_DEFAULT_SPEED = 0.05f;
    constexpr float PLAYER_DEFAULT_HEIGHT = 0.5f;
}

// Предварительные объявления классов
class Engine;
class Scene;
class Entity;
class TileMap;
class MapTile;
class Player;
class InteractiveObject;
class PickupItem;
class Camera;
class CollisionSystem;
class IsometricRenderer;
class TileRenderer;
class ResourceManager;
class RoomGenerator;

/**
 * @brief Типы тайлов карты
 */
enum class TileType {
    EMPTY,      ///< Пустой тайл (отсутствует)
    FLOOR,      ///< Обычный пол
    WALL,       ///< Стена
    DOOR,       ///< Дверь
    WATER,      ///< Вода
    GRASS,      ///< Трава
    STONE,      ///< Камень
    METAL,      ///< Металл
    GLASS,      ///< Стекло
    WOOD,       ///< Дерево
    SPECIAL,    ///< Специальный тайл (телепорт, ловушка и т.д.)
    OBSTACLE,   ///< Непроходимое препятствие
    SAND,       ///< Песок
    SNOW,       ///< Снег
    ICE,        ///< Лед
    ROCK_FORMATION, ///< Скалы
    LAVA,       ///< Лава
    FOREST      ///< Лес
};

/**
 * @brief Типы интерактивных объектов
 */
enum class InteractiveType {
    PICKUP,        ///< Предмет, который можно подобрать
    DOOR,          ///< Дверь, которую можно открыть/закрыть
    SWITCH,        ///< Переключатель, который можно активировать
    TERMINAL,      ///< Терминал, с которым можно взаимодействовать
    CONTAINER,     ///< Контейнер, который можно открыть
    CUSTOM         ///< Пользовательский тип
};

/**
 * @brief Типы предметов
 */
enum class ItemType {
    RESOURCE,   ///< Ресурс (материал)
    WEAPON,     ///< Оружие
    ARMOR,      ///< Броня
    CONSUMABLE, ///< Расходуемый предмет
    KEY,        ///< Ключ или предмет квеста
    GENERIC     ///< Обычный предмет
};

/**
 * @brief Типы биомов для генерации карт
 */
enum class BiomeType {
    DEFAULT,    ///< Стандартный биом со стенами и полом
    FOREST,     ///< Лесной биом с деревьями и травой
    DESERT,     ///< Пустынный биом с песком и камнями
    TUNDRA,     ///< Тундра со снегом и льдом
    VOLCANIC    ///< Вулканический биом с камнями и лавой
};

/**
 * @brief Уровни логирования
 */
enum class LogLevel {
    DEBUG,      ///< Отладочная информация
    INFO,       ///< Информационное сообщение
    WARNING,    ///< Предупреждение
    ERROR,      ///< Ошибка
    NONE        ///< Вывод отключен
};

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
 * @brief Структура, содержащая результат проверки коллизии
 */
struct CollisionResult {
    bool collision;       ///< Было ли столкновение
    float adjustedX;      ///< Скорректированная X-координата
    float adjustedY;      ///< Скорректированная Y-координата
    bool slidingX;        ///< Происходит ли скольжение по оси X
    bool slidingY;        ///< Происходит ли скольжение по оси Y

    /**
     * @brief Конструктор по умолчанию
     */
    CollisionResult() :
        collision(false),
        adjustedX(0.0f),
        adjustedY(0.0f),
        slidingX(false),
        slidingY(false) {
    }
};

} // namespace Satellite