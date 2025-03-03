#include "PickupItem.h"
#include "Player.h"
#include <cmath>
#include <iostream>
#include "Logger.h"

PickupItem::PickupItem(const std::string& name, ItemType itemType)
    : InteractiveObject(name, InteractiveType::PICKUP),
    m_itemType(itemType), m_value(1), m_weight(1.0f),
    m_isPulsating(true), m_pulsePhase(0.0f),
    m_floatHeight(0.0f), m_rotationAngle(0.0f)
{
    // Настройка базовых параметров
    setInteractionRadius(1.2f); // Немного увеличенный радиус для удобства
    setInteractionHint("Press E to pick up");

    // Настройка цвета в зависимости от типа предмета
    switch (itemType) {
    case ItemType::RESOURCE:
        m_color = { 150, 150, 200, 255 }; // Бледно-голубой для ресурсов
        break;
    case ItemType::WEAPON:
        m_color = { 255, 50, 50, 255 };   // Красный для оружия
        break;
    case ItemType::ARMOR:
        m_color = { 50, 50, 255, 255 };   // Синий для брони
        break;
    case ItemType::CONSUMABLE:
        m_color = { 50, 255, 50, 255 };   // Зеленый для расходуемых предметов
        break;
    case ItemType::KEY:
        m_color = { 255, 215, 0, 255 };   // Золотой для ключей
        break;
    default:
        m_color = { 255, 255, 0, 255 };   // Желтый для обычных предметов
        break;
    }

    // Установка стандартного описания
    m_description = "A " + name;
}

PickupItem::~PickupItem()
{
    // Освобождение ресурсов, если необходимо
}

bool PickupItem::initialize()
{
    // Базовая инициализация
    return InteractiveObject::initialize();
}

void PickupItem::update(float deltaTime) {
    // Обновляем базовый класс
    InteractiveObject::update(deltaTime);

    // Обновление визуальных эффектов
    if (m_isPulsating) {
        // Обновляем фазу пульсации
        m_pulsePhase += deltaTime * 2.0f; // Скорость пульсации
        if (m_pulsePhase >= 2.0f * M_PI) {
            m_pulsePhase -= 2.0f * M_PI;
        }

        // Вычисляем высоту "парения" на основе синусоиды
        m_floatHeight = 0.15f * sinf(m_pulsePhase); // Увеличенная амплитуда для лучшей видимости

        // Обновляем угол вращения
        m_rotationAngle += deltaTime * 60.0f; // Увеличенная скорость вращения
        if (m_rotationAngle >= 360.0f) {
            m_rotationAngle -= 360.0f;
        }
    }
}

void PickupItem::render(SDL_Renderer* renderer)
{
    // Базовая отрисовка будет осуществляться через TileRenderer в MapScene
    // Особая отрисовка может быть реализована здесь, если необходимо
    InteractiveObject::render(renderer);
}

bool PickupItem::interact(Player* player) {
    if (!player) {
        return false;
    }

    // Проверяем, можно ли взаимодействовать с предметом
    if (!isInteractable()) {
        return false;
    }

    // Логика подбора предмета
    // В будущем здесь будет добавление предмета в инвентарь игрока
    LOG_INFO("Player picked up " + m_name);

    // Вызываем обратный вызов, если он установлен
    if (m_interactionCallback) {
        m_interactionCallback(player);
    }

    // Удаляем предмет после подбора, установив его неактивным
    setActive(false);
    setInteractable(false);

    return true;
}