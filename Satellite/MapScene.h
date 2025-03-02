#pragma once

#include "Scene.h"
#include "TileMap.h"
#include "TileRenderer.h"
#include "IsometricRenderer.h"
#include "Camera.h"
#include <SDL.h>
#include <memory>
#include "Player.h"

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


    /**
     * @brief Отрисовка отладочной информации
     * @param renderer Указатель на SDL_Renderer
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void renderDebug(SDL_Renderer* renderer, int centerX, int centerY);

    /**
     * @brief Отрисовка сцены с использованием блочной Z-сортировки
     * @param renderer SDL рендерер
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void renderWithBlockSorting(SDL_Renderer* renderer, int centerX, int centerY);

    /**
     * @brief Добавление тайла в рендерер с указанным приоритетом
     * @param x Координата X тайла на карте
     * @param y Координата Y тайла на карте
     * @param priority Приоритет отрисовки
     */
    void addTileToRenderer(int x, int y, float priority);

    /**
     * @brief Отрисовка индикатора персонажа, когда он скрыт стенами
     * @param renderer SDL рендерер
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void renderPlayerIndicator(SDL_Renderer* renderer, int centerX, int centerY);

    /**
     * @brief Добавление объемного тайла в рендерер с указанным приоритетом
     * @param x Координата X тайла на карте
     * @param y Координата Y тайла на карте
     * @param priority Приоритет отрисовки
     */
    void addVolumetricTileToRenderer(int x, int y, float priority);

    /**
     * @brief Отрисовка персонажа с гарантией, что он будет виден над полом
     * @param renderer SDL рендерер
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     * @param priority Приоритет отрисовки
     */
    void renderPlayer(SDL_Renderer* renderer, int centerX, int centerY, float priority);

    /**
 * @brief Получение текущего биома
 * @return Номер текущего биома
 */
    int getCurrentBiome() const { return m_currentBiome; }

private:

    int m_currentBiome = 0;  ///< Текущий биом карты
    bool m_showDebug;           ///< Флаг отображения отладочной информации

    Engine* m_engine;                           ///< Указатель на движок
    std::shared_ptr<TileMap> m_tileMap;         ///< Карта
    std::shared_ptr<IsometricRenderer> m_isoRenderer; ///< Изометрический рендерер
    std::shared_ptr<TileRenderer> m_tileRenderer;     ///< Рендерер тайлов
    std::shared_ptr<Camera> m_camera;           ///< Камера
    std::shared_ptr<Player> m_player;     ///< Указатель на игрока
};