#include "MapScene.h"
#include "Engine.h"
#include "ResourceManager.h"
#include <iostream>
#include <cmath>

MapScene::MapScene(const std::string& name, Engine* engine)
    : Scene(name), m_engine(engine), m_playerX(0.0f), m_playerY(0.0f) {
}

MapScene::~MapScene() {
}

bool MapScene::initialize() {
    // Инициализация изометрического рендерера
    m_isoRenderer = std::make_shared<IsometricRenderer>(64, 32);

    // Инициализация камеры
    m_camera = std::make_shared<Camera>(800, 600);
    m_camera->setTarget(&m_playerX, &m_playerY);

    // Создание карты размером 50x50 тайлов
    m_tileMap = std::make_shared<TileMap>(50, 50);
    if (!m_tileMap->initialize()) {
        std::cerr << "Failed to initialize tile map" << std::endl;
        return false;
    }

    // Генерация тестовой карты
    generateTestMap();

    // Инициализация рендерера тайлов
    m_tileRenderer = std::make_shared<TileRenderer>(m_isoRenderer.get());

    std::cout << "MapScene initialized successfully" << std::endl;
    return true;
}

/**
 * @brief Проверяет возможность диагонального перемещения
 * @param fromX Начальная X координата
 * @param fromY Начальная Y координата
 * @param toX Конечная X координата
 * @param toY Конечная Y координата
 * @return true, если диагональное перемещение возможно, false в противном случае
 */
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

    // Проверяем соседние тайлы, через которые проходит диагональ
    // Для диагонального движения оба соседних тайла должны быть проходимыми
    return (m_tileMap->isValidCoordinate(fromX, toY) && m_tileMap->isTileWalkable(fromX, toY)) &&
        (m_tileMap->isValidCoordinate(toX, fromY) && m_tileMap->isTileWalkable(toX, fromY));
}


void MapScene::handleEvent(const SDL_Event& event) {
    // Обработка событий камеры
    m_camera->handleEvent(event);

    // Обработка клавиатуры для перемещения персонажа
    if (event.type == SDL_KEYDOWN) {
        int curX = static_cast<int>(m_playerX);
        int curY = static_cast<int>(m_playerY);
        int newX = curX;
        int newY = curY;

        switch (event.key.keysym.sym) {
        case SDLK_w: // Северо-запад (NW)
            newX -= 1;
            newY -= 1;
            break;
        case SDLK_s: // Юго-восток (SE)
            newX += 1;
            newY += 1;
            break;
        case SDLK_a: // Юго-запад (SW)
            newX -= 1;
            newY += 1;
            break;
        case SDLK_d: // Северо-восток (NE)
            newX += 1;
            newY -= 1;
            break;
        case SDLK_UP: // Север (N)
            newY -= 1;
            break;
        case SDLK_DOWN: // Юг (S)
            newY += 1;
            break;
        case SDLK_LEFT: // Запад (W)
            newX -= 1;
            break;
        case SDLK_RIGHT: // Восток (E)
            newX += 1;
            break;
        case SDLK_r: // Сброс позиции
            m_playerX = 25.0f;
            m_playerY = 25.0f;
            break;
        case SDLK_g: // Генерация новой карты
            generateTestMap();
            break;
        }

        // Проверяем возможность перемещения
        if (newX != curX || newY != curY) { // Если позиция изменилась
            if (abs(newX - curX) == 1 && abs(newY - curY) == 1) {
                // Диагональное перемещение
                if (canMoveDiagonally(curX, curY, newX, newY)) {
                    m_playerX = static_cast<float>(newX);
                    m_playerY = static_cast<float>(newY);
                }
            }
            else {
                // Ортогональное перемещение (по вертикали или горизонтали)
                if (m_tileMap->isValidCoordinate(newX, newY) && m_tileMap->isTileWalkable(newX, newY)) {
                    m_playerX = static_cast<float>(newX);
                    m_playerY = static_cast<float>(newY);
                }
            }
        }
    }

    // Передаем события в базовый класс
    Scene::handleEvent(event);
}

void MapScene::update(float deltaTime) {
    // Обновление камеры
    m_camera->update(deltaTime);

    // Передаем события в базовый класс
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

    // Настраиваем рендерер с учетом камеры
    m_isoRenderer->setCameraPosition(m_camera->getX(), m_camera->getY());
    m_isoRenderer->setCameraZoom(m_camera->getZoom());

    // Очищаем и заполняем TileRenderer тайлами
    m_tileRenderer->clear();

    // Определяем видимую область карты
    float cameraX = m_camera->getX();
    float cameraY = m_camera->getY();
    float zoom = m_camera->getZoom();
    int viewportRadius = static_cast<int>(30.0f / zoom) + 1; // Радиус видимой области

    // Преобразуем мировые координаты камеры в координаты тайла
    int camTileX = static_cast<int>(cameraX);
    int camTileY = static_cast<int>(cameraY);

    // Определяем границы видимой области (с запасом)
    int startX = std::max(0, camTileX - viewportRadius);
    int startY = std::max(0, camTileY - viewportRadius);
    int endX = std::min(m_tileMap->getWidth() - 1, camTileX + viewportRadius);
    int endY = std::min(m_tileMap->getHeight() - 1, camTileY + viewportRadius);

    // Добавляем тайлы в видимой области
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile) {
                TileType type = tile->getType();
                float height = tile->getHeight();
                SDL_Color color = tile->getColor();

                if (type == TileType::EMPTY) {
                    continue; // Не рендерим пустые тайлы
                }

                if (height > 0.0f) {
                    // Объемный тайл
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
                        0 // Приоритет
                    );
                }
                else {
                    // Плоский тайл
                    m_tileRenderer->addFlatTile(
                        static_cast<float>(x), static_cast<float>(y),
                        nullptr, // Без текстуры
                        color,
                        0 // Приоритет
                    );
                }
            }
        }
    }

    // Добавляем игрока (красный куб)
    SDL_Color playerColor = { 255, 0, 0, 255 };
    SDL_Color playerLeftColor = { 200, 0, 0, 255 };
    SDL_Color playerRightColor = { 150, 0, 0, 255 };

    m_tileRenderer->addVolumetricTile(
        m_playerX, m_playerY, 0.5f,
        nullptr, nullptr, nullptr,
        playerColor, playerLeftColor, playerRightColor,
        100 // Наивысший приоритет для игрока
    );

    // Рендерим все тайлы
    m_tileRenderer->render(renderer, centerX, centerY);

    // Отрисовываем индикатор над игроком
    int indicatorX, indicatorY;
    m_isoRenderer->worldToDisplay(m_playerX, m_playerY, 0.7f,
        centerX, centerY,
        indicatorX, indicatorY);

    // Размер индикатора с учетом масштаба
    int indicatorSize = m_isoRenderer->getScaledSize(8);

    // Рисуем белый квадрат с черной обводкой
    SDL_Rect indicator = {
        indicatorX - indicatorSize,
        indicatorY - indicatorSize,
        indicatorSize * 2,
        indicatorSize * 2
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &indicator);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &indicator);

    // Отрисовываем дополнительные сущности
    Scene::render(renderer);
}

void MapScene::generateTestMap() {
    // Очищаем карту
    m_tileMap->clear();

    // Создаем тестовую карту с комнатами, коридорами и дверями
    
    // 1. Создаем основную комнату в центре
    m_tileMap->createRoom(20, 20, 30, 30, TileType::FLOOR, TileType::WALL);
    
    // 2. Добавляем комнату сверху
    m_tileMap->createRoom(22, 12, 28, 18, TileType::FLOOR, TileType::WALL);
    
    // 3. Добавляем комнату справа
    m_tileMap->createRoom(32, 22, 38, 28, TileType::FLOOR, TileType::WALL);
    
    // 4. Добавляем комнату снизу
    m_tileMap->createRoom(22, 32, 28, 38, TileType::FLOOR, TileType::WALL);
    
    // 5. Добавляем комнату слева
    m_tileMap->createRoom(12, 22, 18, 28, TileType::FLOOR, TileType::WALL);
    
    // 6. Соединяем комнаты коридорами и дверями
    
    // Коридор сверху
    m_tileMap->createVerticalCorridor(25, 18, 20, TileType::FLOOR);
    m_tileMap->createDoor(25, 19);
    
    // Коридор справа
    m_tileMap->createHorizontalCorridor(30, 32, 25, TileType::FLOOR);
    m_tileMap->createDoor(31, 25);
    
    // Коридор снизу
    m_tileMap->createVerticalCorridor(25, 30, 32, TileType::FLOOR);
    m_tileMap->createDoor(25, 31);
    
    // Коридор слева
    m_tileMap->createHorizontalCorridor(18, 20, 25, TileType::FLOOR);
    m_tileMap->createDoor(19, 25);
    
    // 7. Добавляем немного водных тайлов
    for (int x = 22; x <= 28; x++) {
        for (int y = 22; y <= 23; y++) {
            m_tileMap->setTileType(x, y, TileType::WATER);
        }
    }
    
    // 8. Добавляем травяные тайлы в верхней комнате
    for (int x = 23; x <= 27; x++) {
        for (int y = 13; y <= 17; y++) {
            m_tileMap->setTileType(x, y, TileType::GRASS);
        }
    }
    
    // 9. Добавляем каменные тайлы в правой комнате
    for (int x = 33; x <= 37; x++) {
        for (int y = 23; y <= 27; y++) {
            m_tileMap->setTileType(x, y, TileType::STONE);
        }
    }
    
    // 10. Добавляем деревянные тайлы в нижней комнате
    for (int x = 23; x <= 27; x++) {
        for (int y = 33; y <= 37; y++) {
            m_tileMap->setTileType(x, y, TileType::WOOD);
        }
    }
    
    // 11. Добавляем металлические тайлы в левой комнате
    for (int x = 13; x <= 17; x++) {
        for (int y = 23; y <= 27; y++) {
            m_tileMap->setTileType(x, y, TileType::METAL);
        }
    }
    
    // Устанавливаем позицию игрока в центре карты
    m_playerX = 25.0f;
    m_playerY = 25.0f;
}