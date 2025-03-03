#pragma once

#include "Entity.h"
#include "TileMap.h"
#include "CollisionSystem.h"
#include <memory>
#include <unordered_map>
#include <string>

// Предварительные объявления
class PlayerState;

/**
 * @brief Класс игрока, управляемый пользователем
 */
class Player : public Entity {
public:
    /**
     * @brief Конструктор
     * @param name Имя игрока
     * @param tileMap Указатель на карту тайлов
     * @param collisionSystem Указатель на систему коллизий
     */
    Player(const std::string& name, std::shared_ptr<TileMap> tileMap,
        std::shared_ptr<CollisionSystem> collisionSystem);

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
     * @brief Установка позиции игрока
     * @param x Координата X
     * @param y Координата Y
     * @param subX Позиция внутри тайла по X (0.0-1.0)
     * @param subY Позиция внутри тайла по Y (0.0-1.0)
     */
    void setPlayerPosition(float x, float y, float subX = 0.5f, float subY = 0.5f);

    /**
     * @brief Получение X координаты игрока
     * @return X координата
     */
    float getPlayerX() const { return m_position.x; }

    /**
     * @brief Получение Y координаты игрока
     * @return Y координата
     */
    float getPlayerY() const { return m_position.y; }

    /**
     * @brief Получение X субкоординаты игрока внутри тайла
     * @return X субкоордината (0.0-1.0)
     */
    float getPlayerSubX() const { return m_subX; }

    /**
     * @brief Получение Y субкоординаты игрока внутри тайла
     * @return Y субкоордината (0.0-1.0)
     */
    float getPlayerSubY() const { return m_subY; }

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
     * @brief Получение размера коллизии
     * @return Размер коллизии
     */
    float getCollisionSize() const { return m_collisionSize; }

    /**
     * @brief Установка размера коллизии
     * @param size Размер коллизии
     */
    void setCollisionSize(float size) { m_collisionSize = size; }

    /**
     * @brief Получение скорости движения
     * @return Скорость движения
     */
    float getMoveSpeed() const { return m_moveSpeed; }

    /**
     * @brief Установка скорости движения
     * @param speed Скорость движения
     */
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }

    /**
     * @brief Получение направления движения по X
     * @return Направление движения по X (-1, 0, 1)
     */
    float getMoveDirectionX() const { return m_dX; }

    /**
     * @brief Получение направления движения по Y
     * @return Направление движения по Y (-1, 0, 1)
     */
    float getMoveDirectionY() const { return m_dY; }

    /**
     * @brief Установка направления движения
     * @param dx Направление по X
     * @param dy Направление по Y
     */
    void setMoveDirection(float dx, float dy);

    /**
     * @brief Расчет приоритета визуального порядка для изометрической проекции
     * @return Значение приоритета для сортировки
     */
    float calculateZOrderPriority();

    /**
     * @brief Получение текущего состояния игрока
     * @return Указатель на текущее состояние
     */
    PlayerState* getCurrentState() const { return m_currentState; }

    /**
     * @brief Получение имени текущего состояния
     * @return Строка с именем текущего состояния
     */
    std::string getCurrentStateName() const;

    /**
     * @brief Смена текущего состояния
     * @param stateName Имя нового состояния
     * @return true, если состояние успешно изменено, false в противном случае
     */
    bool changeState(const std::string& stateName);

    /**
     * @brief Обработка события текущим состоянием
     * @param event Событие SDL
     */
    void handleCurrentStateEvent(const SDL_Event& event);

    /**
     * @brief Получение указателя на карту тайлов
     * @return Указатель на TileMap
     */
    std::shared_ptr<TileMap> getTileMap() const { return m_tileMap; }

    /**
     * @brief Получение формы коллизии игрока
     * @return Форма коллизии
     */
    CollisionShape getCollisionShape() const;
    /**
* @brief Обновление игрока с фиксированным временным шагом
* @param fixedDeltaTime Фиксированный временной шаг
*/
    void moveWithFixedStep(float fixedDeltaTime);



    /**
 * @brief Обновление направления движения с плавной интерполяцией
 * @param deltaTime Время, прошедшее с предыдущего кадра
 */
    void updateSmoothDirection(float deltaTime);

    // Параметры для плавного движения
    float m_targetDX;     ///< Целевое направление движения по X
    float m_targetDY;     ///< Целевое направление движения по Y
    float m_smoothFactor; ///< Коэффициент сглаживания движения

    /**
     * @brief Перемещение игрока с учетом коллизий
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void moveWithCollision(float deltaTime);

    /**
     * @brief Инициализация состояний игрока
     */
    void initializeStates();
    void initPhysics();
    void updatePhysics(float deltaTime);

    /**
 * @brief Обновление движения персонажа
 * @param deltaTime Время, прошедшее с предыдущего кадра
 */
    void updateMovement(float deltaTime);

    /**
 * @brief Получение визуальной позиции X для отрисовки
 * @return Визуальная X-координата
 */
    float getVisualX() const { return m_visualX; }

    /**
     * @brief Получение визуальной позиции Y для отрисовки
     * @return Визуальная Y-координата
     */
    float getVisualY() const { return m_visualY; }

    /**
     * @brief Получение визуальной субпозиции X для отрисовки
     * @return Визуальная субпозиция X
     */
    float getVisualSubX() const { return m_visualSubX; }

    /**
     * @brief Получение визуальной субпозиции Y для отрисовки
     * @return Визуальная субпозиция Y
     */
    float getVisualSubY() const { return m_visualSubY; }

    /**
     * @brief Обновление визуальной позиции с интерполяцией
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void updateVisualPosition(float deltaTime);


private:

    /**
 * @brief Сохранение текущей позиции в буфер истории
 */
    void savePositionToHistory();

    /**
     * @brief Поиск стабильной позиции в истории
     * @param maxAge Максимальный возраст позиции в секундах
     * @return true, если стабильная позиция найдена и восстановлена
     */
    bool findStablePosition(float maxAge = 0.5f);

    /**
     * @brief Принудительная стабилизация позиции
     * Перемещает персонажа в центр ближайшего проходимого тайла
     */
    void forceStabilizePosition();

    struct PositionHistory {
        float x;
        float y;
        float time;  // Время, когда позиция была сохранена
    };

    static constexpr int HISTORY_SIZE = 10;  ///< Размер буфера истории позиций
    PositionHistory m_positionHistory[HISTORY_SIZE];  ///< Буфер истории позиций
    int m_historyIndex;                      ///< Текущий индекс в буфере истории
    float m_positionTime;                    ///< Общее время для отслеживания истории
    bool m_usingHistory;                     ///< Флаг использования истории для восстановления
    float m_stabilityTime;                   ///< Время после последней нестабильности

    float m_visualX;           ///< Визуальная X-координата для плавной отрисовки
    float m_visualY;           ///< Визуальная Y-координата для плавной отрисовки
    float m_visualSubX;        ///< Визуальная суб-X для плавной отрисовки
    float m_visualSubY;        ///< Визуальная суб-Y для плавной отрисовки
    static constexpr float VISUAL_SMOOTHING = 12.0f; ///< Коэффициент сглаживания визуальной позиции

    float moveTowards(float current, float target, float maxDelta);

    float m_targetX;                   ///< Целевая X-координата для плавного движения
    float m_targetY;                   ///< Целевая Y-координата для плавного движения
    bool m_isMoving;                   ///< Флаг активного движения
    static constexpr float ACCELERATION = 8.0f;          ///< Ускорение при движении
    static constexpr float DECELERATION = 16.0f;         ///< Замедление при остановке
    static constexpr float MAX_VELOCITY = 2.0f;          ///< Максимальная скорость
    float m_velocityX;                 ///< Текущая скорость по X
    float m_velocityY;                 ///< Текущая скорость по Y
    bool m_collisionOccurred;          ///< Флаг столкновения в текущем кадре
    float m_lastValidX;                ///< Последняя валидная X-координата
    float m_lastValidY;                ///< Последняя валидная Y-координата

    std::shared_ptr<TileMap> m_tileMap;  ///< Указатель на карту тайлов
    std::shared_ptr<CollisionSystem> m_collisionSystem; ///< Указатель на систему коллизий
    float m_subX;         ///< Позиция внутри тайла по X (0.0-1.0)
    float m_subY;         ///< Позиция внутри тайла по Y (0.0-1.0)
    float m_moveSpeed;    ///< Скорость движения персонажа
    float m_dX;           ///< Направление движения по X (с плавающей точкой)
    float m_dY;           ///< Направление движения по Y (с плавающей точкой)
    float m_collisionSize; ///< Размер коллизии персонажа

    // Визуальные параметры
    SDL_Color m_color;    ///< Цвет игрока

    // Система состояний
    PlayerState* m_currentState; ///< Текущее состояние игрока
    std::unordered_map<std::string, std::unique_ptr<PlayerState>> m_states; ///< Словарь всех состояний
    float m_accumulatedTime;      ///< Накопленное время для фиксированного шага
    static constexpr float FIXED_TIME_STEP = 1.0f / 60.0f;  ///< Фиксированный временной шаг

};