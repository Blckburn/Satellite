#pragma once

#include "Scene.h"
#include "TileMap.h"
#include "IsometricRenderer.h"
#include "TileRenderer.h"
#include "Camera.h"
#include "WorldGenerator.h"
#include <SDL.h>
#include <memory>
#include <string>

// Forward declaration
class Engine;

/**
 * @brief Сцена для демонстрации генерации планетарных поверхностей
 */
class PlanetScene : public Scene {
public:
    /**
     * @brief Конструктор
     * @param name Имя сцены
     * @param engine Указатель на движок
     */
    PlanetScene(const std::string& name, Engine* engine);

    /**
     * @brief Деструктор
     */
    ~PlanetScene();

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
     * @brief Генерация новой случайной планеты
     */
    void generateRandomPlanet();

    /**
     * @brief Генерация планеты с заданными параметрами
     * @param temperature Средняя температура
     * @param waterCoverage Покрытие водой (0.0-1.0)
     * @param terrainType Тип ландшафта
     */
    void generateCustomPlanet(float temperature, float waterCoverage, MapGenerator::GenerationType terrainType);

    /**
     * @brief Переключение между режимами отображения (базовый, температура, влажность и т.д.)
     */
    void toggleDisplayMode();

    /**
     * @brief Получение информации о планете
     * @return Строка с информацией о текущей планете
     */
    std::string getPlanetInfo() const;

    /**
     * @brief Получение информации о тайле под курсором
     * @param x Координата курсора X на экране
     * @param y Координата курсора Y на экране
     * @return Строка с информацией о тайле
     */
    std::string getTileInfo(int x, int y) const;

private:
    /**
     * @brief Рендеринг тайлов карты с учетом режима отображения
     * @param renderer SDL рендерер
     * @param centerX Центр экрана X
     * @param centerY Центр экрана Y
     */
    void renderTiles(SDL_Renderer* renderer, int centerX, int centerY);

    /**
     * @brief Рендеринг информации о планете
     * @param renderer SDL рендерер
     */
    void renderPlanetInfo(SDL_Renderer* renderer);

    /**
     * @brief Рендеринг легенды для текущего режима отображения
     * @param renderer SDL рендерер
     */
    void renderDisplayLegend(SDL_Renderer* renderer);

private:
    enum class DisplayMode {
        NORMAL,      // Обычное отображение
        TEMPERATURE, // Отображение температуры
        HUMIDITY,    // Отображение влажности
        ELEVATION,   // Отображение высоты
        RADIATION,   // Отображение радиации
        RESOURCES,   // Отображение ресурсов
        BIOMES       // Отображение биомов
    };

    Engine* m_engine;                         ///< Указатель на движок
    std::shared_ptr<TileMap> m_tileMap;                ///< Карта
    std::shared_ptr<IsometricRenderer> m_isoRenderer;  ///< Изометрический рендерер
    std::shared_ptr<TileRenderer> m_tileRenderer;      ///< Рендерер тайлов
    std::shared_ptr<Camera> m_camera;                  ///< Камера
    std::shared_ptr<WorldGenerator> m_worldGenerator;  ///< Генератор миров

    float m_playerX;                          ///< X координата игрока
    float m_playerY;                          ///< Y координата игрока

    DisplayMode m_displayMode;                ///< Текущий режим отображения
    WorldGenerator::PlanetData m_planetData;  ///< Данные о текущей планете
};