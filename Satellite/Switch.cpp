#include "Switch.h"
#include "Player.h"
#include "Logger.h"
#include "TileMap.h"
#include "MapScene.h"
#include <cmath>

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

    // Если переключатель не активируется в данный момент и не активирован,
    // начинаем процесс активации
    if (!m_isActivating && !m_activated) {
        return startActivation();
    }
    // Если переключатель уже активирован и имеет временный эффект, можем продлить
    else if (m_activated && m_effectDuration > 0 && !m_isActivating) {
        LOG_INFO("Extending effect duration for " + getName());
        m_effectTimer = 0.0f; // Сбрасываем таймер эффекта
        m_displayingInfo = true;
        return true;
    }

    return false;
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
        return;
    }

    LOG_INFO("Completing activation of switch: " + getName());
    m_isActivating = false;
    m_activated = true;
    m_effectActive = true;
    m_effectTimer = 0.0f;

    // Применяем эффект к окружению
    applyEffect();

    // Вызываем функцию обратного вызова, если она задана
    if (m_activationCallback) {
        m_activationCallback(nullptr, this);
    }

    // Обновляем подсказку
    if (m_effectDuration > 0) {
        setInteractionHint("Effect active: " + std::to_string(static_cast<int>(m_effectDuration)) + " seconds");
    }
    else {
        setInteractionHint("Effect active: permanent");
    }

    // Меняем цвет на активный
    setColor(m_activeColor);
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

    // Если идет процесс активации
    if (m_isActivating) {
        // Увеличиваем таймер и прогресс
        m_activationTimer += deltaTime;
        m_activationProgress = std::min(1.0f, m_activationTimer / m_activationRequiredTime);

        // Обновляем подсказку с текущим прогрессом
        updateActivationHint();

        // Если процесс завершен
        if (m_activationProgress >= 1.0f) {
            completeActivation();
        }

        // Обновляем визуальные эффекты в зависимости от прогресса
        updateVisualEffects(deltaTime);
    }

    // Если переключатель активирован и эффект имеет ограниченную длительность
    if (m_effectActive && m_effectDuration > 0) {
        m_effectTimer += deltaTime;

        // Обновляем подсказку с оставшимся временем
        if (static_cast<int>(m_effectTimer) % 5 == 0) { // Обновляем каждые 5 секунд
            updateActivationHint();
        }

        // Если время эффекта истекло
        if (m_effectTimer >= m_effectDuration) {
            deactivateEffect();
        }
    }

    // Если переключатель отображает информацию, проверяем, не пора ли скрыть её
    if (m_displayingInfo) {
        m_activationTime += deltaTime;
        if (m_activationTime > 5.0f) {
            m_displayingInfo = false;
            m_activationTime = 0.0f;
        }
    }

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
    LOG_INFO("Applying effect of switch: " + getName());

    // Различные эффекты в зависимости от типа переключателя
    switch (m_switchType) {
    case SwitchType::GRAVITY_ANOMALY:
        // Для гравитационной аномалии - меняем свойства тайлов в зоне действия
        // Например, делаем непроходимые тайлы проходимыми на время
        if (m_tileMap) {
            // Получаем позицию переключателя
            int switchX = static_cast<int>(getPosition().x);
            int switchY = static_cast<int>(getPosition().y);

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
                            // Здесь можно изменять свойства тайлов, например:
                            MapTile* tile = m_tileMap->getTile(x, y);
                            if (tile) {
                                // Запоминаем исходное состояние тайла (для будущего восстановления)
                                // В будущем можно добавить карту исходных состояний тайлов

                                // Меняем свойства тайла (например, делаем непроходимые
                                // препятствия временно проходимыми)
                                if (!tile->isWalkable() && tile->getType() != TileType::WATER) {
                                    tile->setWalkable(true);
                                    LOG_INFO("Made tile walkable at: " + std::to_string(x) + ", " + std::to_string(y));
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
        // Это может потребовать взаимодействия с MapScene
        if (m_parentScene) {
            // В будущей реализации: активация телепорта
            LOG_INFO("Teleport gate activated, but functionality not yet implemented");
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
            // Аналогично applyEffect, но восстанавливаем исходные свойства
            // В полной реализации нужно хранить и восстанавливать исходные состояния
            LOG_INFO("Restoring normal gravity");
        }
        break;

    case SwitchType::TELEPORT_GATE:
        // Деактивируем телепорт
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
    setInteractionHint("Press E to activate " + getName());
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