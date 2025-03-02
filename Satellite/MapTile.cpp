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

    // ВАЖНО: Всегда обновляем свойства при изменении типа
    // для обеспечения согласованности проходимости и высоты
    m_walkable = IsWalkable(type);
    m_transparent = IsTransparent(type);
    m_height = GetDefaultHeight(type);

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
        // Дополнительно подчеркиваем, что стены имеют высоту
        m_height = 1.0f;
        m_walkable = false;
        break;
    case TileType::DOOR:
        m_color = { 120, 80, 40, 255 }; // Коричневый
        // Двери имеют высоту и могут быть проходимыми
        m_height = 1.0f;
        // m_walkable устанавливается извне в зависимости от состояния двери (открыта/закрыта)
        break;
    case TileType::WATER:
        m_color = { 64, 164, 223, 255 }; // Синий
        // ИЗМЕНЕНО: Вода должна иметь минимальную высоту для визуального эффекта,
        // но быть полностью непроходимой со всех сторон
        m_height = 0.1f;  // Меньшая высота для лучшего визуального эффекта
        m_walkable = false; // Вода всегда непроходима
        break;
        // остальные case без изменений...
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