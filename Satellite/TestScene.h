#pragma once

#include "Scene.h"
#include "IsometricRenderer.h"
#include "Camera.h"
#include "TileRenderer.h"
#include <SDL.h>
#include <memory>

class Engine;  // Forward declaration

/**
 * @brief Тестовая сцена для проверки работы движка
 */
class TestScene : public Scene {
public:
    /**
     * @brief Конструктор
     * @param name Имя сцены
     * @param engine Указатель на движок (для доступа к ResourceManager)
     */
    TestScene(const std::string& name, Engine* engine);

    /**
     * @brief Деструктор
     */
    ~TestScene();

    /**
     * @brief Инициализация сцены
     * @return true в случае успеха, false при ошибке
     */
    bool initialize() override;

    /**
     * @brief Обработка пользовательского ввода
     * @param event Событие SDL
     */
    void handleEvent(const SDL_Event& event) override;

    /**
     * @brief Обновление сцены
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void update(float deltaTime) override;

    /**
     * @brief Отрисовка сцены
     * @param renderer Указатель на SDL_Renderer
     */
    void render(SDL_Renderer* renderer) override;

    /**
     * @brief Добавление сущности на сцену (переопределение для диагностики)
     * @param entity Указатель на сущность
     */
    void addEntity(std::shared_ptr<Entity> entity) override;

private:
    /**
     * @brief Рендерер изометрических объектов
     */
    std::shared_ptr<IsometricRenderer> m_isoRenderer;

    /**
     * @brief Камера для управления видом
     */
    std::shared_ptr<Camera> m_camera;

    /**
     * @brief Тестовая точка для демонстрации
     */
    SDL_Point m_testPoint;

    /**
     * @brief Угол вращения для анимации
     */
    float m_angle;

    /**
     * @brief Координаты тестового объекта в мировом пространстве
     */
    float m_testObjectX;
    float m_testObjectY;

    /**
     * @brief Указатель на движок (для доступа к ResourceManager)
     */
    Engine* m_engine;

    /**
     * @brief Рендерер тайлов
     */
    std::shared_ptr<TileRenderer> m_tileRenderer;

    // Кэшированные указатели на текстуры
    SDL_Texture* m_grassTexture;
    SDL_Texture* m_stoneTexture;
    SDL_Texture* m_wallTexture;
    SDL_Texture* m_wallLeftTexture;  // Новая текстура для левой грани стены
    SDL_Texture* m_wallRightTexture; // Новая текстура для правой грани стены
};