/*#include "TestScene.h"
#include "Entity.h"
#include "Engine.h"
#include "ResourceManager.h"
#include <iostream>
#include <cmath>

TestScene::TestScene(const std::string& name, Engine* engine)
    : Scene(name), m_angle(0.0f), m_testObjectX(0.0f), m_testObjectY(0.0f),
    m_engine(engine), m_grassTexture(nullptr), m_stoneTexture(nullptr),
    m_wallTexture(nullptr), m_wallLeftTexture(nullptr), m_wallRightTexture(nullptr) {
    // Инициализация тестовой точки
    m_testPoint = { 400, 300 };
}

TestScene::~TestScene() {
    // Текстуры освобождаются в ResourceManager, нам не нужно их освобождать здесь
}

bool TestScene::initialize() {
    // 1. Создаем рендерер изометрических объектов
    m_isoRenderer = std::make_shared<IsometricRenderer>(64, 32);

    // 2. Создаем камеру
    m_camera = std::make_shared<Camera>(800, 600);

    // 3. По умолчанию камера следует за персонажем
    m_camera->setTarget(&m_testObjectX, &m_testObjectY);
    std::cout << "Camera is following the player by default" << std::endl;

    // 4. Загрузка текстур через ResourceManager
    if (m_engine) {
        auto resourceManager = m_engine->getResourceManager();
        if (resourceManager) {
            std::cout << "Loading textures..." << std::endl;

            // Загружаем текстуры с проверкой
            bool success = true;

            // 4.1 Пытаемся загрузить текстуру травы
            success &= resourceManager->loadTexture("grass", "assets/textures/grass.png");

            // 4.2 Пытаемся загрузить текстуру камня
            success &= resourceManager->loadTexture("stone", "assets/textures/stone.png");

            // 4.3 Пытаемся загрузить текстуру стены
            success &= resourceManager->loadTexture("wall", "assets/textures/wall.png");

            if (success) {
                std::cout << "All textures loaded successfully" << std::endl;

                // 5. Создаем изометрические версии текстур для улучшенного отображения
                int tileWidth = m_isoRenderer->getTileWidth();
                int tileHeight = m_isoRenderer->getTileHeight();

                std::cout << "Creating isometric textures..." << std::endl;

                // 5.1 ВАЖНО: удаляем существующие изометрические текстуры, если они есть
                if (resourceManager->hasTexture("iso_grass")) {
                    resourceManager->removeTexture("iso_grass");
                }
                if (resourceManager->hasTexture("iso_stone")) {
                    resourceManager->removeTexture("iso_stone");
                }
                if (resourceManager->hasTexture("iso_wall_top")) {
                    resourceManager->removeTexture("iso_wall_top");
                }
                if (resourceManager->hasTexture("iso_wall_left")) {
                    resourceManager->removeTexture("iso_wall_left");
                }
                if (resourceManager->hasTexture("iso_wall_right")) {
                    resourceManager->removeTexture("iso_wall_right");
                }

                // 5.2 Создаем новые изометрические текстуры
                bool isoSuccess = true;
                isoSuccess &= resourceManager->createIsometricTexture("grass", "iso_grass", tileWidth, tileHeight);
                isoSuccess &= resourceManager->createIsometricTexture("stone", "iso_stone", tileWidth, tileHeight);

                // 5.3 Создаем изометрические текстуры для граней объемных тайлов
                // Верхняя грань (тип 0)
                isoSuccess &= resourceManager->createIsometricFaceTexture("wall", "iso_wall_top", 0, tileWidth, tileHeight);
                // Левая грань (тип 1)
                isoSuccess &= resourceManager->createIsometricFaceTexture("wall", "iso_wall_left", 1, tileWidth, tileHeight);
                // Правая грань (тип 2)
                isoSuccess &= resourceManager->createIsometricFaceTexture("wall", "iso_wall_right", 2, tileWidth, tileHeight);

                // 5.4 Получаем созданные текстуры
                if (isoSuccess) {
                    std::cout << "All isometric textures created successfully" << std::endl;
                    m_grassTexture = resourceManager->getTexture("iso_grass");
                    m_stoneTexture = resourceManager->getTexture("iso_stone");
                    m_wallTexture = resourceManager->getTexture("iso_wall_top");
                    m_wallLeftTexture = resourceManager->getTexture("iso_wall_left");
                    m_wallRightTexture = resourceManager->getTexture("iso_wall_right");

                    // Проверяем и выводим информацию о текстурах
                    resourceManager->debugTextureInfo("iso_grass");
                    resourceManager->debugTextureInfo("iso_stone");
                    resourceManager->debugTextureInfo("iso_wall_top");
                    resourceManager->debugTextureInfo("iso_wall_left");
                    resourceManager->debugTextureInfo("iso_wall_right");
                }
                else {
                    std::cerr << "Failed to create some isometric textures!" << std::endl;
                    // Используем оригинальные текстуры в случае неудачи
                    m_grassTexture = resourceManager->getTexture("grass");
                    m_stoneTexture = resourceManager->getTexture("stone");
                    m_wallTexture = resourceManager->getTexture("wall");
                    m_wallLeftTexture = resourceManager->getTexture("wall");
                    m_wallRightTexture = resourceManager->getTexture("wall");
                }
            }
            else {
                std::cerr << "Failed to load some textures" << std::endl;
                // Продолжаем работу даже в случае ошибки загрузки текстур
                m_grassTexture = resourceManager->getTexture("grass");
                m_stoneTexture = resourceManager->getTexture("stone");
                m_wallTexture = resourceManager->getTexture("wall");
                m_wallLeftTexture = resourceManager->getTexture("wall");
                m_wallRightTexture = resourceManager->getTexture("wall");
            }
        }
        else {
            std::cerr << "ResourceManager not available" << std::endl;
        }
    }
    else {
        std::cerr << "Engine pointer not available" << std::endl;
    }

    // 6. Создаем рендерер тайлов
    m_tileRenderer = std::make_shared<TileRenderer>(m_isoRenderer.get());

    return true;
}


void TestScene::handleEvent(const SDL_Event& event) {
    // Передаем события в камеру для обработки масштабирования и т.д.
    m_camera->handleEvent(event);

    // Дополнительная обработка для тестовой сцены
    if (event.type == SDL_KEYDOWN) {
        // Определяем скорость движения
        float moveSpeed = 0.2f;

        switch (event.key.keysym.sym) {
            // Управление с помощью WASD в соответствии с изометрической схемой
        case SDLK_w:
            // Северо-запад (NW) - уменьшаем X и Y
            m_testObjectX -= moveSpeed;
            m_testObjectY -= moveSpeed;
            break;
        case SDLK_s:
            // Юго-восток (SE) - увеличиваем X и Y
            m_testObjectX += moveSpeed;
            m_testObjectY += moveSpeed;
            break;
        case SDLK_a:
            // Юго-запад (SW) - уменьшаем X, увеличиваем Y
            m_testObjectX -= moveSpeed;
            m_testObjectY += moveSpeed;
            break;
        case SDLK_d:
            // Северо-восток (NE) - увеличиваем X, уменьшаем Y
            m_testObjectX += moveSpeed;
            m_testObjectY -= moveSpeed;
            break;

            // Добавим клавиши-стрелки для основных направлений
        case SDLK_UP:
            // Север (N) - уменьшаем Y
            m_testObjectY -= moveSpeed;
            break;
        case SDLK_DOWN:
            // Юг (S) - увеличиваем Y
            m_testObjectY += moveSpeed;
            break;
        case SDLK_LEFT:
            // Запад (W) - уменьшаем X
            m_testObjectX -= moveSpeed;
            break;
        case SDLK_RIGHT:
            // Восток (E) - увеличиваем X
            m_testObjectX += moveSpeed;
            break;

        case SDLK_r:
            // Сбросить позицию персонажа и камеры
            m_testObjectX = 0.0f;
            m_testObjectY = 0.0f;
            m_camera->setZoom(1.0f);
            break;

        case SDLK_i:
            // Добавим клавишу для печати диагностики (i - info)
            std::cout << "DIAGNOSTIC INFO:" << std::endl;
            std::cout << "  Number of entities: " << m_entities.size() << std::endl;
            std::cout << "  Camera position: (" << m_camera->getX() << ", " << m_camera->getY() << ")" << std::endl;
            std::cout << "  Camera zoom: " << m_camera->getZoom() << std::endl;
            std::cout << "  Object position: (" << m_testObjectX << ", " << m_testObjectY << ")" << std::endl;
            break;
        }
    }

    // Передаем события в базовый класс
    Scene::handleEvent(event);
}

void TestScene::update(float deltaTime) {
    // Обновляем камеру
    m_camera->update(deltaTime);

    // Анимация для демонстрации
    m_angle += deltaTime * 45.0f;  // 45 градусов в секунду
    if (m_angle >= 360.0f) {
        m_angle -= 360.0f;
    }

    // Обновляем базовый класс
    Scene::update(deltaTime);
}

void TestScene::render(SDL_Renderer* renderer) {
    // 1. Очищаем экран темно-зеленым цветом
    SDL_SetRenderDrawColor(renderer, 20, 35, 20, 255);
    SDL_RenderClear(renderer);

    // 2. Получаем размеры окна для центрирования
    int centerX = 400;
    int centerY = 300;

    // 3. Настраиваем рендерер с учетом камеры
    m_isoRenderer->setCameraPosition(m_camera->getX(), m_camera->getY());
    m_isoRenderer->setCameraZoom(m_camera->getZoom());

    // 4. Очищаем и заполняем TileRenderer тайлами
    m_tileRenderer->clear();

    // 5. Добавляем плоские тайлы поля в шахматном порядке
    for (int y = -5; y <= 5; ++y) {
        for (int x = -5; x <= 5; ++x) {
            // Определяем цвет в зависимости от шахматного порядка
            if ((x + y) % 2 == 0) {
                // Зеленый тайл (трава)
                SDL_Color grassColor = { 30, 150, 30, 255 };
                m_tileRenderer->addFlatTile(
                    x, y,
                    nullptr, // Отключаем текстуры полностью
                    grassColor
                );
            }
            else {
                // Светло-серый тайл (камень)
                SDL_Color stoneColor = { 180, 180, 180, 255 };
                m_tileRenderer->addFlatTile(
                    x, y,
                    nullptr, // Отключаем текстуры полностью
                    stoneColor
                );
            }
        }
    }

    // 6. Добавляем стены (объемные блоки) по периметру
    // Внешние края карты
    for (int i = -5; i <= 5; i++) {
        // Стены по северному краю (верхние)
        if (i != 0) { // Оставляем проход посередине
            m_tileRenderer->addVolumetricTile(
                i, -5, 1.0f,
                nullptr, nullptr, nullptr,
                { 150, 150, 150, 255 }, { 100, 100, 100, 255 }, { 70, 70, 70, 255 }
            );
        }

        // Стены по южному краю (нижние)
        if (i != 0) { // Оставляем проход посередине
            m_tileRenderer->addVolumetricTile(
                i, 5, 1.0f,
                nullptr, nullptr, nullptr,
                { 150, 150, 150, 255 }, { 100, 100, 100, 255 }, { 70, 70, 70, 255 }
            );
        }
    }

    // Стены по боковым краям
    for (int j = -4; j <= 4; j++) {
        // Стены по западному краю (левые)
        m_tileRenderer->addVolumetricTile(
            -5, j, 1.0f,
            nullptr, nullptr, nullptr,
            { 150, 150, 150, 255 }, { 100, 100, 100, 255 }, { 70, 70, 70, 255 }
        );

        // Стены по восточному краю (правые)
        m_tileRenderer->addVolumetricTile(
            5, j, 1.0f,
            nullptr, nullptr, nullptr,
            { 150, 150, 150, 255 }, { 100, 100, 100, 255 }, { 70, 70, 70, 255 }
        );
    }

    // 7. Добавляем желтый движущийся ромб с высоким приоритетом (50)
    float animX = cosf(m_angle * M_PI / 180.0f) * 3.0f;
    float animY = sinf(m_angle * M_PI / 180.0f) * 3.0f;

    SDL_Color animColor = { 255, 255, 0, 255 };
    m_tileRenderer->addFlatTile(animX, animY, nullptr, animColor, 50);  // Приоритет 50

    // 8. Добавляем красный управляемый объект с наивысшим приоритетом (100)
    SDL_Color playerTopColor = { 255, 0, 0, 255 };    // Ярко-красный верх
    SDL_Color playerLeftColor = { 200, 0, 0, 255 };   // Красная левая грань
    SDL_Color playerRightColor = { 150, 0, 0, 255 };  // Темно-красная правая грань

    m_tileRenderer->addVolumetricTile(
        m_testObjectX, m_testObjectY, 0.5f,
        nullptr, nullptr, nullptr,
        playerTopColor, playerLeftColor, playerRightColor,
        100  // Приоритет 100
    );

    // 9. Рендерим все тайлы в правильном порядке
    m_tileRenderer->render(renderer, centerX, centerY);

    // 10. Отрисовка индикатора над объектом
    int indicatorX, indicatorY;
    m_isoRenderer->worldToDisplay(m_testObjectX, m_testObjectY, 0.7f,
        centerX, centerY,
        indicatorX, indicatorY);

    // Размер индикатора с учетом масштаба
    int indicatorSize = m_isoRenderer->getScaledSize(8);

    // Рисуем индикатор
    SDL_Rect indicator = {
        indicatorX - indicatorSize,
        indicatorY - indicatorSize,
        indicatorSize * 2,
        indicatorSize * 2
    };

    // Отображаем белый квадрат с черной обводкой
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &indicator);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &indicator);

    // 11. Отрисовка сущностей
    for (auto& entity : m_entities) {
        if (entity->isActive()) {
            entity->render(renderer);
        }
    }
}


// Переопределяем метод addEntity для отслеживания добавления сущностей
void TestScene::addEntity(std::shared_ptr<Entity> entity) {
    Scene::addEntity(entity);
}*/

