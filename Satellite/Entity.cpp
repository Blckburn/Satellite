#include "Entity.h"
#include <iostream>

Entity::Entity(const std::string& name)
    : m_name(name), m_isActive(true) {
    // Инициализация позиции нулевыми значениями уже выполнена в структуре Position
    std::cout << "Creating entity: " << name << std::endl;
}

Entity::~Entity() {
    std::cout << "Destroying entity: " << m_name << std::endl;
}

void Entity::handleEvent(const SDL_Event& event) {
    // Базовая реализация пустая, потомки переопределяют при необходимости
}

void Entity::setPosition(float x, float y, float z) {
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
}