/**
 * @file Entity.cpp
 * @brief Реализация базового класса для всех игровых объектов
 */

#include "Core/Base/Entity.h"
#include "Core/System/Logger.h"

namespace Satellite {

Entity::Entity(const std::string& name)
    : m_name(name), m_isActive(true) {
    // Инициализация позиции нулевыми значениями уже выполнена в структуре Position
    LogDebug("Creating entity: " + name);
}

Entity::~Entity() {
    LogDebug("Destroying entity: " + m_name);
}

void Entity::handleEvent(const SDL_Event& event) {
    // Базовая реализация пустая, потомки переопределяют при необходимости
}

void Entity::setPosition(float x, float y, float z) {
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
}

} // namespace Satellite