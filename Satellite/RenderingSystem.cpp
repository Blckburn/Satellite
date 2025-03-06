#include "RenderingSystem.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>

RenderingSystem::RenderingSystem(std::shared_ptr<TileMap> tileMap,
    std::shared_ptr<TileRenderer> tileRenderer,
    std::shared_ptr<IsometricRenderer> isoRenderer)
    : m_tileMap(tileMap), m_tileRenderer(tileRenderer), m_isoRenderer(isoRenderer) {
    LOG_INFO("RenderingSystem initialized");
}

void RenderingSystem::render(SDL_Renderer* renderer,
    std::shared_ptr<Camera> camera,
    std::shared_ptr<Player> player,
    std::shared_ptr<EntityManager> entityManager,
    int biomeType) {
    // Очищаем экран
    SDL_SetRenderDrawColor(renderer, 20, 35, 20, 255);
    SDL_RenderClear(renderer);

    // Получаем размер окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    // Настраиваем рендерер с учетом камеры
    m_isoRenderer->setCameraPosition(camera->getX(), camera->getY());
    m_isoRenderer->setCameraZoom(camera->getZoom());

    // Используем блочную сортировку для отрисовки тайлов, игрока и интерактивных объектов
    renderWithBlockSorting(renderer, player, entityManager, centerX, centerY, biomeType);

    // Добавляем индикатор игрока, чтобы его можно было видеть за стенами
    renderPlayerIndicator(renderer, player, centerX, centerY);
}

float RenderingSystem::calculateZOrderPriority(float x, float y, float z,
    float playerFullX, float playerFullY,
    float playerDirectionX, float playerDirectionY) {
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
        if (xFractional < 0.1f && playerDirectionX < 0.0f) {
            boundaryFactor = 1.0f;  // Движение влево
        }
        else if (xFractional > 0.9f && playerDirectionX > 0.0f) {
            boundaryFactor = 1.0f;  // Движение вправо
        }

        if (yFractional < 0.1f && playerDirectionY < 0.0f) {
            boundaryFactor = 1.0f;  // Движение вверх
        }
        else if (yFractional > 0.9f && playerDirectionY > 0.0f) {
            boundaryFactor = 1.0f;  // Движение вниз
        }
    }

    // 4. Финальный приоритет
    return baseDepth + heightFactor + boundaryFactor;
}

void RenderingSystem::renderWithBlockSorting(SDL_Renderer* renderer,
    std::shared_ptr<Player> player,
    std::shared_ptr<EntityManager> entityManager,
    int centerX, int centerY,
    int biomeType) {
    // 1. Очистка рендерера перед отрисовкой
    m_tileRenderer->clear();

    // 2. Получение координат игрока и определение текущего блока
    float playerFullX = player ? player->getFullX() : 0.0f;
    float playerFullY = player ? player->getFullY() : 0.0f;

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
    for (auto& object : entityManager->getInteractiveObjects()) {
        if (!object->isActive()) continue;

        float objectX = object->getPosition().x;
        float objectY = object->getPosition().y;
        float objectZ = object->getPosition().z;

        // Пропускаем объекты вне видимой области
        if (objectX < startX - 5 || objectX > endX + 5 ||
            objectY < startY - 5 || objectY > endY + 5) {
            continue;
        }

        float priority = calculateZOrderPriority(objectX, objectY, objectZ,
            playerFullX, playerFullY,
            player->getDirectionX(), player->getDirectionY());

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
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y),
                    tile->getHeight(), playerFullX, playerFullY,
                    player->getDirectionX(), player->getDirectionY());

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
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y),
                    tile->getHeight(), playerFullX, playerFullY,
                    player->getDirectionX(), player->getDirectionY());

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
            if (player) {
                isPlayerTile = (x == static_cast<int>(playerFullX) &&
                    y == static_cast<int>(playerFullY));
            }

            // Если это тайл с игроком, то рисуем его
            if (isPlayerTile) {
                RenderObject renderObj;
                renderObj.type = RenderObject::Type::PLAYER;
                renderObj.x = playerFullX;
                renderObj.y = playerFullY;
                renderObj.z = player->getHeight();
                renderObj.tileX = static_cast<int>(playerFullX);
                renderObj.tileY = static_cast<int>(playerFullY);
                renderObj.priority = calculateZOrderPriority(playerFullX, playerFullY,
                    player->getHeight(), playerFullX, playerFullY,
                    player->getDirectionX(), player->getDirectionY()) + 0.5f;

                objectsToRender.push_back(renderObj);
            }

            // Затем добавляем стены или другие объемные объекты в этом тайле
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::EMPTY && tile->getHeight() > 0.0f) {
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y),
                    tile->getHeight(), playerFullX, playerFullY,
                    player->getDirectionX(), player->getDirectionY());

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
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y),
                    tile->getHeight(), playerFullX, playerFullY,
                    player->getDirectionX(), player->getDirectionY());

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
                float priority = calculateZOrderPriority(static_cast<float>(x), static_cast<float>(y),
                    tile->getHeight(), playerFullX, playerFullY,
                    player->getDirectionX(), player->getDirectionY());

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
            if (player) {
                SDL_Color playerColor = player->getColor();

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
                    playerFullX, playerFullY, player->getHeight(),
                    nullptr, nullptr, nullptr,
                    playerColor, leftColor, rightColor,
                    obj.priority
                );

                // Отрисовка указателя направления персонажа
                if (player && player->isShowingDirectionIndicator()) {
                    player->renderDirectionIndicator(renderer, m_isoRenderer.get(), centerX, centerY);
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
                    switch (biomeType) {
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
                    // Специальная визуализация для терминалов
                    height = obj.z;
                    SDL_Color color = terminalObj->getColor();
                    SDL_Color screenColor;

                    if (terminalObj->shouldShowIndicator()) {
                        // Для непрочитанных терминалов делаем намного ярче с пульсацией
                        screenColor = {
                            static_cast<Uint8>(std::min(255, color.r + 100)),
                            static_cast<Uint8>(std::min(255, color.g + 100)),
                            static_cast<Uint8>(std::min(255, color.b + 100)),
                            255
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
                        color, leftColor, rightColor,
                        obj.priority
                    );

                    // Экран терминала (верхняя часть)
                    m_tileRenderer->addVolumetricTile(
                        obj.x, obj.y, height + pulseEffect,
                        nullptr, nullptr, nullptr,
                        screenColor, screenColor, screenColor,
                        obj.priority + 0.1f
                    );
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

    if (player) {
        player->renderDirectionIndicator(renderer, m_isoRenderer.get(), centerX, centerY);
    }

    // 10. Отрисовываем индикаторы прогресса над дверями
    for (auto& obj : entityManager->getInteractiveObjects()) {
        if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
            // Отрисовка прогресс-бара над дверью
            doorObj->render(renderer, m_isoRenderer.get(), centerX, centerY);
        }
    }
}

void RenderingSystem::renderPlayerIndicator(SDL_Renderer* renderer,
    std::shared_ptr<Player> player,
    int centerX, int centerY) {
    if (!player) return;

    // Получаем координаты персонажа в мировом пространстве
    float playerFullX = player->getFullX();
    float playerFullY = player->getFullY();
    float playerHeight = player->getHeight();

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

void RenderingSystem::renderPlayer(SDL_Renderer* renderer,
    std::shared_ptr<Player> player,
    float priority) {
    if (!player) return;

    // Полные координаты игрока
    float playerFullX = player->getFullX();
    float playerFullY = player->getFullY();
    float playerHeight = player->getHeight();

    // Получаем цвета для игрока
    SDL_Color playerColor = player->getColor();
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

TileRenderer* RenderingSystem::getTileRenderer() const {
    return m_tileRenderer.get();
}