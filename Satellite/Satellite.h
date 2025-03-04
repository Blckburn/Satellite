/**
 * @file Satellite.h
 * @brief Единый заголовочный файл для движка Satellite Engine
 *
 * Этот файл содержит все основные интерфейсы, классы, перечисления
 * и структуры данных, необходимые для работы с Satellite Engine.
 */

#pragma once

 // Стандартные включения
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <cmath>

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
}
/**
 * @brief Структура для хранения информации о тайле для рендеринга
 */


namespace Satellite {

    // =============================
    // ПРЕДВАРИТЕЛЬНЫЕ ОБЪЯВЛЕНИЯ
    // =============================
    class Engine;
    class Scene;
    class Entity;
    class TileMap;
    class MapTile;
    class Player;
    class InteractiveObject;
    class PickupItem;
    class Camera;
    class CollisionSystem;
    class IsometricRenderer;
    class TileRenderer;
    class ResourceManager;
    class RoomGenerator;

    // =============================
    // ПЕРЕЧИСЛЕНИЯ И СТРУКТУРЫ
    // =============================

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
        SDL_Color /rightColor = { 150, 150, 150, 255 };

        enum class TileType {
            FLAT,       ///< Плоский тайл
            VOLUMETRIC  ///< Объемный тайл
        } type = TileType::FLAT;
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
         * @brief Генерация тестовой карты
         */
        void generateTestMap();

        /**
         * @brief Добавление интерактивного объекта на сцену
         * @param object Указатель на интерактивный объект
         */
        void addInteractiveObject(std::shared_ptr<InteractiveObject> object);

        /**
         * @brief Получение карты сцены
         * @return Указатель на карту
         */
        TileMap* getMap();
    };


    /**
     * @brief Типы тайлов карты
     */
    enum class TileType {
        EMPTY,      ///< Пустой тайл (отсутствует)
        FLOOR,      ///< Обычный пол
        WALL,       ///< Стена
        DOOR,       ///< Дверь
        WATER,      ///< Вода
        GRASS,      ///< Трава
        STONE,      ///< Камень
        METAL,      ///< Металл
        GLASS,      ///< Стекло
        WOOD,       ///< Дерево
        SPECIAL,    ///< Специальный тайл (телепорт, ловушка и т.д.)
        OBSTACLE,   ///< Непроходимое препятствие
        SAND,       ///< Песок
        SNOW,       ///< Снег
        ICE,        ///< Лед
        ROCK_FORMATION, ///< Скалы
        LAVA,       ///< Лава
        FOREST      ///< Лес
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
        CollisionResult() :
            collision(false),
            adjustedX(0.0f),
            adjustedY(0.0f),
            slidingX(false),
            slidingY(false) {
        }
    };

    // =============================
    // ОСНОВНЫЕ КЛАССЫ
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
 * @brief Создание комнаты на карте
 * @param startX Начальная X координата
 * @param startY Начальная Y координата
 * @param endX Конечная X координата
 * @param endY Конечная Y координата
 * @param floorType Тип тайла для пола
 * @param wallType Тип тайла для стен
 */
    void createRoom(int startX, int startY, int endX, int endY,
        TileType floorType, TileType wallType);

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
         * @brief Инициализация игрока
         * @return true в случае успеха, false при ошибке
         */
        bool initialize() override;

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
         * @brief Устанавливает систему коллизий для использования игроком
         * @param collisionSystem Указатель на систему коллизий
         */
        void setCollisionSystem(CollisionSystem* collisionSystem);

        /**
         * @brief Получение цвета игрока
         * @return Цвет игрока
         */
        SDL_Color getColor() const;
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
         * @brief Взаимодействие с предметом (подбор)
         * @param player Указатель на игрока
         * @return true, если взаимодействие было успешным
         */
        bool interact(Player* player) override;

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
         * @brief Установка типа тайла по координатам
         * @param x X координата
         * @param y Y координата
         * @param type Тип тайла
         * @return true, если операция успешна, false в противном случае
         */
        bool setTileType(int x, int y, TileType type);
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
         * @brief Проверка прозрачности тайла
         * @return true, если тайл прозрачный, false в противном случае
         */
        bool isTransparent() const;

        /**
         * @brief Получение высоты тайла
         * @return Высота тайла
         */
        float getHeight() const;

        /**
         * @brief Получение цвета для рендеринга тайла
         * @return Цвет тайла (SDL_Color)
         */
        SDL_Color getColor() const;
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
         * @brief Установка позиции камеры
         * @param x X координата камеры в мировом пространстве
         * @param y Y координата камеры в мировом пространстве
         */
        void setCameraPosition(float x, float y);

        /**
         * @brief Установка масштаба камеры
         * @param scale Масштаб (1.0 - нормальный размер)
         */
        void setCameraZoom(float scale);
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
    };

    /**
     * @brief Класс для процедурной генерации комнат на карте
     */
    class RoomGenerator {
    public:
        /**
         * @brief Конструктор
         * @param seed Сид для генератора случайных чисел (0 = случайный)
         */
        RoomGenerator(unsigned int seed = 0);

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
         * @brief Установка ограничений размера комнат
         * @param minSize Минимальный размер комнаты
         * @param maxSize Максимальный размер комнаты
         */
        void setRoomSizeLimits(int minSize, int maxSize);
    };

    /**
     * @brief Класс для управления видом
     */

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
         * @brief Очистка состояния рендерера
         */
        void clear();

        /**
         * @brief Добавление плоского тайла
         * @param x X-координата в мировом пространстве
         * @param y Y-координата в мировом пространстве
         * @param texture Текстура (может быть nullptr)
         * @param color Цвет тайла
         * @param priority Приоритет отрисовки
         */
        void addFlatTile(float x, float y, SDL_Texture* texture,
            SDL_Color color, float priority = 0.0f);

        /**
         * @brief Добавление объемного тайла
         * @param x X-координата
         * @param y Y-координата
         * @param z Высота (Z-координата)
         * @param topTexture Текстура верхней грани
         * @param leftTexture Текстура левой грани
         * @param rightTexture Текстура правой грани
         * @param topColor Цвет верхней грани
         * @param leftColor Цвет левой грани
         * @param rightColor Цвет правой грани
         * @param priority Приоритет отрисовки
         */
        void addVolumetricTile(float x, float y, float z,
            SDL_Texture* topTexture, SDL_Texture* leftTexture, SDL_Texture* rightTexture,
            SDL_Color topColor, SDL_Color leftColor, SDL_Color rightColor,
            float priority = 0.0f);

        /**
         * @brief Отрисовка всех добавленных тайлов
         * @param renderer SDL рендерер
         * @param centerX X-координата центра экрана
         * @param centerY Y-координата центра экрана
         */
        void render(SDL_Renderer* renderer, int centerX, int centerY);
    };


    class Camera {
    public:
        /**
         * @brief Конструктор
         * @param screenWidth Ширина экрана
         * @param screenHeight Высота экрана
         */
        Camera(int screenWidth, int screenHeight);

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




} // namespace Satellite

// Определения макросов для удобного логирования
#define LOG_DEBUG(msg) Satellite::LogDebug(msg)
#define LOG_INFO(msg) Satellite::LogInfo(msg)
#define LOG_WARNING(msg) Satellite::LogWarning(msg)
#define LOG_ERROR(msg) Satellite::LogError(msg)


