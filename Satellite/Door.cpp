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
    m_interactionSystem(nullptr),
    m_isVertical(false),
    m_biomeType(biomeType),
    m_isInteracting(false),
    m_interactionTimer(0.0f),
    m_interactionRequiredTime(1.0f),
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

    // ИЗМЕНЕНО: Устанавливаем высоту преграды для закрытой двери
    // Используем низкую высоту, чтобы дверь выглядела как часть пола, а не как куб
    setHeight(0.2f);
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

    // ВАЖНОЕ ИЗМЕНЕНИЕ: НЕ меняем тип тайла, только его проходимость
    // Тайл должен иметь тип пола соответствующий биому, но быть непроходимым
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
            floorType = TileType::STONE;
            break;
        default:
            floorType = TileType::FLOOR;
            break;
        }

        // Получаем текущий тайл и сохраняем его тип, если он еще не биомный
        MapTile* tile = m_tileMap->getTile(m_tileX, m_tileY);
        if (tile) {
            // Если тайл нужного типа - оставляем как есть, иначе меняем на биомный тип
            if (tile->getType() != floorType) {
                m_tileMap->setTileType(m_tileX, m_tileY, floorType);
            }

            // Делаем тайл непроходимым
            tile->setWalkable(false);
        }
    }

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

    // ВАЖНО: Устанавливаем высоту преграды для закрытой двери
    setHeight(0.2f);

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

    // УЛУЧШЕННАЯ ОБРАБОТКА КУЛДАУНА
    // Обновление кулдауна после завершения действия
    if (m_actionJustCompleted) {
        // Уменьшаем таймер кулдауна
        m_cooldownTimer -= deltaTime;

        // Если кулдаун завершен, сбрасываем флаг завершения действия
        if (m_cooldownTimer <= 0.0f) {
            m_actionJustCompleted = false;
            m_cooldownTimer = 0.0f;

            // НОВОЕ: Сбрасываем флаг требования отпускания клавиши через защитный таймер
            if (m_requireKeyRelease) {
                // Добавляем задержку, чтобы имитировать отпускание клавиши
                static float keyReleaseTimer = 0.0f;
                keyReleaseTimer += deltaTime;

                if (keyReleaseTimer > 0.5f) {
                    keyReleaseTimer = 0.0f;
                    LOG_DEBUG("Auto-reset key release requirement in update for door: " + getName());
                    m_requireKeyRelease = false;
                }
            }

            LOG_DEBUG("Door cooldown finished for: " + getName());
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

    // НОВАЯ ДИАГНОСТИКА: Периодическая проверка состояния тайла двери
    static float doorCheckTimer = 0.0f;
    doorCheckTimer += deltaTime;

    if (doorCheckTimer > 1.0f) {  // Проверяем каждую секунду
        doorCheckTimer = 0.0f;

        // НОВОЕ: Защитный сброс флага, если он остался установленным слишком долго
        static float totalTimeWithKeyReleaseFlag = 0.0f;
        if (m_requireKeyRelease) {
            totalTimeWithKeyReleaseFlag += 1.0f;
            if (totalTimeWithKeyReleaseFlag > 3.0f) {  // Если флаг установлен больше 3 секунд
                LOG_WARNING("Door " + getName() + " has had requireKeyRelease flag set for too long - resetting");
                m_requireKeyRelease = false;
                totalTimeWithKeyReleaseFlag = 0.0f;
            }
        }
        else {
            totalTimeWithKeyReleaseFlag = 0.0f;
        }

        // Проверяем, что дверь интерактивна
        if (!isInteractable()) {
            LOG_WARNING("Door found to be non-interactable during update: " + getName());
            setInteractable(true);  // Принудительное восстановление интерактивности
        }

        // НОВАЯ ПРОВЕРКА: Проверяем состояние тайла под дверью
        if (m_tileMap && m_tileMap->isValidCoordinate(m_tileX, m_tileY)) {
            MapTile* tile = m_tileMap->getTile(m_tileX, m_tileY);
            if (tile) {
                TileType currentType = tile->getType();
                bool isWalkable = tile->isWalkable();

                // Проверка соответствия состояния тайла и двери
                if (!m_isOpen && currentType != TileType::DOOR) {
                    LOG_WARNING("Closed door has wrong tile type: " + std::to_string(static_cast<int>(currentType)) +
                        ", fixing to DOOR");
                    m_tileMap->setTileType(m_tileX, m_tileY, TileType::DOOR);

                    // Проверяем проходимость тайла после изменения типа
                    tile = m_tileMap->getTile(m_tileX, m_tileY);
                    if (tile && tile->isWalkable()) {
                        LOG_WARNING("Closed door tile is walkable, fixing to non-walkable");
                        tile->setWalkable(false);
                    }
                }
                else if (m_isOpen && isWalkable != true) {
                    LOG_WARNING("Open door tile is not walkable, fixing");
                    tile->setWalkable(true);
                }
                else if (!m_isOpen && isWalkable == true) {
                    LOG_WARNING("Closed door tile is walkable, fixing to non-walkable");
                    tile->setWalkable(false);
                }
            }
        }
    }
}

bool Door::interact(Player* player) {
    // Важная диагностика для отслеживания состояния двери
    LOG_INFO("Door::interact() called for " + getName() +
        ", isOpen=" + std::string(m_isOpen ? "true" : "false") +
        ", coords=(" + std::to_string(m_tileX) + "," + std::to_string(m_tileY) + ")" +
        ", requireKeyRelease=" + std::string(m_requireKeyRelease ? "true" : "false") +
        ", actionJustCompleted=" + std::string(m_actionJustCompleted ? "true" : "false") +
        ", isInteracting=" + std::string(m_isInteracting ? "true" : "false"));

    // Проверка базовых условий
    if (!player || !isInteractable() || !m_tileMap) {
        LOG_ERROR("Door::interact failed - invalid state");
        return false;
    }

    // НОВОЕ: Если флаг m_requireKeyRelease установлен, требуем полного отпускания клавиши E
    if (m_requireKeyRelease) {
        LOG_DEBUG("Interaction blocked until key E is completely released and pressed again");
        return false;
    }

    // Проверка: не идет ли уже взаимодействие
    if (m_isInteracting) {
        LOG_DEBUG("Door already in interaction process, ignoring new interaction request");
        return false;
    }

    // Сбросим флаги блокировки для открытых дверей, чтобы их было легче закрыть
    if (m_isOpen) {
        LOG_DEBUG("Interaction with open door: resetting blocking flags");
        m_actionJustCompleted = false;
        m_cooldownTimer = 0.0f;
    }

    // ВОССТАНОВЛЕНИЕ ФУНКЦИОНАЛЬНОСТИ КАСТ-ВРЕМЕНИ
    // Вместо мгновенного взаимодействия начинаем процесс с каст-временем
    if (startInteraction()) {
        LOG_INFO("Started interaction process with door " + getName());
        // ВАЖНО: Устанавливаем флаг, требующий отпускания клавиши для следующего взаимодействия
        m_requireKeyRelease = true;
        return true;
    }
    else {
        LOG_WARNING("Failed to start interaction with door " + getName());

        // Если не удалось начать взаимодействие и у нас нет активного процесса взаимодействия,
        // сбрасываем некоторые блокирующие флаги, кроме m_requireKeyRelease
        if (!m_isInteracting) {
            LOG_DEBUG("Resetting some blocking flags to allow interaction");
            m_actionJustCompleted = false;
            m_cooldownTimer = 0.0f;

            // Повторная попытка
            if (startInteraction()) {
                LOG_INFO("Started interaction after resetting some flags: " + getName());
                // ВАЖНО: Устанавливаем флаг, требующий отпускания клавиши
                m_requireKeyRelease = true;
                return true;
            }
        }

        return false;
    }
}

bool Door::startInteraction() {
    // Подробный диагностический вывод текущего состояния двери
    LOG_DEBUG("Door::startInteraction() - Name: " + getName() +
        ", isOpen: " + std::string(m_isOpen ? "true" : "false") +
        ", isInteractable: " + std::string(isInteractable() ? "true" : "false") +
        ", requireKeyRelease: " + std::string(m_requireKeyRelease ? "true" : "false") +
        ", actionJustCompleted: " + std::string(m_actionJustCompleted ? "true" : "false") +
        ", isInteracting: " + std::string(m_isInteracting ? "true" : "false") +
        ", cooldownTimer: " + std::to_string(m_cooldownTimer));

    // Если уже идет взаимодействие, не начинаем новое
    if (m_isInteracting) {
        LOG_DEBUG("Door already interacting, ignoring additional interaction requests");
        return false;
    }

    // ПРОВЕРКА: должна ли быть отпущена клавиша перед новым взаимодействием
    if (m_requireKeyRelease) {
        LOG_DEBUG("Interaction blocked: key release required first for door: " + getName());
        return false;
    }

    // Проверка на кулдаун
    if (m_actionJustCompleted) {
        LOG_DEBUG("Interaction blocked: action just completed, in cooldown: " + std::to_string(m_cooldownTimer));
        return false;
    }

    // Аварийная проверка - дверь должна быть интерактивной
    if (!isInteractable()) {
        LOG_WARNING("Door was not interactable! Forcing interactable flag to true.");
        setInteractable(true);
    }

    // Начинаем процесс взаимодействия
    m_isInteracting = true;
    m_interactionTimer = 0.0f;
    m_interactionProgress = 0.0f;

    // Обновляем подсказку для отображения процесса
    updateInteractionHintDuringCast();

    LOG_INFO("Started interaction with door " + getName() +
        (m_isOpen ? " (closing)" : " (opening)"));

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
    m_interactionProgress = 0.0f;

    // Сохраняем текущие координаты для проверки
    float oldX = getPosition().x;
    float oldY = getPosition().y;
    float oldZ = getPosition().z;

    // ВАЖНО - обязательно устанавливаем интерактивность
    setInteractable(true);
    setActive(true);

    // Устанавливаем кулдаун и флаг завершения действия
    m_actionJustCompleted = true;
    m_cooldownTimer = 0.3f;  // Увеличили кулдаун для большей защиты от случайных взаимодействий

    // КРИТИЧЕСКИ ВАЖНО: Устанавливаем флаг, требующий отпускания клавиши перед следующим взаимодействием
    m_requireKeyRelease = true;

    // Меняем состояние двери
    if (m_tileMap && m_tileMap->isValidCoordinate(m_tileX, m_tileY)) {
        MapTile* tile = m_tileMap->getTile(m_tileX, m_tileY);
        if (tile) {
            if (!m_isOpen) {
                // ОТКРЫВАЕМ ДВЕРЬ

                // 1. Делаем тайл проходимым
                tile->setWalkable(true);

                // 2. Затем меняем состояние объекта
                m_isOpen = true;

                // 3. ВАЖНО: НЕ меняем тип тайла!
                // Используем только walkable для управления проходимостью

                // 4. Визуально "открываем" дверь - делаем полупрозрачной и уменьшаем высоту
                SDL_Color openColor = m_closedColor;
                openColor.a = 128; // Полупрозрачная
                setColor(openColor);

                // 5. Уменьшаем высоту для открытой двери
                setHeight(0.05f); // Очень низкая высота

                // 6. Уведомляем систему взаимодействия
                if (m_interactionSystem) {
                    m_interactionSystem->rememberDoorPosition(m_tileX, m_tileY, getName());
                }

                LOG_INFO("Door opened successfully: " + getName() + " at position (" +
                    std::to_string(m_tileX) + ", " + std::to_string(m_tileY) + ")");
            }
            else {
                // ЗАКРЫВАЕМ ДВЕРЬ

                // 1. Делаем тайл непроходимым
                tile->setWalkable(false);

                // 2. Меняем состояние объекта
                m_isOpen = false;

                // 3. ВАЖНО: НЕ меняем тип тайла!
                // Используем только walkable для управления проходимостью

                // 4. Визуально "закрываем" дверь - восстанавливаем непрозрачность
                setColor(m_closedColor);

                // 5. Восстанавливаем высоту для закрытой двери
                setHeight(0.2f);

                // 6. Кулдаун и флаги обрабатываются выше, оставляем только обнуление
                // специфичных для закрытого состояния флагов, если такие будут добавлены

                // 7. Уведомляем систему взаимодействия
                if (m_interactionSystem) {
                    m_interactionSystem->forgetDoorPosition(m_tileX, m_tileY);
                }

                LOG_INFO("Door closed successfully: " + getName() + " at position (" +
                    std::to_string(m_tileX) + ", " + std::to_string(m_tileY) + ")");
            }
        }
    }

    // ПРОВЕРКА И КОРРЕКЦИЯ ПОЗИЦИИ ДВЕРИ
    if (getPosition().x != oldX || getPosition().y != oldY || getPosition().z != oldZ) {
        LOG_WARNING("Door position changed during interaction! Fixing...");
        setPosition(oldX, oldY, oldZ);
    }

    // Принудительно устанавливаем точную позицию двери на основе тайловых координат
    setPosition(static_cast<float>(m_tileX), static_cast<float>(m_tileY), getPosition().z);

    // Принудительно обновляем радиус взаимодействия
    setInteractionRadius(1.8f);

    // Обновляем подсказку для нового состояния
    updateInteractionHint();

    // Обновляем подсказку, включая в нее информацию о необходимости отпустить и нажать E снова
    std::string baseHint = getInteractionHint();
    setInteractionHint(baseHint + " (press E again)");

    // Дополнительный диагностический вывод
    LOG_DEBUG("Door state after completion - Name: " + getName() +
        ", isOpen: " + std::string(m_isOpen ? "true" : "false") +
        ", height: " + std::to_string(getHeight()) +
        ", isInteractable: " + std::string(isInteractable() ? "true" : "false") +
        ", isActive: " + std::string(isActive() ? "true" : "false") +
        ", requireKeyRelease: " + std::string(m_requireKeyRelease ? "true" : "false") +
        ", actionJustCompleted: " + std::string(m_actionJustCompleted ? "true" : "false"));
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
            // Устанавливаем проходимость в зависимости от состояния двери
            tile->setWalkable(m_isOpen);

            // ВАЖНО: НЕ меняем тип тайла! Оставляем его тем же самым.
            // Дверь - это интерактивный объект, а не тайл особого типа.

            // Дополнительная проверка проходимости
            MapTile* verifyTile = m_tileMap->getTile(m_tileX, m_tileY);
            if (verifyTile && verifyTile->isWalkable() != m_isOpen) {
                LOG_WARNING("Walkability mismatch after update! Fixing...");
                verifyTile->setWalkable(m_isOpen);
            }
        }
    }

    LOG_DEBUG("Door tile walkability updated: " + getName() + ", isOpen=" +
        std::string(m_isOpen ? "true" : "false") + ", position=(" +
        std::to_string(m_tileX) + "," + std::to_string(m_tileY) + ")");
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

    // НОВОЕ: Проверяем, требуется ли отпускание клавиши
    if (m_requireKeyRelease) {
        // Если требуется отпускание клавиши, показываем особую подсказку
        setInteractionHint("Press E again to " + action);
    }
    else {
        // Обычная подсказка 
        setInteractionHint("Hold E to " + action);
    }
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
    LOG_DEBUG("Key release requirement reset for door: " + getName());
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
