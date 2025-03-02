#include "MapScene.h"
#include "Engine.h"
#include "ResourceManager.h"
#include <iostream>
#include <cmath>
#include "RoomGenerator.h"
#include "Logger.h"
#include "Player.h" // Добавляем включение Player.h

MapScene::MapScene(const std::string& name, Engine* engine)
    : Scene(name), m_engine(engine), m_showDebug(false), m_currentBiome(0) {
}

MapScene::~MapScene() {
}

bool MapScene::initialize() {
    // 1. Инициализация изометрического рендерера
    m_isoRenderer = std::make_shared<IsometricRenderer>(64, 32);

    // 2. Инициализация камеры
    m_camera = std::make_shared<Camera>(800, 600);

    // 3. Создание карты размером 50x50 тайлов
    m_tileMap = std::make_shared<TileMap>(50, 50);
    if (!m_tileMap->initialize()) {
        std::cerr << "Failed to initialize tile map" << std::endl;
        return false;
    }

    // 4. Инициализация системы коллизий
    m_collisionSystem = std::make_shared<CollisionSystem>(m_tileMap);

    // 5. Создаем и инициализируем игрока
    m_player = std::make_shared<Player>("Player1", m_tileMap, m_collisionSystem);
    m_player->setPosition(25.0f, 25.0f, 0.5f); // Z=0.5f для высоты
    m_player->setPlayerPosition(25.0f, 25.0f, 0.5f, 0.5f);

    if (!m_player->initialize()) {
        std::cerr << "Failed to initialize player" << std::endl;
        return false;
    }

    // Устанавливаем указатель игрока для камеры
    m_camera->setTarget(&m_player->getPosition().x, &m_player->getPosition().y);

    // Добавляем игрока в список сущностей сцены
    addEntity(m_player);

    // 6. Генерация тестовой карты
    generateTestMap();

    // 7. Инициализация рендерера тайлов
    m_tileRenderer = std::make_shared<TileRenderer>(m_isoRenderer.get());

    LOG_INFO("MapScene initialized successfully");
    return true;
}

void MapScene::handleEvent(const SDL_Event& event) {
    // Обработка событий камеры
    m_camera->handleEvent(event);

    // Обработка клавиатурных событий
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_r:
            // Сброс позиции персонажа
            if (m_player) {
                m_player->setPlayerPosition(25.0f, 25.0f, 0.5f, 0.5f);
            }
            break;

        case SDLK_g:
            // Генерация новой карты
            generateTestMap();
            break;

        case SDLK_F1:
            // Переключение режима отладки
            m_showDebug = !m_showDebug;
            break;
        }
    }

    // Обработка событий игрока
    if (m_player) {
        m_player->handleEvent(event);
    }

    // Передаем события в базовый класс
    Scene::handleEvent(event);
}

void MapScene::update(float deltaTime) {
    // 1. Обновление игрока
    if (m_player) {
        m_player->update(deltaTime);
    }

    // 2. Привязка камеры к игроку
    if (m_player && m_camera) {
        m_camera->setPosition(
            m_player->getPlayerX() + m_player->getPlayerSubX(),
            m_player->getPlayerY() + m_player->getPlayerSubY()
        );
    }

    // 3. Обновление камеры и базового класса
    m_camera->update(deltaTime);
    Scene::update(deltaTime);
}

void MapScene::render(SDL_Renderer* renderer) {
    // Очищаем экран
    SDL_SetRenderDrawColor(renderer, 20, 35, 20, 255);
    SDL_RenderClear(renderer);

    // Получаем размер окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    if (m_player) {
        // Привязываем камеру к персонажу
        m_camera->setPosition(
            m_player->getPlayerX() + m_player->getPlayerSubX(),
            m_player->getPlayerY() + m_player->getPlayerSubY()
        );
    }

    m_camera->setZoom(1.0f);

    // Настраиваем рендерер с учетом камеры
    m_isoRenderer->setCameraPosition(m_camera->getX(), m_camera->getY());
    m_isoRenderer->setCameraZoom(m_camera->getZoom());

    // Используем блочную сортировку для отрисовки тайлов и игрока
    renderWithBlockSorting(renderer, centerX, centerY);

    // Добавляем индикатор игрока, чтобы его можно было видеть за стенами
    renderPlayerIndicator(renderer, centerX, centerY);

    // Отображаем отладочную информацию, если включен режим отладки
    if (m_showDebug) {
        renderDebug(renderer, centerX, centerY);
    }
}

void MapScene::generateTestMap() {
    // Очищаем карту
    m_tileMap->clear();

    // Создаем генератор комнат с случайным сидом
    static RoomGenerator roomGen(static_cast<unsigned int>(std::time(nullptr)));


    // Устанавливаем размеры комнат в зависимости от размера карты
    int minSize = std::max(5, m_tileMap->getWidth() / 10);
    int maxSize = std::max(minSize + 5, m_tileMap->getWidth() / 5);
    roomGen.setRoomSizeLimits(minSize, maxSize);
    roomGen.setRoomCountLimits(5, 10); // Минимум 5, максимум 10 комнат

    // Выбираем случайный тип биома для генерации
    srand(static_cast<unsigned int>(time(nullptr)));
    int biomeIndex = rand() % 4 + 1; // 1-4 (FOREST, DESERT, TUNDRA, VOLCANIC)
    RoomGenerator::BiomeType biomeType = static_cast<RoomGenerator::BiomeType>(biomeIndex);
    m_currentBiome = biomeIndex; // Запоминаем текущий биом

    // Устанавливаем биом и в движке для фона
    if (m_engine) {
        m_engine->setCurrentBiome(biomeIndex);
    }
    std::string biomeName;
    switch (biomeType) {
    case RoomGenerator::BiomeType::FOREST:
        biomeName = "Forest";
        break;
    case RoomGenerator::BiomeType::DESERT:
        biomeName = "Desert";
        break;
    case RoomGenerator::BiomeType::TUNDRA:
        biomeName = "Tundra";
        break;
    case RoomGenerator::BiomeType::VOLCANIC:
        biomeName = "Volcanic";
        break;
    default:
        biomeName = "Default";
        break;
    }
    LOG_INFO("Selected biome: " + biomeName + " (type " + std::to_string(biomeIndex) + ")");

    // Генерируем карту
    roomGen.generateMap(m_tileMap.get(), biomeType);

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

    if (m_tileMap->isValidCoordinate(centerTileX, centerTileY) &&
        m_tileMap->isTileWalkable(centerTileX, centerTileY)) {
        // Центральный тайл проходим, используем его
        if (m_player) {
            m_player->setPlayerPosition(centerX, centerY, 0.5f, 0.5f);
        }
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
                        if (m_player) {
                            m_player->setPlayerPosition(static_cast<float>(x), static_cast<float>(y), 0.5f, 0.5f);
                        }
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
        if (m_player) {
            m_player->setPlayerPosition(centerX, centerY, 0.5f, 0.5f);
        }
        LOG_WARNING("Could not find walkable tile for player spawn, using center position");
    }

    LOG_INFO("Generated test map with biome type: " + std::to_string(static_cast<int>(biomeType)));
}

void MapScene::renderDebug(SDL_Renderer* renderer, int centerX, int centerY) {
    if (!m_player) return;

    // Получаем координаты персонажа
    float playerFullX = m_player->getPlayerX() + m_player->getPlayerSubX();
    float playerFullY = m_player->getPlayerY() + m_player->getPlayerSubY();
    float collisionSize = m_player->getCollisionSize();

    // 1. Отображение коллизионного прямоугольника персонажа
    int screenX[4], screenY[4];

    // Получаем экранные координаты для каждого угла коллизионной области
    // Верхний левый угол
    m_isoRenderer->worldToDisplay(
        playerFullX - collisionSize,
        playerFullY - collisionSize,
        0.0f, centerX, centerY, screenX[0], screenY[0]
    );

    // Верхний правый угол
    m_isoRenderer->worldToDisplay(
        playerFullX + collisionSize,
        playerFullY - collisionSize,
        0.0f, centerX, centerY, screenX[1], screenY[1]
    );

    // Нижний правый угол
    m_isoRenderer->worldToDisplay(
        playerFullX + collisionSize,
        playerFullY + collisionSize,
        0.0f, centerX, centerY, screenX[2], screenY[2]
    );

    // Нижний левый угол
    m_isoRenderer->worldToDisplay(
        playerFullX - collisionSize,
        playerFullY + collisionSize,
        0.0f, centerX, centerY, screenX[3], screenY[3]
    );

    // Рисуем коллизионную область
    SDL_Point collisionPoints[5] = {
        {screenX[0], screenY[0]},
        {screenX[1], screenY[1]},
        {screenX[2], screenY[2]},
        {screenX[3], screenY[3]},
        {screenX[0], screenY[0]} // Замыкаем контур
    };

    // Рисуем жёлтую коллизионную рамку
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderDrawLines(renderer, collisionPoints, 5);

    // 2. Отображение границ текущего тайла
    int currentTileX = static_cast<int>(playerFullX);
    int currentTileY = static_cast<int>(playerFullY);

    int tileScreenX[4], tileScreenY[4];

    // Верхний левый угол тайла
    m_isoRenderer->worldToDisplay(
        currentTileX, currentTileY,
        0.0f, centerX, centerY, tileScreenX[0], tileScreenY[0]
    );

    // Верхний правый угол тайла
    m_isoRenderer->worldToDisplay(
        currentTileX + 1.0f, currentTileY,
        0.0f, centerX, centerY, tileScreenX[1], tileScreenY[1]
    );

    // Нижний правый угол тайла
    m_isoRenderer->worldToDisplay(
        currentTileX + 1.0f, currentTileY + 1.0f,
        0.0f, centerX, centerY, tileScreenX[2], tileScreenY[2]
    );

    // Нижний левый угол тайла
    m_isoRenderer->worldToDisplay(
        currentTileX, currentTileY + 1.0f,
        0.0f, centerX, centerY, tileScreenX[3], tileScreenY[3]
    );

    // Рисуем границы текущего тайла
    SDL_Point tilePoints[5] = {
        {tileScreenX[0], tileScreenY[0]},
        {tileScreenX[1], tileScreenY[1]},
        {tileScreenX[2], tileScreenY[2]},
        {tileScreenX[3], tileScreenY[3]},
        {tileScreenX[0], tileScreenY[0]} // Замыкаем контур
    };

    // Используем бирюзовый цвет для текущего тайла
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderDrawLines(renderer, tilePoints, 5);

    // 3. Отображение окрестных тайлов и информации о проходимости
    int neighborOffsets[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},  // Верхний ряд тайлов
        {-1, 0},            {1, 0},   // Средний ряд (без центра)
        {-1, 1},  {0, 1},  {1, 1}     // Нижний ряд тайлов
    };

    for (int i = 0; i < 8; i++) {
        int neighborX = currentTileX + neighborOffsets[i][0];
        int neighborY = currentTileY + neighborOffsets[i][1];

        if (m_tileMap->isValidCoordinate(neighborX, neighborY)) {
            bool isWalkable = m_tileMap->isTileWalkable(neighborX, neighborY);

            // Получаем экранные координаты углов соседнего тайла
            int neighborScreenX[4], neighborScreenY[4];

            // Верхний левый угол
            m_isoRenderer->worldToDisplay(
                neighborX, neighborY,
                0.0f, centerX, centerY, neighborScreenX[0], neighborScreenY[0]
            );

            // Верхний правый угол
            m_isoRenderer->worldToDisplay(
                neighborX + 1.0f, neighborY,
                0.0f, centerX, centerY, neighborScreenX[1], neighborScreenY[1]
            );

            // Нижний правый угол
            m_isoRenderer->worldToDisplay(
                neighborX + 1.0f, neighborY + 1.0f,
                0.0f, centerX, centerY, neighborScreenX[2], neighborScreenY[2]
            );

            // Нижний левый угол
            m_isoRenderer->worldToDisplay(
                neighborX, neighborY + 1.0f,
                0.0f, centerX, centerY, neighborScreenX[3], neighborScreenY[3]
            );

            // Создаем массив точек для соседнего тайла
            SDL_Point neighborPoints[5] = {
                {neighborScreenX[0], neighborScreenY[0]},
                {neighborScreenX[1], neighborScreenY[1]},
                {neighborScreenX[2], neighborScreenY[2]},
                {neighborScreenX[3], neighborScreenY[3]},
                {neighborScreenX[0], neighborScreenY[0]}
            };

            // Цвет зависит от проходимости тайла
            if (isWalkable) {
                // Зеленый для проходимых тайлов
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
            }
            else {
                // Красный для непроходимых тайлов
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
            }

            // Рисуем границы соседнего тайла
            SDL_RenderDrawLines(renderer, neighborPoints, 5);
        }
    }

    // 4. Отображение информации о текущем состоянии игрока
    // Положение текста с информацией о состоянии
    int textX = 20;
    int textY = 20;

    // Получаем имя текущего состояния
    std::string stateName = m_player->getCurrentStateName();

    // Создаем строку с направлением движения
    std::string directionStr = "Dir: (" +
        std::to_string(m_player->getMoveDirectionX()) + ", " +
        std::to_string(m_player->getMoveDirectionY()) + ")";

    // Создаем цвет для текста в зависимости от состояния
    SDL_Color textColor;
    if (stateName == "Idle") {
        textColor = { 0, 255, 0, 255 }; // Зеленый для покоя
    }
    else if (stateName == "Moving") {
        textColor = { 255, 165, 0, 255 }; // Оранжевый для движения
    }
    else if (stateName == "Action") {
        textColor = { 255, 0, 0, 255 }; // Красный для действия
    }
    else {
        textColor = { 255, 255, 255, 255 }; // Белый для других состояний
    }

    // Рисуем фон для текста
    SDL_Rect textBg = { textX - 5, textY - 5, 210, 60 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &textBg);
    SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, 255);
    SDL_RenderDrawRect(renderer, &textBg);

    // В реальном проекте здесь будет использоваться SDL_ttf для отрисовки текста
    // Но пока обойдемся без этого, поскольку мы не настроили работу с шрифтами

    // Отрисовка положения игрока (в реальности используйте SDL_ttf)
    std::string posStr = "Pos: (" +
        std::to_string(static_cast<int>(playerFullX)) + "." +
        std::to_string(static_cast<int>(m_player->getPlayerSubX() * 100)) + ", " +
        std::to_string(static_cast<int>(playerFullY)) + "." +
        std::to_string(static_cast<int>(m_player->getPlayerSubY() * 100)) + ")";

    // В реальном проекте здесь будет код для отрисовки текста с состоянием и позицией
    renderCollisionDebug(renderer, centerX, centerY);
}

void MapScene::renderWithBlockSorting(SDL_Renderer* renderer, int centerX, int centerY) {
    if (!m_player) return;

    // 1. Очистка рендерера перед отрисовкой
    m_tileRenderer->clear();

    // 2. Получение координат игрока и определение текущего блока
    float playerFullX = m_player->getPlayerX() + m_player->getPlayerSubX();
    float playerFullY = m_player->getPlayerY() + m_player->getPlayerSubY();

    // Определяем блок, в котором находится игрок (2x2 тайла)
    int blockColStart = static_cast<int>(playerFullX);
    int blockRowStart = static_cast<int>(playerFullY);

    // 3. Определение радиуса рендеринга вокруг игрока
    int renderRadius = 30;

    // 4. Вычисление границ видимой области
    int startX = std::max(0, blockColStart - renderRadius);
    int startY = std::max(0, blockRowStart - renderRadius);
    int endX = std::min(m_tileMap->getWidth() - 1, blockColStart + renderRadius);
    int endY = std::min(m_tileMap->getHeight() - 1, blockRowStart + renderRadius);

    // 5. ЭТАП 1: ОТРИСОВКА ВСЕХ ПОЛОВ (ПЛОСКИХ ТАЙЛОВ) ПЕРЕД ВСЕМИ ОБЪЕКТАМИ
    int depth = 1; // Начальный приоритет

    // Отрисовываем все полы в видимой области
    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::EMPTY && tile->getHeight() <= 0.0f) {
                // Это плоский тайл (пол)
                SDL_Color color = tile->getColor();
                m_tileRenderer->addFlatTile(
                    static_cast<float>(x), static_cast<float>(y),
                    nullptr,
                    color,
                    depth++
                );
            }
        }
    }

    // 6. ЭТАП 2: ОПРЕДЕЛЕНИЕ ОБЩЕГО ПОРЯДКА ОТРИСОВКИ ОБЪЕКТОВ

    // 6.1. Отрисовка объектов и персонажа с учетом блочного порядка
    // Сначала блоки перед блоком игрока
    for (int y = startY; y < blockRowStart; y++) {
        for (int x = startX; x <= endX; x++) {
            addVolumetricTileToRenderer(x, y, depth++);
        }
    }

    // 6.2. Отрисовка объектов в том же ряду, но перед блоком игрока
    for (int y = blockRowStart; y < blockRowStart + 2; y++) {
        for (int x = startX; x < blockColStart; x++) {
            addVolumetricTileToRenderer(x, y, depth++);
        }
    }

    // 6.3. Отрисовка блока игрока (2x2 тайла)
    for (int y = blockRowStart; y < blockRowStart + 2; y++) {
        for (int x = blockColStart; x < blockColStart + 2; x++) {
            // Проверяем, совпадает ли тайл с позицией игрока
            bool isPlayerTile = (x == static_cast<int>(playerFullX) &&
                y == static_cast<int>(playerFullY));

            // Если это тайл с игроком, то рисуем его
            if (isPlayerTile) {
                renderPlayer(renderer, centerX, centerY, depth++);
            }

            // Затем добавляем стены или другие объемные объекты в этом тайле
            addVolumetricTileToRenderer(x, y, depth++);
        }
    }

    // 6.4. Отрисовка объектов в том же ряду, но после блока игрока
    for (int y = blockRowStart; y < blockRowStart + 2; y++) {
        for (int x = blockColStart + 2; x <= endX; x++) {
            addVolumetricTileToRenderer(x, y, depth++);
        }
    }

    // 6.5. Отрисовка объектов после блока игрока
    for (int y = blockRowStart + 2; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            addVolumetricTileToRenderer(x, y, depth++);
        }
    }

    // 7. Отрисовка всех добавленных тайлов
    m_tileRenderer->render(renderer, centerX, centerY);
}

void MapScene::addVolumetricTileToRenderer(int x, int y, float priority) {
    const MapTile* tile = m_tileMap->getTile(x, y);
    if (!tile || tile->getType() == TileType::EMPTY || tile->getHeight() <= 0.0f) return;

    // Проверяем тип тайла - для воды устанавливаем очень низкий приоритет
    if (tile->getType() == TileType::WATER) {
        // Устанавливаем очень низкий приоритет для воды, чтобы всегда рисовать её под объектами
        priority = -1000.0f;
    }

    // Добавляем только объемные тайлы (высота > 0)
    float height = tile->getHeight();
    SDL_Color color = tile->getColor();

    // Убедимся, что альфа-канал цвета правильно установлен
    if (color.a == 0) color.a = 255;

    // Проверка на некорректные координаты
    if (std::isnan(x) || std::isnan(y) || std::isnan(height)) {
        return;
    }

    // Создаем оттенки для граней
    SDL_Color topColor = color;
    SDL_Color leftColor = {
        static_cast<Uint8>(color.r * 0.7f),
        static_cast<Uint8>(color.g * 0.7f),
        static_cast<Uint8>(color.b * 0.7f),
        color.a
    };
    SDL_Color rightColor = {
        static_cast<Uint8>(color.r * 0.5f),
        static_cast<Uint8>(color.g * 0.5f),
        static_cast<Uint8>(color.b * 0.5f),
        color.a
    };

    m_tileRenderer->addVolumetricTile(
        static_cast<float>(x), static_cast<float>(y), height,
        nullptr, nullptr, nullptr,
        topColor, leftColor, rightColor,
        priority
    );
}

void MapScene::addTileToRenderer(int x, int y, float priority) {
    const MapTile* tile = m_tileMap->getTile(x, y);
    if (!tile || tile->getType() == TileType::EMPTY) return;

    float height = tile->getHeight();
    SDL_Color color = tile->getColor();

    // Проверяем тип тайла - для воды устанавливаем очень низкий приоритет
    // чтобы гарантировать, что она отображается под всеми объектами
    if (tile->getType() == TileType::WATER) {
        // Устанавливаем очень низкий приоритет для воды, чтобы всегда рисовать её под объектами
        priority = -1000.0f;
    }

    // Убедимся, что альфа-канал цвета правильно установлен
    if (color.a == 0) color.a = 255;

    // Проверка на некорректные координаты
    if (std::isnan(x) || std::isnan(y) || std::isnan(height)) {
        return;
    }

    if (height > 0.0f) {
        // Объемный тайл (стена, препятствие и т.д.)
        SDL_Color topColor = color;
        SDL_Color leftColor = {
            static_cast<Uint8>(color.r * 0.7f),
            static_cast<Uint8>(color.g * 0.7f),
            static_cast<Uint8>(color.b * 0.7f),
            color.a
        };
        SDL_Color rightColor = {
            static_cast<Uint8>(color.r * 0.5f),
            static_cast<Uint8>(color.g * 0.5f),
            static_cast<Uint8>(color.b * 0.5f),
            color.a
        };

        m_tileRenderer->addVolumetricTile(
            static_cast<float>(x), static_cast<float>(y), height,
            nullptr, nullptr, nullptr,
            topColor, leftColor, rightColor,
            priority
        );
    }
    else {
        // Плоский тайл (пол, трава и т.д.)
        m_tileRenderer->addFlatTile(
            static_cast<float>(x), static_cast<float>(y),
            nullptr,
            color,
            priority
        );
    }
}

void MapScene::renderPlayerIndicator(SDL_Renderer* renderer, int centerX, int centerY) {
    if (!m_player) return;

    // Получаем координаты персонажа в мировом пространстве
    float playerFullX = m_player->getPlayerX() + m_player->getPlayerSubX();
    float playerFullY = m_player->getPlayerY() + m_player->getPlayerSubY();
    float playerHeight = 0.5f;

    // Преобразуем мировые координаты в экранные
    int screenX, screenY;
    m_isoRenderer->worldToDisplay(
        playerFullX, playerFullY, playerHeight + 0.5f, // Добавляем смещение по высоте
        centerX, centerY, screenX, screenY
    );

    // Рисуем индикатор - маленький желтый кружок
    int indicatorSize = 4;
    SDL_Rect indicator = {
        screenX - indicatorSize / 2,
        screenY - indicatorSize / 2,
        indicatorSize, indicatorSize
    };

    // Рисуем желтый индикатор
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderFillRect(renderer, &indicator);

    // Добавляем тонкую черную обводку для лучшей видимости
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &indicator);
}

void MapScene::renderPlayer(SDL_Renderer* renderer, int centerX, int centerY, float priority) {
    if (!m_player) return;

    // Полные координаты игрока
    float playerFullX = m_player->getPlayerX() + m_player->getPlayerSubX();
    float playerFullY = m_player->getPlayerY() + m_player->getPlayerSubY();

    // Цвета для игрока
    SDL_Color playerColor = { 255, 50, 50, 255 };      // Ярко-красный верх
    SDL_Color playerLeftColor = { 200, 40, 40, 255 };  // Левая грань темнее
    SDL_Color playerRightColor = { 150, 30, 30, 255 }; // Правая грань еще темнее

    // Высота персонажа: больше, чем высота пола (0.0f), но меньше, чем высота стен (1.0f)
    float playerHeight = 0.5f;

    // Добавляем персонажа в рендерер
    m_tileRenderer->addVolumetricTile(
        playerFullX, playerFullY, playerHeight,
        nullptr, nullptr, nullptr,
        playerColor, playerLeftColor, playerRightColor,
        priority
    );
}

void MapScene::renderCollisionDebug(SDL_Renderer* renderer, int centerX, int centerY) {
    if (!m_player) return;

    // Получаем форму коллизии игрока
    CollisionShape playerShape = m_player->getCollisionShape();

    // Получаем координаты игрока
    float playerX = m_player->getPlayerX() + m_player->getPlayerSubX();
    float playerY = m_player->getPlayerY() + m_player->getPlayerSubY();

    // Рисуем форму коллизии в зависимости от типа
    if (playerShape.getType() == CollisionShape::Type::CIRCLE) {
        // Для круговой формы рисуем окружность
        float radius = playerShape.getRadius();

        // Преобразуем координаты центра в экранные
        int screenCenterX, screenCenterY;
        m_isoRenderer->worldToDisplay(
            playerX, playerY, 0.0f,
            centerX, centerY, screenCenterX, screenCenterY
        );

        // Рисуем точки вокруг, формирующие окружность
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Красный цвет

        // Количество точек для отрисовки окружности
        const int numPoints = 32;
        SDL_Point points[numPoints + 1];

        for (int i = 0; i < numPoints; i++) {
            float angle = i * 2.0f * M_PI / numPoints;
            float worldX = playerX + radius * cos(angle);
            float worldY = playerY + radius * sin(angle);

            int screenX, screenY;
            m_isoRenderer->worldToDisplay(
                worldX, worldY, 0.0f,
                centerX, centerY, screenX, screenY
            );

            points[i] = { screenX, screenY };
        }

        // Замыкаем контур
        points[numPoints] = points[0];

        // Рисуем окружность
        SDL_RenderDrawLines(renderer, points, numPoints + 1);
    }
    else if (playerShape.getType() == CollisionShape::Type::RECTANGLE) {
        // Для прямоугольной формы рисуем прямоугольник
        float halfWidth = playerShape.getWidth() / 2.0f;
        float halfHeight = playerShape.getHeight() / 2.0f;

        // Координаты углов прямоугольника
        float corners[4][2] = {
            {playerX - halfWidth, playerY - halfHeight}, // Верхний левый
            {playerX + halfWidth, playerY - halfHeight}, // Верхний правый
            {playerX + halfWidth, playerY + halfHeight}, // Нижний правый
            {playerX - halfWidth, playerY + halfHeight}  // Нижний левый
        };

        // Преобразуем координаты углов в экранные
        SDL_Point points[5];
        for (int i = 0; i < 4; i++) {
            int screenX, screenY;
            m_isoRenderer->worldToDisplay(
                corners[i][0], corners[i][1], 0.0f,
                centerX, centerY, screenX, screenY
            );

            points[i] = { screenX, screenY };
        }

        // Замыкаем контур
        points[4] = points[0];

        // Рисуем прямоугольник
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Красный цвет
        SDL_RenderDrawLines(renderer, points, 5);
    }

    // Отображение проходимых и непроходимых тайлов
    // Диапазон тайлов для проверки
    int tileRadius = 5;
    int playerTileX = static_cast<int>(playerX);
    int playerTileY = static_cast<int>(playerY);

    for (int y = playerTileY - tileRadius; y <= playerTileY + tileRadius; y++) {
        for (int x = playerTileX - tileRadius; x <= playerTileX + tileRadius; x++) {
            if (!m_tileMap->isValidCoordinate(x, y)) continue;

            bool isWalkable = m_tileMap->isTileWalkable(x, y);

            // Получаем экранные координаты углов тайла
            int screenX[4], screenY[4];

            // Верхний левый угол
            m_isoRenderer->worldToDisplay(x, y, 0.0f, centerX, centerY, screenX[0], screenY[0]);

            // Верхний правый угол
            m_isoRenderer->worldToDisplay(x + 1.0f, y, 0.0f, centerX, centerY, screenX[1], screenY[1]);

            // Нижний правый угол
            m_isoRenderer->worldToDisplay(x + 1.0f, y + 1.0f, 0.0f, centerX, centerY, screenX[2], screenY[2]);

            // Нижний левый угол
            m_isoRenderer->worldToDisplay(x, y + 1.0f, 0.0f, centerX, centerY, screenX[3], screenY[3]);

            // Создаем массив точек для тайла
            SDL_Point tilePoints[5] = {
                {screenX[0], screenY[0]},
                {screenX[1], screenY[1]},
                {screenX[2], screenY[2]},
                {screenX[3], screenY[3]},
                {screenX[0], screenY[0]} // Замыкаем контур
            };

            // Рисуем границы тайла с цветом, зависящим от проходимости
            if (isWalkable) {
                // Зеленый для проходимых тайлов
                SDL_SetRenderDrawColor(renderer, 0, 200, 0, 100);
            }
            else {
                // Красный для непроходимых тайлов
                SDL_SetRenderDrawColor(renderer, 200, 0, 0, 100);
            }

            SDL_RenderDrawLines(renderer, tilePoints, 5);
        }
    }
}