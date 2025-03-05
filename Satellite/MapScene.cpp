#include "MapScene.h"
#include "Engine.h"
#include "ResourceManager.h"
#include "CollisionSystem.h" 
#include "Player.h"
#include <iostream>
#include <cmath>
#include "Logger.h"
#include <set>
#include "WorldGenerator.h"


MapScene::MapScene(const std::string& name, Engine* engine)
    : Scene(name), m_engine(engine), m_showDebug(false),
    m_currentInteractingDoor(nullptr), m_isInteractingWithDoor(false),
    m_currentInteractingTerminal(nullptr), m_isDisplayingTerminalInfo(false) {
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

    // 5.2. Загрузка шрифта для интерфейса
    if (m_engine && m_engine->getResourceManager()) {
        // Пытаемся загрузить шрифт (путь нужно адаптировать под вашу структуру проекта)
        bool fontLoaded = m_engine->getResourceManager()->loadFont("default", "assets/fonts/Font.ttf", 16);
        if (fontLoaded) {
            LOG_INFO("Default font loaded successfully");
        }
        else {
            LOG_WARNING("Failed to load default font. Text rendering will be disabled.");
        }
    }

    // 5.3. Инициализация генератора мира (НОВОЕ)
    m_worldGenerator = std::make_shared<WorldGenerator>(
        m_tileMap, m_engine, this, m_player);

    m_isDisplayingTerminalInfo = false;

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

        case SDLK_e:
            // Начало взаимодействия с объектами
            handleInteraction();
            break;

        case SDLK_ESCAPE:
            // Если отображается информация терминала, скрываем её
            if (m_isDisplayingTerminalInfo && m_currentInteractingTerminal) {
                m_isDisplayingTerminalInfo = false;
                m_currentInteractingTerminal = nullptr;
                LOG_INFO("Terminal info closed with ESC key");
                return; // Прерываем обработку, чтобы ESC не влиял на другие системы
            }
            // В противном случае продолжаем стандартную обработку
            break;
        }
    }
    // Добавляем обработку отпускания клавиши E для системы каст-времени
    else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_e) {
        // Если идет взаимодействие с дверью, отменяем его при отпускании клавиши
        if (m_currentInteractingDoor) {
            m_currentInteractingDoor->cancelInteraction();
            m_currentInteractingDoor = nullptr;
            m_isInteractingWithDoor = false;
            LOG_INFO("Interaction with door cancelled (key released)");
        }

        // Сбрасываем флаг требования отпускания клавиши у всех дверей
        for (auto& obj : m_interactiveObjects) {
            if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
                if (doorObj->isRequiringKeyRelease()) {
                    doorObj->resetKeyReleaseRequirement();
                }
            }
        }
    }

    // Передаем события в базовый класс
    Scene::handleEvent(event);
}

std::string MapScene::truncateText(const std::string& text, size_t maxLength) {
    if (text.length() <= maxLength) {
        return text;
    }

    // Отрезаем часть текста и добавляем многоточие
    return text.substr(0, maxLength - 3) + "...";
}

void MapScene::update(float deltaTime) {
    // 1. Обнаружение нажатий клавиш и обновление игрока
    if (m_player) {
        m_player->detectKeyInput();
        m_player->update(deltaTime);
    }

    // 2. Обновление камеры
    m_camera->update(deltaTime);

    // Проверяем состояние текущего взаимодействия с дверью
    if (m_isInteractingWithDoor && m_currentInteractingDoor) {
        // Если дверь перестала взаимодействовать (завершила каст или игрок слишком далеко)
        if (!m_currentInteractingDoor->isInteracting()) {
            m_currentInteractingDoor = nullptr;
            m_isInteractingWithDoor = false;
        }
        // Проверяем, не отошел ли игрок слишком далеко от двери
        else if (m_player) {
            float playerX = m_player->getFullX();
            float playerY = m_player->getFullY();
            float doorX = m_currentInteractingDoor->getPosition().x;
            float doorY = m_currentInteractingDoor->getPosition().y;

            float dx = playerX - doorX;
            float dy = playerY - doorY;
            float distanceToDoorsSquared = dx * dx + dy * dy;

            // Если игрок отошел дальше радиуса взаимодействия, отменяем взаимодействие
            float interactionRadius = m_currentInteractingDoor->getInteractionRadius();
            if (distanceToDoorsSquared > interactionRadius * interactionRadius) {
                m_currentInteractingDoor->cancelInteraction();
                m_currentInteractingDoor = nullptr;
                m_isInteractingWithDoor = false;
                LOG_INFO("Interaction with door cancelled (player moved away)");
            }
        }
    }

    // Обновление состояния терминала
    if (m_isDisplayingTerminalInfo && m_currentInteractingTerminal) {
        // Проверяем, не отошел ли игрок слишком далеко от терминала
        if (m_player) {
            float playerX = m_player->getFullX();
            float playerY = m_player->getFullY();
            float terminalX = m_currentInteractingTerminal->getPosition().x;
            float terminalY = m_currentInteractingTerminal->getPosition().y;

            float dx = playerX - terminalX;
            float dy = playerY - terminalY;
            float distanceSquared = dx * dx + dy * dy;

            // Если игрок отошел дальше радиуса взаимодействия, скрываем информацию
            float interactionRadius = m_currentInteractingTerminal->getInteractionRadius();
            if (distanceSquared > interactionRadius * interactionRadius * 1.5f) {
                m_currentInteractingTerminal = nullptr;
                m_isDisplayingTerminalInfo = false;
                LOG_INFO("Terminal info hidden (player moved away)");
            }
        }
    }

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
            // Создаем подсказку с информацией об объекте
            std::string objectName = nearestObject->getName();
            InteractiveType objectType = nearestObject->getInteractiveType();

            // Проверяем, есть ли у объекта уже своя подсказка
            // Для дверей попробуем использовать их собственную подсказку
            if (auto doorObj = std::dynamic_pointer_cast<Door>(nearestObject)) {
                // Для дверей используем их собственную подсказку
                m_interactionPrompt = doorObj->getInteractionHint();
                m_showInteractionPrompt = true;
                m_interactionPromptTimer = 0.0f;
            }
            // Добавляем обработку для терминалов
            else if (auto terminalObj = std::dynamic_pointer_cast<Terminal>(nearestObject)) {
                // Для терминалов используем их собственную подсказку
                m_interactionPrompt = terminalObj->getInteractionHint();
                m_showInteractionPrompt = true;
                m_interactionPromptTimer = 0.0f;
            }
            else {
                // Для других типов объектов формируем подсказку, как раньше
                std::string actionText = "interact with";
                std::string typeText = "";

                switch (objectType) {
                case InteractiveType::PICKUP:
                    actionText = "pick up";

                    // Если это предмет, можем получить дополнительную информацию о типе предмета
                    if (auto pickupItem = std::dynamic_pointer_cast<PickupItem>(nearestObject)) {
                        PickupItem::ItemType itemType = pickupItem->getItemType();
                        switch (itemType) {
                        case PickupItem::ItemType::RESOURCE:
                            typeText = " [Resource]";
                            break;
                        case PickupItem::ItemType::WEAPON:
                            typeText = " [Weapon]";
                            break;
                        case PickupItem::ItemType::ARMOR:
                            typeText = " [Armor]";
                            break;
                        case PickupItem::ItemType::CONSUMABLE:
                            typeText = " [Consumable]";
                            break;
                        case PickupItem::ItemType::KEY:
                            typeText = " [Key]";
                            break;
                        default:
                            typeText = " [Item]";
                            break;
                        }
                    }
                    break;
                case InteractiveType::DOOR:
                    actionText = "open/close";
                    typeText = " [Door]";
                    break;
                case InteractiveType::SWITCH:
                    actionText = "activate";
                    typeText = " [Switch]";
                    break;
                case InteractiveType::TERMINAL:
                    actionText = "use";
                    typeText = " [Terminal]";
                    break;
                case InteractiveType::CONTAINER:
                    actionText = "open";
                    typeText = " [Container]";
                    break;
                default:
                    // Для других типов используем подсказку по умолчанию
                    break;
                }

                // Формируем финальный текст подсказки
                // Сокращаем имя объекта, если оно слишком длинное
                std::string truncatedName = truncateText(objectName, 20); // Ограничиваем длину имени объекта
                m_interactionPrompt = "Press E to " + actionText + " " + objectName + typeText;
                m_showInteractionPrompt = true;
                m_interactionPromptTimer = 0.0f;
            }
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

    // Отрисовка информации терминала
    if (m_isDisplayingTerminalInfo && m_currentInteractingTerminal) {
        renderTerminalInfo(renderer);
    }
}

void MapScene::generateTestMap() {
    // Очищаем список интерактивных объектов
    m_interactiveObjects.clear();
    m_openDoors.clear();

    // Используем WorldGenerator для генерации карты
    srand(static_cast<unsigned int>(time(nullptr)));
    int biomeIndex = rand() % 4 + 1; // 1-4 (FOREST, DESERT, TUNDRA, VOLCANIC)
    m_currentBiome = biomeIndex;

    // Генерируем карту и получаем стартовую позицию игрока
    auto playerStartPos = m_worldGenerator->generateTestMap(m_currentBiome);

    // Устанавливаем игрока на стартовую позицию
    if (m_player) {
        m_player->setPosition(playerStartPos.first, playerStartPos.second, 0.0f);
        m_player->setSubX(0.5f);
        m_player->setSubY(0.5f);
    }
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

                // Отрисовка указателя направления персонажа
                if (m_player && m_player->isShowingDirectionIndicator()) {
                    m_player->renderDirectionIndicator(renderer, m_isoRenderer.get(), centerX, centerY);
                }
            }
            break;
        }
        case RenderObject::Type::INTERACTIVE: {
            auto& interactive = obj.interactiveObj;
            if (interactive) {
                SDL_Color color = interactive->getColor();
                float height = obj.z;

                // Обработка двери как визуального объекта
                if (auto doorObj = std::dynamic_pointer_cast<Door>(interactive)) {
                    height = doorObj->getHeight();

                    bool isVertical = doorObj->isVertical();

                    // Цвета основаны на состоянии (открыта/закрыта)
                    SDL_Color topColor = color;

                    // Создаем дополнительные цвета для граней
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

                    // Расчет размера и положения двери в зависимости от ориентации
                    float doorWidth = 0.3f;  // Ширина двери (доля от тайла)
                    float doorLength = 0.8f; // Длина двери (доля от тайла)

                    float offsetX = 0.0f;
                    float offsetY = 0.0f;

                    if (isVertical) {
                        // Вертикальная дверь (проход север-юг)
                        offsetX = (1.0f - doorWidth) / 2.0f;
                    }
                    else {
                        // Горизонтальная дверь (проход запад-восток)
                        offsetY = (1.0f - doorWidth) / 2.0f;
                    }

                    // Увеличиваем ширину для лучшей видимости в зависимости от биома
                    switch (m_currentBiome) {
                    case 1: // FOREST - ветви более протяженные
                        doorWidth = 0.4f;
                        break;
                    case 2: // DESERT - песчаные завалы более широкие
                        doorWidth = 0.5f;
                        break;
                    case 3: // TUNDRA - ледяные преграды тонкие
                        doorWidth = 0.25f;
                        break;
                    case 4: // VOLCANIC - каменные завалы широкие
                        doorWidth = 0.5f;
                        break;
                    }

                    if (isVertical) {
                        // Рендерим вертикальную дверь
                        m_tileRenderer->addVolumetricTile(
                            obj.x + offsetX,
                            obj.y + (1.0f - doorLength) / 2.0f,
                            height,
                            nullptr, nullptr, nullptr,
                            topColor, leftColor, rightColor,
                            obj.priority
                        );
                    }
                    else {
                        // Рендерим горизонтальную дверь
                        m_tileRenderer->addVolumetricTile(
                            obj.x + (1.0f - doorLength) / 2.0f,
                            obj.y + offsetY,
                            height,
                            nullptr, nullptr, nullptr,
                            topColor, leftColor, rightColor,
                            obj.priority
                        );
                    }
                }
                else if (auto terminalObj = std::dynamic_pointer_cast<Terminal>(interactive)) {
                    // Специальная визуализация для терминалов - делаем их больше
                    height = obj.z;
                    SDL_Color color = terminalObj->getColor();

                    // Создаем цвета граней с более яркими контрастами
                    SDL_Color topColor = color;

                    // Верхняя часть с ярким цветом ("экран" терминала)
                    SDL_Color screenColor;
                    if (terminalObj->shouldShowIndicator()) {
                        // Для непрочитанных терминалов делаем намного ярче с пульсацией
                        float pulseEffect = 0.3f * sinf(SDL_GetTicks() / 150.0f); // Более быстрая и заметная пульсация
                        screenColor = {
                            static_cast<Uint8>(std::min(255, color.r + 100)),
                            static_cast<Uint8>(std::min(255, color.g + 100)),
                            static_cast<Uint8>(std::min(255, color.b + 100)),
                            255 // Полная непрозрачность
                        };
                    }
                    else {
                        // Для уже прочитанных - стандартная яркость
                        screenColor = {
                            static_cast<Uint8>(std::min(255, color.r + 50)),
                            static_cast<Uint8>(std::min(255, color.g + 50)),
                            static_cast<Uint8>(std::min(255, color.b + 50)),
                            color.a
                        };
                    }

                    // Боковые части (корпус) темнее
                    SDL_Color leftColor = {
                        static_cast<Uint8>(color.r * 0.6f),
                        static_cast<Uint8>(color.g * 0.6f),
                        static_cast<Uint8>(color.b * 0.6f),
                        color.a
                    };

                    SDL_Color rightColor = {
                        static_cast<Uint8>(color.r * 0.4f),
                        static_cast<Uint8>(color.g * 0.4f),
                        static_cast<Uint8>(color.b * 0.4f),
                        color.a
                    };

                    // Добавляем плавающий эффект и пульсацию для терминалов
                    float pulseEffect;
                    if (terminalObj->shouldShowIndicator()) {
                        // Более заметная пульсация для непрочитанных терминалов
                        pulseEffect = 0.3f * sinf(SDL_GetTicks() / 150.0f);
                    }
                    else {
                        // Обычная пульсация для прочитанных терминалов
                        pulseEffect = 0.1f * sinf(SDL_GetTicks() / 200.0f);
                    }

                    // Основная база терминала (нижняя часть)
                    m_tileRenderer->addVolumetricTile(
                        obj.x, obj.y, height * 0.6f,
                        nullptr, nullptr, nullptr,
                        topColor, leftColor, rightColor,
                        obj.priority
                    );

                    // Экран терминала (верхняя часть)
                    m_tileRenderer->addVolumetricTile(
                        obj.x, obj.y, height + pulseEffect,
                        nullptr, nullptr, nullptr,
                        screenColor, screenColor, screenColor,
                        obj.priority + 0.1f
                    );

                    // Отображаем индикатор над терминалом, если он еще не прочитан
                    if (terminalObj->shouldShowIndicator()) {
                        // Индикаторы символами отключены - используем только эффекты на самом терминале
                    }
                }
                else if (auto pickupItem = std::dynamic_pointer_cast<PickupItem>(interactive)) {
                    // Эффект парения для предметов
                    height += 0.15f * sinf(SDL_GetTicks() / 500.0f);

                    // Стандартные цвета для других интерактивных объектов
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
                else {
                    // Стандартные цвета для других интерактивных объектов
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
            }
            break;
        }
        }
    }

    // 9. Рендерим все тайлы в правильном порядке
    m_tileRenderer->render(renderer, centerX, centerY);

    if (m_player) {
        m_player->renderDirectionIndicator(renderer, m_isoRenderer.get(), centerX, centerY);
    }

    // 10. Отрисовываем индикаторы прогресса над дверями
    for (const auto& obj : m_interactiveObjects) {
        if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
            // Отрисовка прогресс-бара над дверью
            doorObj->render(renderer, m_isoRenderer.get(), centerX, centerY);
        }
    }
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

void MapScene::handleInteraction() {
    if (!m_player) return;

    float playerX = m_player->getFullX();
    float playerY = m_player->getFullY();

    // Если уже идет взаимодействие с дверью, не ищем новые объекты
    if (m_isInteractingWithDoor && m_currentInteractingDoor) {
        // Продолжаем текущее взаимодействие
        return;
    }

    std::shared_ptr<InteractiveObject> nearestObject = findNearestInteractiveObject(playerX, playerY);

    // Если уже отображается информация терминала и найден ближайший объект
    if (m_isDisplayingTerminalInfo && m_currentInteractingTerminal && nearestObject) {
        // Проверяем, является ли ближайший объект текущим терминалом
        if (nearestObject.get() == m_currentInteractingTerminal.get()) {
            // Закрываем окно терминала при повторном нажатии E
            m_isDisplayingTerminalInfo = false;
            m_currentInteractingTerminal = nullptr;
            LOG_INFO("Terminal info closed by pressing E again");
            return;
        }
    }

    if (nearestObject && nearestObject->isInteractable()) {
        // Проверяем, является ли объект дверью
        if (auto doorObj = std::dynamic_pointer_cast<Door>(nearestObject)) {
            // Начинаем процесс взаимодействия с дверью (с каст-временем)
            if (doorObj->startInteraction()) {
                m_currentInteractingDoor = doorObj;
                m_isInteractingWithDoor = true;
                LOG_INFO("Started interaction process with door " + doorObj->getName());
            }
        }
        // Проверяем, является ли объект терминалом
        else if (auto terminalObj = std::dynamic_pointer_cast<Terminal>(nearestObject)) {
            // Начинаем отображение информации терминала
            if (terminalObj->interact(m_player.get())) {
                m_currentInteractingTerminal = terminalObj;
                m_isDisplayingTerminalInfo = true;
                LOG_INFO("Started displaying information from terminal " + terminalObj->getName());

                // Формируем сообщение о взаимодействии
                std::string actionMessage = "Accessing " + nearestObject->getName();

                // Отображаем подсказку с результатом взаимодействия
                m_showInteractionPrompt = true;
                m_interactionPromptTimer = 0.0f;
                m_interactionPrompt = actionMessage;
            }
        }
        else {
            // Для других объектов используем обычное мгновенное взаимодействие
            if (nearestObject->interact(m_player.get())) {
                // Взаимодействие успешно
                LOG_INFO("Interaction with " + nearestObject->getName() + " successful");

                // Формируем сообщение о взаимодействии в зависимости от типа объекта
                std::string actionMessage;

                if (auto pickupItem = std::dynamic_pointer_cast<PickupItem>(nearestObject)) {
                    // Для предметов
                    PickupItem::ItemType itemType = pickupItem->getItemType();
                    std::string itemTypeStr;

                    switch (itemType) {
                    case PickupItem::ItemType::RESOURCE:
                        itemTypeStr = " [Resource]";
                        break;
                    case PickupItem::ItemType::WEAPON:
                        itemTypeStr = " [Weapon]";
                        break;
                    case PickupItem::ItemType::ARMOR:
                        itemTypeStr = " [Armor]";
                        break;
                    case PickupItem::ItemType::CONSUMABLE:
                        itemTypeStr = " [Consumable]";
                        break;
                    case PickupItem::ItemType::KEY:
                        itemTypeStr = " [Key]";
                        break;
                    default:
                        itemTypeStr = " [Item]";
                        break;
                    }

                    actionMessage = "Picked up " + nearestObject->getName() + itemTypeStr;
                }
                else {
                    // Для других интерактивных объектов
                    switch (nearestObject->getInteractiveType()) {
                    case InteractiveType::SWITCH:
                        actionMessage = "Activated " + nearestObject->getName();
                        break;
                    case InteractiveType::TERMINAL:
                        actionMessage = "Used " + nearestObject->getName();
                        break;
                    case InteractiveType::CONTAINER:
                        actionMessage = "Opened " + nearestObject->getName();
                        break;
                    default:
                        actionMessage = "Interacted with " + nearestObject->getName();
                        break;
                    }
                }

                // Отображаем подсказку с результатом взаимодействия
                m_showInteractionPrompt = true;
                m_interactionPromptTimer = 0.0f;
                m_interactionPrompt = actionMessage;
            }
        }
    }
    else {
        LOG_INFO("No interactive objects in range");
    }
}

std::shared_ptr<InteractiveObject> MapScene::findNearestInteractiveObject(float playerX, float playerY) {
    std::shared_ptr<InteractiveObject> nearest = nullptr;
    float minDistanceSquared = std::numeric_limits<float>::max();

    // Получаем направление взгляда игрока
    float playerDirX = 0.0f;
    float playerDirY = 0.0f;

    if (m_player) {
        playerDirX = m_player->getDirectionX();
        playerDirY = m_player->getDirectionY();
    }

    // Если направление не определено (игрок стоит на месте), используем последнее известное направление
    if (std::abs(playerDirX) < 0.01f && std::abs(playerDirY) < 0.01f) {
        // Если нет направления, будем считать, что игрок может взаимодействовать
        // с объектами в любом направлении (классическое поведение)
        playerDirX = 0.0f;
        playerDirY = 0.0f;
    }

    for (auto& object : m_interactiveObjects) {
        if (!object->isActive() || !object->isInteractable()) continue;

        // Координаты объекта
        float objectX = object->getPosition().x;
        float objectY = object->getPosition().y;

        // Вычисление вектора от игрока к объекту
        float dx = objectX - playerX;
        float dy = objectY - playerY;
        float distanceSquared = dx * dx + dy * dy;

        // Получение радиуса взаимодействия объекта
        float interactionRadius = object->getInteractionRadius();
        float radiusSquared = interactionRadius * interactionRadius;

        // Если объект находится в радиусе взаимодействия
        if (distanceSquared <= radiusSquared) {
            // Проверяем угол между направлением игрока и направлением к объекту,
            // если у игрока есть направление движения
            bool objectInFront = true; // По умолчанию считаем, что объект перед игроком

            if (std::abs(playerDirX) > 0.01f || std::abs(playerDirY) > 0.01f) {
                // Нормализуем вектор направления к объекту
                float length = std::sqrt(distanceSquared);
                if (length > 0.0001f) { // Избегаем деления на ноль
                    float normDx = dx / length;
                    float normDy = dy / length;

                    // Вычисляем скалярное произведение (dot product) между направлением игрока
                    // и нормализованным вектором направления к объекту
                    float dotProduct = playerDirX * normDx + playerDirY * normDy;

                    // Если скалярное произведение положительное, объект находится перед игроком
                    // (в пределах 180-градусного сектора)
                    objectInFront = dotProduct > 0.0f;

                    // Для более узкого сектора можно использовать порог больше 0
                    // например, dotProduct > 0.5f будет давать 60-градусный сектор
                    // а dotProduct > 0.7071f будет давать 45-градусный сектор

                    // Используем более узкий сектор для точности выбора объекта
                    objectInFront = dotProduct > 0.5f; // Примерно 60 градусов в каждую сторону
                }
            }

            // Если объект находится перед игроком и ближе предыдущего ближайшего
            if (objectInFront && distanceSquared < minDistanceSquared) {
                minDistanceSquared = distanceSquared;
                nearest = object;
            }
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
    // Проверяем, должна ли подсказка отображаться
    if (!m_showInteractionPrompt || m_interactionPrompt.empty()) {
        return;
    }

    // Получаем размеры окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Проверяем, доступен ли ResourceManager и есть ли шрифт
    if (m_engine && m_engine->getResourceManager() &&
        m_engine->getResourceManager()->hasFont("default")) {

        // Цвет текста (ярко-белый)
        SDL_Color textColor = { 255, 255, 255, 255 };

        // Создаем временную текстуру с текстом, чтобы определить её размеры
        TTF_Font* font = m_engine->getResourceManager()->getFont("default");
        if (!font) return;

        // Получаем размеры текста
        int textWidth, textHeight;
        TTF_SizeText(font, m_interactionPrompt.c_str(), &textWidth, &textHeight);

        // Добавляем отступы
        int padding = 20;
        int promptWidth = textWidth + padding * 2;
        int promptHeight = textHeight + padding;

        // Минимальная ширина подложки для коротких сообщений
        int minWidth = 300;
        if (promptWidth < minWidth) {
            promptWidth = minWidth;
        }

        // Максимальная ширина подложки, не выходящая за пределы экрана
        int maxWidth = windowWidth - 60; // Оставляем отступ по 30 пикселей с каждой стороны
        if (promptWidth > maxWidth) {
            promptWidth = maxWidth;
        }

        // Отрисовываем полупрозрачный прямоугольник внизу экрана
        SDL_Rect promptRect = {
            windowWidth / 2 - promptWidth / 2,
            windowHeight - 60,
            promptWidth,
            promptHeight
        };

        // Устанавливаем цвет прямоугольника (полупрозрачный черный)
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);  // Немного больше непрозрачности
        SDL_RenderFillRect(renderer, &promptRect);

        // Рисуем рамку с лучшим визуальным эффектом
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);  // Серая рамка
        SDL_RenderDrawRect(renderer, &promptRect);

        // Добавляем внутреннюю рамку для эффекта углубления
        SDL_Rect innerRect = {
            promptRect.x + 2,
            promptRect.y + 2,
            promptRect.w - 4,
            promptRect.h - 4
        };
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);  // Темно-серая внутренняя рамка
        SDL_RenderDrawRect(renderer, &innerRect);

        // Для очень длинных текстов, которые могут не поместиться даже в максимальную ширину,
        // мы можем разбить текст на несколько строк или обрезать, но пока просто отрисуем
        // с урезанной шириной текстуры

        // Отрисовываем текст с использованием ResourceManager
        m_engine->getResourceManager()->renderText(
            renderer,
            m_interactionPrompt,
            "default",
            windowWidth / 2,  // X-координата (центр экрана)
            windowHeight - 60 + promptHeight / 2, // Y-координата (центр подложки)
            textColor
        );
    }
    else {
        // Отображаем подсказку только при первом взаимодействии, если нет доступных шрифтов
        static std::string lastPrompt;
        if (lastPrompt != m_interactionPrompt) {
            lastPrompt = m_interactionPrompt;
            LOG_INFO("Interaction prompt: " + m_interactionPrompt);
        }

        // Если идет взаимодействие с дверью, рисуем полоску прогресса
        if (m_isInteractingWithDoor && m_currentInteractingDoor) {
            float progress = m_currentInteractingDoor->getInteractionProgress();

            // Получаем размеры окна
            int windowWidth, windowHeight;
            SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

            // Создаем полоску прогресса под основной подсказкой
            int progressWidth = 300;
            int progressHeight = 8;

            SDL_Rect progressBg = {
                windowWidth / 2 - progressWidth / 2,
                windowHeight - 40, // Расположение под текстом подсказки
                progressWidth,
                progressHeight
            };

            // Фон полоски (темно-серый, полупрозрачный)
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 180);
            SDL_RenderFillRect(renderer, &progressBg);

            // Заполненная часть полоски (зеленая)
            SDL_Rect progressFill = progressBg;
            progressFill.w = static_cast<int>(progressFill.w * progress);

            // Цвет прогресс-бара зависит от того, открываем или закрываем
            if (m_currentInteractingDoor->isOpen()) {
                // Закрываем дверь - красный прогресс-бар
                SDL_SetRenderDrawColor(renderer, 220, 50, 50, 220);
            }
            else {
                // Открываем дверь - зеленый прогресс-бар
                SDL_SetRenderDrawColor(renderer, 50, 220, 50, 220);
            }
            SDL_RenderFillRect(renderer, &progressFill);

            // Рамка для полоски
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
            SDL_RenderDrawRect(renderer, &progressBg);
        }
    }
}   

void MapScene::rememberDoorPosition(int x, int y, const std::string& name) {
    // Добавляем информацию об открытой двери в список
    m_openDoors.push_back({ x, y, name });
    LOG_INFO("Remembered open door " + name + " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

bool MapScene::isOpenDoorTile(int x, int y) const {
    // Проверяем, есть ли в списке открытая дверь с указанными координатами
    for (const auto& doorInfo : m_openDoors) {
        if (doorInfo.tileX == x && doorInfo.tileY == y) {
            return true;
        }
    }
    return false;
}

void MapScene::closeDoorAtPosition(int x, int y) {
    // Ищем дверь с указанными координатами
    for (auto it = m_openDoors.begin(); it != m_openDoors.end(); ++it) {
        if (it->tileX == x && it->tileY == y) {
            // Запоминаем имя двери
            std::string doorName = it->name;

            // Удаляем информацию об открытой двери из списка
            m_openDoors.erase(it);

            // Сначала меняем тип тайла на WALL, чтобы гарантировать непроходимость
            if (m_tileMap) {
                m_tileMap->setTileType(x, y, TileType::WALL);
                LOG_DEBUG("Door tile updated to WALL before creating new door entity");
            }

            // Используем WorldGenerator для создания новой двери
            if (m_worldGenerator) {
                m_worldGenerator->createTestDoor(static_cast<float>(x), static_cast<float>(y), doorName);
            }

            LOG_INFO("Closed door " + doorName + " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            break;
        }
    }
}

void MapScene::forgetDoorPosition(int x, int y) {
    // Ищем дверь с указанными координатами в списке открытых дверей
    for (auto it = m_openDoors.begin(); it != m_openDoors.end(); ++it) {
        if (it->tileX == x && it->tileY == y) {
            LOG_INFO("Removed door " + it->name + " from open doors list");
            m_openDoors.erase(it);
            break;
        }
    }
}

void MapScene::renderTerminalInfo(SDL_Renderer* renderer) {
    if (!m_isDisplayingTerminalInfo || !m_currentInteractingTerminal || !renderer) {
        return;
    }

    // Получаем размеры окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Проверяем, доступен ли ResourceManager и есть ли шрифт
    if (m_engine && m_engine->getResourceManager() &&
        m_engine->getResourceManager()->hasFont("default")) {

        // Получаем шрифт для отображения текста
        TTF_Font* font = m_engine->getResourceManager()->getFont("default");
        if (!font) return;

        // Эффект "компрометации системы" - кратковременное появление предупреждения
        // Каждые 3 секунды на 0.8 секунд появляется предупреждение
        Uint32 currentTime = SDL_GetTicks();
        bool showCompromisedMessage = (currentTime % 3800) < 800; // 0.8 секунды каждые 3.8 секунды

        // Получаем записи терминала
        const auto& entries = m_currentInteractingTerminal->getEntries();

        // Проверяем наличие записей
        if (entries.size() < 2) return;

        // Определяем, какую запись показывать
        int selectedIndex = -1;

        // Инициализируем индекс при первом вызове
        if (m_currentInteractingTerminal->getSelectedEntryIndex() < 0) {
            // Пропускаем первую запись, если она совпадает с названием терминала
            int startIndex = (entries[0].first == m_currentInteractingTerminal->getName()) ? 1 : 0;

            // Для выбора используем только содержательные записи, исключая предупреждение (последняя запись)
            int endIndex = entries.size() - 1;

            if (startIndex < endIndex) {
                // Генерируем случайный индекс для интересных записей
                selectedIndex = startIndex + rand() % (endIndex - startIndex);
                // Сохраняем выбранный индекс
                m_currentInteractingTerminal->setSelectedEntryIndex(selectedIndex);
            }
            else {
                // Если нет других записей, используем первую
                selectedIndex = startIndex;
                m_currentInteractingTerminal->setSelectedEntryIndex(selectedIndex);
            }
        }
        else {
            // Используем ранее выбранный индекс
            selectedIndex = m_currentInteractingTerminal->getSelectedEntryIndex();
        }

        // Определяем текущий контент и заголовок
        std::string headerText;
        std::string contentText;
        SDL_Color textColor;
        SDL_Color bgColor;

        if (showCompromisedMessage && entries.size() > 0) {
            // Показываем предупреждение о компрометации (используем последнюю запись)
            size_t warningIndex = entries.size() - 1;
            headerText = entries[warningIndex].first;
            contentText = entries[warningIndex].second;

            // Яркий красный цвет для предупреждения
            textColor = { 255, 70, 70, 255 };

            // Темный фон с красным оттенком
            bgColor = { 40, 0, 0, 220 };
        }
        else if (selectedIndex >= 0 && selectedIndex < entries.size()) {
            // Показываем выбранную запись в обычном цвете
            headerText = entries[selectedIndex].first;
            contentText = entries[selectedIndex].second;

            // Определяем цвет в зависимости от типа терминала
            switch (m_currentInteractingTerminal->getTerminalType()) {
            case Terminal::TerminalType::RESEARCH_SENSOR:
                textColor = { 220, 255, 255, 255 }; // Светло-бирюзовый
                bgColor = { 0, 45, 45, 220 }; // Темно-бирюзовый фон
                break;
            case Terminal::TerminalType::ANCIENT_CONSOLE:
                textColor = { 230, 200, 255, 255 }; // Светло-фиолетовый
                bgColor = { 40, 0, 60, 220 }; // Темно-фиолетовый фон
                break;
            case Terminal::TerminalType::EMERGENCY_BEACON:
                textColor = { 255, 220, 180, 255 }; // Светло-оранжевый
                bgColor = { 60, 20, 0, 220 }; // Темно-оранжевый фон
                break;
            case Terminal::TerminalType::SCIENCE_STATION:
                textColor = { 180, 220, 255, 255 }; // Светло-синий
                bgColor = { 0, 30, 60, 220 }; // Темно-синий фон
                break;
            default:
                textColor = { 255, 255, 255, 255 }; // Белый
                bgColor = { 0, 0, 0, 220 }; // Черный фон
            }
        }
        else {
            // Если нет подходящих записей, показываем стандартный текст
            headerText = m_currentInteractingTerminal->getName();
            contentText = "No data available.";
            textColor = { 255, 255, 255, 255 }; // Белый
            bgColor = { 0, 0, 0, 220 }; // Черный фон
        }

        // Ширина и высота информационного окна
        int infoWidth = windowWidth / 2 + 100;
        int infoHeight = windowHeight / 2 + 50;

        // Отрисовываем полупрозрачный прямоугольник для фона
        SDL_Rect infoRect = {
            windowWidth / 2 - infoWidth / 2,
            windowHeight / 2 - infoHeight / 2,
            infoWidth,
            infoHeight
        };

        // Устанавливаем цвет фона
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &infoRect);

        // Рисуем рамку для окна терминала
        // Для предупреждения рисуем красную рамку
        if (showCompromisedMessage) {
            SDL_SetRenderDrawColor(renderer, 255, 70, 70, 200);
        }
        else {
            SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, 180);
        }
        SDL_RenderDrawRect(renderer, &infoRect);

        // Отображаем название терминала вверху (всегда)
        std::string terminalTitle = m_currentInteractingTerminal->getName();

        // Рисуем заголовок
        SDL_Surface* titleSurface = TTF_RenderText_Blended(font, terminalTitle.c_str(), textColor);
        if (titleSurface) {
            SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
            if (titleTexture) {
                SDL_Rect titleRect;
                titleRect.w = titleSurface->w;
                titleRect.h = titleSurface->h;
                titleRect.x = windowWidth / 2 - titleRect.w / 2;
                titleRect.y = infoRect.y + 20;

                SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
                SDL_DestroyTexture(titleTexture);
            }
            SDL_FreeSurface(titleSurface);
        }

        // Отрисовываем разделительную линию под заголовком
        SDL_Rect dividerRect = {
            infoRect.x + 40,
            infoRect.y + 55,
            infoRect.w - 80,
            1
        };
        SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, 150);
        SDL_RenderFillRect(renderer, &dividerRect);

        // Максимальная ширина текста для размещения внутри окна
        int maxTextWidth = infoWidth - 100; // Оставляем отступы по бокам

        // Вертикальное смещение
        int yOffset = 80;

        // Отображаем заголовок записи
        SDL_Surface* headerSurface = TTF_RenderText_Blended(font, headerText.c_str(), textColor);
        if (headerSurface) {
            SDL_Texture* headerTexture = SDL_CreateTextureFromSurface(renderer, headerSurface);
            if (headerTexture) {
                SDL_Rect headerRect;
                headerRect.w = headerSurface->w;
                headerRect.h = headerSurface->h;
                headerRect.x = infoRect.x + 40;
                headerRect.y = infoRect.y + yOffset;

                SDL_RenderCopy(renderer, headerTexture, NULL, &headerRect);
                SDL_DestroyTexture(headerTexture);
            }
            SDL_FreeSurface(headerSurface);
        }

        // Создаем цвет для содержимого (немного прозрачнее)
        SDL_Color contentColor = { textColor.r, textColor.g, textColor.b, 200 };

        // Разбиваем текст на строки, чтобы поместить их в окно
        std::vector<std::string> lines;

        // Разделяем текст на строки максимум по 40-45 символов
        int maxLineLength = 40;

        int startPos = 0;
        while (startPos < contentText.length()) {
            int endPos = startPos + maxLineLength;
            if (endPos >= contentText.length()) {
                // Если это конец текста, добавляем оставшуюся часть
                lines.push_back(contentText.substr(startPos));
                break;
            }

            // Находим последний пробел перед endPos
            int lastSpace = contentText.rfind(' ', endPos);
            if (lastSpace > startPos) {
                // Если есть пробел, разбиваем по нему
                lines.push_back(contentText.substr(startPos, lastSpace - startPos));
                startPos = lastSpace + 1;
            }
            else {
                // Если пробела нет, просто разбиваем по maxLineLength
                lines.push_back(contentText.substr(startPos, maxLineLength));
                startPos += maxLineLength;
            }
        }

        // Отображаем каждую строку содержимого
        int lineOffset = 30; // Начальное смещение от заголовка
        for (const auto& line : lines) {
            SDL_Surface* lineSurface = TTF_RenderText_Blended(font, line.c_str(), contentColor);
            if (lineSurface) {
                SDL_Texture* lineTexture = SDL_CreateTextureFromSurface(renderer, lineSurface);
                if (lineTexture) {
                    SDL_Rect lineRect;
                    lineRect.w = lineSurface->w;
                    lineRect.h = lineSurface->h;
                    lineRect.x = infoRect.x + 45; // Небольшой отступ от края
                    lineRect.y = infoRect.y + yOffset + lineOffset;

                    SDL_RenderCopy(renderer, lineTexture, NULL, &lineRect);
                    SDL_DestroyTexture(lineTexture);
                }
                SDL_FreeSurface(lineSurface);
            }

            lineOffset += 25; // Переходим к следующей строке
        }

        // Добавляем подсказку для закрытия внизу
        SDL_Surface* promptSurface = TTF_RenderText_Blended(font, "Press E to close",
            { textColor.r, textColor.g, textColor.b, 180 });
        if (promptSurface) {
            SDL_Texture* promptTexture = SDL_CreateTextureFromSurface(renderer, promptSurface);
            if (promptTexture) {
                SDL_Rect promptRect;
                promptRect.w = promptSurface->w;
                promptRect.h = promptSurface->h;
                promptRect.x = windowWidth / 2 - promptRect.w / 2;
                promptRect.y = infoRect.y + infoHeight - 25;

                SDL_RenderCopy(renderer, promptTexture, NULL, &promptRect);
                SDL_DestroyTexture(promptTexture);
            }
            SDL_FreeSurface(promptSurface);
        }
    }
    else {
        // Если шрифты недоступны, просто выводим в лог
        LOG_INFO("Terminal info display: " + m_currentInteractingTerminal->getName());
    }
}