#pragma once

#include "Scene.h"
#include "TileMap.h"
#include "TileRenderer.h"
#include "IsometricRenderer.h"
#include "Camera.h"
#include "Player.h"
#include "CollisionSystem.h"
#include "EntityManager.h"
#include "InteractionSystem.h"
#include "RenderingSystem.h"
#include "UIManager.h"  // Добавлено новое включение
#include <SDL.h>
#include <memory>
#include <vector>

// Предварительное объявление классов
class Engine;
class WorldGenerator;

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
     * @brief Получение карты сцены
     * @return Указатель на карту
     */
    TileMap* getMap() { return m_tileMap.get(); }

    /**
     * @brief Расчет приоритета визуального порядка для изометрической проекции
     * @param x Координата X объекта в мировом пространстве
     * @param y Координата Y объекта в мировом пространстве
     * @param z Высота объекта
     * @return Значение приоритета для сортировки
     */
    float calculateZOrderPriority(float x, float y, float z);

    /**
     * @brief Обработка клавиш для перемещения персонажа
     */
    void detectKeyInput();


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

    /**
     * @brief Проверка возможности диагонального перемещения
     * @param fromX Начальная X координата
     * @param fromY Начальная Y координата
     * @param toX Конечная X координата
     * @param toY Конечная Y координата
     * @return true, если диагональное перемещение возможно, false в противном случае
     */
    bool canMoveDiagonally(int fromX, int fromY, int toX, int toY);

    /**
     * @brief Добавление интерактивного объекта на сцену
     * @param object Указатель на интерактивный объект
     */
    void addInteractiveObject(std::shared_ptr<InteractiveObject> object);

    /**
     * @brief Удаление интерактивного объекта со сцены
     * @param object Указатель на интерактивный объект
     */
    void removeInteractiveObject(std::shared_ptr<InteractiveObject> object);

    /**
     * @brief Метод для создания двери (используется как callback для InteractionSystem)
     * @param x X-координата двери
     * @param y Y-координата двери
     * @param name Имя двери
     */
    void createDoor(float x, float y, const std::string& name);

    /**
     * @brief Инициализирует все двери на карте, устанавливая для них систему взаимодействия
     */
    void initializeDoors();


    /**
 * @brief Получение указателя на движок
 * @return Указатель на объект Engine
 */
    Engine* getEngine() const { return m_engine; }

private:
    /**
     * @brief Отрисовка интерактивных объектов
     * @param renderer SDL рендерер
     * @param centerX X-координата центра экрана
     * @param centerY Y-координата центра экрана
     */
    void renderInteractiveObjects(SDL_Renderer* renderer, int centerX, int centerY);


    std::shared_ptr<WorldGenerator> m_worldGenerator;    ///< Генератор игрового мира
    std::shared_ptr<EntityManager> m_entityManager;      ///< Менеджер сущностей
    std::shared_ptr<InteractionSystem> m_interactionSystem; ///< Система взаимодействия
    Engine* m_engine;                                    ///< Указатель на движок
    std::shared_ptr<TileMap> m_tileMap;                  ///< Карта
    std::shared_ptr<IsometricRenderer> m_isoRenderer;    ///< Изометрический рендерер
    std::shared_ptr<TileRenderer> m_tileRenderer;        ///< Рендерер тайлов
    std::shared_ptr<Camera> m_camera;                    ///< Камера
    std::shared_ptr<Player> m_player;                    ///< Игрок
    std::shared_ptr<CollisionSystem> m_collisionSystem;  ///< Система коллизий
    std::shared_ptr<RenderingSystem> m_renderingSystem;  ///< Система рендеринга
    std::shared_ptr<UIManager> m_uiManager;              /// Добавлен новый член класса

    bool m_showDebug;                                    ///< Флаг отображения отладочной информации
    int m_currentBiome;                                  ///< Текущий биом карты
};