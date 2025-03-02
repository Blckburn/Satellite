#pragma once

#include <SDL.h>
#include <memory>
#include <string>

// Forward declarations
class Player;

/**
 * @brief Базовый абстрактный класс для состояний игрока
 */
class PlayerState {
public:
    /**
     * @brief Конструктор
     * @param player Указатель на игрока
     */
    PlayerState(Player* player);

    /**
     * @brief Виртуальный деструктор
     */
    virtual ~PlayerState() = default;

    /**
     * @brief Обработка ввода
     * @param event Событие SDL
     */
    virtual void handleEvent(const SDL_Event& event) = 0;

    /**
     * @brief Обновление состояния
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    virtual void update(float deltaTime) = 0;

    /**
     * @brief Вход в состояние
     */
    virtual void enter() = 0;

    /**
     * @brief Выход из состояния
     */
    virtual void exit() = 0;

    /**
     * @brief Получение имени состояния
     * @return Строка с именем состояния
     */
    virtual std::string getName() const = 0;

protected:
    Player* m_player; ///< Указатель на игрока
};

/**
 * @brief Состояние покоя игрока
 */
class IdleState : public PlayerState {
public:
    IdleState(Player* player);
    virtual ~IdleState() = default;

    void handleEvent(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void enter() override;
    void exit() override;
    std::string getName() const override { return "Idle"; }
};

/**
 * @brief Состояние движения игрока
 */
class MovingState : public PlayerState {
public:
    MovingState(Player* player);
    virtual ~MovingState() = default;

    void handleEvent(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void enter() override;
    void exit() override;
    std::string getName() const override { return "Moving"; }

private:
    float m_lastDx; ///< Последнее направление движения по X
    float m_lastDy; ///< Последнее направление движения по Y
    bool m_keyPressed; ///< Флаг нажатия клавиши направления
};

/**
 * @brief Состояние выполнения действия игроком
 */
class ActionState : public PlayerState {
public:
    /**
     * @brief Типы действий
     */
    enum class ActionType {
        INTERACT, ///< Взаимодействие с объектом
        USE_ITEM,  ///< Использование предмета
        ATTACK     ///< Атака
    };

    ActionState(Player* player, ActionType actionType = ActionType::INTERACT);
    virtual ~ActionState() = default;

    void handleEvent(const SDL_Event& event) override;
    void update(float deltaTime) override;
    void enter() override;
    void exit() override;
    std::string getName() const override { return "Action"; }

    /**
     * @brief Получение типа действия
     * @return Тип действия
     */
    ActionType getActionType() const { return m_actionType; }

    /**
     * @brief Установка типа действия
     * @param actionType Тип действия
     */
    void setActionType(ActionType actionType) { m_actionType = actionType; }

private:
    ActionType m_actionType; ///< Тип действия
    float m_actionTimer; ///< Таймер для действия
    float m_actionDuration; ///< Продолжительность действия
};