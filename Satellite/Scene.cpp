#include "Scene.h"
#include "Entity.h"
#include <algorithm>
#include <iostream>

Scene::Scene(const std::string& name) : m_name(name) {
    std::cout << "Creating scene: " << name << std::endl;
}

Scene::~Scene() {
    std::cout << "Destroying scene: " << m_name << std::endl;
    m_entities.clear();
}

void Scene::handleEvent(const SDL_Event& event) {
    // Базовая реализация - передаем события всем сущностям
    for (auto& entity : m_entities) {
        if (entity->isActive()) {
            entity->handleEvent(event);
        }
    }
}

void Scene::update(float deltaTime) {
    // Обновляем все активные сущности
    for (auto& entity : m_entities) {
        if (entity->isActive()) {
            entity->update(deltaTime);
        }
    }
}

void Scene::render(SDL_Renderer* renderer) {
    // Отрисовываем все активные сущности
    for (auto& entity : m_entities) {
        if (entity->isActive()) {
            entity->render(renderer);
        }
    }
}

void Scene::addEntity(std::shared_ptr<Entity> entity) {
    // Добавляем сущность в список, если она еще не существует
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it == m_entities.end()) {
        m_entities.push_back(entity);
    }
}

void Scene::removeEntity(std::shared_ptr<Entity> entity) {
    // Удаляем сущность из списка
    m_entities.erase(
        std::remove(m_entities.begin(), m_entities.end(), entity),
        m_entities.end()
    );
}