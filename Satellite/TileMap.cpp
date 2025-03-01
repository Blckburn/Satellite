#include "TileMap.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

TileMap::TileMap(int width, int height)
    : m_width(width), m_height(height) {
    // Инициализация вектора тайлов будет выполнена в методе initialize()
}

TileMap::~TileMap() {
    // Вектор тайлов будет автоматически очищен при уничтожении объекта
}

bool TileMap::initialize() {
    // Изменяем размер внешнего вектора
    m_tiles.resize(m_height);

    // Инициализируем каждую строку карты
    for (int y = 0; y < m_height; ++y) {
        // Изменяем размер внутреннего вектора (строки)
        m_tiles[y].resize(m_width);

        // Инициализируем каждый тайл в строке
        for (int x = 0; x < m_width; ++x) {
            m_tiles[y][x] = MapTile(TileType::EMPTY);
        }
    }

    return true;
}

bool TileMap::isValidCoordinate(int x, int y) const {
    return (x >= 0 && x < m_width && y >= 0 && y < m_height);
}

MapTile* TileMap::getTile(int x, int y) {
    if (!isValidCoordinate(x, y)) {
        return nullptr;
    }

    return &m_tiles[y][x];
}

const MapTile* TileMap::getTile(int x, int y) const {
    if (!isValidCoordinate(x, y)) {
        return nullptr;
    }

    return &m_tiles[y][x];
}

bool TileMap::setTileType(int x, int y, TileType type) {
    if (!isValidCoordinate(x, y)) {
        return false;
    }

    m_tiles[y][x].setType(type);
    return true;
}

bool TileMap::setTileWalkable(int x, int y, bool walkable) {
    if (!isValidCoordinate(x, y)) {
        return false;
    }

    m_tiles[y][x].setWalkable(walkable);
    return true;
}

bool TileMap::setTileTransparent(int x, int y, bool transparent) {
    if (!isValidCoordinate(x, y)) {
        return false;
    }

    m_tiles[y][x].setTransparent(transparent);
    return true;
}

bool TileMap::setTileHeight(int x, int y, float height) {
    if (!isValidCoordinate(x, y)) {
        return false;
    }

    m_tiles[y][x].setHeight(height);
    return true;
}

void TileMap::fillRect(int startX, int startY, int endX, int endY, TileType type) {
    // Нормализуем координаты (убеждаемся, что startX <= endX и startY <= endY)
    if (startX > endX) std::swap(startX, endX);
    if (startY > endY) std::swap(startY, endY);

    // Ограничиваем координаты размерами карты
    startX = std::max(0, startX);
    startY = std::max(0, startY);
    endX = std::min(m_width - 1, endX);
    endY = std::min(m_height - 1, endY);

    // Заполняем прямоугольник
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            setTileType(x, y, type);
        }
    }
}

void TileMap::createRoom(int startX, int startY, int endX, int endY, TileType floorType, TileType wallType) {
    // Нормализуем координаты (убеждаемся, что startX <= endX и startY <= endY)
    if (startX > endX) std::swap(startX, endX);
    if (startY > endY) std::swap(startY, endY);

    // Ограничиваем координаты размерами карты
    startX = std::max(0, startX);
    startY = std::max(0, startY);
    endX = std::min(m_width - 1, endX);
    endY = std::min(m_height - 1, endY);

    // Заполняем пол
    for (int y = startY + 1; y < endY; ++y) {
        for (int x = startX + 1; x < endX; ++x) {
            setTileType(x, y, floorType);
        }
    }

    // Создаем стены (горизонтальные)
    for (int x = startX; x <= endX; ++x) {
        setTileType(x, startY, wallType);
        setTileType(x, endY, wallType);
    }

    // Создаем стены (вертикальные)
    for (int y = startY + 1; y < endY; ++y) {
        setTileType(startX, y, wallType);
        setTileType(endX, y, wallType);
    }
}

void TileMap::createHorizontalCorridor(int startX, int endX, int y, TileType floorType) {
    // Нормализуем координаты
    if (startX > endX) std::swap(startX, endX);

    // Ограничиваем координаты размерами карты
    startX = std::max(0, startX);
    endX = std::min(m_width - 1, endX);
    y = std::max(0, std::min(m_height - 1, y));

    // Создаем коридор
    for (int x = startX; x <= endX; ++x) {
        setTileType(x, y, floorType);
    }
}

void TileMap::createVerticalCorridor(int x, int startY, int endY, TileType floorType) {
    // Нормализуем координаты
    if (startY > endY) std::swap(startY, endY);

    // Ограничиваем координаты размерами карты
    x = std::max(0, std::min(m_width - 1, x));
    startY = std::max(0, startY);
    endY = std::min(m_height - 1, endY);

    // Создаем коридор
    for (int y = startY; y <= endY; ++y) {
        setTileType(x, y, floorType);
    }
}

void TileMap::createDoor(int x, int y, TileType doorType) {
    if (isValidCoordinate(x, y)) {
        setTileType(x, y, doorType);
        setTileWalkable(x, y, true); // По умолчанию дверь проходима (открыта)
    }
}

bool TileMap::isTileWalkable(int x, int y) const {
    const MapTile* tile = getTile(x, y);
    if (!tile) {
        return false; // Если координаты за пределами карты, считаем тайл непроходимым
    }

    return tile->isWalkable();
}

bool TileMap::isTileTransparent(int x, int y) const {
    const MapTile* tile = getTile(x, y);
    if (!tile) {
        return true; // Если координаты за пределами карты, считаем тайл прозрачным
    }

    return tile->isTransparent();
}

void TileMap::clear() {
    // Очищаем карту, заполняя ее пустыми тайлами
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            m_tiles[y][x] = MapTile(TileType::EMPTY);
        }
    }
}

bool TileMap::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }

    // Записываем размеры карты
    file.write(reinterpret_cast<const char*>(&m_width), sizeof(m_width));
    file.write(reinterpret_cast<const char*>(&m_height), sizeof(m_height));

    // Записываем тайлы
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const MapTile& tile = m_tiles[y][x];

            // Записываем тип тайла
            TileType type = tile.getType();
            file.write(reinterpret_cast<const char*>(&type), sizeof(type));

            // Записываем свойства тайла
            bool walkable = tile.isWalkable();
            bool transparent = tile.isTransparent();
            float height = tile.getHeight();
            file.write(reinterpret_cast<const char*>(&walkable), sizeof(walkable));
            file.write(reinterpret_cast<const char*>(&transparent), sizeof(transparent));
            file.write(reinterpret_cast<const char*>(&height), sizeof(height));
        }
    }

    file.close();
    return true;
}

bool TileMap::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for reading: " << filename << std::endl;
        return false;
    }

    // Считываем размеры карты
    int width, height;
    file.read(reinterpret_cast<char*>(&width), sizeof(width));
    file.read(reinterpret_cast<char*>(&height), sizeof(height));

    // Проверяем размеры
    if (width <= 0 || height <= 0 || width > 1000 || height > 1000) {
        std::cerr << "Invalid map dimensions: " << width << "x" << height << std::endl;
        file.close();
        return false;
    }

    // Изменяем размеры карты
    m_width = width;
    m_height = height;
    initialize();

    // Считываем тайлы
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            // Считываем тип тайла
            TileType type;
            file.read(reinterpret_cast<char*>(&type), sizeof(type));

            // Считываем свойства тайла
            bool walkable;
            bool transparent;
            float height;
            file.read(reinterpret_cast<char*>(&walkable), sizeof(walkable));
            file.read(reinterpret_cast<char*>(&transparent), sizeof(transparent));
            file.read(reinterpret_cast<char*>(&height), sizeof(height));

            // Устанавливаем свойства тайла
            MapTile tile(type, walkable, transparent, height);
            m_tiles[y][x] = tile;
        }
    }

    file.close();
    return true;
}