/**
 * @file Satellite.h
 * @brief Единый заголовочный файл для движка Satellite Engine
 *
 * Этот файл содержит объявления всех основных интерфейсов, классов, перечислений
 * и структур данных, необходимых для работы с Satellite Engine.
 */

#pragma once

 // Стандартные включения
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <cmath>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>

// Включения SDL
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Color;
struct SDL_Window;
struct SDL_Event;
struct SDL_Point;
struct SDL_Rect;
typedef struct _TTF_Font TTF_Font;

// Константы движка
namespace Constants {
    // Размеры тайлов по умолчанию
    constexpr int DEFAULT_TILE_WIDTH = 64;
    constexpr int DEFAULT_TILE_HEIGHT = 32;

    // Ограничения карты
    constexpr int MAX_MAP_SIZE = 256;

    // Параметры рендеринга
    constexpr float HEIGHT_SCALE = 30.0f;  // Масштаб для высоты в изометрии

    // Параметры игрока
    constexpr float PLAYER_DEFAULT_SPEED = 0.05f;
    constexpr float PLAYER_DEFAULT_HEIGHT = 0.5f;

    // Параметры взаимодействия
    constexpr float DEFAULT_INTERACTION_RADIUS = 1.5f;

    // Параметры генерации карты
    constexpr int MIN_ROOM_SIZE = 7;
    constexpr int MAX_ROOM_SIZE = 15;
    constexpr int MIN_ROOMS = 5;
    constexpr int MAX_ROOMS = 10;
}

namespace Satellite {

    // =============================
    // ПРЕДВАРИТЕЛЬНЫЕ ОБЪЯВЛЕНИЯ
    // =============================
    class Engine;
    class Scene;
    class MapScene;
    class Entity;
    class TileMap;
    class MapTile;
    class Player;
    class InteractiveObject;
    class PickupItem;
    class Door;
    class Camera;
    class CollisionSystem;
    class IsometricRenderer;
    class TileRenderer;
    class ResourceManager;
    class RoomGenerator;
    class RenderableTile;
    class Logger;

    // =============================
    // ПЕРЕЧИСЛЕНИЯ
    // =============================

    /**
     * @brief Типы тайлов карты
     */
    enum class TileType {
        EMPTY,          ///< Пустой тайл (отсутствует)
        FLOOR,          ///< Обычный пол
        WALL,           ///< Стена
        DOOR,           ///< Дверь
        WATER,          ///< Вода
        GRASS,          ///< Трава
        STONE,          ///< Камень
        METAL,          ///< Металл
        GLASS,          ///< Стекло
        WOOD,           ///< Дерево
        SPECIAL,        ///< Специальный тайл (телепорт, ловушка и т.д.)
        OBSTACLE,       ///< Непроходимое препятствие
        SAND,           ///< Песок
        SNOW,           ///< Снег
        ICE,            ///< Лед
        ROCK_FORMATION, ///< Скалы
        LAVA,           ///< Лава
        FOREST          ///< Лес
    };

    /**
     * @brief Типы интерактивных объектов
     */
    enum class InteractiveType {
        PICKUP,        ///< Предмет, который можно подобрать
        DOOR,          ///< Дверь, которую можно открыть/закрыть
        SWITCH,        ///< Переключатель, который можно активировать
        TERMINAL,      ///< Терминал, с которым можно взаимодействовать
        CONTAINER,     ///< Контейнер, который можно открыть
        CUSTOM         ///< Пользовательский тип
    };

    /**
     * @brief Типы предметов
     */
    enum class ItemType {
        RESOURCE,   ///< Ресурс (материал)
        WEAPON,     ///< Оружие
        ARMOR,      ///< Броня
        CONSUMABLE, ///< Расходуемый предмет
        KEY,        ///< Ключ или предмет квеста
        GENERIC     ///< Обычный предмет
    };

    /**
     * @brief Типы биомов для генерации карт
     */
    enum class BiomeType {
        DEFAULT,    ///< Стандартный биом со стенами и полом
        FOREST,     ///< Лесной биом с деревьями и травой
        DESERT,     ///< Пустынный биом с песком и камнями
        TUNDRA,     ///< Тундра со снегом и льдом
        VOLCANIC    ///< Вулканический биом с камнями и лавой
    };

    /**
     * @brief Уровни логирования
     */
    enum class LogLevel {
        DEBUG,      ///< Отладочная информация
        INFO,       ///< Информационное сообщение
        WARNING,    ///< Предупреждение
        ERROR,      ///< Ошибка
        NONE        ///< Вывод отключен
    };

    /**
     * @brief Направления персонажа
     */
    enum class Direction {
        NORTH,      ///< Север
        NORTHEAST,  ///< Северо-восток
        EAST,       ///< Восток
        SOUTHEAST,  ///< Юго-восток
        SOUTH,      ///< Юг
        SOUTHWEST,  ///< Юго-запад
        WEST,       ///< Запад
        NORTHWEST   ///< Северо-запад
    };

    // =============================
    // СТРУКТУРЫ
    // =============================

    /**
     * @brief Структура, содержащая результат проверки коллизии
     */
    struct CollisionResult {
        bool collision;       ///< Было ли столкновение
        float adjustedX;      ///< Скорректированная X-координата
        float adjustedY;      ///< Скорректированная Y-координата
        bool slidingX;        ///< Происходит ли скольжение по оси X
        bool slidingY;        ///< Происходит ли скольжение по оси Y

        /**
         * @brief Конструктор по умолчанию
         */
        CollisionResult()
            : collision(false), adjustedX(0.0f), adjustedY(0.0f), slidingX(false), slidingY(false) {
        }
    };

    /**
     * @brief Структура для хранения информации о тайле для рендеринга
     */
    struct RenderableTile {
        float worldX = 0.0f;       ///< X-координата в мировом пространстве
        float worldY = 0.0f;       ///< Y-координата в мировом пространстве
        float worldZ = 0.0f;       ///< Высота (Z-координата)
        float renderPriority = 0.0f; ///< Приоритет рендеринга

        // Текстуры для разных граней
        SDL_Texture* topTexture = nullptr;
        SDL_Texture* leftTexture = nullptr;
        SDL_Texture* rightTexture = nullptr;

        // Цвета для граней
        SDL_Color topColor = { 255, 255, 255, 255 };
        SDL_Color leftColor = { 200, 200, 200, 255 };
        SDL_Color rightColor = { 150, 150, 150, 255 };

        enum class TileType {
            FLAT,       ///< Плоский тайл
            VOLUMETRIC  ///< Объемный тайл
        } type = TileType::FLAT;
    };

    /**
     * @brief Структура для хранения информации об открытой двери
     */
    struct OpenDoorInfo {
        int tileX;          ///< X-координата двери
        int tileY;          ///< Y-координата двери
        std::string name;   ///< Имя двери
    };

    // =============================
    // БАЗОВЫЕ КЛАССЫ
    // =============================

    /**
     * @brief Основной класс движка, управляющий игровым циклом
     */
    class Engine {
    public:
        /**
         * @brief Конструктор
         * @param title Заголовок окна
         * @param width Ширина окна
         * @param height Высота окна
         */
        Engine(const std::string& title, int width, int height);

        /**
         * @brief Инициализация движка
         * @return true в случае успеха, false при ошибке
         */
        bool initialize();

        /**
         * @brief Запускает игровой цикл
         */
        void run();

        /**
         * @brief Завершает работу движка
         */
        void shutdown();

        /**
         * @brief Устанавливает активную сцену
         * @param scene Указатель на сцену
         */
        void setActiveScene(std::shared_ptr<Scene> scene);

        /**
         * @brief Получает указатель на SDL_Renderer
         * @return Указатель на SDL_Renderer
         */
        SDL_Renderer* getRenderer() const;

        /**
         * @brief Получает указатель на ResourceManager
         * @return Указатель на ResourceManager
         */
        std::shared_ptr<ResourceManager> getResourceManager() const;

        /**
         * @brief Получает время, прошедшее с последнего кадра
         * @return Время в секундах
         */
        float getDeltaTime() const;

        /**
         * @brief Проверяет, работает ли движок
         * @return true, если движок работает, false в противном случае
         */
        bool isRunning() const;

        /**
         * @brief Установка текущего биома для визуализации
         * @param biomeType Тип биома
         */
        void setCurrentBiome(int biomeType);
    };

    /**
     * @brief Базовый класс для игровых сцен
     */
    class Scene {
    public:
        /**
         * @brief Конструктор
         * @param name Имя сцены
         */
        Scene(const std::string& name);

        /**
         * @brief Виртуальный деструктор
         */
        virtual ~Scene();

        /**
         * @brief Инициализация сцены
         * @return true в случае успеха, false при ошибке
         */
        virtual bool initialize() = 0;

        /**
         * @brief Обработка пользовательского ввода
         * @param event Событие SDL
         */
        virtual void handleEvent(const SDL_Event& event);

        /**
         * @brief Обновление сцены
         * @param deltaTime Время, прошедшее с предыдущего кадра
         */
        virtual void update(float deltaTime);

        /**
         * @brief Отрисовка сцены
         * @param renderer Указатель на SDL_Renderer
         */
        virtual void render(SDL_Renderer* renderer);

        /**
         * @brief Добавление сущности на сцену
         * @param entity Указатель на сущность
         */
        virtual void addEntity(std::shared_ptr<Entity> entity);

        /**
         * @brief Удаление сущности со сцены
         * @param entity Указатель на сущность
         */
        virtual void removeEntity(std::shared_ptr<Entity> entity);

        /**
         * @brief Получение имени сцены
         * @return Имя сцены
         */
        const std::string& getName() const;

    protected:
        std::string m_name;                         ///< Имя сцены
        std::vector<std::shared_ptr<Entity>> m_entities;  ///< Список сущностей на сцене
    };

    /**
     * @brief Основная игровая сцена с рендерингом карты
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
        TileMap* getMap();

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

        /**
         * @brief Получение текущего биома
         * @return Номер текущего биома
         */
        int getCurrentBiome() const;

    private:
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
         * @brief Создание интерактивных предметов на карте
         */
        void createInteractiveItems();

        /**
         * @brief Генерирует двери в коридорах карты
         * @param doorProbability Вероятность размещения двери в подходящем месте (0.0-1.0)
         * @param maxDoors Максимальное количество дверей для генерации
         */
        void generateDoors(float doorProbability = 0.4f, int maxDoors = 8);

        Engine* m_engine;                           ///< Указатель на движок
        std::shared_ptr<TileMap> m_tileMap;         ///< Карта
        std::shared_ptr<IsometricRenderer> m_isoRenderer; ///< Изометрический рендерер
        std::shared_ptr<TileRenderer> m_tileRenderer;     ///< Рендерер тайлов
        std::shared_ptr<Camera> m_camera;           ///< Камера
        std::shared_ptr<Player> m_player;           ///< Игрок
        std::shared_ptr<CollisionSystem> m_collisionSystem;  ///< Система коллизий
        std::vector<std::shared_ptr<InteractiveObject>> m_interactiveObjects;  ///< Интерактивные объекты
        std::vector<OpenDoorInfo> m_openDoors;      ///< Список открытых дверей
        std::shared_ptr<Door> m_currentInteractingDoor; ///< Текущая дверь для взаимодействия
        bool m_isInteractingWithDoor;               ///< Флаг взаимодействия с дверью
        int m_currentBiome;                         ///< Текущий биом карты
        bool m_showDebug;                           ///< Флаг отображения отладочной информации
    };

    /**
     * @brief Базовый класс для всех игровых объектов
     */
    class Entity {
    public:
        /**
         * @brief Структура для хранения позиции сущности
         */
        struct Position {
            float x = 0.0f;  ///< Координата X
            float y = 0.0f;  ///< Координата Y
            float z = 0.0f;  ///< Координата Z (для изометрической проекции и сортировки)
        };

        /**
         * @brief Конструктор
         * @param name Имя сущности
         */
        Entity(const std::string& name);

        /**
         * @brief Виртуальный деструктор
         */
        virtual ~Entity();

        /**
         * @brief Инициализация сущности
         * @return true в случае успеха, false при ошибке
         */
        virtual bool initialize() = 0;

        /**
         * @brief Обработка пользовательского ввода
         * @param event Событие SDL
         */
        virtual void handleEvent(const SDL_Event& event);

        /**
         * @brief Обновление состояния сущности
         * @param deltaTime Время, прошедшее с предыдущего кадра
         */
        virtual void update(float deltaTime) = 0;

        /**
         * @brief Отрисовка сущности
         * @param renderer Указатель на SDL_Renderer
         */
        virtual void render(SDL_Renderer* renderer) = 0;

        /**
         * @brief Установка позиции сущности
         * @param x Координата X
         * @param y Координата Y
         * @param z Координата Z (для сортировки по глубине)
         */
        void setPosition(float x, float y, float z = 0.0f);

        /**
         * @brief Получение текущей позиции сущности
         * @return Структура с координатами
         */
        const Position& getPosition() const;

        /**
         * @brief Получение имени сущности
         * @return Имя сущности
         */
        const std::string& getName() const;

        /**
         * @brief Проверка, активна ли сущность
         * @return true, если сущность активна, false в противном случае
         */
        bool isActive() const;

        /**
         * @brief Установка активности сущности
         * @param active Флаг активности
         */
        void setActive(bool active);

    protected:
        std::string m_name;    ///< Имя сущности
        Position m_position;   ///< Позиция сущности
        bool m_isActive;       ///< Флаг активности сущности
    };

    /**
     * @brief Базовый класс для всех интерактивных объектов игрового мира
     */
    class InteractiveObject : public Entity {
    public:
        /**
         * @brief Конструктор
         * @param name Имя объекта
         * @param type Тип интерактивного объекта
         */
        InteractiveObject(const std::string& name, InteractiveType type);

        /**
         * @brief Виртуальный деструктор
         */
        virtual ~InteractiveObject();

        /**
         * @brief Инициализация объекта
         * @return true в случае успеха, false при ошибке
         */
        virtual bool initialize() override;

        /**
         * @brief Обработка пользовательского ввода
         * @param event Событие SDL
         */
        virtual void handleEvent(const SDL_Event& event) override;

        /**
         * @brief Обновление состояния объекта
         * @param deltaTime Время, прошедшее с предыдущего кадра
         */
        virtual void update(float deltaTime) override;

        /**
         * @brief Отрисовка объекта
         * @param renderer Указатель на SDL_Renderer
         */
        virtual void render(SDL_Renderer* renderer) override;

        /**
         * @brief Взаимодействие с объектом
         * @param player Указатель на игрока
         * @return true, если взаимодействие было успешным
         */
        virtual bool interact(Player* player);

        /**
         * @brief Проверка, может ли игрок взаимодействовать с объектом
         * @param playerX X-координата игрока
         * @param playerY Y-координата игрока
         * @return true, если взаимодействие возможно
         */
        bool canInteract(float playerX, float playerY) const;

        /**
         * @brief Получение типа интерактивного объекта
         * @return Тип объекта
         */
        InteractiveType getInteractiveType() const;

        /**
         * @brief Установка радиуса взаимодействия
         * @param radius Радиус взаимодействия
         */
        void setInteractionRadius(float radius);

        /**
         * @brief Получение радиуса взаимодействия
         * @return Радиус взаимодействия
         */
        float getInteractionRadius() const;

        /**
         * @brief Проверка, активен ли объект для взаимодействия
         * @return true, если объект активен
         */
        bool isInteractable() const;

        /**
         * @brief Установка активности объекта для взаимодействия
         * @param interactable true, если объект должен быть активен
         */
        void setInteractable(bool interactable);

        /**
         * @brief Установка подсказки для взаимодействия
         * @param hint Текст подсказки
         */
        void setInteractionHint(const std::string& hint);

        /**
         * @brief Получение подсказки для взаимодействия
         * @return Текст подсказки
         */
        const std::string& getInteractionHint() const;

        /**
         * @brief Установка обратного вызова для взаимодействия
         * @param callback Функция обратного вызова
         */
        void setInteractionCallback(std::function<void(Player*)> callback);

        /**
         * @brief Получение цвета объекта
         * @return Цвет объекта
         */
        const SDL_Color& getColor() const;

        /**
         * @brief Установка цвета объекта
         * @param color Цвет объекта
         */
        void setColor(const SDL_Color& color);

        /**
         * @brief Получение высоты объекта
         * @return Высота объекта
         */
        virtual float getHeight() const;

        /**
         * @brief Установка высоты объекта
         * @param height Высота объекта
         */
        virtual void setHeight(float height);

    protected:
        InteractiveType m_interactiveType;                  ///< Тип интерактивного объекта
        float m_interactionRadius;                          ///< Радиус, в котором возможно взаимодействие
        bool m_isInteractable;                              ///< Можно ли взаимодействовать с объектом
        std::string m_interactionHint;                      ///< Подсказка при взаимодействии
        std::function<void(Player*)> m_interactionCallback; ///< Обратный вызов при взаимодействии
        SDL_Color m_color;                                  ///< Цвет объекта
        float m_height;                                     ///< Высота объекта
    };

    /**
     * @brief Класс для представления двери в игровом мире
     */
    class Door : public InteractiveObject, public std::enable_shared_from_this<Door> {
    public:
        /**
         * @brief Конструктор
         * @param name Имя двери
         * @param tileMap Указатель на карту для обновления проходимости
         * @param parentScene Указатель на родительскую сцену
         * @param biomeType Тип биома (1-Forest, 2-Desert, 3-Tundra, 4-Volcanic)
         */
        Door(const std::string& name, TileMap* tileMap, MapScene* parentScene = nullptr, int biomeType = 1);

        /**
         * @brief Инициализация двери
         * @return true в случае успеха, false при ошибке
         */
        bool initialize() override;

        /**
         * @brief Обновление состояния двери
         * @param deltaTime Время, прошедшее с предыдущего кадра
         */
        void update(float deltaTime) override;

        /**
         * @brief Взаимодействие с дверью (открытие/закрытие)
         * @param player Указатель на игрока
         * @return true, если взаимодействие было успешным
         */
        bool interact(Player* player) override;

        /**
         * @brief Начать процесс взаимодействия с дверью (открытие/закрытие с кастом)
         * @return true, если взаимодействие было начато успешно
         */
        bool startInteraction();

        /**
         * @brief Отменить процесс взаимодействия с дверью
         */
        void cancelInteraction();

        /**
         * @brief Проверить, идет ли процесс взаимодействия с дверью
         * @return true, если идет процесс взаимодействия
         */
        bool isInteracting() const;

        /**
         * @brief Получить текущий прогресс взаимодействия
         * @return Значение от 0.0 до 1.0
         */
        float getInteractionProgress() const;

        /**
         * @brief Отрисовка индикатора взаимодействия над дверью
         * @param renderer SDL рендерер
         * @param isoRenderer Изометрический рендерер для преобразования координат
         * @param centerX X координата центра экрана
         * @param centerY Y координата центра экрана
         */
        void render(SDL_Renderer* renderer, IsometricRenderer* isoRenderer, int centerX, int centerY);

        /**
         * @brief Проверка, открыта ли дверь
         * @return true, если дверь открыта, false если закрыта
         */
        bool isOpen() const;

        /**
         * @brief Открытие/закрытие двери
         * @param open true для открытия, false для закрытия
         */
        void setOpen(bool open);

        /**
         * @brief Задает ориентацию двери (горизонтальная/вертикальная)
         * @param isVertical true для вертикальной двери, false для горизонтальной
         */
        void setVertical(bool isVertical);

        /**
         * @brief Возвращает ориентацию двери
         * @return true если дверь вертикальная, false если горизонтальная
         */
        bool isVertical() const;

        /**
         * @brief Сбрасывает флаг требования отпускания клавиши
         */
        void resetKeyReleaseRequirement();

        /**
         * @brief Проверяет, требуется ли отпустить клавишу перед новым взаимодействием
         * @return true, если требуется отпустить клавишу
         */
        bool isRequiringKeyRelease() const;

    private:
        bool m_isOpen;                      ///< Флаг состояния двери (открыта/закрыта)
        TileMap* m_tileMap;                 ///< Указатель на карту для обновления проходимости
        int m_tileX;                        ///< X координата тайла двери
        int m_tileY;                        ///< Y координата тайла двери
        SDL_Color m_openColor;              ///< Цвет двери в открытом состоянии
        SDL_Color m_closedColor;            ///< Цвет двери в закрытом состоянии
        MapScene* m_parentScene;            ///< Указатель на родительскую сцену
        bool m_isVertical;                  ///< Ориентация двери
        int m_biomeType;                    ///< Тип биома
        bool m_isInteracting;               ///< Флаг процесса взаимодействия
        float m_interactionTimer;           ///< Таймер взаимодействия
        float m_interactionRequiredTime;    ///< Требуемое время взаимодействия
        float m_interactionProgress;        ///< Прогресс взаимодействия
        bool m_actionJustCompleted;         ///< Флаг завершения действия
        float m_cooldownTimer;              ///< Таймер кулдауна
        bool m_requireKeyRelease;           ///< Требуется отпустить клавишу
    };

    /**
     * @brief Класс предмета, который можно подобрать
     */
    class PickupItem : public InteractiveObject {
    public:
        /**
         * @brief Конструктор
         * @param name Имя предмета
         * @param itemType Тип предмета
         */
        PickupItem(const std::string& name, ItemType itemType = ItemType::GENERIC);

        /**
         * @brief Деструктор
         */
        virtual ~PickupItem();

        /**
         * @brief Инициализация предмета
         * @return true в случае успеха, false при ошибке
         */
        virtual bool initialize() override;

        /**
         * @brief Обновление состояния предмета
         * @param deltaTime Время, прошедшее с предыдущего кадра
         */
        virtual void update(float deltaTime) override;

        /**
         * @brief Отрисовка предмета
         * @param renderer Указатель на SDL_Renderer
         */
        virtual void render(SDL_Renderer* renderer) override;

        /**
         * @brief Взаимодействие с предметом (подбор)
         * @param player Указатель на игрока
         * @return true, если взаимодействие было успешным
         */
        virtual bool interact(Player* player) override;

        /**
         * @brief Получение типа предмета
         * @return Тип предмета
         */
        ItemType getItemType() const;

        /**
         * @brief Установка ценности предмета
         * @param value Ценность предмета
         */
        void setValue(int value);

        /**
         * @brief Получение ценности предмета
         * @return Ценность предмета
         */
        int getValue() const;

        /**
         * @brief Установка веса предмета
         * @param weight Вес предмета
         */
        void setWeight(float weight);

        /**
         * @brief Получение веса предмета
         * @return Вес предмета
         */
        float getWeight() const;

        /**
         * @brief Установка описания предмета
         * @param description Описание предмета
         */
        void setDescription(const std::string& description);

        /**
         * @brief Получение описания предмета
         * @return Описание предмета
         */
        const std::string& getDescription() const;

        /**
         * @brief Установка иконки предмета
         * @param icon Путь к иконке предмета
         */
        void setIcon(const std::string& icon);

        /**
         * @brief Получение пути к иконке предмета
         * @return Путь к иконке предмета
         */
        const std::string& getIcon() const;

        /**
         * @brief Пульсация для визуального выделения
         * @param enable Включение/выключение пульсации
         */
        void setPulsating(bool enable);

    private:
        ItemType m_itemType;          ///< Тип предмета
        int m_value;                  ///< Ценность предмета
        float m_weight;               ///< Вес предмета
        std::string m_description;    ///< Описание предмета
        std::string m_icon;           ///< Путь к иконке предмета

        // Визуальные эффекты
        bool m_isPulsating;           ///< Флаг пульсации предмета
        float m_pulsePhase;           ///< Фаза пульсации
        float m_floatHeight;          ///< Высота "парения" предмета
        float m_rotationAngle;        ///< Угол вращения предмета
    };

    /**
     * @brief Класс игрока
     */
    class Player : public Entity {
    public:
        /**
         * @brief Конструктор
         * @param name Имя сущности
         * @param tileMap Указатель на карту для проверки коллизий
         */
        Player(const std::string& name, TileMap* tileMap);

        /**
         * @brief Деструктор
         */
        ~Player();

        /**
         * @brief Инициализация игрока
         * @return true в случае успеха, false при ошибке
         */
        bool initialize() override;

        /**
         * @brief Обработка пользовательского ввода
         * @param event Событие SDL
         */
        void handleEvent(const SDL_Event& event) override;

        /**
         * @brief Обработка нажатий клавиш для определения направления движения
         */
        void detectKeyInput();

        /**
         * @brief Обновление состояния игрока
         * @param deltaTime Время, прошедшее с предыдущего кадра
         */
        void update(float deltaTime) override;

        /**
         * @brief Отрисовка игрока
         * @param renderer Указатель на SDL_Renderer
         */
        void render(SDL_Renderer* renderer) override;

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
         * @brief Получение текущего направления игрока
         * @return Текущее направление
         */
        Direction getCurrentDirection() const;

        /**
         * @brief Получение субкоординаты X внутри тайла
         * @return Субкоордината X (0.0-1.0)
         */
        float getSubX() const;

        /**
         * @brief Установка субкоординаты X внутри тайла
         * @param subX Субкоордината X (0.0-1.0)
         */
        void setSubX(float subX);

        /**
         * @brief Получение субкоординаты Y внутри тайла
         * @return Субкоордината Y (0.0-1.0)
         */
        float getSubY() const;

        /**
         * @brief Установка субкоординаты Y внутри тайла
         * @param subY Субкоордината Y (0.0-1.0)
         */
        void setSubY(float subY);

        /**
         * @brief Получение полной X координаты (тайл + субпозиция)
         * @return Полная X координата
         */
        float getFullX() const;

        /**
         * @brief Получение полной Y координаты (тайл + субпозиция)
         * @return Полная Y координата
         */
        float getFullY() const;

        /**
         * @brief Получение высоты игрока
         * @return Высота игрока
         */
        float getHeight() const;

        /**
         * @brief Установка высоты игрока
         * @param height Высота игрока
         */
        void setHeight(float height);

        /**
         * @brief Получение цвета игрока
         * @return Цвет игрока
         */
        SDL_Color getColor() const;

        /**
         * @brief Установка цвета игрока
         * @param color Цвет игрока
         */
        void setColor(const SDL_Color& color);

        /**
         * @brief Получение текущей скорости движения
         * @return Скорость движения
         */
        float getMoveSpeed() const;

        /**
         * @brief Установка скорости движения
         * @param speed Скорость движения
         */
        void setMoveSpeed(float speed);

        /**
         * @brief Получение размера коллизии
         * @return Размер коллизии (радиус)
         */
        float getCollisionSize() const;

        /**
         * @brief Установка размера коллизии
         * @param size Размер коллизии (радиус)
         */
        void setCollisionSize(float size);

        /**
         * @brief Получение направления движения по X
         * @return Направление движения по X (-1.0, 0.0 или 1.0)
         */
        float getDirectionX() const;

        /**
         * @brief Получение направления движения по Y
         * @return Направление движения по Y (-1.0, 0.0 или 1.0)
         */
        float getDirectionY() const;

        /**
         * @brief Проверка движения игрока
         * @return true, если игрок движется, false в противном случае
         */
        bool isMoving() const;

        /**
         * @brief Устанавливает систему коллизий для использования игроком
         * @param collisionSystem Указатель на систему коллизий
         */
        void setCollisionSystem(CollisionSystem* collisionSystem);

        /**
         * @brief Отрисовка указателя направления
         * @param renderer SDL рендерер
         * @param isoRenderer Изометрический рендерер
         * @param centerX X-координата центра экрана
         * @param centerY Y-координата центра экрана
         */
        void renderDirectionIndicator(SDL_Renderer* renderer, IsometricRenderer* isoRenderer,
            int centerX, int centerY) const;

        /**
         * @brief Проверка, следует ли отображать указатель направления
         * @return true, если указатель должен отображаться
         */
        bool isShowingDirectionIndicator() const;

        /**
         * @brief Установка отображения указателя направления
         * @param show true для включения, false для отключения
         */
        void setShowDirectionIndicator(bool show);

    private:
        TileMap* m_tileMap;             ///< Указатель на карту для проверки коллизий
        Direction m_currentDirection;   ///< Текущее направление игрока
        float m_subX;                   ///< Позиция внутри тайла по X (0.0-1.0)
        float m_subY;                   ///< Позиция внутри тайла по Y (0.0-1.0)
        float m_moveSpeed;              ///< Скорость движения
        float m_dX;                     ///< Направление движения по X
        float m_dY;                     ///< Направление движения по Y
        float m_collisionSize;          ///< Размер коллизии (радиус)
        float m_height;                 ///< Высота игрока
        SDL_Color m_color;              ///< Основной цвет игрока
        SDL_Color m_leftFaceColor;      ///< Цвет левой грани
        SDL_Color m_rightFaceColor;     ///< Цвет правой грани
        CollisionSystem* m_collisionSystem; ///< Указатель на систему коллизий
        bool m_showDirectionIndicator;  ///< Флаг отображения указателя направления
        SDL_Color m_directionIndicatorColor; ///< Цвет указателя направления
    };

    /**
     * @brief Класс для представления тайла на карте
     */
    class MapTile {
    public:
        /**
         * @brief Конструктор по умолчанию (создает пустой тайл)
         */
        MapTile();

        /**
         * @brief Конструктор с указанием типа
         * @param type Тип тайла
         */
        MapTile(TileType type);

        /**
         * @brief Конструктор с указанием всех параметров
         * @param type Тип тайла
         * @param walkable Флаг проходимости
         * @param transparent Флаг прозрачности
         * @param height Высота тайла
         */
        MapTile(TileType type, bool walkable, bool transparent, float height);

        /**
         * @brief Деструктор
         */
        ~MapTile();

        /**
         * @brief Получение типа тайла
         * @return Тип тайла
         */
        TileType getType() const;

        /**
         * @brief Установка типа тайла
         * @param type Тип тайла
         */
        void setType(TileType type);

        /**
         * @brief Проверка проходимости тайла
         * @return true, если тайл проходим, false в противном случае
         */
        bool isWalkable() const;

        /**
         * @brief Установка признака проходимости
         * @param walkable Признак проходимости
         */
        void setWalkable(bool walkable);

        /**
         * @brief Проверка прозрачности тайла
         * @return true, если тайл прозрачный, false в противном случае
         */
        bool isTransparent() const;

        /**
         * @brief Установка признака прозрачности
         * @param transparent Признак прозрачности
         */
        void setTransparent(bool transparent);

        /**
         * @brief Получение высоты тайла
         * @return Высота тайла
         */
        float getHeight() const;

        /**
         * @brief Установка высоты тайла
         * @param height Высота тайла
         */
        void setHeight(float height);

        /**
         * @brief Получение цвета для рендеринга тайла
         * @return Цвет тайла (SDL_Color)
         */
        SDL_Color getColor() const;

        /**
         * @brief Установка цвета для рендеринга тайла
         * @param color Цвет тайла
         */
        void setColor(const SDL_Color& color);

        /**
         * @brief Получение строкового представления тайла
         * @return Строковое представление тайла
         */
        std::string toString() const;

        /**
         * @brief Проверка, является ли тайл водой
         * @return true, если тайл является водой, false в противном случае
         */
        bool isWater() const;

    private:
        TileType m_type;       ///< Тип тайла
        bool m_walkable;       ///< Признак проходимости
        bool m_transparent;    ///< Признак прозрачности
        float m_height;        ///< Высота тайла
        SDL_Color m_color;     ///< Цвет для рендеринга
    };

    /**
     * @brief Класс для управления картой из тайлов
     */
    class TileMap {
    public:
        /**
         * @brief Конструктор
         * @param width Ширина карты в тайлах
         * @param height Высота карты в тайлах
         */
        TileMap(int width, int height);

        /**
         * @brief Деструктор
         */
        ~TileMap();

        /**
         * @brief Инициализация карты
         * @return true в случае успеха, false при ошибке
         */
        bool initialize();

        /**
         * @brief Получение ширины карты
         * @return Ширина карты в тайлах
         */
        int getWidth() const;

        /**
         * @brief Получение высоты карты
         * @return Высота карты в тайлах
         */
        int getHeight() const;

        /**
         * @brief Проверка, находятся ли координаты в пределах карты
         * @param x X координата
         * @param y Y координата
         * @return true, если координаты в пределах карты, false в противном случае
         */
        bool isValidCoordinate(int x, int y) const;

        /**
         * @brief Получение тайла по координатам
         * @param x X координата
         * @param y Y координата
         * @return Указатель на тайл или nullptr, если координаты вне карты
         */
        MapTile* getTile(int x, int y);

        /**
         * @brief Получение тайла по координатам (константная версия)
         * @param x X координата
         * @param y Y координата
         * @return Указатель на тайл или nullptr, если координаты вне карты
         */
        const MapTile* getTile(int x, int y) const;

        /**
         * @brief Установка типа тайла по координатам
         * @param x X координата
         * @param y Y координата
         * @param type Тип тайла
         * @return true, если операция успешна, false в противном случае
         */
        bool setTileType(int x, int y, TileType type);

        /**
         * @brief Проверка проходимости тайла по координатам
         * @param x X координата
         * @param y Y координата
         * @return true, если тайл проходим, false в противном случае
         */
        bool isTileWalkable(int x, int y) const;

        /**
         * @brief Проверка прозрачности тайла по координатам
         * @param x X координата
         * @param y Y координата
         * @return true, если тайл прозрачен, false в противном случае
         */
        bool isTileTransparent(int x, int y) const;

        /**
         * @brief Очистка карты (заполнение пустыми тайлами)
         */
        void clear();

        /**
         * @brief Заполнение прямоугольной области карты одним типом тайлов
         * @param startX Начальная X координата
         * @param startY Начальная Y координата
         * @param endX Конечная X координата
         * @param endY Конечная Y координата
         * @param type Тип тайла
         */
        void fillRect(int startX, int startY, int endX, int endY, TileType type);

        /**
         * @brief Создание прямоугольной комнаты с полом и стенами
         * @param startX Начальная X координата
         * @param startY Начальная Y координата
         * @param endX Конечная X координата
         * @param endY Конечная Y координата
         * @param floorType Тип тайла для пола
         * @param wallType Тип тайла для стен
         */
        void createRoom(int startX, int startY, int endX, int endY, TileType floorType, TileType wallType);

        /**
         * @brief Создание горизонтального коридора
         * @param startX Начальная X координата
         * @param endX Конечная X координата
         * @param y Y координата
         * @param floorType Тип тайла для пола
         */
        void createHorizontalCorridor(int startX, int endX, int y, TileType floorType);

        /**
         * @brief Создание вертикального коридора
         * @param x X координата
         * @param startY Начальная Y координата
         * @param endY Конечная Y координата
         * @param floorType Тип тайла для пола
         */
        void createVerticalCorridor(int x, int startY, int endY, TileType floorType);

        /**
         * @brief Создание двери
         * @param x X координата
         * @param y Y координата
         * @param doorType Тип тайла для двери (по умолчанию TileType::DOOR)
         */
        void createDoor(int x, int y, TileType doorType = TileType::DOOR);

    private:
        int m_width;                           ///< Ширина карты в тайлах
        int m_height;                          ///< Высота карты в тайлах
        std::vector<std::vector<MapTile>> m_tiles; ///< Двумерный массив тайлов
    };

    /**
     * @brief Класс для процедурной генерации комнат на карте
     */
    class RoomGenerator {
    public:
        /**
         * @brief Структура для хранения информации о комнате
         */
        struct Room {
            int x = 0;              ///< X координата левого верхнего угла
            int y = 0;              ///< Y координата левого верхнего угла
            int width = 0;          ///< Ширина комнаты
            int height = 0;         ///< Высота комнаты
            TileType floorType = TileType::FLOOR;  ///< Тип пола
            TileType wallType = TileType::WALL;    ///< Тип стен
            int biomeId = 0;        ///< ID биома для комнаты
        };

        /**
         * @brief Тип комнаты для упрощения инициализации
         */
        typedef Room RoomType;

        /**
         * @brief Конструктор
         * @param seed Сид для генератора случайных чисел (0 = случайный)
         */
        RoomGenerator(unsigned int seed = 0);

        /**
         * @brief Деструктор
         */
        ~RoomGenerator();

        /**
         * @brief Генерация карты с комнатами
         * @param tileMap Указатель на карту для заполнения
         * @param biomeType Тип биома для генерации
         * @return true в случае успеха, false при ошибке
         */
        bool generateMap(TileMap* tileMap, BiomeType biomeType = BiomeType::DEFAULT);

        /**
         * @brief Установка сида генератора
         * @param seed Сид (0 = случайный)
         */
        void setSeed(unsigned int seed);

        /**
         * @brief Получение текущего сида
         * @return Текущий сид
         */
        unsigned int getSeed() const;

        /**
         * @brief Установка ограничений размера комнат
         * @param minSize Минимальный размер комнаты
         * @param maxSize Максимальный размер комнаты
         */
        void setRoomSizeLimits(int minSize, int maxSize);

        /**
         * @brief Установка ограничений количества комнат
         * @param minRooms Минимальное количество комнат
         * @param maxRooms Максимальное количество комнат
         */
        void setRoomCountLimits(int minRooms, int maxRooms);

    private:
        unsigned int m_seed;            ///< Сид генератора
        std::mt19937 m_rng;             ///< Генератор случайных чисел
        int m_maxRoomSize;              ///< Максимальный размер комнаты
        int m_minRoomSize;              ///< Минимальный размер комнаты
        int m_maxCorridorLength;        ///< Максимальная длина коридора
        int m_minRooms;                 ///< Минимальное количество комнат
        int m_maxRooms;                 ///< Максимальное количество комнат
    };

    /**
     * @brief Система для обработки коллизий между сущностями и картой
     */
    class CollisionSystem {
    public:
        /**
         * @brief Конструктор
         * @param tileMap Указатель на карту для проверки коллизий с тайлами
         */
        CollisionSystem(TileMap* tileMap);

        /**
         * @brief Деструктор
         */
        ~CollisionSystem();

        /**
         * @brief Проверка возможности перемещения
         * @param fromX Начальная X координата
         * @param fromY Начальная Y координата
         * @param toX Конечная X координата
         * @param toY Конечная Y координата
         * @return true, если перемещение возможно, false в противном случае
         */
        bool canMoveTo(int fromX, int fromY, int toX, int toY);

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
         * @brief Обработка коллизий с учетом скольжения вдоль стен
         * @param currentX Текущая целочисленная X координата
         * @param currentY Текущая целочисленная Y координата
         * @param subX Текущая суб-координата X (0.0-1.0)
         * @param subY Текущая суб-координата Y (0.0-1.0)
         * @param deltaX Изменение по X
         * @param deltaY Изменение по Y
         * @param collisionSize Размер объекта для коллизий
         * @return Результат обработки коллизии
         */
        CollisionResult handleCollisionWithSliding(
            int currentX, int currentY,
            float subX, float subY,
            float deltaX, float deltaY,
            float collisionSize);

        /**
         * @brief Проверка пересечения прямоугольной области с картой
         * @param x X-координата центра области
         * @param y Y-координата центра области
         * @param width Ширина области
         * @param height Высота области
         * @return true, если есть коллизия с непроходимым тайлом
         */
        bool checkRectangleCollision(float x, float y, float width, float height);

        /**
         * @brief Проверка коллизии между двумя окружностями
         * @param x1 X-координата центра первой окружности
         * @param y1 Y-координата центра первой окружности
         * @param radius1 Радиус первой окружности
         * @param x2 X-координата центра второй окружности
         * @param y2 Y-координата центра второй окружности
         * @param radius2 Радиус второй окружности
         * @return true, если окружности пересекаются
         */
        static bool checkCircleCollision(
            float x1, float y1, float radius1,
            float x2, float y2, float radius2);

        /**
         * @brief Проверка коллизии между окружностью и прямоугольником
         * @param circleX X-координата центра окружности
         * @param circleY Y-координата центра окружности
         * @param radius Радиус окружности
         * @param rectX X-координата центра прямоугольника
         * @param rectY Y-координата центра прямоугольника
         * @param rectWidth Ширина прямоугольника
         * @param rectHeight Высота прямоугольника
         * @return true, если есть пересечение
         */
        static bool checkCircleRectCollision(
            float circleX, float circleY, float radius,
            float rectX, float rectY, float rectWidth, float rectHeight);

    private:
        TileMap* m_tileMap;                     ///< Указатель на карту
    };

    /**
     * @brief Класс для изометрического рендеринга
     */
    class IsometricRenderer {
    public:
        /**
         * @brief Конструктор
         * @param tileWidth Ширина изометрического тайла
         * @param tileHeight Высота изометрического тайла
         */
        IsometricRenderer(int tileWidth, int tileHeight);

        /**
         * @brief Деструктор
         */
        ~IsometricRenderer();

        /**
         * @brief Преобразование из мировых координат в экранные
         * @param worldX X координата в мировом пространстве
         * @param worldY Y координата в мировом пространстве
         * @param screenX X координата на экране (выходной параметр)
         * @param screenY Y координата на экране (выходной параметр)
         */
        void worldToScreen(float worldX, float worldY, int& screenX, int& screenY) const;

        /**
         * @brief Преобразование из экранных координат в мировые
         * @param screenX X координата на экране
         * @param screenY Y координата на экране
         * @param worldX X координата в мировом пространстве (выходной параметр)
         * @param worldY Y координата в мировом пространстве (выходной параметр)
         */
        void screenToWorld(int screenX, int screenY, float& worldX, float& worldY) const;

        /**
         * @brief Преобразование из мировых координат в экранные с учетом высоты и центра экрана
         * @param worldX X координата в мировом пространстве
         * @param worldY Y координата в мировом пространстве
         * @param worldZ Высота (Z координата) объекта
         * @param centerX X координата центра экрана
         * @param centerY Y координата центра экрана
         * @param displayX X координата на экране (выходной параметр)
         * @param displayY Y координата на экране (выходной параметр)
         */
        void worldToDisplay(float worldX, float worldY, float worldZ,
            int centerX, int centerY,
            int& displayX, int& displayY) const;

        /**
         * @brief Отрисовка изометрического тайла
         * @param renderer SDL рендерер
         * @param worldX X координата в мировом пространстве
         * @param worldY Y координата в мировом пространстве
         * @param height Высота тайла (для объемных тайлов)
         * @param color Цвет тайла
         * @param centerX X координата центра экрана (по умолчанию 0)
         * @param centerY Y координата центра экрана (по умолчанию 0)
         */
        void renderTile(SDL_Renderer* renderer, float worldX, float worldY, float height,
            SDL_Color color, int centerX = 0, int centerY = 0) const;

        /**
         * @brief Отрисовка изометрического объемного тайла
         * @param renderer SDL рендерер
         * @param worldX X координата в мировом пространстве
         * @param worldY Y координата в мировом пространстве
         * @param height Высота тайла (для объемных тайлов)
         * @param topColor Цвет верхней грани
         * @param leftColor Цвет левой грани
         * @param rightColor Цвет правой грани
         * @param centerX X координата центра экрана (по умолчанию 0)
         * @param centerY Y координата центра экрана (по умолчанию 0)
         */
        void renderVolumetricTile(SDL_Renderer* renderer, float worldX, float worldY, float height,
            SDL_Color topColor, SDL_Color leftColor, SDL_Color rightColor,
            int centerX = 0, int centerY = 0) const;

        /**
         * @brief Отрисовка изометрической сетки
         * @param renderer SDL рендерер
         * @param centerX X координата центра сетки на экране
         * @param centerY Y координата центра сетки на экране
         * @param gridSize Размер сетки (количество тайлов от центра)
         * @param color Цвет линий сетки
         */
        void renderGrid(SDL_Renderer* renderer, int centerX, int centerY, int gridSize, SDL_Color color) const;

        /**
         * @brief Отрисовка точки в мировых координатах для отладки
         * @param renderer SDL рендерер
         * @param worldX X координата в мировом пространстве
         * @param worldY Y координата в мировом пространстве
         * @param worldZ Z координата (высота) в мировом пространстве
         * @param color Цвет точки
         * @param centerX X координата центра экрана
         * @param centerY Y координата центра экрана
         */
        void renderDebugPoint(SDL_Renderer* renderer, float worldX, float worldY, float worldZ,
            SDL_Color color, int centerX, int centerY) const;

        /**
         * @brief Установка позиции камеры
         * @param x X координата камеры в мировом пространстве
         * @param y Y координата камеры в мировом пространстве
         */
        void setCameraPosition(float x, float y);

        /**
         * @brief Получение X координаты камеры
         * @return X координата камеры в мировом пространстве
         */
        float getCameraX() const;

        /**
         * @brief Получение Y координаты камеры
         * @return Y координата камеры в мировом пространстве
         */
        float getCameraY() const;

        /**
         * @brief Установка масштаба камеры
         * @param scale Масштаб (1.0 - нормальный размер)
         */
        void setCameraZoom(float scale);

        /**
         * @brief Получение текущего масштаба камеры
         * @return Текущий масштаб камеры
         */
        float getCameraZoom() const;

        /**
         * @brief Получение высоты объекта в пикселях
         * @param worldHeight Высота объекта в мировых единицах
         * @return Высота в пикселях
         */
        int getHeightInPixels(float worldHeight) const;

        /**
         * @brief Получение размера с учетом масштаба камеры
         * @param size Исходный размер
         * @return Размер с учетом масштаба
         */
        int getScaledSize(int size) const;

    private:
        int m_tileWidth;    ///< Ширина изометрического тайла
        int m_tileHeight;   ///< Высота изометрического тайла
        float m_cameraX;    ///< X координата камеры в мировом пространстве
        float m_cameraY;    ///< Y координата камеры в мировом пространстве
        float m_cameraZoom; ///< Масштаб камеры (1.0 - нормальный размер)
    };

    /**
     * @brief Класс для оптимизированного рендеринга коллекций тайлов
     */
    class TileRenderer {
    public:
        /**
         * @brief Конструктор
         * @param isoRenderer Указатель на изометрический рендерер
         */
        TileRenderer(IsometricRenderer* isoRenderer);

        /**
         * @brief Деструктор
         */
        ~TileRenderer();

        /**
         * @brief Очистка всех тайлов
         */
        void clear();

        /**
         * @brief Добавление плоского тайла
         * @param x X координата в мировом пространстве
         * @param y Y координата в мировом пространстве
         * @param texture Текстура для отрисовки (может быть nullptr)
         * @param color Цвет для отрисовки, если текстура отсутствует
         * @param priority Приоритет отрисовки (выше значение = отображается поверх)
         */
        void addFlatTile(float x, float y, SDL_Texture* texture, SDL_Color color, float priority = 0.0f);

        /**
         * @brief Добавление объемного тайла
         * @param x X координата в мировом пространстве
         * @param y Y координата в мировом пространстве
         * @param z Z координата (высота)
         * @param topTexture Текстура для верхней грани (может быть nullptr)
         * @param leftTexture Текстура для левой грани (может быть nullptr)
         * @param rightTexture Текстура для правой грани (может быть nullptr)
         * @param topColor Цвет верхней грани
         * @param leftColor Цвет левой грани
         * @param rightColor Цвет правой грани
         * @param priority Приоритет отрисовки (выше значение = отображается поверх)
         */
        void addVolumetricTile(float x, float y, float z,
            SDL_Texture* topTexture,
            SDL_Texture* leftTexture,
            SDL_Texture* rightTexture,
            SDL_Color topColor,
            SDL_Color leftColor,
            SDL_Color rightColor,
            float priority = 0.0f);

        /**
         * @brief Отрисовка всех тайлов
         * @param renderer SDL рендерер
         * @param centerX X координата центра экрана
         * @param centerY Y координата центра экрана
         */
        void render(SDL_Renderer* renderer, int centerX, int centerY);

    private:
        std::vector<RenderableTile> m_tiles;  ///< Вектор тайлов для отрисовки
        IsometricRenderer* m_isoRenderer;     ///< Указатель на изометрический рендерер
    };

    /**
     * @brief Класс для управления камерой
     */
    class Camera {
    public:
        /**
         * @brief Конструктор
         * @param screenWidth Ширина экрана
         * @param screenHeight Высота экрана
         */
        Camera(int screenWidth, int screenHeight);

        /**
         * @brief Деструктор
         */
        ~Camera();

        /**
         * @brief Обновление камеры
         * @param deltaTime Время, прошедшее с предыдущего кадра
         */
        void update(float deltaTime);

        /**
         * @brief Обработка событий камеры (перемещение, масштабирование)
         * @param event SDL событие
         */
        void handleEvent(const SDL_Event& event);

        /**
         * @brief Установка позиции камеры
         * @param x X координата в мировом пространстве
         * @param y Y координата в мировом пространстве
         */
        void setPosition(float x, float y);

        /**
         * @brief Получение X координаты камеры
         * @return X координата в мировом пространстве
         */
        float getX() const;

        /**
         * @brief Получение Y координаты камеры
         * @return Y координата в мировом пространстве
         */
        float getY() const;

        /**
         * @brief Установка масштаба камеры
         * @param scale Масштаб (1.0 - нормальный размер)
         */
        void setZoom(float scale);

        /**
         * @brief Получение текущего масштаба камеры
         * @return Текущий масштаб
         */
        float getZoom() const;

        /**
         * @brief Установка целевого объекта для слежения
         * @param targetX Указатель на X координату цели
         * @param targetY Указатель на Y координату цели
         */
        void setTarget(const float* targetX, const float* targetY);

        /**
         * @brief Перемещение камеры
         * @param dx Изменение по X
         * @param dy Изменение по Y
         */
        void move(float dx, float dy);

        /**
         * @brief Масштабирование камеры
         * @param amount Величина изменения масштаба
         */
        void zoom(float amount);

    private:
        float m_x;                  ///< X координата камеры в мировом пространстве
        float m_y;                  ///< Y координата камеры в мировом пространстве
        float m_zoom;               ///< Масштаб камеры
        int m_screenWidth;          ///< Ширина экрана
        int m_screenHeight;         ///< Высота экрана
        float m_moveSpeed;          ///< Скорость перемещения
        float m_zoomSpeed;          ///< Скорость масштабирования
        const float* m_targetX;     ///< Указатель на X координату целевого объекта
        const float* m_targetY;     ///< Указатель на Y координату целевого объекта
        bool m_isDragging;          ///< Флаг перемещения камеры мышью
    };

    /**
     * @brief Класс для управления ресурсами (текстурами, звуками, шрифтами и т.д.)
     */
    class ResourceManager {
    public:
        /**
         * @brief Конструктор
         * @param renderer Указатель на SDL рендерер
         */
        ResourceManager(SDL_Renderer* renderer);

        /**
         * @brief Деструктор
         */
        ~ResourceManager();

        /**
         * @brief Загружает текстуру из файла
         * @param id Идентификатор ресурса
         * @param filePath Путь к файлу текстуры
         * @return true в случае успеха, false при ошибке
         */
        bool loadTexture(const std::string& id, const std::string& filePath);

        /**
         * @brief Получает текстуру по идентификатору
         * @param id Идентификатор ресурса
         * @return Указатель на текстуру или nullptr, если текстура не найдена
         */
        SDL_Texture* getTexture(const std::string& id) const;

        /**
         * @brief Проверяет, загружена ли текстура с указанным идентификатором
         * @param id Идентификатор ресурса
         * @return true, если текстура загружена, false если нет
         */
        bool hasTexture(const std::string& id) const;

        /**
         * @brief Удаляет текстуру из менеджера ресурсов
         * @param id Идентификатор ресурса
         */
        void removeTexture(const std::string& id);

        /**
         * @brief Загружает шрифт из файла
         * @param id Идентификатор шрифта
         * @param filePath Путь к файлу шрифта
         * @param fontSize Размер шрифта
         * @return true в случае успеха, false при ошибке
         */
        bool loadFont(const std::string& id, const std::string& filePath, int fontSize);

        /**
         * @brief Получает шрифт по идентификатору
         * @param id Идентификатор шрифта
         * @return Указатель на шрифт или nullptr, если шрифт не найден
         */
        TTF_Font* getFont(const std::string& id) const;

        /**
         * @brief Проверяет, загружен ли шрифт с указанным идентификатором
         * @param id Идентификатор шрифта
         * @return true, если шрифт загружен, false если нет
         */
        bool hasFont(const std::string& id) const;

        /**
         * @brief Удаляет шрифт из менеджера ресурсов
         * @param id Идентификатор шрифта
         */
        void removeFont(const std::string& id);

        /**
         * @brief Отрисовывает текст на экране
         * @param renderer Указатель на SDL_Renderer
         * @param text Текст для отображения
         * @param fontId Идентификатор шрифта
         * @param x X-координата
         * @param y Y-координата
         * @param color Цвет текста
         */
        void renderText(SDL_Renderer* renderer, const std::string& text, const std::string& fontId,
            int x, int y, SDL_Color color);

        /**
         * @brief Создает текстуру с текстом
         * @param text Текст для отображения
         * @param fontId Идентификатор шрифта
         * @param color Цвет текста
         * @return Указатель на созданную текстуру или nullptr при ошибке
         */
        SDL_Texture* createTextTexture(const std::string& text, const std::string& fontId, SDL_Color color);

        /**
         * @brief Освобождает все ресурсы
         */
        void clearAll();

    private:
        SDL_Renderer* m_renderer;                                  ///< Указатель на SDL рендерер
        std::unordered_map<std::string, SDL_Texture*> m_textures;  ///< Хранилище текстур
        std::unordered_map<std::string, TTF_Font*> m_fonts;        ///< Хранилище шрифтов
    };

    /**
     * @brief Класс для логирования
     */
    class Logger {
    public:
        /**
         * @brief Получение экземпляра синглтона
         * @return Ссылка на экземпляр логгера
         */
        static Logger& getInstance();

        /**
         * @brief Инициализация логгера
         * @param logToFile Флаг логирования в файл
         * @param logFileName Имя файла для логирования
         * @param consoleLogLevel Уровень логирования для консоли
         * @param fileLogLevel Уровень логирования для файла
         */
        void initialize(bool logToFile = false, const std::string& logFileName = "satellite.log",
            LogLevel consoleLogLevel = LogLevel::INFO, LogLevel fileLogLevel = LogLevel::DEBUG);

        /**
         * @brief Завершение работы логгера
         */
        void shutdown();

        /**
         * @brief Установка уровня логирования для консоли
         * @param level Новый уровень логирования
         */
        void setConsoleLogLevel(LogLevel level);

        /**
         * @brief Установка уровня логирования для файла
         * @param level Новый уровень логирования
         */
        void setFileLogLevel(LogLevel level);

        /**
         * @brief Логирование на уровне Debug
         * @param message Сообщение для логирования
         */
        void debug(const std::string& message);

        /**
         * @brief Логирование на уровне Info
         * @param message Сообщение для логирования
         */
        void info(const std::string& message);

        /**
         * @brief Логирование на уровне Warning
         * @param message Сообщение для логирования
         */
        void warning(const std::string& message);

        /**
         * @brief Логирование на уровне Error
         * @param message Сообщение для логирования
         */
        void error(const std::string& message);

    private:
        Logger();
        ~Logger();
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void log(LogLevel level, const std::string& message);

        bool m_logToFile;
        std::ofstream m_logFile;
        LogLevel m_consoleLogLevel;
        LogLevel m_fileLogLevel;
        std::mutex m_mutex;
    };

    // =============================
    // ГЛОБАЛЬНЫЕ ФУНКЦИИ
    // =============================

    /**
     * @brief Запись в лог с указанным уровнем
     * @param level Уровень логирования
     * @param message Сообщение для лога
     */
    void Log(LogLevel level, const std::string& message);

    /**
     * @brief Запись отладочного сообщения в лог
     * @param message Сообщение для лога
     */
    inline void LogDebug(const std::string& message) { Log(LogLevel::DEBUG, message); }

    /**
     * @brief Запись информационного сообщения в лог
     * @param message Сообщение для лога
     */
    inline void LogInfo(const std::string& message) { Log(LogLevel::INFO, message); }

    /**
     * @brief Запись предупреждения в лог
     * @param message Сообщение для лога
     */
    inline void LogWarning(const std::string& message) { Log(LogLevel::WARNING, message); }

    /**
     * @brief Запись ошибки в лог
     * @param message Сообщение для лога
     */
    inline void LogError(const std::string& message) { Log(LogLevel::ERROR, message); }

    /**
     * @brief Проверка, является ли тайл проходимым по умолчанию
     * @param type Тип тайла
     * @return true, если тайл проходим, false в противном случае
     */
    bool IsWalkable(TileType type);

    /**
     * @brief Проверка, является ли тайл прозрачным (можно ли видеть сквозь него)
     * @param type Тип тайла
     * @return true, если тайл прозрачный, false в противном случае
     */
    bool IsTransparent(TileType type);

    /**
     * @brief Получение базовой высоты тайла по умолчанию
     * @param type Тип тайла
     * @return Высота тайла
     */
    float GetDefaultHeight(TileType type);

    /**
     * @brief Преобразование типа тайла в строку
     * @param type Тип тайла
     * @return Строковое представление типа тайла
     */
    std::string TileTypeToString(TileType type);

} // namespace Satellite

// Определения макросов для удобного логирования
#define LOG_DEBUG(msg) Satellite::LogDebug(msg)
#define LOG_INFO(msg) Satellite::LogInfo(msg)
#define LOG_WARNING(msg) Satellite::LogWarning(msg)
#define LOG_ERROR(msg) Satellite::LogError(msg)