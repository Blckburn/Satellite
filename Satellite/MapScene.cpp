#include "MapScene.h"
#include "Engine.h"
#include "ResourceManager.h"
#include <iostream>
#include <cmath>
#include "MapGenerator.h"
#include <ctime>  // Добавлено для time()

MapScene::MapScene(const std::string& name, Engine* engine)
    : Scene(name), m_engine(engine), m_playerX(25.0f), m_playerY(25.0f),
    m_playerSubX(0.0f), m_playerSubY(0.0f), m_moveSpeed(0.05f),
    m_dX(0.0f), m_dY(0.0f), m_showDebug(false), m_collisionSize(0.35f) {
}

MapScene::~MapScene() {
}

bool MapScene::initialize() {
    // 1. Инициализация изометрического рендерера
    m_isoRenderer = std::make_shared<IsometricRenderer>(64, 32);

    // 2. Инициализация камеры
    m_camera = std::make_shared<Camera>(800, 600);
    m_camera->setTarget(&m_playerX, &m_playerY);

    // 3. Создание карты размером 50x50 тайлов
    m_tileMap = std::make_shared<TileMap>(50, 50);
    if (!m_tileMap->initialize()) {
        std::cerr << "Failed to initialize tile map" << std::endl;
        return false;
    }

    // 4. Генерация тестовой карты
    generateTestMap();

    // 5. Инициализация рендерера тайлов
    m_tileRenderer = std::make_shared<TileRenderer>(m_isoRenderer.get());

    std::cout << "MapScene initialized successfully" << std::endl;
    return true;
}

bool MapScene::canMoveDiagonally(int fromX, int fromY, int toX, int toY) {
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

        // Корректировка в зависимости от направления движения
        if (xFractional < 0.1f && m_dX < 0.0f) {
            boundaryFactor = 1.0f;  // Движение влево
        }
        else if (xFractional > 0.9f && m_dX > 0.0f) {
            boundaryFactor = 1.0f;  // Движение вправо
        }

        if (yFractional < 0.1f && m_dY < 0.0f) {
            boundaryFactor = 1.0f;  // Движение вверх
        }
        else if (yFractional > 0.9f && m_dY > 0.0f) {
            boundaryFactor = 1.0f;  // Движение вниз
        }
    }

    // 4. Финальный приоритет
    return baseDepth + heightFactor + boundaryFactor;
}

void MapScene::handleEvent(const SDL_Event& event) {
    // Обработка событий камеры
    m_camera->handleEvent(event);

    // Обработка клавиатурных событий
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
        case SDLK_r:
            // Сброс позиции персонажа
            m_playerX = 25.0f;
            m_playerY = 25.0f;
            m_playerSubX = 0.0f;
            m_playerSubY = 0.0f;
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

    // Передаем события в базовый класс
    Scene::handleEvent(event);
}

void MapScene::update(float deltaTime) {
    // 1. Обнаружение нажатий клавиш
    detectKeyInput();

    // 2. Получаем текущую позицию персонажа
    int currentTileX = static_cast<int>(m_playerX);
    int currentTileY = static_cast<int>(m_playerY);

    // 3. Обработка перемещения при наличии направления
    if (m_dX != 0.0f || m_dY != 0.0f) {
        // Нормализация скорости для разных частот кадров
        float normalizedSpeed = m_moveSpeed * deltaTime * 60.0f;

        // Рассчитываем следующую позицию
        float nextSubX = m_playerSubX + m_dX * normalizedSpeed;
        float nextSubY = m_playerSubY + m_dY * normalizedSpeed;

        // Определяем, выходит ли персонаж за пределы текущего тайла
        bool crossingTileBoundaryX = nextSubX >= 1.0f || nextSubX < 0.0f;
        bool crossingTileBoundaryY = nextSubY >= 1.0f || nextSubY < 0.0f;

        // Координаты следующего тайла
        int nextTileX = currentTileX + (nextSubX >= 1.0f ? 1 : (nextSubX < 0.0f ? -1 : 0));
        int nextTileY = currentTileY + (nextSubY >= 1.0f ? 1 : (nextSubY < 0.0f ? -1 : 0));

        // Проверка возможности движения по X
        bool canMoveX = true;
        if (crossingTileBoundaryX) {
            canMoveX = m_tileMap->isValidCoordinate(nextTileX, currentTileY) &&
                m_tileMap->isTileWalkable(nextTileX, currentTileY);
        }

        // Проверка возможности движения по Y
        bool canMoveY = true;
        if (crossingTileBoundaryY) {
            canMoveY = m_tileMap->isValidCoordinate(currentTileX, nextTileY) &&
                m_tileMap->isTileWalkable(currentTileX, nextTileY);
        }

        // Проверка диагонального движения
        bool diagonalMove = crossingTileBoundaryX && crossingTileBoundaryY;
        bool canMoveDiag = true;

        if (diagonalMove) {
            canMoveDiag = m_tileMap->isValidCoordinate(nextTileX, nextTileY) &&
                m_tileMap->isTileWalkable(nextTileX, nextTileY) &&
                canMoveX && canMoveY;
        }

        // Применяем движение с учетом коллизий
        if (canMoveX && (!diagonalMove || canMoveDiag)) {
            m_playerSubX = nextSubX;

            // Если перешли в новый тайл, обновляем координаты
            if (nextSubX >= 1.0f) {
                m_playerX += 1.0f;
                m_playerSubX -= 1.0f;
            }
            else if (nextSubX < 0.0f) {
                m_playerX -= 1.0f;
                m_playerSubX += 1.0f;
            }
        }
        else if (crossingTileBoundaryX) {
            // Останавливаемся у границы тайла
            m_playerSubX = nextSubX >= 1.0f ? 0.99f : 0.01f;
        }
        else {
            m_playerSubX = nextSubX; // Двигаемся в пределах текущего тайла
        }

        if (canMoveY && (!diagonalMove || canMoveDiag)) {
            m_playerSubY = nextSubY;

            // Если перешли в новый тайл, обновляем координаты
            if (nextSubY >= 1.0f) {
                m_playerY += 1.0f;
                m_playerSubY -= 1.0f;
            }
            else if (nextSubY < 0.0f) {
                m_playerY -= 1.0f;
                m_playerSubY += 1.0f;
            }
        }
        else if (crossingTileBoundaryY) {
            // Останавливаемся у границы тайла
            m_playerSubY = nextSubY >= 1.0f ? 0.99f : 0.01f;
        }
        else {
            m_playerSubY = nextSubY; // Двигаемся в пределах текущего тайла
        }

        // Гарантируем, что субкоординаты остаются в диапазоне [0, 1)
        if (m_playerSubX >= 1.0f) {
            m_playerX += 1.0f;
            m_playerSubX -= 1.0f;
        }
        else if (m_playerSubX < 0.0f) {
            m_playerX -= 1.0f;
            m_playerSubX += 1.0f;
        }

        if (m_playerSubY >= 1.0f) {
            m_playerY += 1.0f;
            m_playerSubY -= 1.0f;
        }
        else if (m_playerSubY < 0.0f) {
            m_playerY -= 1.0f;
            m_playerSubY += 1.0f;
        }
    }

    // 4. Обновление камеры
    m_camera->update(deltaTime);

    // 5. Обновление базового класса
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
    m_camera->setPosition(m_playerX + m_playerSubX, m_playerY + m_playerSubY);
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

    // Создаем генератор карт с случайным сидом
    MapGenerator mapGen(static_cast<unsigned int>(std::time(nullptr)));

    // Устанавливаем параметры генерации
    mapGen.setParameters(
        20.0f,  // Средняя температура
        0.5f,   // Влажность
        0.6f,   // Неровность поверхности
        0.3f,   // Уровень воды
        0.5f    // Богатство ресурсами
    );

    // Устанавливаем биомы по умолчанию
    mapGen.setupDefaultBiomes();

    // Генерируем карту с типом DEFAULT
    mapGen.generate(m_tileMap.get(), MapGenerator::GenerationType::DEFAULT);

    // Если предпочитаете использовать оригинальный метод как запасной вариант,
    // закомментируйте код выше и раскомментируйте этот блок:
    /*
    // 1. Создаем основную комнату в центре
    m_tileMap->createRoom(20, 20, 30, 30, TileType::FLOOR, TileType::WALL);

    // 2. Добавляем комнату сверху
    m_tileMap->createRoom(22, 12, 28, 18, TileType::FLOOR, TileType::WALL);

    // ... [остальной оригинальный код] ...
    */

    // Устанавливаем позицию игрока в центре карты
    m_playerX = m_tileMap->getWidth() / 2.0f;
    m_playerY = m_tileMap->getHeight() / 2.0f;
    m_playerSubX = 0.0f;
    m_playerSubY = 0.0f;
}

void MapScene::detectKeyInput() {
    // Получаем текущее состояние клавиатуры
    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    // Определяем нажатие клавиш направления
    bool upPressed = keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP];
    bool downPressed = keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN];
    bool leftPressed = keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT];
    bool rightPressed = keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT];

    // Сбрасываем направление перед установкой нового
    m_dX = 0.0f;
    m_dY = 0.0f;

    // Определение направления (изометрическая система координат)
    if (upPressed && !downPressed) {
        m_dY = -1.0f; // Север в изометрии
    }
    else if (!upPressed && downPressed) {
        m_dY = 1.0f;  // Юг в изометрии
    }

    if (rightPressed && !leftPressed) {
        m_dX = 1.0f;  // Восток в изометрии
    }
    else if (!rightPressed && leftPressed) {
        m_dX = -1.0f; // Запад в изометрии
    }

    // Нормализация для диагонального движения
    if (m_dX != 0.0f && m_dY != 0.0f) {
        float length = std::sqrt(m_dX * m_dX + m_dY * m_dY);
        m_dX /= length;
        m_dY /= length;
    }
}

void MapScene::renderDebug(SDL_Renderer* renderer, int centerX, int centerY) {
    // Включается только при m_showDebug = true
    // Рисуем коллизионную рамку вокруг персонажа
    float playerFullX = m_playerX + m_playerSubX;
    float playerFullY = m_playerY + m_playerSubY;

    // 1. Отображение коллизионного прямоугольника персонажа
    int screenX[4], screenY[4];

    // Получаем экранные координаты для каждого угла коллизионной области
    // Верхний левый угол
    m_isoRenderer->worldToDisplay(
        playerFullX - m_collisionSize,
        playerFullY - m_collisionSize,
        0.0f, centerX, centerY, screenX[0], screenY[0]
    );

    // Верхний правый угол
    m_isoRenderer->worldToDisplay(
        playerFullX + m_collisionSize,
        playerFullY - m_collisionSize,
        0.0f, centerX, centerY, screenX[1], screenY[1]
    );

    // Нижний правый угол
    m_isoRenderer->worldToDisplay(
        playerFullX + m_collisionSize,
        playerFullY + m_collisionSize,
        0.0f, centerX, centerY, screenX[2], screenY[2]
    );

    // Нижний левый угол
    m_isoRenderer->worldToDisplay(
        playerFullX - m_collisionSize,
        playerFullY + m_collisionSize,
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
    int currentTileX = static_cast<int>(m_playerX);
    int currentTileY = static_cast<int>(m_playerY);

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
    float playerFullX = m_playerX + m_playerSubX;
    float playerFullY = m_playerY + m_playerSubY;

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
    // Получаем координаты персонажа в мировом пространстве
    float playerFullX = m_playerX + m_playerSubX;
    float playerFullY = m_playerY + m_playerSubY;
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
    // Полные координаты игрока
    float playerFullX = m_playerX + m_playerSubX;
    float playerFullY = m_playerY + m_playerSubY;

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