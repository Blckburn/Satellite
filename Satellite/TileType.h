#pragma once

#include <string>

/**
 * @brief Перечисление типов тайлов
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
    SAND,            // Песок
    SNOW,            // Снег
    ICE,             // Лед
    ROCK_FORMATION,  // Скалы
    LAVA,            // Лава
    FOREST           // Лес
};

/**
 * @brief Преобразование типа тайла в строку
 * @param type Тип тайла
 * @return Строковое представление типа тайла
 */
inline std::string TileTypeToString(TileType type) {
    switch (type) {
    case TileType::EMPTY: return "Empty";
    case TileType::FLOOR: return "Floor";
    case TileType::WALL: return "Wall";
    case TileType::DOOR: return "Door";
    case TileType::WATER: return "Water";
    case TileType::GRASS: return "Grass";
    case TileType::STONE: return "Stone";
    case TileType::METAL: return "Metal";
    case TileType::GLASS: return "Glass";
    case TileType::WOOD: return "Wood";
    case TileType::SPECIAL: return "Special";
    case TileType::OBSTACLE: return "Obstacle";
    case TileType::SAND: return "Sand";
    case TileType::SNOW: return "Snow";
    case TileType::ICE: return "Ice";
    case TileType::ROCK_FORMATION: return "Rock Formation";
    case TileType::LAVA: return "Lava";
    case TileType::FOREST: return "Forest";
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
    case TileType::FLOOR:
    case TileType::GRASS:
    case TileType::STONE:
    case TileType::METAL:
    case TileType::WOOD:
    case TileType::SAND:     // Песок проходим
    case TileType::SNOW:     // Снег проходим
    case TileType::DOOR:     // Открытая дверь проходима
        return true;
    case TileType::EMPTY:
    case TileType::WALL:     // Стена (и закрытая дверь) непроходима
    case TileType::WATER:    // Вода непроходима
    case TileType::GLASS:
    case TileType::OBSTACLE:
    case TileType::SPECIAL:  // Специальные тайлы могут быть как проходимыми, так и нет
    case TileType::ICE:      // Лед непроходим
    case TileType::ROCK_FORMATION: // Скалы непроходимы
    case TileType::LAVA:     // Лава непроходима
    case TileType::FOREST:   // Лес непроходим
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
    case TileType::EMPTY:
    case TileType::FLOOR:
    case TileType::GRASS:
    case TileType::STONE:
    case TileType::METAL:
    case TileType::WOOD:
    case TileType::WATER:
    case TileType::GLASS:
    case TileType::SAND:     // Песок прозрачен
    case TileType::SNOW:     // Снег прозрачен
    case TileType::ICE:      // Лед прозрачен
        return true;
    case TileType::WALL:
    case TileType::OBSTACLE:
    case TileType::SPECIAL:  // Зависит от конкретного типа специального тайла
    case TileType::DOOR:     // Двери могут быть прозрачными или нет
    case TileType::ROCK_FORMATION: // Скалы непрозрачны
    case TileType::LAVA:     // Лава непрозрачна
    case TileType::FOREST:   // Лес непрозрачен
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
    case TileType::WALL:
        return 1.0f;  // Полная высота для стен
    case TileType::OBSTACLE:
        return 1.0f;  // Полная высота для препятствий
    case TileType::DOOR:
        return 1.0f;  // Полная высота для дверей
    case TileType::WATER:
        return 0.1f;  // Пониженная высота для воды
    case TileType::GLASS:
        return 0.8f;  // Высота для стеклянных перегородок
    case TileType::SPECIAL:
        return 0.3f;  // Средняя высота для специальных тайлов
    case TileType::ROCK_FORMATION:
        return 1.0f;  // Полная высота для скал
    case TileType::LAVA:
        return 0.2f;  // Небольшая высота для лавы
    case TileType::FOREST:
        return 1.0f;  // Полная высота для леса
    case TileType::ICE:
        return 0.5f;  // Средняя высота для льда

        // Все плоские тайлы имеют высоту ровно 0.0f
    case TileType::GRASS:
    case TileType::STONE:
    case TileType::METAL:
    case TileType::WOOD:
    case TileType::FLOOR:
    case TileType::SAND:     // Песок плоский
    case TileType::SNOW:     // Снег плоский
        return 0.0f;  // Плоские тайлы

    case TileType::EMPTY:
    default:
        return 0.0f;  // Пустые тайлы тоже плоские
    }
}

inline bool IsWater(TileType type) {
    return type == TileType::WATER;
}