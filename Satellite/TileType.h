#pragma once

#include <string>

/**
 * @brief Перечисление типов тайлов
 */
enum class TileType {
    // Базовые типы
    EMPTY,      ///< Пустой тайл (отсутствует)
    FLOOR,      ///< Обычный пол
    WALL,       ///< Стена
    DOOR,       ///< Дверь

    // Природные типы
    WATER,      ///< Вода
    GRASS,      ///< Трава
    STONE,      ///< Камень
    FOREST,   ///< Лес

    // Искусственные материалы
    METAL,      ///< Металл
    GLASS,      ///< Стекло
    WOOD,       ///< Дерево

    // Специальные типы
    SPECIAL,    ///< Специальный тайл (телепорт, ловушка и т.д.)
    OBSTACLE,   ///< Непроходимое препятствие

    // Новые типы для планетарных поверхностей
    SAND,           ///< Песок (пустыни)
    SNOW,           ///< Снег (полярные регионы)
    ICE,            ///< Лёд (замерзшие водоемы)
    LAVA,           ///< Лава (вулканические зоны)
    MUD,            ///< Грязь/болото
    SHALLOW_WATER,  ///< Мелководье
    MOUNTAIN,       ///< Горы (высокие непроходимые участки)
    HILL,           ///< Холмы (проходимые возвышенности) 
    ROCK_FORMATION, ///< Скальные образования
    ALIEN_GROWTH,   ///< Инопланетная растительность
    CRATER,         ///< Кратер от метеорита
    RUINS,          ///< Древние руины
    MINERAL_DEPOSIT ///< Месторождение ресурсов
};

/**
 * @brief Преобразование типа тайла в строку
 * @param type Тип тайла
 * @return Строковое представление типа тайла
 */
inline std::string TileTypeToString(TileType type) {
    switch (type) {
        // Базовые типы
    case TileType::EMPTY: return "Empty";
    case TileType::FLOOR: return "Floor";
    case TileType::WALL: return "Wall";
    case TileType::DOOR: return "Door";

        // Природные типы
    case TileType::WATER: return "Water";
    case TileType::GRASS: return "Grass";
    case TileType::STONE: return "Stone";
    case TileType::FOREST: return "FOREST";

        // Искусственные материалы
    case TileType::METAL: return "Metal";
    case TileType::GLASS: return "Glass";
    case TileType::WOOD: return "Wood";

        // Специальные типы
    case TileType::SPECIAL: return "Special";
    case TileType::OBSTACLE: return "Obstacle";

        // Новые типы для планетарных поверхностей
    case TileType::SAND: return "Sand";
    case TileType::SNOW: return "Snow";
    case TileType::ICE: return "Ice";
    case TileType::LAVA: return "Lava";
    case TileType::MUD: return "Mud";
    case TileType::SHALLOW_WATER: return "Shallow Water";
    case TileType::MOUNTAIN: return "Mountain";
    case TileType::HILL: return "Hill";
    case TileType::ROCK_FORMATION: return "Rock Formation";
    case TileType::ALIEN_GROWTH: return "Alien Growth";
    case TileType::CRATER: return "Crater";
    case TileType::RUINS: return "Ruins";
    case TileType::MINERAL_DEPOSIT: return "Mineral Deposit";

    default: return "Unknown";
    }
}

/**
 * @brief Проверка, является ли тайл проходимым по умолчанию
 * @param type Тип тайла
 * @return true, если тайл проходим, false в противном случае
 */
inline bool IsWalkable(TileType type) {
    switch (type) {
        // Проходимые типы
    case TileType::FLOOR:
    case TileType::GRASS:
    case TileType::STONE:
    case TileType::FOREST:
    case TileType::METAL:
    case TileType::WOOD:
    case TileType::SAND:
    case TileType::SNOW:
    case TileType::MUD:       // Замедляет, но проходимо
    case TileType::HILL:      // Проходимые возвышенности
    case TileType::MINERAL_DEPOSIT:
    case TileType::RUINS:     // Проходимые руины
        return true;

        // Непроходимые типы
    case TileType::EMPTY:
    case TileType::WALL:
    case TileType::WATER:
    case TileType::ICE:       // Скользкий, опасный
    case TileType::LAVA:      // Смертельно опасный
    case TileType::GLASS:
    case TileType::OBSTACLE:
    case TileType::SHALLOW_WATER: // Замедляет, но проходимо
    case TileType::MOUNTAIN:  // Непроходимые горы
    case TileType::ROCK_FORMATION:
    case TileType::ALIEN_GROWTH: // Может быть опасным
    case TileType::CRATER:
    case TileType::SPECIAL:   // Зависит от типа специального тайла
    case TileType::DOOR:      // Зависит от состояния двери
        return false;
    default:
        return false;
    }
}

/**
 * @brief Проверка, является ли тайл прозрачным (можно ли видеть сквозь него)
 * @param type Тип тайла
 * @return true, если тайл прозрачный, false в противном случае
 */
inline bool IsTransparent(TileType type) {
    switch (type) {
        // Прозрачные типы
    case TileType::EMPTY:
    case TileType::FLOOR:
    case TileType::GRASS:
    case TileType::STONE:
    case TileType::FOREST:
    case TileType::METAL:
    case TileType::WOOD:
    case TileType::WATER:
    case TileType::GLASS:
    case TileType::SAND:
    case TileType::SNOW:
    case TileType::ICE:
    case TileType::MUD:
    case TileType::SHALLOW_WATER:
    case TileType::HILL:
    case TileType::CRATER:
    case TileType::MINERAL_DEPOSIT:
        return true;

        // Непрозрачные типы
    case TileType::WALL:
    case TileType::OBSTACLE:
    case TileType::MOUNTAIN:
    case TileType::ROCK_FORMATION:
    case TileType::ALIEN_GROWTH: // Может быть полупрозрачным
    case TileType::LAVA:        // Испарения могут заслонять видимость
    case TileType::RUINS:       // Зависит от типа руин
    case TileType::SPECIAL:     // Зависит от типа
    case TileType::DOOR:        // Двери могут быть прозрачными или нет
        return false;
    default:
        return false;
    }
}

/**
 * @brief Получение базовой высоты тайла по умолчанию
 * @param type Тип тайла
 * @return Высота тайла
 */
inline float GetDefaultHeight(TileType type) {
    switch (type) {
        // Высокие объекты
    case TileType::WALL:
    case TileType::OBSTACLE:
    case TileType::MOUNTAIN:
        return 1.0f;

        // Средние по высоте
    case TileType::DOOR:
    case TileType::ROCK_FORMATION:
    case TileType::RUINS:
        return 0.8f;

        // Низкие объекты
    case TileType::HILL:
    case TileType::ALIEN_GROWTH:
        return 0.5f;

        // Минимальная высота для объектов
    case TileType::WATER:
    case TileType::LAVA:
        return 0.1f;

        // Вровень с землей
    case TileType::SHALLOW_WATER:
    case TileType::ICE:
        return 0.05f;

        // Без высоты (плоские)
    case TileType::GRASS:
    case TileType::STONE:
    case TileType::FOREST:
    case TileType::METAL:
    case TileType::WOOD:
    case TileType::GLASS:
    case TileType::FLOOR:
    case TileType::SPECIAL:
    case TileType::SAND:
    case TileType::SNOW:
    case TileType::MUD:
    case TileType::CRATER:
    case TileType::MINERAL_DEPOSIT:
        return 0.0f;

        // Пустой тайл
    case TileType::EMPTY:
    default:
        return 0.0f;
    }
}

/**
 * @brief Проверка, является ли тайл типом воды
 * @param type Тип тайла
 * @return true, если тайл является водой, false в противном случае
 */
inline bool IsWater(TileType type) {
    return type == TileType::WATER || type == TileType::SHALLOW_WATER;
}

/**
 * @brief Проверка, является ли тайл типом твердой поверхности (земля, камень и т.д.)
 * @param type Тип тайла
 * @return true, если тайл является твердой поверхностью
 */
inline bool IsSolidGround(TileType type) {
    switch (type) {
    case TileType::FLOOR:
    case TileType::GRASS:
    case TileType::STONE:
    case TileType::FOREST:
    case TileType::METAL:
    case TileType::WOOD:
    case TileType::SAND:
    case TileType::SNOW:
    case TileType::MUD:
    case TileType::HILL:
    case TileType::CRATER:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Проверка, является ли тайл опасным для перемещения по нему
 * @param type Тип тайла
 * @return true, если тайл опасен, false в противном случае
 */
inline bool IsHazardous(TileType type) {
    switch (type) {
    case TileType::LAVA:
    case TileType::ICE:
    case TileType::ALIEN_GROWTH: // Потенциально опасная флора
        return true;
    default:
        return false;
    }
}