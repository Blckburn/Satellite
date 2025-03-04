#pragma once

#include "Entity_old.h"
#include <functional>

class Player; // Forward declaration

/**
 * @brief Тип интерактивного объекта
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
    InteractiveType getInteractiveType() const { return m_interactiveType; }

    /**
     * @brief Установка радиуса взаимодействия
     * @param radius Радиус взаимодействия
     */
    void setInteractionRadius(float radius) { m_interactionRadius = radius; }

    /**
     * @brief Получение радиуса взаимодействия
     * @return Радиус взаимодействия
     */
    float getInteractionRadius() const { return m_interactionRadius; }

    /**
     * @brief Проверка, активен ли объект для взаимодействия
     * @return true, если объект активен
     */
    bool isInteractable() const { return m_isInteractable; }

    /**
     * @brief Установка активности объекта для взаимодействия
     * @param interactable true, если объект должен быть активен
     */
    void setInteractable(bool interactable) { m_isInteractable = interactable; }

    /**
     * @brief Установка подсказки для взаимодействия
     * @param hint Текст подсказки
     */
    void setInteractionHint(const std::string& hint) { m_interactionHint = hint; }

    /**
     * @brief Получение подсказки для взаимодействия
     * @return Текст подсказки
     */
    const std::string& getInteractionHint() const { return m_interactionHint; }

    /**
     * @brief Установка обратного вызова для взаимодействия
     * @param callback Функция обратного вызова
     */
    void setInteractionCallback(std::function<void(Player*)> callback) {
        m_interactionCallback = callback;
    }

    /**
     * @brief Получение цвета объекта
     * @return Цвет объекта
     */
    const SDL_Color& getColor() const { return m_color; }

    /**
     * @brief Установка цвета объекта
     * @param color Цвет объекта
     */
    void setColor(const SDL_Color& color) { m_color = color; }

protected:
    InteractiveType m_interactiveType;                  ///< Тип интерактивного объекта
    float m_interactionRadius;                          ///< Радиус, в котором возможно взаимодействие
    bool m_isInteractable;                              ///< Можно ли взаимодействовать с объектом
    std::string m_interactionHint;                      ///< Подсказка при взаимодействии
    std::function<void(Player*)> m_interactionCallback; ///< Обратный вызов при взаимодействии
    SDL_Color m_color;                                  ///< Цвет объекта
    float m_height;                                     ///< Высота объекта
};