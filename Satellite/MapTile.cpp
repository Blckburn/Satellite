#include "MapTile.h"
#include <sstream>

MapTile::MapTile() : m_type(TileType::EMPTY), m_walkable(false), m_transparent(true), m_height(0.0f) {
    m_color = { 200, 200, 200, 255 }; // Серый по умолчанию
}

MapTile::MapTile(TileType type)
    : m_type(type), m_walkable(IsWalkable(type)), m_transparent(IsTransparent(type)),
    m_height(GetDefaultHeight(type)) {
    // Устанавливаем цвет в зависимости от типа тайла
    setType(type); // Это установит правильный цвет
}

MapTile::MapTile(TileType type, bool walkable, bool transparent, float height)
    : m_type(type), m_walkable(walkable), m_transparent(transparent), m_height(height) {
    // Устанавливаем цвет в зависимости от типа тайла
    switch (type) {
    case TileType::EMPTY:
        m_color = { 0, 0, 0, 0 }; // Полностью прозрачный
        break;
    case TileType::FLOOR:
        m_color = { 180, 180, 180, 255 }; // Светло-серый
        break;
    case TileType::WALL:
        m_color = { 100, 100, 100, 255 }; // Темно-серый
        break;
    case TileType::DOOR:
        m_color = { 120, 60, 0, 255 }; // Коричневый
        break;
    case TileType::WATER:
        m_color = { 0, 100, 255, 200 }; // Полупрозрачный синий
        break;
    case TileType::GRASS:
        m_color = { 50, 180, 50, 255 }; // Зеленый
        break;
    case TileType::STONE:
        m_color = { 150, 150, 150, 255 }; // Серый камень
        break;
    case TileType::METAL:
        m_color = { 170, 170, 190, 255 }; // Серебристый
        break;
    case TileType::GLASS:
        m_color = { 200, 200, 255, 150 }; // Полупрозрачный голубой
        break;
    case TileType::WOOD:
        m_color = { 150, 100, 50, 255 }; // Коричневый
        break;
    case TileType::SAND:
        m_color = { 230, 220, 170, 255 }; // Песочный
        break;
    case TileType::SNOW:
        m_color = { 240, 240, 255, 255 }; // Белый с оттенком синего
        break;
    case TileType::ICE:
        m_color = { 200, 220, 255, 200 }; // Голубой полупрозрачный
        break;
    case TileType::LAVA:
        m_color = { 255, 100, 0, 255 }; // Оранжево-красный
        break;
    case TileType::OBSTACLE:
    case TileType::SPECIAL:
    case TileType::ROCK_FORMATION:
    case TileType::FOREST:
    default:
        m_color = { 120, 120, 120, 255 }; // Серый по умолчанию
        break;
    }
}

MapTile::~MapTile() {
    // Пустой деструктор, так как у нас нет ресурсов для освобождения
}

void MapTile::setType(TileType type) {
    m_type = type;

    // Устанавливаем стандартные параметры для данного типа тайла
    m_height = GetDefaultHeight(type);
    m_walkable = IsWalkable(type);
    m_transparent = IsTransparent(type);

    // Устанавливаем цвет в зависимости от типа
    switch (type) {
    case TileType::EMPTY:
        m_color = { 0, 0, 0, 0 }; // Полностью прозрачный
        break;
    case TileType::FLOOR:
        m_color = { 180, 180, 180, 255 }; // Светло-серый
        break;
    case TileType::WALL:
        m_color = { 100, 100, 100, 255 }; // Темно-серый
        break;
    case TileType::DOOR:
        m_color = { 120, 60, 0, 255 }; // Коричневый
        break;
    case TileType::WATER:
        m_color = { 0, 100, 255, 200 }; // Полупрозрачный синий
        break;
    case TileType::GRASS:
        m_color = { 50, 180, 50, 255 }; // Зеленый
        break;
    case TileType::STONE:
        m_color = { 150, 150, 150, 255 }; // Серый камень
        break;
    case TileType::METAL:
        m_color = { 170, 170, 190, 255 }; // Серебристый
        break;
    case TileType::GLASS:
        m_color = { 200, 200, 255, 150 }; // Полупрозрачный голубой
        break;
    case TileType::WOOD:
        m_color = { 150, 100, 50, 255 }; // Коричневый
        break;
    case TileType::SAND:
        m_color = { 230, 220, 170, 255 }; // Песочный
        break;
    case TileType::SNOW:
        m_color = { 240, 240, 255, 255 }; // Белый с оттенком синего
        break;
    case TileType::ICE:
        m_color = { 200, 220, 255, 200 }; // Голубой полупрозрачный
        break;
    case TileType::LAVA:
        m_color = { 255, 100, 0, 255 }; // Оранжево-красный
        break;
    case TileType::OBSTACLE:
    case TileType::SPECIAL:
    case TileType::ROCK_FORMATION:
    case TileType::FOREST:
    default:
        m_color = { 120, 120, 120, 255 }; // Серый по умолчанию
        break;
    }
}

SDL_Color MapTile::getColor() const {
    return m_color;
}

std::string MapTile::toString() const {
    std::ostringstream oss;
    oss << "MapTile[Type: " << TileTypeToString(m_type)
        << ", Walkable: " << (m_walkable ? "true" : "false")
        << ", Transparent: " << (m_transparent ? "true" : "false")
        << ", Height: " << m_height
        << ", Color: (" << (int)m_color.r << "," << (int)m_color.g << "," << (int)m_color.b << "," << (int)m_color.a << ")"
        << "]";
    return oss.str();
}