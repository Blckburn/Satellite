#include "Switch.h"
#include "Player.h"
#include "Logger.h"
#include "TileMap.h"
#include "MapScene.h"
#include <cmath>
#include <map>

Switch::Switch(const std::string& name, SwitchType type, TileMap* tileMap, MapScene* parentScene)
    : InteractiveObject(name, InteractiveType::SWITCH),
    m_switchType(type),
    m_activated(false),
    m_activationTime(0.0f),
    m_displayingInfo(false),
    m_isActivating(false),
    m_activationTimer(0.0f),
    m_activationRequiredTime(2.0f),  // По умолчанию 2 секунды на активацию
    m_activationProgress(0.0f),
    m_effectRadius(3.0f),            // По умолчанию радиус действия 3 тайла
    m_effectDuration(30.0f),         // По умолчанию эффект длится 30 секунд (0 для постоянного)
    m_effectTimer(0.0f),
    m_effectActive(false),
    m_tileMap(tileMap),
    m_parentScene(parentScene),
    m_pulsePhase(0.0f)
{
    // Увеличиваем высоту для всех переключателей для лучшей видимости
    setPosition(getPosition().x, getPosition().y, 0.5f);

    // Настройка параметров в зависимости от типа переключателя
    switch (type) {
    case SwitchType::GRAVITY_ANOMALY:
        // Яркий голубой цвет для гравитационных аномалий
        m_inactiveColor = { 40, 100, 220, 255 }; // Голубой
        m_activeColor = { 100, 200, 255, 255 };  // Яркий голубой
        setInteractionRadius(2.5f);
        setInteractionHint("Press E to activate gravity anomaly");
        m_activationRequiredTime = 1.5f; // Быстрее активируется
        break;

    case SwitchType::TELEPORT_GATE:
        // Фиолетовый оттенок для древних телепортационных врат
        m_inactiveColor = { 120, 40, 180, 255 }; // Фиолетовый
        m_activeColor = { 180, 100, 255, 255 };  // Яркий фиолетовый
        setInteractionRadius(2.0f);
        setInteractionHint("Press E to activate teleport gate");
        m_activationRequiredTime = 3.0f; // Дольше активируется
        break;

    case SwitchType::RESONANCE_STABILIZER:
        // Зеленый оттенок для стабилизаторов
        m_inactiveColor = { 40, 150, 60, 255 };  // Зеленый
        m_activeColor = { 100, 230, 100, 255 };  // Яркий зеленый
        setInteractionRadius(2.0f);
        setInteractionHint("Press E to activate resonance stabilizer");
        m_effectDuration = 45.0f; // Дольше действует
        break;

    case SwitchType::SECURITY_SYSTEM:
        // Красный оттенок для систем безопасности
        m_inactiveColor = { 150, 40, 40, 255 };  // Красный
        m_activeColor = { 230, 100, 100, 255 };  // Яркий красный
        setInteractionRadius(1.8f);
        setInteractionHint("Press E to deactivate security system");
        m_effectDuration = 0.0f; // Постоянный эффект
        break;

    case SwitchType::ENERGY_NODE:
        // Янтарный оттенок для энергетических узлов
        m_inactiveColor = { 180, 140, 20, 255 }; // Янтарный
        m_activeColor = { 255, 200, 50, 255 };   // Яркий янтарный
        setInteractionRadius(2.2f);
        setInteractionHint("Press E to activate energy node");
        m_effectRadius = 5.0f; // Больший радиус действия
        break;
    }

    // Устанавливаем начальный цвет
    setColor(m_inactiveColor);

    // Генерируем описание
    generateDescription();
}

bool Switch::initialize() {
    // Базовая инициализация интерактивного объекта
    if (!InteractiveObject::initialize()) {
        return false;
    }

    LOG_INFO("Switch initialized: " + getName() + " (Type: " + std::to_string(static_cast<int>(m_switchType)) + ")");
    return true;
}

bool Switch::interact(Player* player) {
    if (!player || !isInteractable()) {
        return false;
    }

    LOG_INFO("Switch interaction: " + getName());

    // Проверяем, отображается ли уже информация переключателя
    if (m_displayingInfo) {
        // Если информация уже отображается, просто возвращаем true
        // Закрытие будет обрабатываться в InteractionSystem
        return true;
    }

    // Если переключатель еще не активирован, активируем его
    if (!m_activated) {
        m_activated = true;
        // Меняем цвет на активный для визуальной обратной связи
        setColor(m_activeColor);
        // Генерируем информацию для отображения
        generateInfoContent();
        LOG_INFO("Switch " + getName() + " activated, displaying info");
    }
    else {
        LOG_INFO("Switch " + getName() + " info displayed again");
    }

    // В любом случае показываем информацию
    m_displayingInfo = true;
    m_activationTime = 0.0f;

    return true;
}


bool Switch::startActivation() {
    if (m_activated || m_isActivating) {
        return false;
    }

    LOG_INFO("Starting activation process for switch: " + getName());
    m_isActivating = true;
    m_activationTimer = 0.0f;
    m_activationProgress = 0.0f;

    // Обновляем подсказку с отображением прогресса
    updateActivationHint();

    return true;
}

void Switch::cancelActivation() {
    if (!m_isActivating) {
        return;
    }

    LOG_INFO("Cancelling activation process for switch: " + getName());
    m_isActivating = false;
    m_activationTimer = 0.0f;
    m_activationProgress = 0.0f;

    // Восстанавливаем исходную подсказку
    setInteractionHint("Press E to activate " + getName());
}

void Switch::completeActivation() {
    if (!m_isActivating) {
        LOG_WARNING("completeActivation called but m_isActivating is false");
        return;
    }

    LOG_INFO("Completing activation of switch: " + getName());
    m_isActivating = false;
    m_activated = true;

    // Вместо активации эффекта теперь просто отображаем информацию
    m_displayingInfo = true;
    m_activationTime = 0.0f;

    // Меняем цвет на активный для визуальной обратной связи
    setColor(m_activeColor);

    // Генерируем более подробное описание для информационного окна
    generateInfoContent();

    // Вызываем функцию обратного вызова, если она задана
    if (m_activationCallback) {
        LOG_INFO("Calling activation callback for " + getName());
        m_activationCallback(nullptr, this);
    }

    // Обновляем подсказку
    updateActivationHint();
}


void Switch::updateActivationHint() {
    if (m_isActivating) {
        int percent = static_cast<int>(m_activationProgress * 100.0f);
        setInteractionHint("Activating: " + std::to_string(percent) + "%");
    }
    else if (m_effectActive) {
        if (m_effectDuration > 0) {
            int remainingTime = static_cast<int>(m_effectDuration - m_effectTimer);
            setInteractionHint("Effect active: " + std::to_string(remainingTime) + " seconds");
        }
        else {
            setInteractionHint("Effect active: permanent");
        }
    }
    else {
        // Восстанавливаем исходную подсказку в зависимости от типа
        switch (m_switchType) {
        case SwitchType::GRAVITY_ANOMALY:
            setInteractionHint("Press E to activate gravity anomaly");
            break;
        case SwitchType::TELEPORT_GATE:
            setInteractionHint("Press E to activate teleport gate");
            break;
        case SwitchType::RESONANCE_STABILIZER:
            setInteractionHint("Press E to activate resonance stabilizer");
            break;
        case SwitchType::SECURITY_SYSTEM:
            setInteractionHint("Press E to deactivate security system");
            break;
        case SwitchType::ENERGY_NODE:
            setInteractionHint("Press E to activate energy node");
            break;
        }
    }
}

void Switch::update(float deltaTime) {
    // Обновляем базовый интерактивный объект
    InteractiveObject::update(deltaTime);

    // Обновляем фазу пульсации для визуальных эффектов
    m_pulsePhase += deltaTime * 2.0f;
    if (m_pulsePhase > 6.28f) { // 2*PI
        m_pulsePhase -= 6.28f;
    }

    // Проверяем, не пора ли скрыть информационное окно
    updateInfoDisplay(deltaTime);

    // Обновляем визуальные эффекты
    updateVisualEffects(deltaTime);
}

void Switch::updateVisualEffects(float deltaTime) {
    // Базовая пульсация для всех переключателей
    float pulse = (std::sin(m_pulsePhase) + 1.0f) * 0.5f; // От 0 до 1

    SDL_Color baseColor = m_activated ? m_activeColor : m_inactiveColor;
    SDL_Color currentColor = baseColor;

    // Если идет процесс активации, усиливаем пульсацию
    if (m_isActivating) {
        float activationIntensity = 0.3f + 0.7f * m_activationProgress;
        pulse *= activationIntensity;

        // Меняем цвет в зависимости от прогресса активации
        currentColor.r = static_cast<Uint8>(std::min(255.0f, baseColor.r * (1.0f + pulse * 0.5f)));
        currentColor.g = static_cast<Uint8>(std::min(255.0f, baseColor.g * (1.0f + pulse * 0.5f)));
        currentColor.b = static_cast<Uint8>(std::min(255.0f, baseColor.b * (1.0f + pulse * 0.5f)));
    }
    // Если переключатель активирован, добавляем эффект свечения
    else if (m_effectActive) {
        // Более интенсивная пульсация для активного эффекта
        currentColor.r = static_cast<Uint8>(std::min(255.0f, baseColor.r * (1.0f + pulse * 0.3f)));
        currentColor.g = static_cast<Uint8>(std::min(255.0f, baseColor.g * (1.0f + pulse * 0.3f)));
        currentColor.b = static_cast<Uint8>(std::min(255.0f, baseColor.b * (1.0f + pulse * 0.3f)));
    }
    // Иначе просто слабо пульсируем
    else {
        pulse *= 0.15f; // Менее заметная пульсация
        currentColor.r = static_cast<Uint8>(baseColor.r * (1.0f + pulse));
        currentColor.g = static_cast<Uint8>(baseColor.g * (1.0f + pulse));
        currentColor.b = static_cast<Uint8>(baseColor.b * (1.0f + pulse));
    }

    // Применяем цвет к переключателю
    setColor(currentColor);
}

void Switch::applyEffect() {
    LOG_INFO("Entering applyEffect() for switch: " + getName() + " of type " + std::to_string(static_cast<int>(m_switchType)));

    // Проверяем указатель на карту
    if (!m_tileMap) {
        LOG_ERROR("Cannot apply effect: m_tileMap is nullptr in " + getName());
        return;
    }

    // Проверяем указатель на сцену для телепортов
    if (m_switchType == SwitchType::TELEPORT_GATE && !m_parentScene) {
        LOG_ERROR("Cannot apply teleport effect: m_parentScene is nullptr in " + getName());
    }

    LOG_INFO("Applying effect of switch: " + getName());


    // Различные эффекты в зависимости от типа переключателя
    switch (m_switchType) {
    case SwitchType::GRAVITY_ANOMALY:
        // Для гравитационной аномалии - меняем свойства тайлов в зоне действия
        // делаем непроходимые тайлы проходимыми на время
        if (m_tileMap) {
            // Получаем позицию переключателя
            int switchX = static_cast<int>(getPosition().x);
            int switchY = static_cast<int>(getPosition().y);

            // Очищаем предыдущие записи при повторной активации
            m_affectedTiles.clear();

            // Применяем эффект ко всем тайлам в зоне действия
            for (int y = switchY - m_effectRadius; y <= switchY + m_effectRadius; ++y) {
                for (int x = switchX - m_effectRadius; x <= switchX + m_effectRadius; ++x) {
                    if (m_tileMap->isValidCoordinate(x, y)) {
                        // Проверяем, что тайл находится в зоне действия (окружность)
                        float dx = x - getPosition().x;
                        float dy = y - getPosition().y;
                        float distSq = dx * dx + dy * dy;
                        if (distSq <= m_effectRadius * m_effectRadius) {
                            // Применяем эффект гравитационной аномалии
                            MapTile* tile = m_tileMap->getTile(x, y);
                            if (tile) {
                                // Запоминаем исходное состояние тайла (для будущего восстановления)
                                std::pair<int, int> tilePos(x, y);
                                m_affectedTiles[tilePos] = tile->isWalkable();

                                // Изменяем проходимость тайлов особым образом:
                                // - Делаем стены и препятствия проходимыми
                                // - Не трогаем водные тайлы (они остаются непроходимыми)
                                if (!tile->isWalkable() &&
                                    tile->getType() != TileType::WATER &&
                                    tile->getType() != TileType::LAVA) {
                                    tile->setWalkable(true);
                                    LOG_INFO("Made tile walkable at: " + std::to_string(x) + ", " + std::to_string(y));
                                }
                                // Если это пол - повышаем его, создавая эффект "левитации"
                                // (это только влияет на визуальное представление)
                                else if (tile->isWalkable()) {
                                    float currentHeight = tile->getHeight();
                                    tile->setHeight(currentHeight + 0.2f);
                                }
                            }
                        }
                    }
                }
            }
        }
        break;

    case SwitchType::TELEPORT_GATE:
        // Для телепортационных врат - активируем телепорт в другую зону
        if (m_parentScene && m_tileMap) {
            // Получаем позицию текущего переключателя
            int switchX = static_cast<int>(getPosition().x);
            int switchY = static_cast<int>(getPosition().y);

            // Пытаемся найти подходящую целевую точку телепортации
            // Ищем безопасное место для телепортации (проходимый тайл)
            int targetX = 0, targetY = 0;
            bool targetFound = false;

            // 1. Сначала попробуем найти точку на противоположной стороне карты
            int oppositeX = m_tileMap->getWidth() - 1 - switchX;
            int oppositeY = m_tileMap->getHeight() - 1 - switchY;

            // Ищем ближайший к противоположной точке проходимый тайл
            for (int radius = 0; radius < 10 && !targetFound; radius++) {
                for (int dy = -radius; dy <= radius && !targetFound; dy++) {
                    for (int dx = -radius; dx <= radius && !targetFound; dx++) {
                        // Проверяем только тайлы на текущем "радиусе" поиска
                        if (std::abs(dx) != radius && std::abs(dy) != radius) continue;

                        int checkX = oppositeX + dx;
                        int checkY = oppositeY + dy;

                        if (m_tileMap->isValidCoordinate(checkX, checkY) &&
                            m_tileMap->isTileWalkable(checkX, checkY)) {
                            targetX = checkX;
                            targetY = checkY;
                            targetFound = true;
                            break;
                        }
                    }
                }
            }
            // 2. Если не нашли подходящую точку, ищем любую безопасную точку вдали от текущей
            if (!targetFound) {
                LOG_INFO("Could not find point on opposite side of map, looking for any safe point");
                // Пытаемся найти любой подходящий тайл на расстоянии не менее 10 от текущей позиции
                for (int y = 0; y < m_tileMap->getHeight() && !targetFound; y++) {
                    for (int x = 0; x < m_tileMap->getWidth() && !targetFound; x++) {
                        if (m_tileMap->isTileWalkable(x, y)) {
                            // Проверяем расстояние
                            float dx = x - switchX;
                            float dy = y - switchY;
                            float distSq = dx * dx + dy * dy;

                            if (distSq >= 100.0f) { // Минимум 10 тайлов в стороне
                                targetX = x;
                                targetY = y;
                                targetFound = true;
                                LOG_INFO("Found safe teleport destination at (" + std::to_string(x) + ", " + std::to_string(y) + ")");
                                break;
                            }
                        }
                    }
                }
            }

            // 3. Если нашли подходящее место, сохраняем координаты для телепортации
            if (targetFound) {
                // Сохраняем точку телепортации
                m_teleportDestX = targetX;
                m_teleportDestY = targetY;

                LOG_INFO("Teleport gate activated at (" + std::to_string(switchX) +
                    ", " + std::to_string(switchY) + "), destination set to (" +
                    std::to_string(targetX) + ", " + std::to_string(targetY) + ")");

                // Отметим точку назначения визуально (например, особым цветом тайла)
                if (MapTile* destTile = m_tileMap->getTile(targetX, targetY)) {
                    // Сохраним оригинальный цвет, чтобы потом вернуть
                    m_originalTileColor = destTile->getColor();

                    // Подсветим тайл назначения фиолетовым цветом
                    SDL_Color portalColor = { 180, 100, 220, 255 };
                    destTile->setColor(portalColor);
                }
            }
            else {
                LOG_WARNING("Teleport gate activated but no suitable destination found");
            }
        }
        else {
            LOG_WARNING("Teleport gate activated but mapScene or tileMap is nullptr");
        }
        break;

    case SwitchType::RESONANCE_STABILIZER:
        // Для стабилизаторов - нейтрализуем опасные факторы окружения
        // Например, временно отключаем ловушки или вредоносные эффекты
        LOG_INFO("Resonance stabilizer neutralizing environmental hazards");
        break;

    case SwitchType::SECURITY_SYSTEM:
        // Для систем безопасности - отключаем защитные механизмы
        // Например, открываем заблокированные двери или деактивируем ловушки
        LOG_INFO("Security system deactivated");
        break;

    case SwitchType::ENERGY_NODE:
        // Для энергетических узлов - активируем другие системы
        // Например, включаем освещение или лифты
        LOG_INFO("Energy node activated, powering connected systems");
        break;
    }
}

void Switch::deactivateEffect() {
    if (!m_effectActive) {
        return;
    }

    LOG_INFO("Deactivating effect of switch: " + getName());
    m_effectActive = false;

    // Восстанавливаем исходное состояние
    switch (m_switchType) {
    case SwitchType::GRAVITY_ANOMALY:
        // Восстанавливаем измененные тайлы
        if (m_tileMap) {
            // Восстанавливаем все тайлы из m_affectedTiles
            for (const auto& pair : m_affectedTiles) {
                int x = pair.first.first;
                int y = pair.first.second;
                bool originalWalkable = pair.second;

                if (m_tileMap->isValidCoordinate(x, y)) {
                    MapTile* tile = m_tileMap->getTile(x, y);
                    if (tile) {
                        // Восстанавливаем проходимость
                        if (tile->isWalkable() != originalWalkable) {
                            tile->setWalkable(originalWalkable);
                            LOG_INFO("Restored tile walkability at: " + std::to_string(x) + ", " + std::to_string(y));
                        }

                        // Если это был поднятый пол, опускаем его
                        if (tile->isWalkable() && tile->getHeight() > 0.2f) {
                            tile->setHeight(tile->getHeight() - 0.2f);
                        }
                    }
                }
            }

            // Очищаем список затронутых тайлов
            m_affectedTiles.clear();

            LOG_INFO("Gravity restored to normal");
        }
        break;

    case SwitchType::TELEPORT_GATE:
        // Восстанавливаем оригинальный цвет тайла назначения
        if (m_tileMap && hasTeleportDestination()) {
            if (MapTile* destTile = m_tileMap->getTile(m_teleportDestX, m_teleportDestY)) {
                destTile->setColor(m_originalTileColor);
                LOG_INFO("Teleport destination marker removed");
            }
        }
        LOG_INFO("Teleport gate deactivated");
        break;

    case SwitchType::RESONANCE_STABILIZER:
        // Возвращаем опасные факторы
        LOG_INFO("Resonance stabilizer effect ended, environmental hazards returning");
        break;

    case SwitchType::SECURITY_SYSTEM:
        // Восстанавливаем системы безопасности
        LOG_INFO("Security systems reactivated");
        break;

    case SwitchType::ENERGY_NODE:
        // Отключаем подпитанные системы
        LOG_INFO("Energy node depleted, connected systems powering down");
        break;
    }

    // Обновляем цвет и подсказку
    setColor(m_inactiveColor);
    updateActivationHint();
}

void Switch::displayInfo(SDL_Renderer* renderer, TTF_Font* font, int x, int y) {
    if (!m_displayingInfo || !renderer || !font) {
        return;
    }

    // Здесь будет код для отображения информации о переключателе
    // В полной реализации может использовать UI систему
    LOG_INFO("Switch '" + getName() + "' displaying info");
}

void Switch::setActivationCallback(std::function<void(Player*, Switch*)> callback) {
    m_activationCallback = callback;
}

void Switch::setActivated(bool activated) {
    if (m_activated != activated) {
        m_activated = activated;

        if (m_activated) {
            m_effectActive = true;
            m_effectTimer = 0.0f;
            setColor(m_activeColor);
            applyEffect();
        }
        else {
            m_effectActive = false;
            setColor(m_inactiveColor);
            deactivateEffect();
        }

        updateActivationHint();
    }
}

std::string Switch::getIndicatorSymbol() const {
    // Символы-индикаторы для разных типов переключателей
    switch (m_switchType) {
    case SwitchType::GRAVITY_ANOMALY:
        return "G";  // Символ для гравитационной аномалии
    case SwitchType::TELEPORT_GATE:
        return "T";  // Символ для телепорта
    case SwitchType::RESONANCE_STABILIZER:
        return "R";  // Символ для стабилизатора
    case SwitchType::SECURITY_SYSTEM:
        return "S";  // Символ для системы безопасности
    case SwitchType::ENERGY_NODE:
        return "E";  // Символ для энергетического узла
    default:
        return "*";  // Символ по умолчанию
    }
}

void Switch::generateDescription() {
    // Генерируем описание в зависимости от типа переключателя
    switch (m_switchType) {
    case SwitchType::GRAVITY_ANOMALY:
        m_description = "A natural gravitational anomaly that can alter the local gravitational field, allowing movement through normally impassable areas.";
        break;
    case SwitchType::TELEPORT_GATE:
        m_description = "Ancient teleportation technology left by a long-gone civilization. Activating it could transport you to otherwise inaccessible areas.";
        break;
    case SwitchType::RESONANCE_STABILIZER:
        m_description = "A natural formation that can neutralize dangerous environmental factors temporarily, creating safe zones.";
        break;
    case SwitchType::SECURITY_SYSTEM:
        m_description = "Part of an ancient security system. Deactivating it might disable traps and defensive mechanisms in the surrounding area.";
        break;
    case SwitchType::ENERGY_NODE:
        m_description = "A power node from an ancient complex. Activating it can restore functionality to connected systems like doors, elevators, or lighting.";
        break;
    }
}


void Switch::generateInfoContent() {
    // Генерируем заголовок информационного окна
    switch (m_switchType) {
    case SwitchType::GRAVITY_ANOMALY:
        m_infoTitle = "Gravity Anomaly Control";
        m_infoDescription = "Creates a field that modifies local gravity, allowing passage through normally impassable areas. Creates \"zero-g corridors\" for shortcuts.";
        break;

    case SwitchType::TELEPORT_GATE:
        m_infoTitle = "Ancient Teleportation System";
        m_infoDescription = "A node in a network of transportation devices left by previous civilization. Instantly transfers to a corresponding node elsewhere.";
        break;

    case SwitchType::RESONANCE_STABILIZER:
        m_infoTitle = "Environmental Stabilizer";
        m_infoDescription = "Generates protective field neutralizing hazardous environmental factors. Creates temporary safe zone from radiation and toxins.";
        break;

    case SwitchType::SECURITY_SYSTEM:
        m_infoTitle = "Security Override";
        m_infoDescription = "Interface with facility's defense systems. Deactivates security doors, turrets, and surveillance systems in the immediate area.";
        break;

    case SwitchType::ENERGY_NODE:
        m_infoTitle = "Power Distribution Node";
        m_infoDescription = "Controls power flow to facility systems. Restores energy to elevators, lighting and other electrical systems nearby.";
        break;

    default:
        m_infoTitle = "Unknown Device";
        m_infoDescription = "Device of unknown origin and purpose. Effects cannot be predicted.";
        break;
    }

    // Добавляем общее предупреждение для всех переключателей
    m_infoDescription += "\n\nSTATUS: Currently in diagnostic mode only.";
}


bool Switch::updateInfoDisplay(float deltaTime) {
    if (m_displayingInfo) {
        // Увеличиваем время отображения
        m_activationTime += deltaTime;

        // Проверяем, не истекло ли время отображения (10 секунд)
        if (m_activationTime > 10.0f) {
            m_displayingInfo = false;
            return false;
        }
        return true;
    }
    return false;
}


std::string Switch::getInfoTitle() const {
    return m_infoTitle.empty() ? getName() : m_infoTitle;
}

std::string Switch::getInfoDescription() const {
    return m_infoDescription.empty() ? m_description : m_infoDescription;
}

