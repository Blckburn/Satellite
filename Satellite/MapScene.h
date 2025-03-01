#pragma once

#include "Scene.h"
#include "TileMap.h"
#include "TileRenderer.h"
#include "IsometricRenderer.h"
#include "Camera.h"
#include <SDL.h>
#include <memory>

// Forward declaration
class Engine;

/**
 * @brief Сцена для демонстрации системы тайлов и карты
 */
class MapScene : public Scene {
public:
    /**
     * @brief Конструктор
     * @param name Имя сцены
     * @param engine Указатель на движок
     */
    MapScene(const std::string& name, Engine* engine);

    /**
     * @brief Деструктор
     */
    ~MapScene();

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
     * @brief Генерация тестовой карты
     */
    void generateTestMap();

    /**
     * @brief Получение карты
     * @return Указатель на карту
     */
    TileMap* getMap() { return m_tileMap.get(); }

private:
    /**
     * @brief Проверяет возможность диагонального перемещения
     * @param fromX Начальная X координата
     * @param fromY Начальная Y координата
     * @param toX Конечная X координата
     * @param toY Конечная Y координата
     * @return true, если диагональное перемещение возможно, false в противном случае
     */
    bool canMoveDiagonally(int fromX, int fromY, int toX, int toY);

    Engine* m_engine;                           ///< Указатель на движок
    std::shared_ptr<TileMap> m_tileMap;         ///< Карта
    std::shared_ptr<IsometricRenderer> m_isoRenderer; ///< Изометрический рендерер
    std::shared_ptr<TileRenderer> m_tileRenderer;     ///< Рендерер тайлов
    std::shared_ptr<Camera> m_camera;           ///< Камера

    float m_playerX;                            ///< X координата игрока
    float m_playerY;                            ///< Y координата игрока
};