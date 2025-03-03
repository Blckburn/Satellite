#include "MapScene.h"
#include "Engine.h"
#include "ResourceManager.h"
#include "CollisionSystem.h" 
#include "Player.h"
#include <iostream>
#include <cmath>
#include "RoomGenerator.h"
#include "Logger.h"
#include <set>

MapScene::MapScene(const std::string& name, Engine* engine)
    : Scene(name), m_engine(engine), m_showDebug(false) {
    // Примечание: Игрок будет инициализирован в методе initialize()
}

MapScene::~MapScene() {
}

bool MapScene::initialize() {

    std::cout << "MapScene::initialize() - Starting initialization" << std::endl;
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

    // 4. Инициализация рендерера тайлов
    m_tileRenderer = std::make_shared<TileRenderer>(m_isoRenderer.get());

    // 4.5. Создание и инициализация системы коллизий
    m_collisionSystem = std::make_shared<CollisionSystem>(m_tileMap.get());

    // 5. Создание и инициализация игрока
    m_player = std::make_shared<Player>("Player", m_tileMap.get());
    if (!m_player->initialize()) {
        std::cerr << "Failed to initialize player" << std::endl;
        return false;
    }

    // 5.1. Связываем игрока с системой коллизий
    if (m_player && m_collisionSystem) {
        m_player->setCollisionSystem(m_collisionSystem.get());
        std::cout << "Player successfully linked to CollisionSystem" << std::endl;
    }
    else {
        std::cerr << "Warning: Failed to link player to collision system" << std::endl;
    }

    // 6. Генерация тестовой карты
    generateTestMap();

    // 7. Настройка камеры для слежения за игроком
    m_camera->setTarget(&m_player->getPosition().x, &m_player->getPosition().y);

    std::cout << "MapScene initialized successfully" << std::endl;

    // Инициализация параметров для интерактивных объектов
    m_interactionPromptTimer = 0.0f;
    m_showInteractionPrompt = false;
    m_interactionPrompt = "";

    std::cout << "Interactive objects initialized: " << m_interactiveObjects.size() << std::endl;

    return true;
}

bool MapScene::canMoveDiagonally(int fromX, int fromY, int toX, int toY) {
    // Используем систему коллизий, если она доступна
    if (m_collisionSystem) {
        return m_collisionSystem->canMoveDiagonally(fromX, fromY, toX, toY);
    }

    // Если система коллизий недоступна, используем метод игрока
    if (m_player) {
        return m_player->canMoveDiagonally(fromX, fromY, toX, toY);
    }

    // Убедимся, что это действительно диагональное перемещение
    int dx = toX - fromX;
    int dy = toY - fromY;

    if (abs(dx) != 1 || abs(dy) != 1) {
        // Не диагональное перемещение
        return m_tileMap->isValidCoordinate(toX, toY) && m_tileMap->isTileWalkable(toX, toY);
    }

    // Проверяем целевой тайл
    if (!m_tileMap->isValidCoordinate(toX, toY) || !m_tileMap->isTileWalkable(toX, toY)) {
        return false;
    }

    // Проверка обоих промежуточных тайлов для избежания "срезания углов"
    // Оба тайла должны быть проходимыми
    bool canPassX = m_tileMap->isValidCoordinate(toX, fromY) && m_tileMap->isTileWalkable(toX, fromY);
    bool canPassY = m_tileMap->isValidCoordinate(fromX, toY) && m_tileMap->isTileWalkable(fromX, toY);

    return canPassX && canPassY;
}

float MapScene::calculateZOrderPriority(float x, float y, float z) {
    // 1. Базовый приоритет на основе положения в пространстве
    float baseDepth = (x + y) * 10.0f;

    // 2. Высота (Z) влияет на приоритет отображения
    float heightFactor = z * 5.0f;

    // 3. Для персонажа учитываем положение относительно границ тайлов
    float boundaryFactor = 0.0f;

    // Проверяем близость к границе тайла
    float xFractional = x - floor(x);
    float yFractional = y - floor(y);

    if (xFractional < 0.1f || xFractional > 0.9f ||
        yFractional < 0.1f || yFractional > 0.9f) {
        boundaryFactor = 0.5f;

        // Корректировка в зависимости от направления движения игрока
        if (m_player) {
            float dX = m_player->getDirectionX();
            float dY = m_player->getDirectionY();

            if (xFractional < 0.1f && dX < 0.0f) {
                boundaryFactor = 1.0f;  // Движение влево
            }
            else if (xFractional > 0.9f && dX > 0.0f) {
                boundaryFactor = 1.0f;  // Движение вправо
            }

            if (yFractional < 0.1f && dY < 0.0f) {
                boundaryFactor = 1.0f;  // Движение вверх
            }
            else if (yFractional > 0.9f && dY > 0.0f) {
                boundaryFactor = 1.0f;  // Движение вниз
            }
        }
    }

    // 4. Финальный приоритет
    return baseDepth + heightFactor + boundaryFactor;
}

void MapScene::handleEvent(const SDL_Event& event) {
    // Обработка событий камеры
    m_camera->handleEvent(event);

    // Обработка событий игрока
    if (m_player) {
        m_player->handleEvent(event);
    }

    // Обработка клавиатурных событий
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_r:
            // Сброс позиции персонажа
            generateTestMap();
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

    // Обработка взаимодействия с объектами
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_e) {
        handleInteraction();
    }

    // Передаем события в базовый класс
    Scene::handleEvent(event);
}

void MapScene::update(float deltaTime) {
    // 1. Обнаружение нажатий клавиш и обновление игрока
    if (m_player) {
        m_player->detectKeyInput();
        m_player->update(deltaTime);
    }

    // 2. Обновление камеры
    m_camera->update(deltaTime);

    // Обновление интерактивных объектов
    auto it = m_interactiveObjects.begin();
    while (it != m_interactiveObjects.end()) {
        if ((*it)->isActive()) {
            (*it)->update(deltaTime);
            ++it;
        }
        else {
            // Удаляем неактивные объекты
            it = m_interactiveObjects.erase(it);
        }
    }

    // Обновление таймера подсказки
    if (m_showInteractionPrompt) {
        m_interactionPromptTimer += deltaTime;
        // Скрываем подсказку через 2 секунды
        if (m_interactionPromptTimer > 2.0f) {
            m_showInteractionPrompt = false;
        }
    }

    // Проверка наличия объектов для взаимодействия
    if (m_player) {
        float playerX = m_player->getFullX();
        float playerY = m_player->getFullY();

        std::shared_ptr<InteractiveObject> nearestObject = findNearestInteractiveObject(playerX, playerY);
        if (nearestObject && nearestObject->isInteractable()) {
            // Показываем подсказку
            m_interactionPrompt = nearestObject->getInteractionHint();
            m_showInteractionPrompt = true;
            m_interactionPromptTimer = 0.0f;
        }
    }


    // 3. Обновление базового класса
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

    // Привязываем камеру к персонажу
    if (m_player) {
        m_camera->setPosition(m_player->getFullX(), m_player->getFullY());
    }
    m_camera->setZoom(1.0f);

    // Настраиваем рендерер с учетом камеры
    m_isoRenderer->setCameraPosition(m_camera->getX(), m_camera->getY());
    m_isoRenderer->setCameraZoom(m_camera->getZoom());

    // Используем блочную сортировку для отрисовки тайлов, игрока и интерактивных объектов
    renderWithBlockSorting(renderer, centerX, centerY);

    // Добавляем индикатор игрока, чтобы его можно было видеть за стенами
    renderPlayerIndicator(renderer, centerX, centerY);

    // Отображаем отладочную информацию, если включен режим отладки
    if (m_showDebug) {
        renderDebug(renderer, centerX, centerY);
    }

    // Отрисовка подсказки для взаимодействия
    if (m_showInteractionPrompt) {
        renderInteractionPrompt(renderer);
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

    // Устанавливаем найденную позицию игрока
    if (m_player) {
        m_player->setPosition(playerX, playerY, 0.0f);
        // Устанавливаем субкоординаты в центр тайла
        m_player->setSubX(0.5f);
        m_player->setSubY(0.5f);
    }

    if (m_player) {
        m_player->setPosition(playerX, playerY, 0.0f);
        // Устанавливаем субкоординаты в центр тайла
        m_player->setSubX(0.5f);
        m_player->setSubY(0.5f);
    }

    // Создаем интерактивные предметы на карте
    createInteractiveItems();

    LOG_INFO("Generated test map with biome type: " + std::to_string(static_cast<int>(biomeType)));
}

void MapScene::detectKeyInput() {
    // Этот метод теперь просто делегирует обработку клавиш игроку
    if (m_player) {
        m_player->detectKeyInput();
    }
}

void MapScene::renderDebug(SDL_Renderer* renderer, int centerX, int centerY) {
    if (!m_player) return;

    // Включается только при m_showDebug = true
    // Рисуем коллизионную рамку вокруг персонажа
    float playerFullX = m_player->getFullX();
    float playerFullY = m_player->getFullY();
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
}

void MapScene::renderWithBlockSorting(SDL_Renderer* renderer, int centerX, int centerY) {
    // 1. Очистка рендерера перед отрисовкой
    m_tileRenderer->clear();

    // 2. Получение координат игрока и определение текущего блока
    float playerFullX = m_player ? m_player->getFullX() : 0.0f;
    float playerFullY = m_player ? m_player->getFullY() : 0.0f;

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

    // Добавляем интерактивные объекты в список для сортировки
    struct RenderObject {
        enum class Type { TILE, PLAYER, INTERACTIVE } type;
        float x, y, z;
        int tileX, tileY;
        float priority;
        std::shared_ptr<InteractiveObject> interactiveObj;
    };

    std::vector<RenderObject> objectsToRender;

    // 6.0. Добавляем интерактивные объекты в список сортировки
    for (auto& object : m_interactiveObjects) {
        if (!object->isActive()) continue;

        float objectX = object->getPosition().x;
        float objectY = object->getPosition().y;
        float objectZ = object->getPosition().z;

        // Пропускаем объекты вне видимой области
        if (objectX < startX - 5 || objectX > endX + 5 ||
            objectY < startY - 5 || objectY > endY + 5) {
            continue;
        }

        float priority = calculateZOrderPriority(objectX, objectY, objectZ);

        RenderObject renderObj;
        renderObj.type = RenderObject::Type::INTERACTIVE;
        renderObj.x = objectX;
        renderObj.y = objectY;
        renderObj.z = objectZ;
        renderObj.tileX = static_cast<int>(objectX);
        renderObj.tileY = static_cast<int>(objectY);
        renderObj.priority = priority;
        renderObj.interactiveObj = object;

        objectsToRender.push_back(renderObj);
    }

    // 6.1. Отрисовка объемных тайлов и персонажа с учетом блочного порядка
    // Сначала блоки перед блоком игрока
    for (int y = startY; y < blockRowStart; y++) {
        for (int x = startX; x <= endX; x++) {
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::EMPTY && tile->getHeight() > 0.0f) {
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y), tile->getHeight());

                RenderObject renderObj;
                renderObj.type = RenderObject::Type::TILE;
                renderObj.x = x;
                renderObj.y = y;
                renderObj.z = tile->getHeight();
                renderObj.tileX = x;
                renderObj.tileY = y;
                renderObj.priority = priority;

                objectsToRender.push_back(renderObj);
            }
        }
    }

    // 6.2. Отрисовка объектов в том же ряду, но перед блоком игрока
    for (int y = blockRowStart; y < blockRowStart + 2; y++) {
        for (int x = startX; x < blockColStart; x++) {
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::EMPTY && tile->getHeight() > 0.0f) {
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y), tile->getHeight());

                RenderObject renderObj;
                renderObj.type = RenderObject::Type::TILE;
                renderObj.x = x;
                renderObj.y = y;
                renderObj.z = tile->getHeight();
                renderObj.tileX = x;
                renderObj.tileY = y;
                renderObj.priority = priority;

                objectsToRender.push_back(renderObj);
            }
        }
    }

    // 6.3. Отрисовка блока игрока (2x2 тайла)
    for (int y = blockRowStart; y < blockRowStart + 2; y++) {
        for (int x = blockColStart; x < blockColStart + 2; x++) {
            // Проверяем, совпадает ли тайл с позицией игрока
            bool isPlayerTile = false;
            if (m_player) {
                isPlayerTile = (x == static_cast<int>(playerFullX) &&
                    y == static_cast<int>(playerFullY));
            }

            // Если это тайл с игроком, то рисуем его
            if (isPlayerTile) {
                RenderObject renderObj;
                renderObj.type = RenderObject::Type::PLAYER;
                renderObj.x = playerFullX;
                renderObj.y = playerFullY;
                renderObj.z = m_player->getHeight();
                renderObj.tileX = static_cast<int>(playerFullX);
                renderObj.tileY = static_cast<int>(playerFullY);
                renderObj.priority = calculateZOrderPriority(playerFullX, playerFullY, m_player->getHeight()) + 0.5f; // Небольшой приоритет для игрока

                objectsToRender.push_back(renderObj);
            }

            // Затем добавляем стены или другие объемные объекты в этом тайле
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::EMPTY && tile->getHeight() > 0.0f) {
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y), tile->getHeight());

                RenderObject renderObj;
                renderObj.type = RenderObject::Type::TILE;
                renderObj.x = x;
                renderObj.y = y;
                renderObj.z = tile->getHeight();
                renderObj.tileX = x;
                renderObj.tileY = y;
                renderObj.priority = priority;

                objectsToRender.push_back(renderObj);
            }
        }
    }

    // 6.4. Отрисовка объектов в том же ряду, но после блока игрока
    for (int y = blockRowStart; y < blockRowStart + 2; y++) {
        for (int x = blockColStart + 2; x <= endX; x++) {
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::EMPTY && tile->getHeight() > 0.0f) {
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y), tile->getHeight());

                RenderObject renderObj;
                renderObj.type = RenderObject::Type::TILE;
                renderObj.x = x;
                renderObj.y = y;
                renderObj.z = tile->getHeight();
                renderObj.tileX = x;
                renderObj.tileY = y;
                renderObj.priority = priority;

                objectsToRender.push_back(renderObj);
            }
        }
    }

    // 6.5. Отрисовка объектов после блока игрока
    for (int y = blockRowStart + 2; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::EMPTY && tile->getHeight() > 0.0f) {
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y), tile->getHeight());

                RenderObject renderObj;
                renderObj.type = RenderObject::Type::TILE;
                renderObj.x = x;
                renderObj.y = y;
                renderObj.z = tile->getHeight();
                renderObj.tileX = x;
                renderObj.tileY = y;
                renderObj.priority = priority;

                objectsToRender.push_back(renderObj);
            }
        }
    }

    // 7. Сортируем все объекты по приоритету
    std::sort(objectsToRender.begin(), objectsToRender.end(),
        [](const RenderObject& a, const RenderObject& b) {
            return a.priority < b.priority;
        });

    // 8. Отрисовываем отсортированные объекты
    for (const auto& obj : objectsToRender) {
        switch (obj.type) {
        case RenderObject::Type::TILE: {
            const MapTile* tile = m_tileMap->getTile(obj.tileX, obj.tileY);
            if (tile) {
                float height = tile->getHeight();
                SDL_Color color = tile->getColor();

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
                    obj.x, obj.y, height,
                    nullptr, nullptr, nullptr,
                    topColor, leftColor, rightColor,
                    obj.priority
                );
            }
            break;
        }
        case RenderObject::Type::PLAYER: {
            if (m_player) {
                SDL_Color playerColor = m_player->getColor();

                // Создаем оттенки для граней
                SDL_Color leftColor = {
                    static_cast<Uint8>(playerColor.r * 0.7f),
                    static_cast<Uint8>(playerColor.g * 0.7f),
                    static_cast<Uint8>(playerColor.b * 0.7f),
                    playerColor.a
                };
                SDL_Color rightColor = {
                    static_cast<Uint8>(playerColor.r * 0.5f),
                    static_cast<Uint8>(playerColor.g * 0.5f),
                    static_cast<Uint8>(playerColor.b * 0.5f),
                    playerColor.a
                };

                m_tileRenderer->addVolumetricTile(
                    playerFullX, playerFullY, m_player->getHeight(),
                    nullptr, nullptr, nullptr,
                    playerColor, leftColor, rightColor,
                    obj.priority
                );
            }
            break;
        }
        case RenderObject::Type::INTERACTIVE: {
            auto& interactive = obj.interactiveObj;
            if (interactive) {
                SDL_Color color = interactive->getColor();

                // Эффект парения для предметов
                float height = obj.z;
                if (auto pickupItem = std::dynamic_pointer_cast<PickupItem>(interactive)) {
                    height += 0.15f * sinf(SDL_GetTicks() / 500.0f);
                }

                // Создаем оттенки для граней
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
                    obj.x, obj.y, height,
                    nullptr, nullptr, nullptr,
                    color, leftColor, rightColor,
                    obj.priority
                );
            }
            break;
        }
        }
    }

    // 9. Рендерим все тайлы в правильном порядке
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
    float playerFullX = m_player->getFullX();
    float playerFullY = m_player->getFullY();
    float playerHeight = m_player->getHeight();

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
    float playerFullX = m_player->getFullX();
    float playerFullY = m_player->getFullY();
    float playerHeight = m_player->getHeight();

    // Получаем цвета для игрока
    SDL_Color playerColor = m_player->getColor();
    SDL_Color playerLeftColor = {
        static_cast<Uint8>(playerColor.r * 0.7f),
        static_cast<Uint8>(playerColor.g * 0.7f),
        static_cast<Uint8>(playerColor.b * 0.7f),
        playerColor.a
    };
    SDL_Color playerRightColor = {
        static_cast<Uint8>(playerColor.r * 0.5f),
        static_cast<Uint8>(playerColor.g * 0.5f),
        static_cast<Uint8>(playerColor.b * 0.5f),
        playerColor.a
    };

    // Добавляем персонажа в рендерер
    m_tileRenderer->addVolumetricTile(
        playerFullX, playerFullY, playerHeight,
        nullptr, nullptr, nullptr,
        playerColor, playerLeftColor, playerRightColor,
        priority
    );
}

void MapScene::addInteractiveObject(std::shared_ptr<InteractiveObject> object) {
    if (object) {
        m_interactiveObjects.push_back(object);
    }
}

void MapScene::removeInteractiveObject(std::shared_ptr<InteractiveObject> object) {
    if (!object) return;

    auto it = std::find(m_interactiveObjects.begin(), m_interactiveObjects.end(), object);
    if (it != m_interactiveObjects.end()) {
        m_interactiveObjects.erase(it);
    }
}

std::shared_ptr<PickupItem> MapScene::createTestPickupItem(float x, float y, const std::string& name, PickupItem::ItemType type) {
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

    // Добавляем предмет на сцену
    addInteractiveObject(item);

    LOG_INFO("Created pickup item: " + name + " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    return item;
}

void MapScene::handleInteraction() {
    if (!m_player) return;

    float playerX = m_player->getFullX();
    float playerY = m_player->getFullY();

    std::shared_ptr<InteractiveObject> nearestObject = findNearestInteractiveObject(playerX, playerY);

    if (nearestObject && nearestObject->isInteractable()) {
        // Взаимодействуем с объектом
        if (nearestObject->interact(m_player.get())) {
            // Взаимодействие успешно
            LOG_INFO("Interaction with " + nearestObject->getName() + " successful");

            // Отображаем подсказку в течение короткого времени
            m_showInteractionPrompt = true;
            m_interactionPromptTimer = 0.0f;
            m_interactionPrompt = "Picked up " + nearestObject->getName();
        }
    }
    else {
        LOG_INFO("No interactive objects in range");
    }
}


std::shared_ptr<InteractiveObject> MapScene::findNearestInteractiveObject(float playerX, float playerY) {
    std::shared_ptr<InteractiveObject> nearest = nullptr;
    float minDistanceSquared = std::numeric_limits<float>::max();

    for (auto& object : m_interactiveObjects) {
        if (!object->isActive() || !object->isInteractable()) continue;

        // Координаты объекта
        float objectX = object->getPosition().x;
        float objectY = object->getPosition().y;

        // Вычисление расстояния до игрока
        float dx = playerX - objectX;
        float dy = playerY - objectY;
        float distanceSquared = dx * dx + dy * dy;

        // Получение радиуса взаимодействия объекта
        float interactionRadius = object->getInteractionRadius();
        float radiusSquared = interactionRadius * interactionRadius;

        // Если объект находится в радиусе взаимодействия и ближе предыдущего
        if (distanceSquared <= radiusSquared && distanceSquared < minDistanceSquared) {
            minDistanceSquared = distanceSquared;
            nearest = object;
        }
    }

    return nearest;
}

void MapScene::renderInteractiveObjects(SDL_Renderer* renderer, int centerX, int centerY) {
    for (auto& object : m_interactiveObjects) {
        if (!object->isActive()) continue;

        // Получаем данные объекта
        float objectX = object->getPosition().x;
        float objectY = object->getPosition().y;
        float objectZ = object->getPosition().z;
        SDL_Color color = object->getColor();

        // Создаем визуальное представление объекта
        // В зависимости от типа объекта, можно использовать разное отображение
        // Здесь для примера используем объемный тайл для всех объектов

        // Вычисляем приоритет для правильной Z-сортировки
        float priority = calculateZOrderPriority(objectX, objectY, objectZ);

        // Для предметов добавляем эффект парения
        float height = objectZ;
        if (auto pickupItem = std::dynamic_pointer_cast<PickupItem>(object)) {
            // Если это предмет для подбора, добавляем эффект "парения"
            height += 0.15f * sinf(SDL_GetTicks() / 500.0f); // Эффект парения
        }

        // Создаем цвета граней на основе основного цвета
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

        // Добавляем объект в рендерер тайлов
        m_tileRenderer->addVolumetricTile(
            objectX, objectY, height,
            nullptr, nullptr, nullptr,
            color, leftColor, rightColor,
            priority + 10.0f // Увеличенный приоритет, чтобы объекты отображались поверх тайлов
        );
    }
}

void MapScene::renderInteractionPrompt(SDL_Renderer* renderer) {
    // В будущем здесь будет отрисовка подсказки для взаимодействия с помощью SDL_ttf
    // Пока что просто отрисуем прямоугольник с текстом в консоли

    // Получаем размеры окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Отрисовываем полупрозрачный прямоугольник внизу экрана
    SDL_Rect promptRect = {
        windowWidth / 2 - 150,
        windowHeight - 50,
        300,
        40
    };

    // Устанавливаем цвет прямоугольника (полупрозрачный черный)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &promptRect);

    // Рисуем рамку
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &promptRect);

    // Отображаем подсказку только при первом взаимодействии, а не каждый кадр
    static std::string lastPrompt;
    if (lastPrompt != m_interactionPrompt) {
        lastPrompt = m_interactionPrompt;
        LOG_INFO("Interaction prompt: " + m_interactionPrompt);
    }
}

/**
 * @brief Создает интерактивные предметы на карте
 */
void MapScene::createInteractiveItems() {
    if (!m_tileMap || !m_player) return;

    // Очищаем существующие объекты
    m_interactiveObjects.clear();

    // Используем случайный сид на основе текущего времени
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Параметры генерации предметов
    int attempts = 0;
    int maxAttempts = 100;      // Увеличиваем максимальное количество попыток
    int itemsToPlace = 15;      // Увеличиваем количество предметов на карте
    int itemsPlaced = 0;

    // Минимальные и максимальные значения для расстояния от игрока
    float minDistanceFromPlayer = 5.0f;
    float maxDistanceFromPlayer = 20.0f;

    // Текущая позиция игрока
    float playerX = m_player->getPosition().x;
    float playerY = m_player->getPosition().y;

    // Хранение уже использованных позиций для предотвращения дублирования
    std::set<std::pair<int, int>> usedPositions;

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

            // Создаем предмет с уникальным именем
            std::string itemName;
            switch (type) {
            case PickupItem::ItemType::RESOURCE:
                itemName = "Resource Fragment";
                break;
            case PickupItem::ItemType::WEAPON:
                itemName = "Energy Weapon";
                break;
            case PickupItem::ItemType::ARMOR:
                itemName = "Shield Module";
                break;
            case PickupItem::ItemType::CONSUMABLE:
                itemName = "Health Kit";
                break;
            case PickupItem::ItemType::KEY:
                itemName = "Access Card";
                break;
            default:
                itemName = "Unknown Item";
                break;
            }

            // Добавляем номер для уникальности
            itemName += " #" + std::to_string(itemsPlaced + 1);

            // Создаем предмет
            auto item = createTestPickupItem(static_cast<float>(x), static_cast<float>(y), itemName, type);

            // Устанавливаем увеличенный радиус взаимодействия для лучшей доступности
            if (item) {
                item->setInteractionRadius(2.0f);
            }

            itemsPlaced++;
        }

        attempts++;
    }

    // Добавляем информацию о покрытии карты предметами
    float coverage = static_cast<float>(itemsPlaced) / (m_tileMap->getWidth() * m_tileMap->getHeight()) * 100.0f;
    LOG_INFO("Created " + std::to_string(itemsPlaced) + " random items on the map (coverage: " +
        std::to_string(coverage) + "%)");
}