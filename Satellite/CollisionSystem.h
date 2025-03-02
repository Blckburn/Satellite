#pragma once

#include <memory>
#include <vector>
#include <SDL.h>
#include "TileMap.h"
#include "Entity.h"

/**
 * @brief Класс для представления коллизионной формы
 */
class CollisionShape {
public:
    enum class Type {
        CIRCLE,     ///< Круговая форма коллизии
        RECTANGLE,  ///< Прямоугольная форма коллизии
        POINT       ///< Точечная форма коллизии
    };

    /**
     * @brief Конструктор для круговой коллизии
     * @param radius Радиус круга
     */
    explicit CollisionShape(float radius);

    /**
     * @brief Конструктор для прямоугольной коллизии
     * @param width Ширина прямоугольника
     * @param height Высота прямоугольника
     */
    CollisionShape(float width, float height);

    /**
     * @brief Конструктор для точечной коллизии
     */
    CollisionShape();

    /**
     * @brief Получение типа коллизионной формы
     * @return Тип формы
     */
    Type getType() const { return m_type; }

    /**
     * @brief Получение радиуса для круговой формы
     * @return Радиус круга
     */
    float getRadius() const { return m_radius; }

    /**
     * @brief Получение ширины для прямоугольной формы
     * @return Ширина прямоугольника
     */
    float getWidth() const { return m_width; }

    /**
     * @brief Получение высоты для прямоугольной формы
     * @return Высота прямоугольника
     */
    float getHeight() const { return m_height; }

private:
    Type m_type;     ///< Тип коллизионной формы
    float m_radius;  ///< Радиус для круговой формы
    float m_width;   ///< Ширина для прямоугольной формы
    float m_height;  ///< Высота для прямоугольной формы
};

/**
 * @brief Структура для хранения информации о коллизии
 */
struct CollisionInfo {
    bool hasCollision;     ///< Флаг наличия коллизии
    float penetrationX;    ///< Глубина проникновения по X
    float penetrationY;    ///< Глубина проникновения по Y
    Entity* entityA;       ///< Указатель на первую сущность
    Entity* entityB;       ///< Указатель на вторую сущность (nullptr для коллизий с тайлом)
    int tileX;             ///< X координата тайла при коллизии с тайлом
    int tileY;             ///< Y координата тайла при коллизии с тайлом

    CollisionInfo() : hasCollision(false), penetrationX(0.0f), penetrationY(0.0f),
        entityA(nullptr), entityB(nullptr), tileX(-1), tileY(-1) {
    }
};

/**
 * @brief Система управления коллизиями
 */
class CollisionSystem {
public:
    /**
     * @brief Конструктор
     * @param tileMap Указатель на карту тайлов
     */
    explicit CollisionSystem(std::shared_ptr<TileMap> tileMap);

    /**
     * @brief Проверка коллизии между сущностью и картой
     * @param entity Указатель на сущность
     * @param nextX Следующая X позиция сущности
     * @param nextY Следующая Y позиция сущности
     * @param shape Коллизионная форма сущности
     * @return Информация о коллизии
     */
    CollisionInfo checkMapCollision(
        Entity* entity, float nextX, float nextY, const CollisionShape& shape);

    /**
     * @brief Проверка коллизии между двумя сущностями
     * @param entityA Указатель на первую сущность
     * @param shapeA Коллизионная форма первой сущности
     * @param entityB Указатель на вторую сущность
     * @param shapeB Коллизионная форма второй сущности
     * @return Информация о коллизии
     */
    CollisionInfo checkEntityCollision(
        Entity* entityA, const CollisionShape& shapeA,
        Entity* entityB, const CollisionShape& shapeB);

    /**
     * @brief Проверка коллизий для всех зарегистрированных сущностей
     * @param entities Вектор указателей на сущности
     * @return Вектор информации о коллизиях
     */
    std::vector<CollisionInfo> checkAllCollisions(
        const std::vector<std::shared_ptr<Entity>>& entities);

    /**
     * @brief Разрешение коллизии путем отталкивания
     * @param info Информация о коллизии
     */
    void resolveCollision(CollisionInfo& info);

    /**
     * @brief Проверка, может ли сущность переместиться
     * @param entity Указатель на сущность
     * @param shape Коллизионная форма сущности
     * @param nextX Следующая X позиция сущности
     * @param nextY Следующая Y позиция сущности
     * @param outCollision Выходная информация о коллизии (опционально)
     * @return true, если перемещение возможно, false в противном случае
     */
    bool canMove(
        Entity* entity, const CollisionShape& shape,
        float nextX, float nextY, CollisionInfo* outCollision = nullptr);

    /**
     * @brief Попытка переместить сущность с учетом коллизий
     * @param entity Указатель на сущность
     * @param shape Коллизионная форма сущности
     * @param deltaX Желаемое смещение по X
     * @param deltaY Желаемое смещение по Y
     * @param slideAlongWalls Флаг разрешения скольжения вдоль стен
     * @return Информация о коллизии (если произошла)
     */
    CollisionInfo tryMove(
        Entity* entity, const CollisionShape& shape,
        float deltaX, float deltaY, bool slideAlongWalls = true);

    /**
     * @brief Проверка, находится ли точка в проходимом тайле
     * @param x X координата в мировом пространстве
     * @param y Y координата в мировом пространстве
     * @return true, если точка в проходимом тайле, false в противном случае
     */
    bool isPointWalkable(float x, float y);

    /**
     * @brief Отрисовка отладочной информации о коллизиях
     * @param renderer Указатель на SDL_Renderer
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void renderDebug(SDL_Renderer* renderer, int centerX, int centerY);

private:
    /**
     * @brief Проверка коллизии точка-прямоугольник
     * @param pointX X координата точки
     * @param pointY Y координата точки
     * @param rectX X координата прямоугольника
     * @param rectY Y координата прямоугольника
     * @param rectWidth Ширина прямоугольника
     * @param rectHeight Высота прямоугольника
     * @return true, если есть коллизия, false в противном случае
     */
    bool checkPointRectCollision(
        float pointX, float pointY,
        float rectX, float rectY, float rectWidth, float rectHeight);

    /**
     * @brief Проверка коллизии прямоугольник-прямоугольник
     * @param rectAX X координата первого прямоугольника
     * @param rectAY Y координата первого прямоугольника
     * @param rectAWidth Ширина первого прямоугольника
     * @param rectAHeight Высота первого прямоугольника
     * @param rectBX X координата второго прямоугольника
     * @param rectBY Y координата второго прямоугольника
     * @param rectBWidth Ширина второго прямоугольника
     * @param rectBHeight Высота второго прямоугольника
     * @param outPenetrationX Выходная глубина проникновения по X
     * @param outPenetrationY Выходная глубина проникновения по Y
     * @return true, если есть коллизия, false в противном случае
     */
    bool checkRectRectCollision(
        float rectAX, float rectAY, float rectAWidth, float rectAHeight,
        float rectBX, float rectBY, float rectBWidth, float rectBHeight,
        float& outPenetrationX, float& outPenetrationY);

    /**
     * @brief Проверка коллизии круг-круг
     * @param circleAX X координата первого круга
     * @param circleAY Y координата первого круга
     * @param circleARadius Радиус первого круга
     * @param circleBX X координата второго круга
     * @param circleBY Y координата второго круга
     * @param circleBRadius Радиус второго круга
     * @param outPenetrationX Выходная глубина проникновения по X
     * @param outPenetrationY Выходная глубина проникновения по Y
     * @return true, если есть коллизия, false в противном случае
     */
    bool checkCircleCircleCollision(
        float circleAX, float circleAY, float circleARadius,
        float circleBX, float circleBY, float circleBRadius,
        float& outPenetrationX, float& outPenetrationY);

    /**
     * @brief Проверка коллизии прямоугольник-круг
     * @param rectX X координата прямоугольника
     * @param rectY Y координата прямоугольника
     * @param rectWidth Ширина прямоугольника
     * @param rectHeight Высота прямоугольника
     * @param circleX X координата круга
     * @param circleY Y координата круга
     * @param circleRadius Радиус круга
     * @param outPenetrationX Выходная глубина проникновения по X
     * @param outPenetrationY Выходная глубина проникновения по Y
     * @return true, если есть коллизия, false в противном случае
     */
    bool checkRectCircleCollision(
        float rectX, float rectY, float rectWidth, float rectHeight,
        float circleX, float circleY, float circleRadius,
        float& outPenetrationX, float& outPenetrationY);

private:
    std::shared_ptr<TileMap> m_tileMap; ///< Указатель на карту тайлов
    std::vector<CollisionInfo> m_debugCollisions; ///< Коллизии для отладки
};