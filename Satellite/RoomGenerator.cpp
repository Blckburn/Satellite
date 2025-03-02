#include "RoomGenerator.h"
#include <random>
#include <algorithm>
#include <ctime>
#include "Logger.h"

RoomGenerator::RoomGenerator(unsigned int seed)
    : m_seed(seed), m_maxRoomSize(15), m_minRoomSize(7), m_maxCorridorLength(5)
{
    // Если сид не задан, генерируем случайный
    if (m_seed == 0) {
        m_seed = static_cast<unsigned int>(std::time(nullptr));
    }
    resetGenerator();
}

RoomGenerator::~RoomGenerator()
{
}

void RoomGenerator::resetGenerator()
{
    // Инициализация генератора случайных чисел с заданным сидом
    m_rng.seed(m_seed);
    LOG_DEBUG("RoomGenerator initialized with seed: " + std::to_string(m_seed));
}

void RoomGenerator::setSeed(unsigned int seed)
{
    m_seed = seed;
    resetGenerator();
}

void RoomGenerator::setRoomSizeLimits(int minSize, int maxSize)
{
    m_minRoomSize = std::max(5, minSize); // Минимум 5x5 для комнаты
    m_maxRoomSize = std::max(m_minRoomSize + 2, maxSize); // Минимальная разница 2
    LOG_DEBUG("Room size limits set to: min=" + std::to_string(m_minRoomSize) +
        ", max=" + std::to_string(m_maxRoomSize));
}

bool RoomGenerator::generateMap(TileMap* tileMap, BiomeType biomeType)
{
    if (!tileMap) {
        LOG_ERROR("Invalid tile map provided to RoomGenerator");
        return false;
    }

    // 1. Очищаем карту
    clearMap(tileMap);

    // 2. Получаем размеры карты
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Проверяем, что карта имеет достаточные размеры
    if (width < m_maxRoomSize * 2 || height < m_maxRoomSize * 2) {
        LOG_WARNING("Map too small for proper room generation. Width: " +
            std::to_string(width) + ", Height: " + std::to_string(height));
    }

    // 3. Генерируем кластер комнат
    std::vector<Room> rooms;

    // Начинаем с центральной комнаты
    Room centralRoom;
    centralRoom.x = width / 2 - m_maxRoomSize / 2;
    centralRoom.y = height / 2 - m_maxRoomSize / 2;
    centralRoom.width = getRandomRoomSize();
    centralRoom.height = getRandomRoomSize();
    centralRoom.type = getBiomeRoomType(biomeType);

    rooms.push_back(centralRoom);

    // 4. Генерируем несколько комнат вокруг центральной
    int numRooms = 3 + m_rng() % 4; // 3-6 комнат

    for (int i = 0; i < numRooms; i++) {
        addRoomWithCorridor(tileMap, rooms, biomeType);
    }

    // 5. Отрисовываем все сгенерированные комнаты на карте
    for (const auto& room : rooms) {
        createRoom(tileMap, room);
    }

    // 6. Создаем корридоры между комнатами
    for (size_t i = 1; i < rooms.size(); i++) {
        createCorridor(tileMap, rooms[0], rooms[i], getBiomeFloorType(biomeType),
            getBiomeWallType(biomeType));
    }

    LOG_INFO("Generated map with " + std::to_string(rooms.size()) + " rooms for biome: " +
        std::to_string(static_cast<int>(biomeType)));

    return true;
}

void RoomGenerator::clearMap(TileMap* tileMap)
{
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Заполняем карту пустыми тайлами
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                tile->setType(TileType::EMPTY);
            }
        }
    }
}

void RoomGenerator::createRoom(TileMap* tileMap, const Room& room)
{
    // Получаем соответствующие типы тайлов для комнаты
    TileType floorType = room.floorType;
    TileType wallType = room.wallType;

    // Создаем комнату с указанными размерами и типами тайлов
    for (int y = room.y; y < room.y + room.height; y++) {
        for (int x = room.x; x < room.x + room.width; x++) {
            // Проверяем, что координаты в пределах карты
            if (!tileMap->isValidCoordinate(x, y))
                continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                // Стены по периметру
                if (x == room.x || x == room.x + room.width - 1 ||
                    y == room.y || y == room.y + room.height - 1) {
                    tile->setType(wallType);
                }
                // Внутренность комнаты - пол
                else {
                    tile->setType(floorType);
                }
            }
        }
    }
}

void RoomGenerator::createCorridor(TileMap* tileMap, const Room& room1, const Room& room2,
    TileType floorType, TileType wallType)
{
    // Определяем центральные точки комнат
    int x1 = room1.x + room1.width / 2;
    int y1 = room1.y + room1.height / 2;
    int x2 = room2.x + room2.width / 2;
    int y2 = room2.y + room2.height / 2;

    // Создаем горизонтальный коридор
    int startX = std::min(x1, x2);
    int endX = std::max(x1, x2);

    for (int x = startX; x <= endX; x++) {
        if (tileMap->isValidCoordinate(x, y1)) {
            MapTile* tile = tileMap->getTile(x, y1);
            if (tile && tile->getType() != floorType) {
                // Проверяем, не является ли тайл частью комнаты
                bool isRoomWall = false;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;

                        int nx = x + dx;
                        int ny = y1 + dy;

                        if (tileMap->isValidCoordinate(nx, ny)) {
                            MapTile* neighbor = tileMap->getTile(nx, ny);
                            if (neighbor && neighbor->getType() == floorType) {
                                isRoomWall = true;
                                break;
                            }
                        }
                    }
                    if (isRoomWall) break;
                }

                if (!isRoomWall) {
                    tile->setType(floorType);
                }
            }
        }
    }

    // Создаем вертикальный коридор
    int startY = std::min(y1, y2);
    int endY = std::max(y1, y2);

    for (int y = startY; y <= endY; y++) {
        if (tileMap->isValidCoordinate(x2, y)) {
            MapTile* tile = tileMap->getTile(x2, y);
            if (tile && tile->getType() != floorType) {
                // Проверяем, не является ли тайл частью комнаты
                bool isRoomWall = false;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        if (dx == 0 && dy == 0) continue;

                        int nx = x2 + dx;
                        int ny = y + dy;

                        if (tileMap->isValidCoordinate(nx, ny)) {
                            MapTile* neighbor = tileMap->getTile(nx, ny);
                            if (neighbor && neighbor->getType() == floorType) {
                                isRoomWall = true;
                                break;
                            }
                        }
                    }
                    if (isRoomWall) break;
                }

                if (!isRoomWall) {
                    tile->setType(floorType);
                }
            }
        }
    }

    // Добавляем стены вокруг коридора
    addWallsAroundCorridor(tileMap, startX, endX, startY, endY, x1, y1, x2, y2, floorType, wallType);
}

void RoomGenerator::addWallsAroundCorridor(TileMap* tileMap, int startX, int endX, int startY, int endY,
    int x1, int y1, int x2, int y2,
    TileType floorType, TileType wallType)
{
    // Горизонтальный коридор
    for (int x = startX; x <= endX; x++) {
        // Стены сверху и снизу
        for (int dy = -1; dy <= 1; dy += 2) {
            int y = y1 + dy;
            if (tileMap->isValidCoordinate(x, y)) {
                MapTile* tile = tileMap->getTile(x, y);
                if (tile && tile->getType() == TileType::EMPTY) {
                    tile->setType(wallType);
                }
            }
        }
    }

    // Вертикальный коридор
    for (int y = startY; y <= endY; y++) {
        // Стены слева и справа
        for (int dx = -1; dx <= 1; dx += 2) {
            int x = x2 + dx;
            if (tileMap->isValidCoordinate(x, y)) {
                MapTile* tile = tileMap->getTile(x, y);
                if (tile && tile->getType() == TileType::EMPTY) {
                    tile->setType(wallType);
                }
            }
        }
    }
}

void RoomGenerator::addRoomWithCorridor(TileMap* tileMap, std::vector<Room>& rooms, BiomeType biomeType)
{
    if (rooms.empty()) return;

    const Room& sourceRoom = rooms[m_rng() % rooms.size()];

    // Определяем направление для новой комнаты (0=север, 1=восток, 2=юг, 3=запад)
    int direction = m_rng() % 4;

    // Создаем новую комнату
    Room newRoom;
    newRoom.width = getRandomRoomSize();
    newRoom.height = getRandomRoomSize();
    newRoom.type = getBiomeRoomType(biomeType);

    // Длина коридора
    int corridorLength = 2 + m_rng() % m_maxCorridorLength;

    // Размещаем комнату в зависимости от направления
    switch (direction) {
    case 0: // Север
        newRoom.x = sourceRoom.x + (sourceRoom.width - newRoom.width) / 2;
        newRoom.y = sourceRoom.y - newRoom.height - corridorLength;
        break;
    case 1: // Восток
        newRoom.x = sourceRoom.x + sourceRoom.width + corridorLength;
        newRoom.y = sourceRoom.y + (sourceRoom.height - newRoom.height) / 2;
        break;
    case 2: // Юг
        newRoom.x = sourceRoom.x + (sourceRoom.width - newRoom.width) / 2;
        newRoom.y = sourceRoom.y + sourceRoom.height + corridorLength;
        break;
    case 3: // Запад
        newRoom.x = sourceRoom.x - newRoom.width - corridorLength;
        newRoom.y = sourceRoom.y + (sourceRoom.height - newRoom.height) / 2;
        break;
    }

    // Проверяем, что комната находится в пределах карты
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    if (newRoom.x < 1 || newRoom.x + newRoom.width >= width - 1 ||
        newRoom.y < 1 || newRoom.y + newRoom.height >= height - 1) {
        // Если комната выходит за пределы карты, пропускаем её
        return;
    }

    // Проверяем, что новая комната не пересекается с существующими
    for (const auto& room : rooms) {
        if (checkRoomOverlap(newRoom, room, 1)) {
            // Если комнаты пересекаются, пропускаем новую комнату
            return;
        }
    }

    // Добавляем комнату в список
    rooms.push_back(newRoom);
}

bool RoomGenerator::checkRoomOverlap(const Room& room1, const Room& room2, int padding)
{
    return (room1.x - padding < room2.x + room2.width + padding &&
        room1.x + room1.width + padding > room2.x - padding &&
        room1.y - padding < room2.y + room2.height + padding &&
        room1.y + room1.height + padding > room2.y - padding);
}

int RoomGenerator::getRandomRoomSize()
{
    // Генерация случайного размера комнаты в пределах заданных ограничений
    return m_minRoomSize + m_rng() % (m_maxRoomSize - m_minRoomSize + 1);
}

TileType RoomGenerator::getBiomeFloorType(BiomeType biomeType)
{
    // Возвращает тип пола в зависимости от биома
    switch (biomeType) {
    case BiomeType::FOREST:
        return TileType::GRASS;
    case BiomeType::DESERT:
        return TileType::SAND;
    case BiomeType::TUNDRA:
        return TileType::SNOW;
    case BiomeType::VOLCANIC:
        return TileType::STONE;
    default:
        return TileType::FLOOR;
    }
}

TileType RoomGenerator::getBiomeWallType(BiomeType biomeType)
{
    // Возвращает тип стены в зависимости от биома
    switch (biomeType) {
    case BiomeType::FOREST:
        return TileType::FOREST;
    case BiomeType::DESERT:
        return TileType::ROCK_FORMATION;
    case BiomeType::TUNDRA:
        return TileType::ICE;
    case BiomeType::VOLCANIC:
        return TileType::ROCK_FORMATION;
    default:
        return TileType::WALL;
    }
}

RoomGenerator::RoomType RoomGenerator::getBiomeRoomType(BiomeType biomeType)
{
    // Структура комнаты зависит от типа биома
    Room room;

    switch (biomeType) {
    case BiomeType::FOREST:
        room.floorType = TileType::GRASS;
        room.wallType = TileType::FOREST;
        break;
    case BiomeType::DESERT:
        room.floorType = TileType::SAND;
        room.wallType = TileType::ROCK_FORMATION;
        break;
    case BiomeType::TUNDRA:
        room.floorType = TileType::SNOW;
        room.wallType = TileType::ICE;
        break;
    case BiomeType::VOLCANIC:
        room.floorType = TileType::STONE;
        room.wallType = TileType::LAVA;
        break;
    default:
        room.floorType = TileType::FLOOR;
        room.wallType = TileType::WALL;
    }

    return room;
}