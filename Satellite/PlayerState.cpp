#include "PlayerState.h"
#include "Player.h"
#include "Logger.h"

// ----------------------------------------------------
// PlayerState - базовый класс
// ----------------------------------------------------

PlayerState::PlayerState(Player* player) : m_player(player) {
}

// ----------------------------------------------------
// IdleState - состояние покоя
// ----------------------------------------------------

IdleState::IdleState(Player* player) : PlayerState(player) {
}

void IdleState::handleEvent(const SDL_Event& event) {
    // Обработка событий клавиатуры для перехода к движению
    if (event.type == SDL_KEYDOWN) {
        // Проверяем нажатие клавиш движения
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_w || key == SDLK_UP ||
            key == SDLK_s || key == SDLK_DOWN ||
            key == SDLK_a || key == SDLK_LEFT ||
            key == SDLK_d || key == SDLK_RIGHT) {

            // Запрашиваем переход в состояние движения
            m_player->changeState("Moving");

            // Передаем событие новому состоянию
            m_player->handleCurrentStateEvent(event);
        }
        // Проверяем нажатие клавиши действия
        else if (key == SDLK_e || key == SDLK_SPACE) {
            m_player->changeState("Action");

            // Передаем событие новому состоянию
            m_player->handleCurrentStateEvent(event);
        }
    }
}

void IdleState::update(float deltaTime) {
    // В состоянии покоя игрок не двигается
    m_player->setMoveDirection(0.0f, 0.0f);

    // Здесь будет логика для анимации покоя
}

void IdleState::enter() {
    LOG_DEBUG("Player entered Idle state");

    // Сбрасываем направление движения
    m_player->setMoveDirection(0.0f, 0.0f);
}

void IdleState::exit() {
    LOG_DEBUG("Player exited Idle state");
}

// ----------------------------------------------------
// MovingState - состояние движения
// ----------------------------------------------------

MovingState::MovingState(Player* player)
    : PlayerState(player), m_lastDx(0.0f), m_lastDy(0.0f), m_keyPressed(false) {
}

void MovingState::handleEvent(const SDL_Event& event) {
    // 1. Обработка событий клавиатуры

    // Проверяем клавиши движения
    if (event.type == SDL_KEYDOWN) {
        m_keyPressed = true;

        // Проверяем нажатие клавиши действия
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_e || key == SDLK_SPACE) {
            m_player->changeState("Action");

            // Передаем событие новому состоянию
            m_player->handleCurrentStateEvent(event);
        }
    }
    else if (event.type == SDL_KEYUP) {
        // При отпускании клавиши проверяем, нужно ли перейти в состояние покоя
        const Uint8* keyState = SDL_GetKeyboardState(NULL);

        // Проверяем, нажаты ли клавиши движения
        bool moveKeyPressed =
            keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP] ||
            keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN] ||
            keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT] ||
            keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT];

        if (!moveKeyPressed) {
            m_keyPressed = false;

            // Если не нажаты клавиши движения, переходим в состояние покоя
            m_player->changeState("Idle");
        }
    }
}

void MovingState::update(float deltaTime) {
    // 1. Получаем текущее состояние клавиатуры
    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    // 2. Определяем нажатие клавиш направления
    bool upPressed = keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP];
    bool downPressed = keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN];
    bool leftPressed = keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT];
    bool rightPressed = keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT];

    // Сбрасываем направление перед установкой нового
    float dX = 0.0f;
    float dY = 0.0f;

    // 3. Определение направления (изометрическая система координат)
    if (upPressed && !downPressed) {
        dY = -1.0f; // Север в изометрии
    }
    else if (!upPressed && downPressed) {
        dY = 1.0f;  // Юг в изометрии
    }

    if (rightPressed && !leftPressed) {
        dX = 1.0f;  // Восток в изометрии
    }
    else if (!rightPressed && leftPressed) {
        dX = -1.0f; // Запад в изометрии
    }

    // 4. Нормализация для диагонального движения
    if (dX != 0.0f && dY != 0.0f) {
        float length = std::sqrt(dX * dX + dY * dY);
        dX /= length;
        dY /= length;
    }

    // 5. Запоминаем последнее направление движения (для анимации)
    if (dX != 0.0f || dY != 0.0f) {
        m_lastDx = dX;
        m_lastDy = dY;
    }

    // 6. Устанавливаем направление движения
    m_player->setMoveDirection(dX, dY);

    // 7. Проверяем, нужно ли перейти в состояние покоя
    if (dX == 0.0f && dY == 0.0f && !m_keyPressed) {
        m_player->changeState("Idle");
    }
}

void MovingState::enter() {
    LOG_DEBUG("Player entered Moving state");
}

void MovingState::exit() {
    LOG_DEBUG("Player exited Moving state");
}

// ----------------------------------------------------
// ActionState - состояние выполнения действия
// ----------------------------------------------------

ActionState::ActionState(Player* player, ActionType actionType)
    : PlayerState(player), m_actionType(actionType), m_actionTimer(0.0f), m_actionDuration(0.5f) {
}

void ActionState::handleEvent(const SDL_Event& event) {
    // В состоянии действия игнорируем ввод до завершения действия
}

void ActionState::update(float deltaTime) {
    // 1. Обновляем таймер действия
    m_actionTimer += deltaTime;

    // 2. Если действие завершено, переходим в состояние покоя
    if (m_actionTimer >= m_actionDuration) {
        m_player->changeState("Idle");
        return;
    }

    // 3. Выполняем логику действия в зависимости от типа
    switch (m_actionType) {
    case ActionType::INTERACT:
        // Логика взаимодействия с объектами
        // В будущем здесь будет проверка объектов вокруг игрока
        break;

    case ActionType::USE_ITEM:
        // Логика использования предмета
        break;

    case ActionType::ATTACK:
        // Логика атаки
        break;
    }
}

void ActionState::enter() {
    LOG_DEBUG("Player entered Action state with type: " + std::to_string(static_cast<int>(m_actionType)));

    // Сбрасываем таймер действия
    m_actionTimer = 0.0f;

    // Останавливаем движение
    m_player->setMoveDirection(0.0f, 0.0f);

    // В будущем здесь будет запуск соответствующей анимации
}

void ActionState::exit() {
    LOG_DEBUG("Player exited Action state");
}