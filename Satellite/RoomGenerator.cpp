#include "RoomGenerator.h"
#include <random>
#include <algorithm>
#include <ctime>
#include "Logger_old.h"

RoomGenerator::RoomGenerator(unsigned int seed)
    : m_seed(seed), m_maxRoomSize(15), m_minRoomSize(7), m_maxCorridorLength(5),
    m_minRooms(5), m_maxRooms(10) { // добавить эти инициализации
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
    centralRoom = getBiomeRoomType(biomeType);
    centralRoom.x = width / 2 - m_maxRoomSize / 2;
    centralRoom.y = height / 2 - m_maxRoomSize / 2;
    centralRoom.width = getRandomRoomSize();
    centralRoom.height = getRandomRoomSize();

    rooms.push_back(centralRoom);

    // 4. Определяем желаемое количество комнат
    int targetRoomCount = m_minRooms + m_rng() % (m_maxRooms - m_minRooms + 1);

    // Информируем о начале генерации
    LOG_INFO("Starting room generation, target count: " + std::to_string(targetRoomCount));

    // 5. Пытаемся добавить комнаты с ограниченным числом попыток
    int maxAttempts = targetRoomCount * 3; // Больше попыток для гарантии результата
    int attempts = 0;

    while (rooms.size() < targetRoomCount && attempts < maxAttempts) {
        addRoomWithCorridor(tileMap, rooms, biomeType);
        attempts++;
    }

    // Если не достигли минимального количества комнат, пробуем с уменьшенными размерами
    if (rooms.size() < m_minRooms) {
        LOG_WARNING("Failed to generate minimum room count with standard sizes. Trying with smaller rooms.");

        // Временно уменьшаем размеры комнат для более компактного размещения
        int originalMinSize = m_minRoomSize;
        int originalMaxSize = m_maxRoomSize;

        m_minRoomSize = std::max(5, m_minRoomSize - 2);
        m_maxRoomSize = std::max(m_minRoomSize + 2, m_maxRoomSize - 2);

        // Дополнительные попытки с уменьшенными размерами
        attempts = 0;
        while (rooms.size() < m_minRooms && attempts < maxAttempts) {
            addRoomWithCorridor(tileMap, rooms, biomeType);
            attempts++;
        }

        // Восстанавливаем оригинальные значения
        m_minRoomSize = originalMinSize;
        m_maxRoomSize = originalMaxSize;
    }

    // 6. Отрисовываем все сгенерированные комнаты на карте
    for (const auto& room : rooms) {
        createRoom(tileMap, room);
    }

    // 7. Создаем корридоры между комнатами
    // Соединяем каждую комнату (кроме первой) с одной из предыдущих
    // Это создает гарантированно связанный граф комнат
    for (size_t i = 1; i < rooms.size(); i++) {
        // Выбираем случайную комнату из уже созданных
        size_t targetRoomIndex = m_rng() % i;
        createCorridor(tileMap, rooms[targetRoomIndex], rooms[i],
            getBiomeFloorType(biomeType), getBiomeWallType(biomeType));
    }

    // 8. Добавляем несколько дополнительных случайных коридоров между комнатами для создания циклов
    // Это сделает карту более интересной для исследования
    int extraCorridors = std::min(3, static_cast<int>(rooms.size() / 3));
    for (int i = 0; i < extraCorridors; i++) {
        if (rooms.size() > 3) { // Только если есть достаточно комнат
            size_t roomIndex1 = m_rng() % rooms.size();
            size_t roomIndex2 = m_rng() % rooms.size();

            // Убедимся, что выбраны две разные комнаты
            while (roomIndex1 == roomIndex2) {
                roomIndex2 = m_rng() % rooms.size();
            }

            createCorridor(tileMap, rooms[roomIndex1], rooms[roomIndex2],
                getBiomeFloorType(biomeType), getBiomeWallType(biomeType));
        }
    }

    LOG_INFO("Generated map with " + std::to_string(rooms.size()) + " rooms for biome: " +
        std::to_string(static_cast<int>(biomeType)));

    return true;
}

void RoomGenerator::setRoomCountLimits(int minRooms, int maxRooms) {
    m_minRooms = std::max(1, minRooms); // Минимум 1 комната
    m_maxRooms = std::max(m_minRooms + 1, maxRooms); // Минимум на 1 больше минимума
    LOG_DEBUG("Room count limits set to: min=" + std::to_string(m_minRooms) +
        ", max=" + std::to_string(m_maxRooms));
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
            if (!tile) continue;

            // Определяем, является ли текущая позиция краем комнаты
            bool isEdge = x == room.x || x == room.x + room.width - 1 ||
                y == room.y || y == room.y + room.height - 1;

            if (isEdge) {
                // Создаем стены по периметру комнаты
                tile->setType(wallType);
                tile->setWalkable(false);
            }
            else {
                // Внутренность комнаты - пол
                tile->setType(floorType);
                tile->setWalkable(true);
            }
        }
    }

    // Дополнительно проверяем углы, чтобы убедиться, что они не пропущены
    int cornerPoints[4][2] = {
        {room.x, room.y},                         // Верхний левый
        {room.x + room.width - 1, room.y},         // Верхний правый
        {room.x, room.y + room.height - 1},         // Нижний левый
        {room.x + room.width - 1, room.y + room.height - 1} // Нижний правый
    };

    for (int i = 0; i < 4; i++) {
        int x = cornerPoints[i][0];
        int y = cornerPoints[i][1];

        if (tileMap->isValidCoordinate(x, y)) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                tile->setType(wallType);
                tile->setWalkable(false);
            }
        }
    }
}

void RoomGenerator::createCorridor(TileMap* tileMap, const Room& room1, const Room& room2,
    TileType floorType, TileType wallType)
{
    // Находим центры комнат
    int x1 = room1.x + room1.width / 2;
    int y1 = room1.y + room1.height / 2;
    int x2 = room2.x + room2.width / 2;
    int y2 = room2.y + room2.height / 2;

    // Определяем, в какой части стены будут проходы
    // Горизонтальный коридор
    int corridorY = y1;
    int startX = std::min(x1, x2);
    int endX = std::max(x1, x2);

    // Вертикальный коридор
    int corridorX = x2;
    int startY = std::min(y1, y2);
    int endY = std::max(y1, y2);

    // Определяем, находятся ли коридоры внутри комнат
    bool horizontalCorridorInsideRooms = false;
    bool verticalCorridorInsideRooms = false;

    // Проверяем горизонтальный коридор
    if ((startX >= room1.x && startX <= room1.x + room1.width &&
        endX >= room1.x && endX <= room1.x + room1.width) ||
        (startX >= room2.x && startX <= room2.x + room2.width &&
            endX >= room2.x && endX <= room2.x + room2.width)) {
        horizontalCorridorInsideRooms = true;
    }

    // Проверяем вертикальный коридор
    if ((startY >= room1.y && startY <= room1.y + room1.height &&
        endY >= room1.y && endY <= room1.y + room1.height) ||
        (startY >= room2.y && startY <= room2.y + room2.height &&
            endY >= room2.y && endY <= room2.y + room2.height)) {
        verticalCorridorInsideRooms = true;
    }

    // Создаем дверные проемы только если коридор действительно проходит через стену

    // Проемы для горизонтального коридора
    if (!horizontalCorridorInsideRooms) {
        // Находим точки пересечения с комнатами
        // Проверяем пересечение с первой комнатой
        if (corridorY >= room1.y && corridorY < room1.y + room1.height) {
            // Восточная стена комнаты 1 (если коридор идет вправо)
            if (x1 < x2 && tileMap->isValidCoordinate(room1.x + room1.width - 1, corridorY)) {
                MapTile* tile = tileMap->getTile(room1.x + room1.width - 1, corridorY);
                if (tile) {
                    tile->setType(floorType);
                    tile->setWalkable(true);
                }
            }

            // Западная стена комнаты 1 (если коридор идет влево)
            if (x1 > x2 && tileMap->isValidCoordinate(room1.x, corridorY)) {
                MapTile* tile = tileMap->getTile(room1.x, corridorY);
                if (tile) {
                    tile->setType(floorType);
                    tile->setWalkable(true);
                }
            }
        }

        // Проверяем пересечение со второй комнатой
        if (corridorY >= room2.y && corridorY < room2.y + room2.height) {
            // Западная стена комнаты 2 (если коридор идет влево от нее)
            if (x2 > x1 && tileMap->isValidCoordinate(room2.x, corridorY)) {
                MapTile* tile = tileMap->getTile(room2.x, corridorY);
                if (tile) {
                    tile->setType(floorType);
                    tile->setWalkable(true);
                }
            }

            // Восточная стена комнаты 2 (если коридор идет вправо от нее)
            if (x2 < x1 && tileMap->isValidCoordinate(room2.x + room2.width - 1, corridorY)) {
                MapTile* tile = tileMap->getTile(room2.x + room2.width - 1, corridorY);
                if (tile) {
                    tile->setType(floorType);
                    tile->setWalkable(true);
                }
            }
        }
    }

    // Проемы для вертикального коридора
    if (!verticalCorridorInsideRooms) {
        // Проверяем пересечение с первой комнатой
        if (corridorX >= room1.x && corridorX < room1.x + room1.width) {
            // Южная стена комнаты 1 (если коридор идет вниз)
            if (y1 < y2 && tileMap->isValidCoordinate(corridorX, room1.y + room1.height - 1)) {
                MapTile* tile = tileMap->getTile(corridorX, room1.y + room1.height - 1);
                if (tile) {
                    tile->setType(floorType);
                    tile->setWalkable(true);
                }
            }

            // Северная стена комнаты 1 (если коридор идет вверх)
            if (y1 > y2 && tileMap->isValidCoordinate(corridorX, room1.y)) {
                MapTile* tile = tileMap->getTile(corridorX, room1.y);
                if (tile) {
                    tile->setType(floorType);
                    tile->setWalkable(true);
                }
            }
        }

        // Проверяем пересечение со второй комнатой
        if (corridorX >= room2.x && corridorX < room2.x + room2.width) {
            // Северная стена комнаты 2 (если коридор идет вверх от нее)
            if (y2 > y1 && tileMap->isValidCoordinate(corridorX, room2.y)) {
                MapTile* tile = tileMap->getTile(corridorX, room2.y);
                if (tile) {
                    tile->setType(floorType);
                    tile->setWalkable(true);
                }
            }

            // Южная стена комнаты 2 (если коридор идет вниз от нее)
            if (y2 < y1 && tileMap->isValidCoordinate(corridorX, room2.y + room2.height - 1)) {
                MapTile* tile = tileMap->getTile(corridorX, room2.y + room2.height - 1);
                if (tile) {
                    tile->setType(floorType);
                    tile->setWalkable(true);
                }
            }
        }
    }

    // Создаем сам горизонтальный коридор
    for (int x = startX; x <= endX; x++) {
        if (tileMap->isValidCoordinate(x, corridorY)) {
            MapTile* tile = tileMap->getTile(x, corridorY);
            if (tile) {
                tile->setType(floorType);
                tile->setWalkable(true);
            }
        }
    }

    // Создаем сам вертикальный коридор
    for (int y = startY; y <= endY; y++) {
        if (tileMap->isValidCoordinate(corridorX, y)) {
            MapTile* tile = tileMap->getTile(corridorX, y);
            if (tile) {
                tile->setType(floorType);
                tile->setWalkable(true);
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
    // Создаем стены вокруг горизонтального коридора
    for (int x = startX; x <= endX; x++) {
        // Проверяем позиции над и под коридором
        for (int dy = -1; dy <= 1; dy += 2) {
            int y = y1 + dy;
            if (tileMap->isValidCoordinate(x, y)) {
                MapTile* tile = tileMap->getTile(x, y);
                if (tile) {
                    // Добавляем стену только если тайл пуст или не является полом/проходимым тайлом
                    TileType currentType = tile->getType();
                    if (currentType == TileType::EMPTY ||
                        !(currentType == floorType || IsWalkable(currentType))) {
                        tile->setType(wallType);
                        tile->setWalkable(false);
                    }
                }
            }
        }
    }

    // Создаем стены вокруг вертикального коридора
    for (int y = startY; y <= endY; y++) {
        // Проверяем позиции слева и справа от коридора
        for (int dx = -1; dx <= 1; dx += 2) {
            int x = x2 + dx;
            if (tileMap->isValidCoordinate(x, y)) {
                MapTile* tile = tileMap->getTile(x, y);
                if (tile) {
                    // Добавляем стену только если тайл пуст или не является полом/проходимым тайлом
                    TileType currentType = tile->getType();
                    if (currentType == TileType::EMPTY ||
                        !(currentType == floorType || IsWalkable(currentType))) {
                        tile->setType(wallType);
                        tile->setWalkable(false);
                    }
                }
            }
        }
    }

    // Добавляем угловые тайлы на пересечениях коридоров
    // Это особенно важно для создания непрерывных стен
    if (x1 != x2 && y1 != y2) {
        // Проверяем и добавляем угловой тайл в точке поворота коридора (x2, y1)
        // Каждый угол может иметь до трех угловых стен
        int cornerX = x2;
        int cornerY = y1;

        // Угловые тайлы вокруг точки поворота
        int corners[4][2] = {
            {-1, -1}, // Верхний левый
            {1, -1},  // Верхний правый
            {-1, 1},  // Нижний левый
            {1, 1}    // Нижний правый
        };

        for (int i = 0; i < 4; i++) {
            int checkX = cornerX + corners[i][0];
            int checkY = cornerY + corners[i][1];

            if (tileMap->isValidCoordinate(checkX, checkY)) {
                MapTile* tile = tileMap->getTile(checkX, checkY);
                if (tile) {
                    // Проверяем, не находится ли это в комнате
                    bool isInRoom = false;

                    // Здесь можно добавить проверку на нахождение в комнате,
                    // но для простоты просто проверим, является ли это уже полом
                    if (tile->getType() == floorType || tile->isWalkable()) {
                        isInRoom = true;
                    }

                    if (!isInRoom && (tile->getType() == TileType::EMPTY ||
                        !(tile->getType() == floorType || tile->isWalkable()))) {
                        tile->setType(wallType);
                        tile->setWalkable(false);
                    }
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
    newRoom = getBiomeRoomType(biomeType);
    newRoom.width = getRandomRoomSize();
    newRoom.height = getRandomRoomSize();

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