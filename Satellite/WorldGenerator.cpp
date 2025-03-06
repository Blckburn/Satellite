#include "WorldGenerator.h"
#include "Engine.h"
#include "MapScene.h"
#include "Player.h"
#include "Logger.h"
#include "RoomGenerator.h"
#include <algorithm>
#include <ctime>
#include <cmath>
#include <random>
#include "Switch.h"

WorldGenerator::WorldGenerator(std::shared_ptr<TileMap> tileMap, Engine* engine,
    MapScene* mapScene, std::shared_ptr<Player> player)
    : m_tileMap(tileMap), m_engine(engine), m_mapScene(mapScene),
    m_player(player), m_currentBiome(1) {
}

std::pair<float, float> WorldGenerator::generateTestMap(int biomeType) {
    // Очищаем карту
    m_tileMap->clear();

    // Запоминаем текущий биом
    m_currentBiome = biomeType;

    // Создаем генератор комнат с случайным сидом
    static RoomGenerator roomGen(static_cast<unsigned int>(std::time(nullptr)));

    // Устанавливаем размеры комнат в зависимости от размера карты
    int minSize = std::max(5, m_tileMap->getWidth() / 10);
    int maxSize = std::max(minSize + 5, m_tileMap->getWidth() / 5);
    roomGen.setRoomSizeLimits(minSize, maxSize);
    roomGen.setRoomCountLimits(5, 10); // Минимум 5, максимум 10 комнат

    // Преобразуем числовой индекс биома в тип биома для RoomGenerator
    RoomGenerator::BiomeType roomGenBiomeType;

    // Логируем название биома для отладки
    std::string biomeName;

    switch (biomeType) {
    case 1: // FOREST
        roomGenBiomeType = RoomGenerator::BiomeType::FOREST;
        biomeName = "Forest";
        break;
    case 2: // DESERT
        roomGenBiomeType = RoomGenerator::BiomeType::DESERT;
        biomeName = "Desert";
        break;
    case 3: // TUNDRA
        roomGenBiomeType = RoomGenerator::BiomeType::TUNDRA;
        biomeName = "Tundra";
        break;
    case 4: // VOLCANIC
        roomGenBiomeType = RoomGenerator::BiomeType::VOLCANIC;
        biomeName = "Volcanic";
        break;
    default:
        roomGenBiomeType = RoomGenerator::BiomeType::DEFAULT;
        biomeName = "Default";
        break;
    }

    LOG_INFO("Selected biome: " + biomeName + " (type " + std::to_string(m_currentBiome) + ")");

    // Устанавливаем биом в движке для фона
    if (m_engine) {
        m_engine->setCurrentBiome(m_currentBiome);
    }

    // Генерируем карту с правильным типом биома
    roomGen.generateMap(m_tileMap.get(), roomGenBiomeType);



    // Получаем фактическое количество сгенерированных комнат
    int actualRoomCount = roomGen.getGeneratedRoomCount();
    LOG_INFO("Generated " + std::to_string(actualRoomCount) + " rooms");

    // Сохраняем количество комнат для использования при генерации переключателей
    m_generatedRoomCount = actualRoomCount;

    // Сохраняем количество комнат для использования при генерации переключателей
    m_generatedRoomCount = actualRoomCount;

    // Сначала устанавливаем позицию игрока в центре карты
    float centerX = m_tileMap->getWidth() / 2.0f;
    float centerY = m_tileMap->getHeight() / 2.0f;

    // Но нам нужно убедиться, что игрок появляется на проходимом тайле
    // Ищем ближайший проходимый тайл к центру
    int searchRadius = 10; // Максимальный радиус поиска
    bool foundWalkable = false;

    // Сначала проверяем сам центральный тайл
    int centerTileX = static_cast<int>(centerX);
    int centerTileY = static_cast<int>(centerY);

    // Переменные для сохранения найденной позиции
    float playerX = centerX;
    float playerY = centerY;

    if (m_tileMap->isValidCoordinate(centerTileX, centerTileY) &&
        m_tileMap->isTileWalkable(centerTileX, centerTileY)) {
        // Центральный тайл проходим, используем его
        playerX = centerX;
        playerY = centerY;
        foundWalkable = true;
    }
    else {
        // Центральный тайл непроходим, ищем проходимый тайл рядом
        for (int radius = 1; radius <= searchRadius && !foundWalkable; radius++) {
            // Проверяем тайлы вокруг центра по спирали
            for (int dx = -radius; dx <= radius && !foundWalkable; dx++) {
                for (int dy = -radius; dy <= radius && !foundWalkable; dy++) {
                    // Пропускаем тайлы, не находящиеся на границе текущего радиуса
                    if (std::abs(dx) != radius && std::abs(dy) != radius) continue;

                    int x = centerTileX + dx;
                    int y = centerTileY + dy;

                    if (m_tileMap->isValidCoordinate(x, y) &&
                        m_tileMap->isTileWalkable(x, y)) {
                        // Нашли проходимый тайл
                        playerX = static_cast<float>(x);
                        playerY = static_cast<float>(y);
                        foundWalkable = true;
                        break;
                    }
                }
            }
        }
    }

    // Если проходимый тайл не найден, устанавливаем игрока в центре карты
    // и надеемся на лучшее (редкий случай)
    if (!foundWalkable) {
        playerX = centerX;
        playerY = centerY;
        LOG_WARNING("Could not find walkable tile for player spawn, using center position");
    }



    // Создаем интерактивные предметы на карте
    createInteractiveItems();
    createTerminals();
    createSwitches();


    LOG_INFO("Generated test map with biome type: " + std::to_string(static_cast<int>(biomeType)));

    // Возвращаем найденную стартовую позицию
    return std::make_pair(playerX, playerY);
}

void WorldGenerator::generateDoors(float doorProbability, int maxDoors) {
    if (!m_tileMap) return;

    LOG_INFO("Generating doors in corridors with probability " + std::to_string(doorProbability) +
        " and max count " + std::to_string(maxDoors));

    int doorsCreated = 0;
    std::vector<std::pair<int, int>> potentialDoorLocations;

    // 1. Сначала найдем все потенциальные места для дверей (коридоры)
    for (int y = 1; y < m_tileMap->getHeight() - 1; y++) {
        for (int x = 1; x < m_tileMap->getWidth() - 1; x++) {
            // Проверяем только проходимые тайлы
            if (!m_tileMap->isTileWalkable(x, y)) continue;

            // Проверка на горизонтальный коридор: стены сверху и снизу
            bool isHorizontalCorridor =
                !m_tileMap->isTileWalkable(x, y - 1) && // Стена сверху
                !m_tileMap->isTileWalkable(x, y + 1) && // Стена снизу
                m_tileMap->isTileWalkable(x - 1, y) && // Проход слева
                m_tileMap->isTileWalkable(x + 1, y);   // Проход справа

            // Проверка на вертикальный коридор: стены слева и справа
            bool isVerticalCorridor =
                !m_tileMap->isTileWalkable(x - 1, y) && // Стена слева
                !m_tileMap->isTileWalkable(x + 1, y) && // Стена справа
                m_tileMap->isTileWalkable(x, y - 1) && // Проход сверху
                m_tileMap->isTileWalkable(x, y + 1);   // Проход снизу

            // Дополнительная проверка для узких проходов между комнатами
            bool isNarrowPathway = false;

            // Проверка для горизонтального прохода между двумя большими открытыми пространствами
            if (m_tileMap->isTileWalkable(x - 1, y) && m_tileMap->isTileWalkable(x + 1, y)) {
                // Считаем проходимые тайлы справа и слева, чтобы определить, является ли это проходом между комнатами
                int leftOpenSpace = 0;
                int rightOpenSpace = 0;

                // Считаем открытое пространство слева
                for (int dx = -1; dx >= -3 && x + dx >= 0; dx--) {
                    if (m_tileMap->isTileWalkable(x + dx, y)) {
                        leftOpenSpace++;
                    }
                    else {
                        break;
                    }
                }

                // Считаем открытое пространство справа
                for (int dx = 1; dx <= 3 && x + dx < m_tileMap->getWidth(); dx++) {
                    if (m_tileMap->isTileWalkable(x + dx, y)) {
                        rightOpenSpace++;
                    }
                    else {
                        break;
                    }
                }

                // Если с обеих сторон есть открытое пространство, это может быть проход между комнатами
                if (leftOpenSpace >= 2 && rightOpenSpace >= 2) {
                    isNarrowPathway = true;
                }
            }

            // Проверка для вертикального прохода между двумя открытыми пространствами
            if (m_tileMap->isTileWalkable(x, y - 1) && m_tileMap->isTileWalkable(x, y + 1)) {
                int topOpenSpace = 0;
                int bottomOpenSpace = 0;

                // Считаем открытое пространство сверху
                for (int dy = -1; dy >= -3 && y + dy >= 0; dy--) {
                    if (m_tileMap->isTileWalkable(x, y + dy)) {
                        topOpenSpace++;
                    }
                    else {
                        break;
                    }
                }

                // Считаем открытое пространство снизу
                for (int dy = 1; dy <= 3 && y + dy < m_tileMap->getHeight(); dy++) {
                    if (m_tileMap->isTileWalkable(x, y + dy)) {
                        bottomOpenSpace++;
                    }
                    else {
                        break;
                    }
                }

                // Если с обеих сторон есть открытое пространство, это может быть проход между комнатами
                if (topOpenSpace >= 2 && bottomOpenSpace >= 2) {
                    isNarrowPathway = true;
                }
            }

            // Это место подходит для двери?
            if (isHorizontalCorridor || isVerticalCorridor || isNarrowPathway) {
                // Не размещаем двери слишком близко к краю карты
                if (x > 3 && x < m_tileMap->getWidth() - 3 &&
                    y > 3 && y < m_tileMap->getHeight() - 3) {

                    // Также убедимся, что это не слишком близко к игроку
                    if (m_player) {
                        float playerX = m_player->getPosition().x;
                        float playerY = m_player->getPosition().y;

                        // Вычисляем расстояние до игрока
                        float dx = playerX - static_cast<float>(x);
                        float dy = playerY - static_cast<float>(y);
                        float distanceToPlayer = std::sqrt(dx * dx + dy * dy);

                        // Не размещаем двери слишком близко к игроку
                        if (distanceToPlayer > 5.0f) {
                            potentialDoorLocations.push_back({ x, y });
                        }
                    }
                    else {
                        potentialDoorLocations.push_back({ x, y });
                    }
                }
            }
        }
    }

    // 2. Перемешиваем потенциальные места, чтобы выбрать случайные
    std::random_shuffle(potentialDoorLocations.begin(), potentialDoorLocations.end());

    // 3. Создаем двери с заданной вероятностью, не превышая максимальное количество
    for (const auto& location : potentialDoorLocations) {
        if (doorsCreated >= maxDoors) break;

        // Применяем вероятность размещения
        float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (randomValue <= doorProbability) {
            int x = location.first;
            int y = location.second;

            // Определяем вертикальная это дверь или горизонтальная
            bool isVertical =
                !m_tileMap->isTileWalkable(x - 1, y) &&
                !m_tileMap->isTileWalkable(x + 1, y);

            // Создаем дверь
            std::string doorName = "Door_" + std::to_string(x) + "_" + std::to_string(y);
            auto door = createTestDoor(static_cast<float>(x), static_cast<float>(y), doorName);

            if (door) {
                // Устанавливаем ориентацию
                door->setVertical(isVertical);
                // Инициализируем дверь (вызов перенесен в метод createTestDoor)

                doorsCreated++;

                LOG_INFO("Created door at position (" + std::to_string(x) + ", " +
                    std::to_string(y) + ") with " + (isVertical ? "vertical" : "horizontal") +
                    " orientation for biome " + std::to_string(m_currentBiome));
            }
        }
    }

    // Разрешим генерацию в комнатах и помещениях
    // Найдем случайные позиции в открытых пространствах (комнатах)
    if (doorsCreated < maxDoors && !potentialDoorLocations.empty()) {
        std::vector<std::pair<int, int>> roomPositions;

        for (int y = 3; y < m_tileMap->getHeight() - 3; y++) {
            for (int x = 3; x < m_tileMap->getWidth() - 3; x++) {
                // Проверяем проходимый тайл с открытым пространством вокруг
                if (m_tileMap->isTileWalkable(x, y)) {
                    // Считаем количество проходимых тайлов вокруг (открытое пространство)
                    int openTiles = 0;

                    for (int dy = -1; dy <= 1; dy++) {
                        for (int dx = -1; dx <= 1; dx++) {
                            if (dx == 0 && dy == 0) continue; // Пропускаем центральный тайл

                            if (m_tileMap->isValidCoordinate(x + dx, y + dy) &&
                                m_tileMap->isTileWalkable(x + dx, y + dy)) {
                                openTiles++;
                            }
                        }
                    }

                    // Если больше 6 из 8 окружающих тайлов проходимы - это внутри комнаты
                    if (openTiles >= 6) {
                        // Также убедимся, что это не слишком близко к игроку
                        if (m_player) {
                            float playerX = m_player->getPosition().x;
                            float playerY = m_player->getPosition().y;

                            // Вычисляем расстояние до игрока
                            float dx = playerX - static_cast<float>(x);
                            float dy = playerY - static_cast<float>(y);
                            float distanceToPlayer = std::sqrt(dx * dx + dy * dy);

                            // Не размещаем двери слишком близко к игроку
                            if (distanceToPlayer > 5.0f) {
                                roomPositions.push_back({ x, y });
                            }
                        }
                        else {
                            roomPositions.push_back({ x, y });
                        }
                    }
                }
            }
        }

        // Перемешиваем позиции в комнатах
        std::random_shuffle(roomPositions.begin(), roomPositions.end());

        // Добавляем двери в комнатах с меньшей вероятностью
        float roomDoorProbability = doorProbability * 0.3f; // 30% от основной вероятности

        // Выбираем случайные позиции из найденных
        for (const auto& pos : roomPositions) {
            if (doorsCreated >= maxDoors) break;

            // Применяем пониженную вероятность для дверей в комнатах
            float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            if (randomValue <= roomDoorProbability) {
                int x = pos.first;
                int y = pos.second;

                // Выбираем случайную ориентацию, поскольку в комнате любая ориентация может подойти
                bool isVertical = (rand() % 2 == 0);

                // Создаем дверь
                std::string doorName = "RoomDoor_" + std::to_string(x) + "_" + std::to_string(y);
                auto door = createTestDoor(static_cast<float>(x), static_cast<float>(y), doorName);

                if (door) {
                    // Устанавливаем ориентацию
                    door->setVertical(isVertical);

                    doorsCreated++;

                    LOG_INFO("Created room door at position (" + std::to_string(x) + ", " +
                        std::to_string(y) + ") with " + (isVertical ? "vertical" : "horizontal") +
                        " orientation for biome " + std::to_string(m_currentBiome));
                }
            }
        }
    }

    LOG_INFO("Generated " + std::to_string(doorsCreated) + " doors out of " +
        std::to_string(potentialDoorLocations.size()) + " potential locations");
}

void WorldGenerator::createInteractiveItems() {
    if (!m_tileMap || !m_player) return;

    // Хранение позиций, которые уже заняты (двери и предметы)
    std::set<std::pair<int, int>> usedPositions;

    // Параметры генерации предметов
    int attempts = 0;
    int maxAttempts = 100;
    int itemsToPlace = 15;
    int itemsPlaced = 0;

    // Минимальные и максимальные значения для расстояния от игрока
    float minDistanceFromPlayer = 5.0f;
    float maxDistanceFromPlayer = 20.0f;

    // Текущая позиция игрока
    float playerX = m_player->getPosition().x;
    float playerY = m_player->getPosition().y;

    // Название биома для специфичных предметов
    std::string biomeName;
    switch (m_currentBiome) {
    case 1: // FOREST
        biomeName = "Forest";
        break;
    case 2: // DESERT
        biomeName = "Desert";
        break;
    case 3: // TUNDRA
        biomeName = "Tundra";
        break;
    case 4: // VOLCANIC
        biomeName = "Volcanic";
        break;
    default:
        biomeName = "Unknown";
        break;
    }

    while (itemsPlaced < itemsToPlace && attempts < maxAttempts) {
        // Выбираем случайную позицию на карте
        int x = std::rand() % m_tileMap->getWidth();
        int y = std::rand() % m_tileMap->getHeight();

        // Проверяем расстояние до игрока, чтобы предметы не были слишком близко или слишком далеко
        float distX = static_cast<float>(x) - playerX;
        float distY = static_cast<float>(y) - playerY;
        float distToPlayer = std::sqrt(distX * distX + distY * distY);

        // Создаем пару для проверки уникальности позиции
        std::pair<int, int> position(x, y);

        // Проверяем, что тайл проходим, находится на подходящем расстоянии от игрока
        // и эта позиция ещё не используется
        if (m_tileMap->isValidCoordinate(x, y) &&
            m_tileMap->isTileWalkable(x, y) &&
            distToPlayer >= minDistanceFromPlayer &&
            distToPlayer <= maxDistanceFromPlayer &&
            usedPositions.find(position) == usedPositions.end()) {

            // Запоминаем использованную позицию
            usedPositions.insert(position);

            // Выбираем случайный тип предмета
            PickupItem::ItemType type = static_cast<PickupItem::ItemType>(std::rand() % 5);

            // Создаем предмет с подробным описательным именем
            std::string itemName;
            SDL_Color itemColor;

            switch (type) {
            case PickupItem::ItemType::RESOURCE:
                // Название ресурса в зависимости от биома
                switch (m_currentBiome) {
                case 1: // FOREST
                    itemName = "Exotic Plant Sample";
                    break;
                case 2: // DESERT
                    itemName = "Rare Mineral Deposit";
                    break;
                case 3: // TUNDRA
                    itemName = "Crystallized Ice Core";
                    break;
                case 4: // VOLCANIC
                    itemName = "Volcanic Crystal";
                    break;
                default:
                    itemName = "Unknown Material";
                    break;
                }
                itemColor = { 100, 200, 255, 255 }; // Ярко-голубой для ресурсов
                break;

            case PickupItem::ItemType::WEAPON:
                // Разные типы оружия
            {
                const std::string weaponTypes[] = {
                    "Pulse Rifle", "Energy Pistol", "Plasma Cannon",
                    "Laser Knife", "Quantum Disruptor"
                };
                int weaponIndex = std::rand() % 5;
                itemName = weaponTypes[weaponIndex];
            }
            itemColor = { 255, 60, 60, 255 }; // Ярко-красный для оружия
            break;

            case PickupItem::ItemType::ARMOR:
                // Разные типы брони
            {
                const std::string armorTypes[] = {
                    "Shield Module", "Energy Barrier", "Armor Plating",
                    "Deflector Array", "Protective Suit"
                };
                int armorIndex = std::rand() % 5;
                itemName = armorTypes[armorIndex];
            }
            itemColor = { 60, 60, 255, 255 }; // Ярко-синий для брони
            break;

            case PickupItem::ItemType::CONSUMABLE:
                // Расходные материалы
            {
                const std::string consumableTypes[] = {
                    "Health Injector", "Energy Cell", "Oxygen Capsule",
                    "Nanite Pack", "Stimulant"
                };
                int consumableIndex = std::rand() % 5;
                itemName = consumableTypes[consumableIndex];
            }
            itemColor = { 60, 255, 60, 255 }; // Ярко-зеленый для расходников
            break;

            case PickupItem::ItemType::KEY:
                // Ключи и важные предметы
            {
                const std::string keyTypes[] = {
                    "Access Card", "Security Key", "Data Crystal",
                    "Command Module", "Encrypted Chip"
                };
                int keyIndex = std::rand() % 5;
                itemName = keyTypes[keyIndex];
            }
            itemColor = { 255, 215, 0, 255 }; // Золотой для ключей
            break;

            default:
                itemName = "Unknown Item";
                itemColor = { 200, 200, 200, 255 }; // Серый для неизвестных предметов
                break;
            }

            // Создаем уникальное имя для предмета
            std::string uniqueItemName = itemName;

            // Создаем предмет и добавляем его на сцену
            auto item = createTestPickupItem(static_cast<float>(x), static_cast<float>(y), uniqueItemName, type);

            if (item) {
                // Устанавливаем цвет
                item->setColor(itemColor);

                // Устанавливаем подсказку
                item->setInteractionHint("Press E to pick up " + uniqueItemName);

                itemsPlaced++;
            }
        }

        attempts++;
    }

    // Добавляем информацию о покрытии карты предметами
    float coverage = static_cast<float>(itemsPlaced) / (m_tileMap->getWidth() * m_tileMap->getHeight()) * 100.0f;
    LOG_INFO("Created " + std::to_string(itemsPlaced) + " random items on the map (coverage: " +
        std::to_string(coverage) + "%)");
}

void WorldGenerator::createTerminals() {
    if (!m_tileMap || !m_player) return;

    // Хранение позиций, которые уже заняты другими объектами
    std::set<std::pair<int, int>> usedPositions;

    // Текущая позиция игрока
    float playerX = m_player->getPosition().x;
    float playerY = m_player->getPosition().y;

    // Определяем тип терминала в зависимости от биома
    Terminal::TerminalType terminalType;
    std::string terminalName;

    switch (m_currentBiome) {
    case 1: // FOREST
        terminalType = Terminal::TerminalType::RESEARCH_SENSOR;
        terminalName = "Research Sensor";
        break;
    case 2: // DESERT
        terminalType = Terminal::TerminalType::ANCIENT_CONSOLE;
        terminalName = "Ancient Console";
        break;
    case 3: // TUNDRA
        terminalType = Terminal::TerminalType::SCIENCE_STATION;
        terminalName = "Science Station";
        break;
    case 4: // VOLCANIC
        terminalType = Terminal::TerminalType::EMERGENCY_BEACON;
        terminalName = "Emergency Beacon";
        break;
    default:
        // Если биом не определен, выбираем случайный тип
        terminalType = static_cast<Terminal::TerminalType>(rand() % 4);
        terminalName = "Unknown Terminal";
        break;
    }

    // Найдем все углы комнат на карте
    std::vector<std::pair<int, int>> roomCorners = findRoomCorners();

    // Если углы не найдены, используем запасной метод с случайным размещением
    if (roomCorners.empty()) {
        LOG_WARNING("No room corners found, using fallback placement method");
        // Здесь можно оставить старую логику поиска случайной позиции
        placeTerminalRandomly(terminalType, terminalName);
        return;
    }

    // Сортируем углы по удаленности от игрока (от дальних к ближним)
    std::sort(roomCorners.begin(), roomCorners.end(),
        [playerX, playerY](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            float distA = std::pow(a.first - playerX, 2) + std::pow(a.second - playerY, 2);
            float distB = std::pow(b.first - playerX, 2) + std::pow(b.second - playerY, 2);
            return distA > distB; // Сортировка от дальних к ближним
        });

    // Ищем подходящий угол, который не занят другими объектами
    bool terminalPlaced = false;

    for (const auto& corner : roomCorners) {
        int x = corner.first;
        int y = corner.second;

        // Проверяем, что позиция не занята
        if (usedPositions.find({ x, y }) == usedPositions.end()) {
            // Проверяем минимальное расстояние от игрока
            float distX = static_cast<float>(x) - playerX;
            float distY = static_cast<float>(y) - playerY;
            float distToPlayer = std::sqrt(distX * distX + distY * distY);

            // Не размещаем терминал слишком близко к игроку
            if (distToPlayer >= 8.0f) {
                // Создаем терминал
                auto terminal = createTestTerminal(static_cast<float>(x), static_cast<float>(y), terminalName, terminalType);

                if (terminal) {
                    // Добавляем контент в зависимости от биома
                    switch (m_currentBiome) {
                    case 1: // FOREST
                        terminal->addEntry("Flora Analysis", "Plant growth rate exceeds natural parameters by 315%. Detected rapid cellular division with unknown catalyst. Species demonstrate enhanced regeneration and resistance to environmental stressors.");
                        terminal->addEntry("Warning", "Presence of unidentified mutagenic compound in soil samples. Exposure may lead to genetic alterations. Field team advised to maintain sealed environment protocols at all times.");
                        terminal->addEntry("Research Notes", "Three specimens collected show signs of rudimentary neural networks forming between separate plants. Recommend immediate containment and priority study of interspecies communication patterns.");
                        break;
                    case 2: // DESERT
                        terminal->addEntry("Mineral Survey", "Discovered lattice structures of crystallized minerals with semiconductor properties. Material exhibits energy absorption and storage capabilities beyond known physics. Potentially revolutionary for power systems.");
                        terminal->addEntry("Climate Analysis", "Nocturnal temperature inversions create localized atmospheric distortions. Thermal imaging reveals geometric cooling patterns inconsistent with natural phenomena. Possible evidence of climate engineering.");
                        terminal->addEntry("Excavation Log", "Uncovered metallic structure at 15m depth. Carbon dating impossible - material rejects all standard dating methods. Geometry suggests artificial origin. Requested specialized equipment for further study.");
                        break;
                    case 3: // TUNDRA
                        terminal->addEntry("Ice Core Analysis", "Ice samples contain microscopic organisms in suspended animation. DNA sequencing reveals no match to Earth taxonomy. Organisms appear viable when subjected to controlled warming under laboratory conditions.");
                        terminal->addEntry("Seismic Monitoring", "Deep-scan detects regular pulsations from beneath permafrost layer. Pattern suggests artificial origin rather than geological processes. Frequency matches no known tectonic activity profile.");
                        terminal->addEntry("Expedition Log", "Team reports auditory anomalies near northern glacier - described as \"whispers\" that intensify at night. Two researchers experienced identical dreams. Recommending psychological evaluation and audio monitoring.");
                        break;
                    case 4: // VOLCANIC
                        terminal->addEntry("Thermal Analysis", "Magma composition contains engineered nanoparticles with thermal regulatory properties. Evidence suggests deliberate temperature control of volcanic activity. Technology far exceeds current capabilities.");
                        terminal->addEntry("Atmospheric Reading", "Gas emissions contain trace elements arranged in mathematical sequences. Analysis confirms non-random distribution impossible in natural formation. Pattern resembles encrypted data transmission.");
                        terminal->addEntry("Security Alert", "Motion sensors detected synchronized movement patterns within lava tubes. Thermal signatures suggest technological origin. Unable to establish visual confirmation due to extreme temperatures.");
                        break;
                    }

                    // Добавляем общую запись для всех терминалов - короткую и важную
                    terminal->addEntry("Encrypted Message", "PRIORITY ALPHA: Project Satellite compromised. Unknown entity has gained access to core systems. Disconnect all terminals. Initiate emergency protocol ECHO-7 immediately.");

                    terminalPlaced = true;
                    LOG_INFO("Terminal placed in room corner at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
                    break;
                }
            }
        }
    }

    // Если не удалось разместить терминал в углу, используем запасной метод
    if (!terminalPlaced) {
        LOG_WARNING("Could not place terminal in any corner, using fallback placement method");
        placeTerminalRandomly(terminalType, terminalName);
    }
}

std::vector<std::pair<int, int>> WorldGenerator::findRoomCorners() {
    std::vector<std::pair<int, int>> corners;

    if (!m_tileMap) return corners;

    // Проходим по всей карте
    for (int y = 1; y < m_tileMap->getHeight() - 1; y++) {
        for (int x = 1; x < m_tileMap->getWidth() - 1; x++) {
            // Проверяем, что текущий тайл проходим (пол)
            if (m_tileMap->isTileWalkable(x, y)) {
                // Проверяем 8 возможных конфигураций угла:
                //  1. Стена сверху и слева
                if (!m_tileMap->isTileWalkable(x, y - 1) && !m_tileMap->isTileWalkable(x - 1, y)) {
                    corners.push_back({ x, y });
                    continue;
                }

                //  2. Стена сверху и справа
                if (!m_tileMap->isTileWalkable(x, y - 1) && !m_tileMap->isTileWalkable(x + 1, y)) {
                    corners.push_back({ x, y });
                    continue;
                }

                //  3. Стена снизу и слева
                if (!m_tileMap->isTileWalkable(x, y + 1) && !m_tileMap->isTileWalkable(x - 1, y)) {
                    corners.push_back({ x, y });
                    continue;
                }

                //  4. Стена снизу и справа
                if (!m_tileMap->isTileWalkable(x, y + 1) && !m_tileMap->isTileWalkable(x + 1, y)) {
                    corners.push_back({ x, y });
                    continue;
                }
            }
        }
    }

    // Фильтрация углов: предпочитаем те, что в больших открытых пространствах
    std::vector<std::pair<int, int>> goodCorners;

    for (const auto& corner : corners) {
        int x = corner.first;
        int y = corner.second;

        // Считаем количество проходимых тайлов вокруг (в радиусе 2)
        int openSpaceCount = 0;

        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                if (dx == 0 && dy == 0) continue; // Пропускаем центральный тайл

                if (m_tileMap->isValidCoordinate(x + dx, y + dy) &&
                    m_tileMap->isTileWalkable(x + dx, y + dy)) {
                    openSpaceCount++;
                }
            }
        }

        // Если вокруг угла достаточно открытого пространства, считаем его хорошим
        if (openSpaceCount >= 10) { // Пороговое значение можно регулировать
            goodCorners.push_back(corner);
        }
    }

    // Если нашли хорошие углы, используем их, иначе возвращаем все углы
    return goodCorners.empty() ? corners : goodCorners;
}

void WorldGenerator::placeTerminalRandomly(Terminal::TerminalType terminalType, const std::string& terminalName) {
    // Параметры поиска
    int attempts = 0;
    int maxAttempts = 100;
    bool terminalPlaced = false;

    // Минимальное и максимальное расстояние от игрока
    float minDistanceFromPlayer = 8.0f;
    float maxDistanceFromPlayer = 20.0f;

    // Текущая позиция игрока
    float playerX = m_player->getPosition().x;
    float playerY = m_player->getPosition().y;

    // Получаем список уже занятых позиций
    std::set<std::pair<int, int>> usedPositions;

    while (!terminalPlaced && attempts < maxAttempts) {
        // Выбираем случайную позицию на карте
        int x = std::rand() % m_tileMap->getWidth();
        int y = std::rand() % m_tileMap->getHeight();

        // Проверяем расстояние до игрока
        float distX = static_cast<float>(x) - playerX;
        float distY = static_cast<float>(y) - playerY;
        float distToPlayer = std::sqrt(distX * distX + distY * distY);

        // Создаем пару для проверки уникальности позиции
        std::pair<int, int> position(x, y);

        // Проверяем, что тайл проходим, находится на подходящем расстоянии от игрока
        // и эта позиция ещё не используется
        if (m_tileMap->isValidCoordinate(x, y) &&
            m_tileMap->isTileWalkable(x, y) &&
            distToPlayer >= minDistanceFromPlayer &&
            distToPlayer <= maxDistanceFromPlayer &&
            usedPositions.find(position) == usedPositions.end()) {

            // Создаем терминал
            auto terminal = createTestTerminal(static_cast<float>(x), static_cast<float>(y), terminalName, terminalType);

            if (terminal) {
                // Добавляем содержимое в зависимости от биома
                switch (m_currentBiome) {
                case 1: // FOREST
                    terminal->addEntry("Flora Analysis", "Dense vegetation indicates unusually accelerated growth cycles. Oxygen levels 28% above baseline.");
                    terminal->addEntry("Warning", "Detected trace elements of unknown mutagenic compound in soil samples.");
                    break;
                case 2: // DESERT
                    terminal->addEntry("Mineral Survey", "High concentration of rare-earth elements detected in subsurface layers.");
                    terminal->addEntry("Climate Data", "Nocturnal temperature variations exceed expected parameters by 200%.");
                    break;
                case 3: // TUNDRA
                    terminal->addEntry("Ice Core Sample", "Crystalline structures contain unknown bacterial microorganisms in suspended animation.");
                    terminal->addEntry("Seismic Activity", "Regular harmonic patterns detected in deep-layer permafrost.");
                    break;
                case 4: // VOLCANIC
                    terminal->addEntry("Thermal Analysis", "Magma composition indicates artificial manipulation of geological processes.");
                    terminal->addEntry("Atmospheric Alert", "Toxic gas emissions follow predictable patterns suggesting controlled release.");
                    break;
                }

                // Добавляем общую запись
                terminal->addEntry("Encrypted Message", "Signal detected from coordinates [REDACTED]. Message: 'Project Satellite compromised. Evacuate immediately.'");

                terminalPlaced = true;
                LOG_INFO("Terminal placed randomly at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
                break;
            }
        }

        attempts++;
    }

    if (!terminalPlaced) {
        LOG_WARNING("Failed to place terminal after " + std::to_string(attempts) + " attempts");
    }
}

std::shared_ptr<PickupItem> WorldGenerator::createTestPickupItem(float x, float y,
    const std::string& name,
    PickupItem::ItemType type) {
    // Создаем новый предмет для подбора
    auto item = std::make_shared<PickupItem>(name, type);

    // Устанавливаем позицию
    item->setPosition(x, y, 0.2f); // Небольшая высота над землей

    // Увеличим радиус взаимодействия для улучшения доступности
    item->setInteractionRadius(1.8f);

    // Инициализируем предмет
    if (!item->initialize()) {
        LOG_ERROR("Failed to initialize pickup item: " + name);
        return nullptr;
    }

    // Добавляем предмет на сцену через MapScene
    m_mapScene->addInteractiveObject(item);

    LOG_INFO("Created pickup item: " + name + " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    return item;
}

std::shared_ptr<Door> WorldGenerator::createTestDoor(float x, float y, const std::string& name) {
    // Создаем новую дверь с передачей указателя на текущую сцену и текущего биома
    auto door = std::make_shared<Door>(name, m_tileMap.get(), m_mapScene, m_currentBiome);

    // Устанавливаем позицию
    door->setPosition(x, y, 0.3f);

    // Устанавливаем время взаимодействия (2-3 секунды)
    door->setInteractionTime(2.5f);

    // Инициализируем дверь
    if (!door->initialize()) {
        LOG_ERROR("Failed to initialize door: " + name);
        return nullptr;
    }

    // Добавляем дверь на сцену
    m_mapScene->addInteractiveObject(door);

    return door;
}

std::shared_ptr<Terminal> WorldGenerator::createTestTerminal(float x, float y,
    const std::string& name,
    Terminal::TerminalType type) {
    // Создаем новый терминал указанного типа
    auto terminal = std::make_shared<Terminal>(name, type);

    // Устанавливаем позицию с высотой 1.0f (вместо 0.3f)
    terminal->setPosition(x, y, 1.0f); // Увеличиваем высоту для лучшей видимости

    // Инициализируем терминал
    if (!terminal->initialize()) {
        LOG_ERROR("Failed to initialize terminal: " + name);
        return nullptr;
    }

    // Добавляем терминал на сцену
    m_mapScene->addInteractiveObject(terminal);

    LOG_INFO("Created terminal: " + name + " of type " +
        std::to_string(static_cast<int>(type)) +
        " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    return terminal;
}

void WorldGenerator::createSwitches() {
    if (!m_tileMap || !m_player) return;

    // Хранение позиций, которые уже заняты другими объектами
    std::set<std::pair<int, int>> usedPositions;

    // Получаем список всех интерактивных объектов для проверки занятых позиций
    if (m_mapScene) {
        const auto& interactiveObjects = m_mapScene->getEntityManager()->getInteractiveObjects();
        for (const auto& obj : interactiveObjects) {
            int x = static_cast<int>(obj->getPosition().x);
            int y = static_cast<int>(obj->getPosition().y);
            usedPositions.insert({ x, y });
        }
    }

    // Текущая позиция игрока
    float playerX = m_player->getPosition().x;
    float playerY = m_player->getPosition().y;

    // Параметры генерации переключателей
    int attempts = 0;
    int maxAttempts = 100;

    // Используем фактическое количество сгенерированных комнат
    int roomCount = m_generatedRoomCount;

    // Если по какой-то причине число комнат не определено, используем минимальное значение
    if (roomCount <= 0) {
        roomCount = 5; // Предполагаем минимум 5 комнат по умолчанию
        LOG_WARNING("Room count not available, using default: " + std::to_string(roomCount));
    }

    // Распределяем по формуле: 1 переключатель на каждые 5 комнат (целочисленное деление)
    int switchesToPlace = roomCount / 5;

    // Гарантируем минимум 1 переключатель
    switchesToPlace = std::max(1, switchesToPlace);

    LOG_INFO("Generating " + std::to_string(switchesToPlace) +
        " switches for " + std::to_string(roomCount) + " rooms");

    // Минимальное и максимальное расстояние от игрока
    float minDistanceFromPlayer = 8.0f;  // Не слишком близко к старту
    float maxDistanceFromPlayer = 20.0f; // Не слишком далеко

    // Счетчик размещенных переключателей
    int switchesPlaced = 0;

    while (switchesPlaced < switchesToPlace && attempts < maxAttempts) {
        // Существующий код размещения переключателей...
        // Выбираем случайную позицию на карте
        int x = std::rand() % m_tileMap->getWidth();
        int y = std::rand() % m_tileMap->getHeight();

        // Проверяем расстояние до игрока
        float distX = static_cast<float>(x) - playerX;
        float distY = static_cast<float>(y) - playerY;
        float distToPlayer = std::sqrt(distX * distX + distY * distY);

        // Создаем пару для проверки уникальности позиции
        std::pair<int, int> position(x, y);

        // Проверяем, что тайл проходим, находится на подходящем расстоянии от игрока
        // и эта позиция ещё не используется
        if (m_tileMap->isValidCoordinate(x, y) &&
            m_tileMap->isTileWalkable(x, y) &&
            distToPlayer >= minDistanceFromPlayer &&
            distToPlayer <= maxDistanceFromPlayer &&
            usedPositions.find(position) == usedPositions.end()) {

            // Запоминаем использованную позицию
            usedPositions.insert(position);

            // Выбираем тип переключателя в зависимости от биома
            Switch::SwitchType switchType;

            // Определяем вероятности для разных типов переключателей в зависимости от биома
            int biomeBasedRand = rand() % 100;

            switch (m_currentBiome) {
            case 1: // FOREST
                // В лесу больше гравитационных аномалий и стабилизаторов
                if (biomeBasedRand < 40) {
                    switchType = Switch::SwitchType::GRAVITY_ANOMALY;
                }
                else if (biomeBasedRand < 80) {
                    switchType = Switch::SwitchType::RESONANCE_STABILIZER;
                }
                else {
                    switchType = Switch::SwitchType::ENERGY_NODE;
                }
                break;

            case 2: // DESERT
                // В пустыне больше древних руин с телепортами и энергетическими узлами
                if (biomeBasedRand < 40) {
                    switchType = Switch::SwitchType::TELEPORT_GATE;
                }
                else if (biomeBasedRand < 70) {
                    switchType = Switch::SwitchType::ENERGY_NODE;
                }
                else if (biomeBasedRand < 90) {
                    switchType = Switch::SwitchType::SECURITY_SYSTEM;
                }
                else {
                    switchType = Switch::SwitchType::GRAVITY_ANOMALY;
                }
                break;

            case 3: // TUNDRA
                // В тундре много аномалий и стабилизаторов
                if (biomeBasedRand < 50) {
                    switchType = Switch::SwitchType::RESONANCE_STABILIZER;
                }
                else if (biomeBasedRand < 80) {
                    switchType = Switch::SwitchType::GRAVITY_ANOMALY;
                }
                else {
                    switchType = Switch::SwitchType::TELEPORT_GATE;
                }
                break;

            case 4: // VOLCANIC
                // В вулканических регионах много энергетических узлов и систем безопасности
                if (biomeBasedRand < 40) {
                    switchType = Switch::SwitchType::ENERGY_NODE;
                }
                else if (biomeBasedRand < 70) {
                    switchType = Switch::SwitchType::SECURITY_SYSTEM;
                }
                else if (biomeBasedRand < 90) {
                    switchType = Switch::SwitchType::GRAVITY_ANOMALY;
                }
                else {
                    switchType = Switch::SwitchType::RESONANCE_STABILIZER;
                }
                break;

            default:
                // По умолчанию - случайный тип
                switchType = static_cast<Switch::SwitchType>(rand() % 5);
                break;
            }

            // Создаем имя для переключателя
            std::string switchName;

            if (switchType == Switch::SwitchType::GRAVITY_ANOMALY) {
                switchName = "Gravity Anomaly";
            }
            else if (switchType == Switch::SwitchType::TELEPORT_GATE) {
                switchName = "Ancient Teleport";
                LOG_INFO("Creating teleport switch at position (" + std::to_string(x) +
                    ", " + std::to_string(y) + ")");
            }
            else if (switchType == Switch::SwitchType::RESONANCE_STABILIZER) {
                switchName = "Resonance Stabilizer";
            }
            else if (switchType == Switch::SwitchType::SECURITY_SYSTEM) {
                switchName = "Security Control";
            }
            else if (switchType == Switch::SwitchType::ENERGY_NODE) {
                switchName = "Energy Node";
            }
            else {
                switchName = "Unknown Switch";
            }

            // Создаем переключатель
            auto switchObj = createTestSwitch(static_cast<float>(x), static_cast<float>(y),
                switchName + "_" + std::to_string(switchesPlaced),
                switchType);

            if (switchObj) {
                switchesPlaced++;
                LOG_INFO("Placed " + switchName + " at position (" +
                    std::to_string(x) + ", " + std::to_string(y) + ")");

                // Дополнительная информация для телепорта
                if (switchName.find("Ancient Teleport") != std::string::npos) {
                    LOG_INFO("Teleport created with mapScene=" +
                        std::string(m_mapScene ? "valid" : "nullptr") +
                        ", tileMap=" +
                        std::string(m_tileMap ? "valid" : "nullptr"));
                }
            }
        }

        attempts++;
    }

    LOG_INFO("Placed " + std::to_string(switchesPlaced) + " switches after " +
        std::to_string(attempts) + " attempts");
}

std::shared_ptr<Switch> WorldGenerator::createTestSwitch(float x, float y,
    const std::string& name,
    Switch::SwitchType type) {

    // Создаем новый переключатель
    auto switchObj = std::make_shared<Switch>(name, type, m_tileMap.get(), m_mapScene);

    // Устанавливаем позицию
    switchObj->setPosition(x, y, 0.5f); // Высота над землей для видимости

    // Инициализируем переключатель
    if (!switchObj->initialize()) {
        LOG_ERROR("Failed to initialize switch: " + name);
        return nullptr;
    }

    // Устанавливаем обратный вызов для обработки эффектов активации
    switchObj->setActivationCallback([this, type](Player* player, Switch* switchObj) {
        // Здесь добавляем специальную логику для разных типов переключателей
        LOG_INFO("Activation callback triggered for " + std::string(switchObj ? switchObj->getName() : "unknown switch"));

        // Используем if-else вместо switch для разных типов
        if (type == Switch::SwitchType::TELEPORT_GATE) {
            // Для телепортационных врат - телепортация игрока
            if (switchObj && m_player) {
                LOG_INFO("Processing teleport in callback...");

                // Проверяем, установлены ли координаты телепортации
                if (switchObj->hasTeleportDestination()) {
                    // Получаем координаты назначения
                    int destX = switchObj->getTeleportDestX();
                    int destY = switchObj->getTeleportDestY();

                    LOG_INFO("Teleport destination found: (" + std::to_string(destX) + ", " + std::to_string(destY) + ")");

                    if (m_tileMap->isValidCoordinate(destX, destY)) {
                        // Телепортируем игрока на целевую позицию
                        LOG_INFO("Teleporting player from (" +
                            std::to_string(m_player->getPosition().x) + ", " +
                            std::to_string(m_player->getPosition().y) + ") to (" +
                            std::to_string(destX) + ", " + std::to_string(destY) + ")");

                        m_player->setPosition(static_cast<float>(destX),
                            static_cast<float>(destY),
                            0.0f);

                        // Добавляем немного случайности к суб-позиции внутри тайла
                        m_player->setSubX(0.5f);
                        m_player->setSubY(0.5f);
                    }
                    else {
                        LOG_WARNING("Invalid teleport destination coordinates!");
                    }
                }
                else {
                    LOG_WARNING("No teleport destination set for " + switchObj->getName());
                }
            }
            else {
                LOG_WARNING("Invalid player or switch pointer in teleport callback");
            }
        }
        else if (type == Switch::SwitchType::SECURITY_SYSTEM) {
            // Для систем безопасности - деактивация ловушек или открытие дверей
            LOG_INFO("Security system deactivated by " +
                std::string(player ? "player" : "unknown trigger"));

            // Здесь можно добавить код для манипуляции дверями или другими объектами
        }
        else {
            LOG_INFO("Switch activation callback: " + switchObj->getName());
        }
        });

    // Добавляем переключатель на сцену
    m_mapScene->addInteractiveObject(switchObj);

    LOG_INFO("Created switch: " + name + " at position (" +
        std::to_string(x) + ", " + std::to_string(y) + ")");
    return switchObj;
}


std::pair<int, int> WorldGenerator::generateTexturedTestMap(int biomeType) {
    LOG_INFO("Generating textured test map for biome: " + std::to_string(biomeType));

    // Сначала генерируем стандартную карту
    auto playerPos = generateTestMap(biomeType);

    // ВРЕМЕННОЕ РЕШЕНИЕ: Пропускаем применение текстур для изоляции проблемы
    LOG_INFO("DIAGNOSTIC: Skipping texture application for testing");

    // При необходимости, загружаем текстуры, но не применяем их
    if (m_engine && m_engine->getResourceManager()) {
        auto resourceManager = m_engine->getResourceManager();
        if (resourceManager) {
            resourceManager.get()->loadTileTextures();

            // Минимальный тест: проверяем получение текстуры без её применения
            SDL_Texture* testTexture = resourceManager.get()->getTileTexture(TileType::FLOOR, biomeType);
            if (testTexture) {
                LOG_INFO("DIAGNOSTIC: Successfully retrieved test texture");
            }
            else {
                LOG_ERROR("DIAGNOSTIC: Failed to retrieve test texture");
            }
        }
    }

    return playerPos;
}

void WorldGenerator::applyTilesToMap(int biomeType) {
    LOG_INFO("DIAGNOSTIC: Called applyTilesToMap but using minimal version");

    // Проверка наличия необходимых компонентов
    if (!m_tileMap) {
        LOG_ERROR("TileMap is nullptr");
        return;
    }

    if (!m_engine) {
        LOG_ERROR("Engine is nullptr");
        return;
    }

    if (!m_parentScene) {
        LOG_ERROR("ParentScene is nullptr");
        return;
    }

    RenderingSystem* renderingSystem = m_parentScene->getRenderingSystem();
    if (!renderingSystem) {
        LOG_ERROR("RenderingSystem is nullptr");
        return;
    }

    TileRenderer* tileRenderer = renderingSystem->getTileRenderer();
    if (!tileRenderer) {
        LOG_ERROR("TileRenderer is nullptr");
        return;
    }

    // Очищаем тайлы, но не добавляем новые
    tileRenderer->clear();

    LOG_INFO("DIAGNOSTIC: Minimal applyTilesToMap completed successfully");
}


