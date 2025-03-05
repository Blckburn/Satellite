﻿#include "Door.h"
#include "Player.h"
#include "Logger.h"
#include "TileType.h"
#include "MapScene.h"
#include "IsometricRenderer.h"

Door::Door(const std::string& name, TileMap* tileMap, MapScene* parentScene, int biomeType)
    : InteractiveObject(name, InteractiveType::DOOR),
    m_isOpen(false),
    m_tileMap(tileMap),
    m_tileX(0),
    m_tileY(0),
    m_parentScene(parentScene),
    m_interactionSystem(nullptr), // Инициализация указателя на InteractionSystem
    m_isVertical(false),
    m_biomeType(biomeType),
    m_isInteracting(false),
    m_interactionTimer(0.0f),
    m_interactionRequiredTime(1.0f), // 1 секунда для полного открытия/закрытия
    m_interactionProgress(0.0f),
    m_actionJustCompleted(false),
    m_cooldownTimer(0.0f),
    m_requireKeyRelease(false) {

    // Установка цвета в зависимости от биома
    switch (m_biomeType) {
    case 1: // FOREST
        m_closedColor = { 60, 120, 40, 255 };  // Темно-зеленый (ветви)
        break;
    case 2: // DESERT
        m_closedColor = { 230, 190, 130, 255 }; // Песочный (песчаные завалы)
        break;
    case 3: // TUNDRA
        m_closedColor = { 200, 220, 255, 220 }; // Голубоватый (ледяная преграда)
        break;
    case 4: // VOLCANIC
        m_closedColor = { 180, 60, 20, 255 };  // Темно-красный (застывшая лава)
        break;
    default:
        m_closedColor = { 140, 70, 20, 255 };  // Стандартный коричневый
        break;
    }

    // Устанавливаем начальный цвет
    setColor(m_closedColor);

    // Устанавливаем радиус взаимодействия
    setInteractionRadius(1.8f);

    // Устанавливаем подсказку в зависимости от биома и состояния
    updateInteractionHint();

    // Устанавливаем высоту преграды
    setHeight(0.3f);
}


bool Door::initialize() {
    // Получаем координаты тайла из позиции двери
    m_tileX = static_cast<int>(getPosition().x);
    m_tileY = static_cast<int>(getPosition().y);

    // Определяем ориентацию двери на основе соседних стен
    bool wallLeft = false;
    bool wallRight = false;
    bool wallUp = false;
    bool wallDown = false;

    if (m_tileMap) {
        if (m_tileMap->isValidCoordinate(m_tileX - 1, m_tileY) &&
            !m_tileMap->isTileWalkable(m_tileX - 1, m_tileY)) {
            wallLeft = true;
        }

        if (m_tileMap->isValidCoordinate(m_tileX + 1, m_tileY) &&
            !m_tileMap->isTileWalkable(m_tileX + 1, m_tileY)) {
            wallRight = true;
        }

        if (m_tileMap->isValidCoordinate(m_tileX, m_tileY - 1) &&
            !m_tileMap->isTileWalkable(m_tileX, m_tileY - 1)) {
            wallUp = true;
        }

        if (m_tileMap->isValidCoordinate(m_tileX, m_tileY + 1) &&
            !m_tileMap->isTileWalkable(m_tileX, m_tileY + 1)) {
            wallDown = true;
        }
    }

    // Устанавливаем ориентацию
    if ((wallLeft && wallRight) || (!wallUp && !wallDown)) {
        setVertical(false); // Горизонтальная дверь (проход запад-восток)
    }
    else {
        setVertical(true);  // Вертикальная дверь (проход север-юг)
    }

    // Вместо EMPTY используем FLOOR, чтобы был виден пол биома
    // (Тайл будет непроходимым, но визуально будет как пол)
    if (m_tileMap && m_tileMap->isValidCoordinate(m_tileX, m_tileY)) {
        // Определяем тип пола в зависимости от биома
        TileType floorType = TileType::FLOOR;
        switch (m_biomeType) {
        case 1: // FOREST
            floorType = TileType::GRASS;
            break;
        case 2: // DESERT
            floorType = TileType::SAND;
            break;
        case 3: // TUNDRA
            floorType = TileType::SNOW;
            break;
        case 4: // VOLCANIC
            floorType = TileType::STONE; // или другой подходящий тип для вулканического биома
            break;
        default:
            floorType = TileType::FLOOR;
            break;
        }

        // Устанавливаем тип тайла и делаем его непроходимым
        m_tileMap->setTileType(m_tileX, m_tileY, floorType);
        MapTile* tile = m_tileMap->getTile(m_tileX, m_tileY);
        if (tile) {
            tile->setWalkable(false); // Изначально дверь закрыта
        }
    }

    LOG_INFO("Door initialized at position (" +
        std::to_string(m_tileX) + ", " +
        std::to_string(m_tileY) + ") with " +
        (m_isVertical ? "vertical" : "horizontal") + " orientation for biome " +
        std::to_string(m_biomeType));

    return InteractiveObject::initialize();
}

void Door::update(float deltaTime) {
    // Базовое обновление интерактивного объекта
    InteractiveObject::update(deltaTime);

    // Обновление кулдауна после завершения действия
    if (m_actionJustCompleted) {
        m_cooldownTimer -= deltaTime;
        if (m_cooldownTimer <= 0.0f) {
            m_actionJustCompleted = false;
        }
    }

    // Если дверь находится в процессе взаимодействия
    if (m_isInteracting) {
        // Инкрементируем таймер и прогресс
        m_interactionTimer += deltaTime;

        // Вычисляем прогресс как отношение текущего времени к требуемому
        // с ограничением до диапазона [0.0, 1.0]
        m_interactionProgress = std::min(1.0f, m_interactionTimer / m_interactionRequiredTime);

        // Логирование прогресса (для отладки)
        if (static_cast<int>(m_interactionProgress * 100) % 10 == 0) {
            LOG_DEBUG("Door interaction progress: " + std::to_string(m_interactionProgress * 100) + "%");
        }

        // Обновляем подсказку с текущим прогрессом
        updateInteractionHintDuringCast();

        // Если процесс завершен
        if (m_interactionProgress >= 1.0f) {
            completeInteraction();
        }
    }
}

bool Door::interact(Player* player) {
    // Базовая проверка интерактивности
    if (!isInteractable()) {
        return false;
    }

    // Завершаем взаимодействие
    if (m_isOpen) {
        // Если дверь уже открыта, закрываем её
        m_isOpen = false;

        // Изменяем тип тайла на непроходимый (стена)
        int tileX = static_cast<int>(getPosition().x);
        int tileY = static_cast<int>(getPosition().y);

        if (m_tileMap) {
            m_tileMap->setTileType(tileX, tileY, TileType::WALL);
            m_tileMap->setTileWalkable(tileX, tileY, false);
        }

        // Обновляем подсказку
        setInteractionHint("Press E to open door");

        // Сообщаем системе взаимодействия, что дверь закрыта
        if (m_interactionSystem) {
            m_interactionSystem->forgetDoorPosition(tileX, tileY);
        }

        LOG_INFO("Door " + getName() + " closed");
    }
    else {
        // Открываем дверь
        m_isOpen = true;

        // Изменяем тип тайла на проходимый (пол)
        int tileX = static_cast<int>(getPosition().x);
        int tileY = static_cast<int>(getPosition().y);

        if (m_tileMap) {
            TileType floorType = TileType::FLOOR;

            // Определяем тип пола в зависимости от биома
            switch (m_biomeType) {
            case 1: // FOREST
                floorType = TileType::GRASS;
                break;
            case 2: // DESERT
                floorType = TileType::SAND;
                break;
            case 3: // TUNDRA
                floorType = TileType::SNOW;
                break;
            case 4: // VOLCANIC
                floorType = TileType::STONE;
                break;
            }

            m_tileMap->setTileType(tileX, tileY, floorType);
            m_tileMap->setTileWalkable(tileX, tileY, true);
        }

        // Обновляем подсказку
        setInteractionHint("Press E to close door");

        // Сообщаем системе взаимодействия, что дверь открыта
        if (m_interactionSystem) {
            m_interactionSystem->rememberDoorPosition(tileX, tileY, getName());
        }

        LOG_INFO("Door " + getName() + " opened");
    }

    // Требуем отпускания клавиши для следующего взаимодействия
    m_requireKeyRelease = true;

    // Вызываем базовый метод взаимодействия
    return InteractiveObject::interact(player);
}

bool Door::startInteraction() {
    // Если требуется отпустить клавишу, запрещаем новое взаимодействие
    if (m_requireKeyRelease) {
        return false;
    }

    // Если действие только что завершилось и идет кулдаун, запрещаем новое взаимодействие
    if (m_actionJustCompleted) {
        return false;
    }

    // Если уже идет взаимодействие, просто продолжаем его
    if (m_isInteracting) {
        return true;
    }

    // Начинаем процесс взаимодействия
    m_isInteracting = true;
    m_interactionTimer = 0.0f;  // Явно сбрасываем таймер
    m_interactionProgress = 0.0f;

    // Обновляем подсказку для отображения процесса
    updateInteractionHintDuringCast();

    LOG_INFO("Started " + std::string(m_isOpen ? "closing" : "opening") + " interaction with door " + getName());

    return true;
}

void Door::cancelInteraction() {
    if (!m_isInteracting) {
        return;
    }

    // Отменяем процесс взаимодействия
    m_isInteracting = false;
    m_interactionTimer = 0.0f;
    m_interactionProgress = 0.0f;

    // Возвращаем стандартную подсказку
    updateInteractionHint();

    LOG_INFO("Cancelled interaction with door " + getName());
}

void Door::completeInteraction() {
    // Завершаем процесс взаимодействия
    m_isInteracting = false;
    m_interactionTimer = 0.0f;

    // Устанавливаем флаг завершения действия для предотвращения автоповтора
    m_actionJustCompleted = true;
    m_cooldownTimer = 0.5f; // Полсекунды кулдауна

    // Устанавливаем флаг требования отпустить клавишу перед новым взаимодействием
    m_requireKeyRelease = true;

    // Меняем состояние двери
    if (m_tileMap && m_tileMap->isValidCoordinate(m_tileX, m_tileY)) {
        MapTile* tile = m_tileMap->getTile(m_tileX, m_tileY);
        if (tile) {
            if (!m_isOpen) {
                // Открываем дверь - делаем тайл проходимым
                tile->setWalkable(true);
                m_isOpen = true;

                // Визуально "открываем" дверь - делаем полупрозрачной
                SDL_Color openColor = m_closedColor;
                openColor.a = 128; // Полупрозрачная
                setColor(openColor);

                // Уведомляем ТОЛЬКО через InteractionSystem, НЕ используем m_parentScene
                if (m_interactionSystem) {
                    m_interactionSystem->rememberDoorPosition(m_tileX, m_tileY, getName());
                }
            }
            else {
                // Закрываем дверь - делаем тайл непроходимым
                tile->setWalkable(false);
                m_isOpen = false;

                // Визуально "закрываем" дверь
                setColor(m_closedColor);

                // Уведомляем ТОЛЬКО через InteractionSystem, НЕ используем m_parentScene
                if (m_interactionSystem) {
                    m_interactionSystem->forgetDoorPosition(m_tileX, m_tileY);
                }
            }
        }
    }

    // Обновляем подсказку для нового состояния
    updateInteractionHint();

    LOG_INFO("Door " + getName() + " interaction completed, now " + (m_isOpen ? "open" : "closed"));
}

void Door::updateInteractionProgress(float progress) {
    // Ограничиваем прогресс в пределах 0.0-1.0
    m_interactionProgress = std::max(0.0f, std::min(1.0f, progress));
}

bool Door::isOpen() const {
    return m_isOpen;
}

void Door::setOpen(bool open) {
    if (m_isOpen != open) {
        m_isOpen = open;
        updateTileWalkability();
        updateInteractionHint();
    }
}

void Door::updateTileWalkability() {
    // Проверяем, что карта существует и координаты действительны
    if (m_tileMap && m_tileMap->isValidCoordinate(m_tileX, m_tileY)) {
        // Получаем текущий тайл
        MapTile* tile = m_tileMap->getTile(m_tileX, m_tileY);
        if (tile) {
            // Если дверь открыта, используем тип DOOR (проходимый)
            // Если дверь закрыта, используем тип WALL (непроходимый)
            TileType tileType = m_isOpen ? TileType::DOOR : TileType::WALL;

            // Обновляем тип тайла
            m_tileMap->setTileType(m_tileX, m_tileY, tileType);
        }
    }
}

void Door::updateInteractionHint() {
    // В зависимости от состояния двери и биома, устанавливаем соответствующую подсказку
    std::string action;

    if (!m_isOpen) {
        // Дверь закрыта - показываем подсказку для открытия
        switch (m_biomeType) {
        case 1: // FOREST
            action = "cut through dense branches";
            break;
        case 2: // DESERT
            action = "dig through sand pile";
            break;
        case 3: // TUNDRA
            action = "break ice formation";
            break;
        case 4: // VOLCANIC
            action = "clear volcanic rubble";
            break;
        default:
            action = "open door";
            break;
        }
    }
    else {
        // Дверь открыта - показываем подсказку для закрытия
        switch (m_biomeType) {
        case 1: // FOREST
            action = "place branches to block path";
            break;
        case 2: // DESERT
            action = "pile up sand to block path";
            break;
        case 3: // TUNDRA
            action = "rebuild ice barrier";
            break;
        case 4: // VOLCANIC
            action = "pile up rocks to block path";
            break;
        default:
            action = "close door";
            break;
        }
    }

    // Для каст-времени мы теперь показываем "Hold E to..." вместо "Press E to..."
    setInteractionHint("Hold E to " + action);
}

void Door::updateInteractionHintDuringCast() {
    std::string action;

    if (!m_isOpen) {
        // Показываем, что идет процесс открытия
        switch (m_biomeType) {
        case 1: // FOREST
            action = "cutting through branches";
            break;
        case 2: // DESERT
            action = "digging through sand";
            break;
        case 3: // TUNDRA
            action = "breaking ice";
            break;
        case 4: // VOLCANIC
            action = "clearing rubble";
            break;
        default:
            action = "opening door";
            break;
        }
    }
    else {
        // Показываем, что идет процесс закрытия
        switch (m_biomeType) {
        case 1: // FOREST
            action = "placing branches";
            break;
        case 2: // DESERT
            action = "piling up sand";
            break;
        case 3: // TUNDRA
            action = "rebuilding ice barrier";
            break;
        case 4: // VOLCANIC
            action = "piling up rocks";
            break;
        default:
            action = "closing door";
            break;
        }
    }

    // Показываем только действие без процента
    setInteractionHint("Hold E: " + action + "...");
}

const std::string& Door::getInteractionHint() const {
    // Возвращаем нашу специфичную для двери подсказку
    return InteractiveObject::getInteractionHint();
}

void Door::render(SDL_Renderer* renderer, IsometricRenderer* isoRenderer, int centerX, int centerY) {
    // Рисуем прогресс-бар только если идет взаимодействие
    if (m_isInteracting && isoRenderer != nullptr) {
        float doorX = getPosition().x;
        float doorY = getPosition().y;
        float doorZ = getPosition().z + 1.0f; // Немного выше двери

        // Получаем экранные координаты двери
        int screenX, screenY;
        isoRenderer->worldToDisplay(doorX, doorY, doorZ, centerX, centerY, screenX, screenY);

        // Увеличенные размеры индикатора
        int progressWidth = 90;   // Увеличен (было 40)
        int progressHeight = 22;  // Увеличен (было 6)

        // Рисуем фон индикатора (прозрачный черный)
        SDL_Rect progressBg = {
            screenX - progressWidth / 2,
            screenY - progressHeight / 2,
            progressWidth,
            progressHeight
        };

        // Фон полупрозрачный черный
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(renderer, &progressBg);

        // Рисуем заполненную часть
        SDL_Rect progressFill = progressBg;
        progressFill.w = static_cast<int>(progressFill.w * m_interactionProgress);

        // Цвет зависит от типа действия
        SDL_Color fillColor;
        if (m_isOpen) {
            // Закрываем дверь - красный индикатор
            fillColor = { 220, 50, 50, 220 };
        }
        else {
            // Открываем дверь - зеленый индикатор
            fillColor = { 50, 220, 50, 220 };
        }

        SDL_SetRenderDrawColor(renderer, fillColor.r, fillColor.g, fillColor.b, fillColor.a);
        SDL_RenderFillRect(renderer, &progressFill);

        // Рамка индикатора
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
        SDL_RenderDrawRect(renderer, &progressBg);

        // Отображаем процент прямо на индикаторе
        int progressPercent = static_cast<int>(m_interactionProgress * 100.0f);
        std::string progressText = std::to_string(progressPercent) + "%";

        // Проверяем, доступен ли менеджер ресурсов через сцену
        if (m_parentScene && m_parentScene->getEngine() &&
            m_parentScene->getEngine()->getResourceManager() &&
            m_parentScene->getEngine()->getResourceManager()->hasFont("default")) {

            // Отрисовываем текст с процентом
            m_parentScene->getEngine()->getResourceManager()->renderText(
                renderer,
                progressText,
                "default",
                screenX,              // Центр прогресс-бара по X
                screenY,              // Центр прогресс-бара по Y
                { 255, 255, 255, 255 } // Белый текст
            );
        }
        else {
            // Если шрифт недоступен, рисуем текст как примитив (упрощенно)
            // Позиция для текста - по центру прогресс-бара
            SDL_Rect textRect = {
                screenX - 15,   // Примерный центр
                screenY - 6,    // Чуть выше центра прогресс-бара
                30,             // Примерная ширина текста
                12              // Примерная высота текста
            };

            // Создаем полупрозрачный черный фон для текста, чтобы он был лучше виден
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 120);
            SDL_RenderFillRect(renderer, &textRect);

            // Здесь мы просто рисуем прямоугольник с текстом внутри
            // так как у нас нет прямого способа рендеринга текста без ресурсов
        }
    }
}

/**
 * @brief Сбрасывает флаг требования отпускания клавиши
 */
void Door::resetKeyReleaseRequirement() {
    m_requireKeyRelease = false;
}

/**
 * @brief Проверяет, требуется ли отпустить клавишу перед новым взаимодействием
 * @return true, если требуется отпустить клавишу
 */
bool Door::isRequiringKeyRelease() const {
    return m_requireKeyRelease;
}

void Door::setInteractionTime(float time) {
    m_interactionRequiredTime = std::max(0.1f, time); // Минимум 0.1 секунды
    LOG_DEBUG("Door " + getName() + " interaction time set to " + std::to_string(m_interactionRequiredTime) + " seconds");
}