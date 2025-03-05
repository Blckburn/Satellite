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
    : Scene(name), m_engine(engine), m_showDebug(false) {
    // Остальные члены будут инициализированы в методе initialize()
}

MapScene::~MapScene() {
    // Очистка ресурсов
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

    // 3.5. Инициализация EntityManager
    m_entityManager = std::make_shared<EntityManager>(m_tileMap);

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

    // Добавляем игрока в EntityManager как сущность
    m_entityManager->addEntity(m_player);

    // 5.1. Связываем игрока с системой коллизий
    if (m_player && m_collisionSystem) {
        m_player->setCollisionSystem(m_collisionSystem.get());
        std::cout << "Player successfully linked to CollisionSystem" << std::endl;
    }
    else {
        std::cerr << "Warning: Failed to link player to collision system" << std::endl;
    }

    // 5.2. Инициализация системы взаимодействия
    m_interactionSystem = std::make_shared<InteractionSystem>(m_player, m_entityManager, m_tileMap);

    // Устанавливаем функцию обратного вызова для создания дверей
    m_interactionSystem->setCreateDoorCallback([this](float x, float y, const std::string& name) {
        this->createDoor(x, y, name);
        });

    // 5.3. Загрузка шрифта для интерфейса
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

    // 5.4. Инициализация генератора мира
    m_worldGenerator = std::make_shared<WorldGenerator>(
        m_tileMap, m_engine, this, m_player);

    // 6. Генерация тестовой карты
    generateTestMap();

    // 7. Настройка камеры для слежения за игроком
    m_camera->setTarget(&m_player->getPosition().x, &m_player->getPosition().y);

    m_renderingSystem = std::make_shared<RenderingSystem>(
        m_tileMap, m_tileRenderer, m_isoRenderer);

    m_uiManager = std::make_shared<UIManager>(m_engine);

    std::cout << "MapScene initialized successfully" << std::endl;

    return true;
}

void MapScene::createDoor(float x, float y, const std::string& name) {
    if (m_worldGenerator) {
        m_worldGenerator->createTestDoor(x, y, name);
    }
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

void MapScene::handleEvent(const SDL_Event& event) {
    // Обработка событий камеры
    m_camera->handleEvent(event);

    // Обработка событий игрока
    if (m_player) {
        m_player->handleEvent(event);
    }

    // Получаем позицию игрока для дополнительной информации
    float playerX = 0.0f;
    float playerY = 0.0f;
    if (m_player) {
        playerX = m_player->getFullX();
        playerY = m_player->getFullY();
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
            LOG_DEBUG("Debug mode: " + std::string(m_showDebug ? "enabled" : "disabled"));
            break;

 case SDLK_e:
{
    // ВАЖНЫЙ ФИХ: Проверяем, идет ли уже взаимодействие с дверью
    bool alreadyInteracting = false;
    
    if (m_interactionSystem) {
        alreadyInteracting = m_interactionSystem->isInteractingWithDoor();
    }
    
    if (alreadyInteracting) {
        LOG_DEBUG("Skipping additional E key processing, door interaction already in progress");
        break; // Прекращаем обработку клавиши
    }
    
    // УЛУЧШЕННЫЙ ПОИСК ОТКРЫТЫХ ДВЕРЕЙ
    // Сначала проверяем, есть ли открытые двери поблизости, с которыми можно взаимодействовать
    std::shared_ptr<Door> openDoorNearby = nullptr;
    float openDoorMinDistance = std::numeric_limits<float>::max();
    
    for (auto& obj : m_entityManager->getInteractiveObjects()) {
        if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
            if (doorObj->isOpen() && doorObj->isActive() && doorObj->isInteractable()) {
                float doorX = doorObj->getPosition().x;
                float doorY = doorObj->getPosition().y;
                float dx = doorX - playerX;
                float dy = doorY - playerY;
                float distanceSq = dx * dx + dy * dy;
                float radius = doorObj->getInteractionRadius();
                
                if (distanceSq <= radius * radius && distanceSq < openDoorMinDistance) {
                    openDoorNearby = doorObj;
                    openDoorMinDistance = distanceSq;
                    LOG_INFO("Found nearby OPEN door: " + doorObj->getName());
                }
            }
        }
    }
    
    // Если нашли открытую дверь поблизости, приоритетно взаимодействуем с ней
    if (openDoorNearby && m_player) {
        LOG_INFO("Prioritizing interaction with open door: " + openDoorNearby->getName());
        
        // Сбрасываем любые проблемные флаги на двери
        if (openDoorNearby->isRequiringKeyRelease()) {
            openDoorNearby->resetKeyReleaseRequirement();
        }
        
        // Прямое взаимодействие с дверью
        bool success = openDoorNearby->interact(m_player.get());
        LOG_INFO(std::string("Direct interaction with open door ") +
            (success ? "succeeded" : "failed"));
        
        if (success) {
            // Если успешно начали взаимодействие, обновляем состояние InteractionSystem
            if (m_interactionSystem) {
                m_interactionSystem->setCurrentInteractingDoor(openDoorNearby);
            }
            break;
        }
    }
    
    // ВАЖНОЕ ИЗМЕНЕНИЕ: Улучшенная диагностика состояния дверей
    LOG_DEBUG("==== E KEY PRESSED - DOOR STATUS CHECK ====");
    LOG_DEBUG("Player position: (" + std::to_string(playerX) + ", " +
        std::to_string(playerY) + ")");

    // Проверяем все двери поблизости от игрока
    {
        bool foundAnyDoors = false;
        float closestDistance = std::numeric_limits<float>::max();
        std::shared_ptr<Door> closestDoor = nullptr;

        for (auto& obj : m_entityManager->getInteractiveObjects()) {
            if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
                foundAnyDoors = true;
                
                float doorX = doorObj->getPosition().x;
                float doorY = doorObj->getPosition().y;
                float dx = doorX - playerX;
                float dy = doorY - playerY;
                float distanceSq = dx * dx + dy * dy;
                float radius = doorObj->getInteractionRadius();
                bool inRange = distanceSq <= radius * radius;

                LOG_DEBUG("Door: " + doorObj->getName() +
                    ", Position: (" + std::to_string(doorX) + ", " + std::to_string(doorY) + ")" +
                    ", Distance: " + std::to_string(sqrt(distanceSq)) +
                    ", Radius: " + std::to_string(radius) +
                    ", IsOpen: " + std::string(doorObj->isOpen() ? "true" : "false") +
                    ", IsActive: " + std::string(doorObj->isActive() ? "true" : "false") +
                    ", IsInteractable: " + std::string(doorObj->isInteractable() ? "true" : "false") +
                    ", Is Interacting: " + std::string(doorObj->isInteracting() ? "true" : "false") +
                    ", In range: " + std::string(inRange ? "YES" : "NO"));
                    
                // Дополнительно проверим тайл на проходимость
                if (m_tileMap && m_tileMap->isValidCoordinate(doorX, doorY)) {
                    MapTile* tile = m_tileMap->getTile(doorX, doorY);
                    if (tile) {
                        LOG_DEBUG("   Tile at door position: walkable=" + 
                            std::string(tile->isWalkable() ? "true" : "false") + 
                            ", type=" + std::to_string(static_cast<int>(tile->getType())));
                    }
                }
                
                // Запоминаем ближайшую дверь
                if (inRange && distanceSq < closestDistance) {
                    closestDistance = distanceSq;
                    closestDoor = doorObj;
                }
            }
        }
        
        if (!foundAnyDoors) {
            LOG_DEBUG("No doors found in the scene!");
        }
        
        // Если нашли ближайшую дверь (не открытую), выполняем с ней взаимодействие напрямую
        if (closestDoor && m_player && !closestDoor->isOpen()) {
            // Проверка: не идет ли уже процесс взаимодействия с этой дверью
            if (!closestDoor->isInteracting()) {
                LOG_DEBUG("Interacting directly with closest door: " + closestDoor->getName());
                
                // Прямое взаимодействие с дверью
                bool success = closestDoor->interact(m_player.get());
                LOG_DEBUG("Direct door interaction " + std::string(success ? "succeeded" : "failed"));
                
                if (success) {
                    // Если дверь теперь в процессе взаимодействия, обновляем InteractionSystem
                    if (m_interactionSystem && closestDoor->isInteracting()) {
                        m_interactionSystem->setCurrentInteractingDoor(closestDoor);
                    }
                    // В случае успеха пропускаем дальнейшую обработку InteractionSystem
                    break;
                }
            }
            else {
                LOG_DEBUG("Closest door is already interacting, skipping direct interaction");
            }
        }
    }

    // Если не взаимодействовали напрямую с дверью, используем стандартную систему взаимодействия
    LOG_DEBUG("Calling InteractionSystem::handleInteraction()");
    if (m_interactionSystem) {
        m_interactionSystem->handleInteraction();
    }
    break;
}

        case SDLK_ESCAPE:
            // Если отображается информация терминала, скрываем её
            if (m_interactionSystem->isDisplayingTerminalInfo()) {
                m_interactionSystem->closeTerminalInfo();
                LOG_INFO("Terminal info closed with ESC key");
                return; // Прерываем обработку, чтобы ESC не влиял на другие системы
            }
            // В противном случае продолжаем стандартную обработку
            break;
        }
    }
    // Обработка отпускания клавиши E
    else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_e) {
        LOG_DEBUG("E key released, resetting key release requirement");
        // Проверяем, есть ли текущие взаимодействия с дверями через EntityManager
        for (auto& obj : m_entityManager->getInteractiveObjects()) {
            if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
                if (doorObj->isRequiringKeyRelease()) {
                    LOG_DEBUG("Resetting key release requirement for door: " + doorObj->getName());
                    doorObj->resetKeyReleaseRequirement();
                }
            }
        }
    }

    // Передаем события в базовый класс
    Scene::handleEvent(event);
}

void MapScene::render(SDL_Renderer* renderer) {
    // Получаем размеры окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Используем RenderingSystem для основного рендеринга
    m_renderingSystem->render(renderer, m_camera, m_player, m_entityManager, m_currentBiome);

    // Используем UIManager для отрисовки интерфейса
    m_uiManager->render(
        renderer,
        m_isoRenderer,
        m_tileMap,
        m_player,
        m_interactionSystem,
        m_showDebug
    );
}

void MapScene::initializeDoors() {
    // Проходим по всем интерактивным объектам через EntityManager
    for (auto& obj : m_entityManager->getInteractiveObjects()) {
        if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
            // Установка системы взаимодействия для каждой двери
            doorObj->setInteractionSystem(m_interactionSystem.get());
            LOG_DEBUG("Door '" + doorObj->getName() + "' initialized with InteractionSystem");
        }
    }

    LOG_INFO("All doors initialized with InteractionSystem");
}

void MapScene::generateTestMap() {
    // Очищаем менеджер сущностей
    if (m_entityManager) {
        m_entityManager->clear();

        // Добавляем игрока обратно после очистки
        if (m_player) {
            m_entityManager->addEntity(m_player);
        }
    }

    // Используем WorldGenerator для генерации карты
    srand(static_cast<unsigned int>(time(nullptr)));
    int biomeIndex = rand() % 4 + 1; // 1-4 (FOREST, DESERT, TUNDRA, VOLCANIC)
    m_currentBiome = biomeIndex;

    // Генерируем карту и получаем стартовую позицию игрока
    auto playerStartPos = m_worldGenerator->generateTestMap(m_currentBiome);
    m_worldGenerator->generateDoors(0.4f, 8);

    // Устанавливаем игрока на стартовую позицию
    if (m_player) {
        m_player->setPosition(playerStartPos.first, playerStartPos.second, 0.0f);
        m_player->setSubX(0.5f);
        m_player->setSubY(0.5f);
    }

    // Инициализируем двери с системой взаимодействия
    // после того как WorldGenerator сгенерировал карту и добавил все объекты
    initializeDoors();

    LOG_INFO("Test map generated with biome " + std::to_string(m_currentBiome) +
        " and player positioned at (" +
        std::to_string(playerStartPos.first) + ", " +
        std::to_string(playerStartPos.second) + ")");
}

void MapScene::detectKeyInput() {
    // Этот метод теперь просто делегирует обработку клавиш игроку
    if (m_player) {
        m_player->detectKeyInput();
    }
}

void MapScene::addInteractiveObject(std::shared_ptr<InteractiveObject> object) {
    if (object) {
        // Если объект - дверь, устанавливаем для нее систему взаимодействия
        if (auto doorObj = std::dynamic_pointer_cast<Door>(object)) {
            doorObj->setInteractionSystem(m_interactionSystem.get());
        }

        // Добавляем объект в EntityManager
        m_entityManager->addInteractiveObject(object);
    }
}

void MapScene::removeInteractiveObject(std::shared_ptr<InteractiveObject> object) {
    if (object) {
        m_entityManager->removeInteractiveObject(object);
    }
}

void MapScene::update(float deltaTime) {
    // 1. Обнаружение нажатий клавиш и обновление игрока
    if (m_player) {
        m_player->detectKeyInput();
        m_player->update(deltaTime);
    }

    // 2. Обновление камеры
    m_camera->update(deltaTime);

    // 3. Обновление системы взаимодействия
    m_interactionSystem->update(deltaTime);

    // 4. Обновление всех сущностей через EntityManager
    m_entityManager->update(deltaTime);

    // НОВЫЙ КОД: Периодическая проверка и обновление состояния дверей
    static float doorCheckTimer = 0.0f;
    doorCheckTimer += deltaTime;

    if (doorCheckTimer >= 1.0f) {  // Проверка раз в секунду
        doorCheckTimer = 0.0f;

        for (auto& obj : m_entityManager->getInteractiveObjects()) {
            if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
                // Проверяем, что дверь активна и интерактивна
                if (!doorObj->isActive() || !doorObj->isInteractable()) {
                    LOG_WARNING("Found inactive/non-interactable door: " + doorObj->getName() +
                        ", fixing...");
                    doorObj->setActive(true);
                    doorObj->setInteractable(true);
                }

                // Проверяем, что радиус взаимодействия двери корректный
                if (doorObj->getInteractionRadius() < 1.5f) {
                    LOG_WARNING("Door has small interaction radius: " + doorObj->getName());
                    doorObj->setInteractionRadius(1.8f);
                }

                // Обновляем подсказку для соответствия состоянию
                if (doorObj->isOpen()) {
                    // Убедимся, что у открытой двери правильная подсказка
                    doorObj->updateInteractionHint();
                }
            }
        }
    }

    // Проверка удержания клавиши E для взаимодействия с дверьми
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    if (keyState[SDL_SCANCODE_E]) {
        // Обновляем состояние взаимодействия если клавиша E удерживается
        if (m_interactionSystem) {
            m_interactionSystem->updateInteraction(deltaTime);
        }
    }

    // 5. Обновление базового класса
    Scene::update(deltaTime);
}