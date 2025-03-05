#include "InteractionSystem.h"
#include "Logger.h"
#include <cmath>
#include "PickupItem.h"

InteractionSystem::InteractionSystem(std::shared_ptr<Player> player,
    std::shared_ptr<EntityManager> entityManager,
    std::shared_ptr<TileMap> tileMap)
    : m_player(player), m_entityManager(entityManager), m_tileMap(tileMap),
    m_interactionPromptTimer(0.0f), m_showInteractionPrompt(false),
    m_isInteractingWithDoor(false), m_currentInteractingDoor(nullptr),
    m_isDisplayingTerminalInfo(false), m_currentInteractingTerminal(nullptr) {
    LOG_INFO("InteractionSystem initialized");
}

void InteractionSystem::handleInteraction() {
    if (!m_player) return;

    float playerX = m_player->getFullX();
    float playerY = m_player->getFullY();
    float playerDirX = m_player->getDirectionX();
    float playerDirY = m_player->getDirectionY();

    // Если уже идет взаимодействие с дверью, не ищем новые объекты
    if (m_isInteractingWithDoor && m_currentInteractingDoor) {
        // Продолжаем текущее взаимодействие
        return;
    }

    std::shared_ptr<InteractiveObject> nearestObject = m_entityManager->findNearestInteractiveObject(
        playerX, playerY, playerDirX, playerDirY);

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

void InteractionSystem::update(float deltaTime) {
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

    // Обновление таймера подсказки
    if (m_showInteractionPrompt) {
        m_interactionPromptTimer += deltaTime;
        // Скрываем подсказку через 2 секунды
        if (m_interactionPromptTimer > 2.0f) {
            m_showInteractionPrompt = false;
        }
    }

    // Проверка наличия объектов для взаимодействия и обновление подсказки
    if (m_player) {
        float playerX = m_player->getFullX();
        float playerY = m_player->getFullY();
        float playerDirX = m_player->getDirectionX();
        float playerDirY = m_player->getDirectionY();

        std::shared_ptr<InteractiveObject> nearestObject = m_entityManager->findNearestInteractiveObject(
            playerX, playerY, playerDirX, playerDirY);

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
                // Для других типов объектов формируем подсказку
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
                m_interactionPrompt = "Press E to " + actionText + " " + truncatedName + typeText;
                m_showInteractionPrompt = true;
                m_interactionPromptTimer = 0.0f;
            }
        }
    }
}

void InteractionSystem::rememberDoorPosition(int x, int y, const std::string& name) {
    // Добавляем информацию об открытой двери в список
    m_openDoors.push_back({ x, y, name });
    LOG_INFO("Remembered open door " + name + " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

bool InteractionSystem::isOpenDoorTile(int x, int y) const {
    // Проверяем, есть ли в списке открытая дверь с указанными координатами
    for (const auto& doorInfo : m_openDoors) {
        if (doorInfo.tileX == x && doorInfo.tileY == y) {
            return true;
        }
    }
    return false;
}

void InteractionSystem::closeDoorAtPosition(int x, int y) {
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

            // Вызываем функцию создания двери, если она задана
            if (m_createDoorCallback) {
                m_createDoorCallback(static_cast<float>(x), static_cast<float>(y), doorName);
            }

            LOG_INFO("Closed door " + doorName + " at position (" + std::to_string(x) + ", " + std::to_string(y) + ")");
            break;
        }
    }
}

void InteractionSystem::forgetDoorPosition(int x, int y) {
    // Ищем дверь с указанными координатами в списке открытых дверей
    for (auto it = m_openDoors.begin(); it != m_openDoors.end(); ++it) {
        if (it->tileX == x && it->tileY == y) {
            LOG_INFO("Removed door " + it->name + " from open doors list");
            m_openDoors.erase(it);
            break;
        }
    }
}

std::string InteractionSystem::truncateText(const std::string& text, size_t maxLength) {
    if (text.length() <= maxLength) {
        return text;
    }

    // Отрезаем часть текста и добавляем многоточие
    return text.substr(0, maxLength - 3) + "...";
}

void InteractionSystem::updateInteraction(float deltaTime) {
    // Проверяем, идет ли взаимодействие с дверью
    if (m_isInteractingWithDoor && m_currentInteractingDoor) {
        // Обновляем время взаимодействия
        float currentProgress = m_currentInteractingDoor->getInteractionProgress();
        float newProgress = currentProgress + deltaTime / m_currentInteractingDoor->getInteractionRequiredTime();

        // Обновляем прогресс
        m_currentInteractingDoor->updateInteractionProgress(newProgress);

        // Добавим логирование для отладки
        if (static_cast<int>(newProgress * 100) % 10 == 0 &&
            static_cast<int>(currentProgress * 100) / 10 != static_cast<int>(newProgress * 100) / 10) {
            LOG_DEBUG("Door interaction progress: " + std::to_string(newProgress * 100) + "%");
        }

        // Если достигли 100%, завершаем взаимодействие
        if (newProgress >= 1.0f) {
            m_currentInteractingDoor->completeInteraction();
            m_isInteractingWithDoor = false;
            m_currentInteractingDoor = nullptr;
        }
    }
}