#include "InteractiveObject.h"
#include "Player.h"
#include <cmath>
#include <iostream>
#include "Logger_old.h"

InteractiveObject::InteractiveObject(const std::string& name, InteractiveType type)
    : Entity(name), m_interactiveType(type),
    m_interactionRadius(1.0f), m_isInteractable(true),
    m_interactionHint("Press E to interact"), m_height(0.3f)
{
    // Устанавливаем цвет по умолчанию в зависимости от типа объекта
    switch (type) {
    case InteractiveType::PICKUP:
        m_color = { 255, 255, 0, 255 }; // Желтый для предметов
        break;
    case InteractiveType::DOOR:
        m_color = { 139, 69, 19, 255 }; // Коричневый для дверей
        break;
    case InteractiveType::SWITCH:
        m_color = { 0, 255, 255, 255 }; // Голубой для переключателей
        break;
    case InteractiveType::TERMINAL:
        m_color = { 0, 255, 0, 255 };   // Зеленый для терминалов
        break;
    case InteractiveType::CONTAINER:
        m_color = { 128, 0, 128, 255 }; // Фиолетовый для контейнеров
        break;
    default:
        m_color = { 200, 200, 200, 255 }; // Серый для других типов
        break;
    }
}

InteractiveObject::~InteractiveObject()
{
    // Виртуальный деструктор
}

bool InteractiveObject::initialize()
{
    // Базовая инициализация
    return true;
}

void InteractiveObject::handleEvent(const SDL_Event& event)
{
    // Базовая обработка событий из Entity
    Entity::handleEvent(event);
}

void InteractiveObject::update(float deltaTime)
{
    // Базовое обновление
}

void InteractiveObject::render(SDL_Renderer* renderer)
{
    // Базовая отрисовка будет осуществляться через TileRenderer в MapScene
}

bool InteractiveObject::interact(Player* player) {
    // Если объект не интерактивен, сразу возвращаем false
    if (!m_isInteractable) {
        return false;
    }

    // Если есть обратный вызов, вызываем его
    if (m_interactionCallback) {
        m_interactionCallback(player);
        return true;
    }

    // Базовая реализация по умолчанию
    LOG_INFO("Interacting with " + m_name);
    return true;
}

bool InteractiveObject::canInteract(float playerX, float playerY) const {
    if (!m_isInteractable) {
        return false;
    }

    // Координаты объекта (добавим учет суб-координат, если они есть)
    float objectX = m_position.x;
    float objectY = m_position.y;

    // Вычисление расстояния до игрока (евклидово расстояние)
    float dx = playerX - objectX;
    float dy = playerY - objectY;
    float distanceSquared = dx * dx + dy * dy;

    // Если расстояние меньше или равно квадрату радиуса взаимодействия, 
    // то взаимодействие возможно
    return distanceSquared <= m_interactionRadius * m_interactionRadius;
}

