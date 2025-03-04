#pragma once

#include "Scene.h"
#include "TileMap.h"
#include "TileRenderer.h"
#include "IsometricRenderer.h"
#include "Camera.h"
#include "Player.h"
#include "CollisionSystem.h"
#include <SDL.h>
#include <memory>
#include "InteractiveObject.h"
#include "PickupItem.h"
#include <vector>
#include "Door.h" 

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
     * @brief Создание тестового предмета для подбора
     * @param x X-координата
     * @param y Y-координата
     * @param name Имя предмета
     * @param type Тип предмета
     * @return Указатель на созданный предмет
     */
    std::shared_ptr<PickupItem> createTestPickupItem(float x, float y, const std::string& name, PickupItem::ItemType type);

    /**
     * @brief Создание тестовой двери
     * @param x X-координата
     * @param y Y-координата
     * @param name Имя двери
     * @return Указатель на созданную дверь
     */
    std::shared_ptr<Door> createTestDoor(float x, float y, const std::string& name);


    /**
 * @brief Запоминает позицию открытой двери
 * @param x X-координата двери
 * @param y Y-координата двери
 * @param name Имя двери
 */
    void rememberDoorPosition(int x, int y, const std::string& name);

    /**
     * @brief Проверяет, является ли тайл открытой дверью
     * @param x X-координата
     * @param y Y-координата
     * @return true, если это открытая дверь
     */
    bool isOpenDoorTile(int x, int y) const;

    /**
     * @brief Закрывает дверь на указанной позиции
     * @param x X-координата
     * @param y Y-координата
     */
    void closeDoorAtPosition(int x, int y);

    /**
 * @brief Удаляет информацию о двери из списка открытых дверей
 * @param x X-координата двери
 * @param y Y-координата двери
 */
    void forgetDoorPosition(int x, int y);


private:
    std::vector<std::shared_ptr<InteractiveObject>> m_interactiveObjects;  ///< Интерактивные объекты на сцене
    float m_interactionPromptTimer;                                        ///< Таймер для отображения подсказки
    std::string m_interactionPrompt;                                       ///< Текст подсказки для взаимодействия
    bool m_showInteractionPrompt;                                          ///< Флаг отображения подсказки
    Engine* m_engine;                           ///< Указатель на движок
    std::shared_ptr<TileMap> m_tileMap;         ///< Карта
    std::shared_ptr<IsometricRenderer> m_isoRenderer; ///< Изометрический рендерер
    std::shared_ptr<TileRenderer> m_tileRenderer;     ///< Рендерер тайлов
    std::shared_ptr<Camera> m_camera;           ///< Камера
    std::shared_ptr<Player> m_player;           ///< Игрок
    std::shared_ptr<CollisionSystem> m_collisionSystem;  ///< Система коллизий

    bool m_showDebug;                           ///< Флаг отображения отладочной информации
    int m_currentBiome;                         ///< Текущий биом карты

    // Структура для хранения информации об открытых дверях
    struct OpenDoorInfo {
        int tileX;          // X-координата двери
        int tileY;          // Y-координата двери
        std::string name;   // Имя двери
    };

    std::vector<OpenDoorInfo> m_openDoors;  ///< Список открытых дверей

    /**
     * @brief Сокращает длинный текст, если он превышает максимальную длину
     * @param text Исходный текст
     * @param maxLength Максимальная длина текста
     * @return Сокращенный текст или исходный текст, если он короче maxLength
     */
    std::string truncateText(const std::string& text, size_t maxLength);


    /**
     * @brief Обработка взаимодействия с объектами
     */
    void handleInteraction();

    /**
     * @brief Поиск ближайшего интерактивного объекта
     * @param playerX X-координата игрока
     * @param playerY Y-координата игрока
     * @return Указатель на ближайший интерактивный объект или nullptr
     */
    std::shared_ptr<InteractiveObject> findNearestInteractiveObject(float playerX, float playerY);

    /**
     * @brief Отрисовка интерактивных объектов
     * @param renderer SDL рендерер
     * @param centerX X-координата центра экрана
     * @param centerY Y-координата центра экрана
     */
    void renderInteractiveObjects(SDL_Renderer* renderer, int centerX, int centerY);

    /**
     * @brief Отрисовка подсказки для взаимодействия
     * @param renderer SDL рендерер
     */
    void renderInteractionPrompt(SDL_Renderer* renderer);

    /**
    * @brief Создает интерактивные предметы на карте
    */
    void createInteractiveItems();

};