#include "MapTile.h"
#include <sstream>

MapTile::MapTile()
    : m_type(TileType::EMPTY), m_walkable(false), m_transparent(true), m_height(0.0f),
    m_color({ 20, 35, 20, 255 }) {
}

MapTile::MapTile(TileType type)
    : m_type(type), m_walkable(IsWalkable(type)), m_transparent(IsTransparent(type)),
    m_height(GetDefaultHeight(type)), m_color({ 20, 35, 20, 255 }) {
    // Устанавливаем цвет в зависимости от типа тайла
    setType(type); // Это установит правильный цвет
}

MapTile::MapTile(TileType type, bool walkable, bool transparent, float height)
    : m_type(type), m_walkable(walkable), m_transparent(transparent), m_height(height),
    m_color({ 20, 35, 20, 255 }) {
    // Устанавливаем цвет в зависимости от типа тайла
    setType(type); // Это установит правильный цвет, но сохранит переданные параметры
}

MapTile::~MapTile() {
    // Нет динамически выделенных ресурсов, поэтому деструктор пуст
}

void MapTile::setType(TileType type) {
    m_type = type;

    // Устанавливаем свойства по умолчанию для данного типа, если только они не были переопределены
    if (m_walkable != IsWalkable(type)) {
        m_walkable = IsWalkable(type);
    }

    if (m_transparent != IsTransparent(type)) {
        m_transparent = IsTransparent(type);
    }

    if (m_height != GetDefaultHeight(type)) {
        m_height = GetDefaultHeight(type);
    }

    // Устанавливаем цвет по умолчанию для данного типа
    switch (type) {
    case TileType::EMPTY:
        m_color = { 20, 35, 20, 255 }; // Темно-зеленый (фон)
        break;
    case TileType::FLOOR:
        m_color = { 180, 180, 180, 255 }; // Светло-серый
        break;
    case TileType::WALL:
        m_color = { 100, 100, 100, 255 }; // Средне-серый
        break;
    case TileType::DOOR:
        m_color = { 120, 80, 40, 255 }; // Коричневый
        break;
    case TileType::WATER:
        m_color = { 64, 164, 223, 255 }; // Синий
        break;
    case TileType::GRASS:
        m_color = { 30, 150, 30, 255 }; // Зеленый
        break;
    case TileType::STONE:
        m_color = { 128, 128, 128, 255 }; // Серый
        break;
    case TileType::METAL:
        m_color = { 192, 192, 192, 255 }; // Серебристый
        break;
    case TileType::GLASS:
        m_color = { 200, 230, 255, 128 }; // Полупрозрачный голубоватый
        break;
    case TileType::WOOD:
        m_color = { 150, 111, 51, 255 }; // Коричневый
        break;
    case TileType::SPECIAL:
        m_color = { 255, 0, 255, 255 }; // Фиолетовый (для выделения)
        break;
    case TileType::OBSTACLE:
        m_color = { 80, 80, 80, 255 }; // Темно-серый
        break;
    default:
        m_color = { 255, 0, 0, 255 }; // Красный (для отладки неопределенных типов)
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