#include "Player.h"
#include "PlayerState.h"
#include <cmath>
#include <iostream>
#include "Logger.h"

Player::Player(const std::string& name, std::shared_ptr<TileMap> tileMap,
    std::shared_ptr<CollisionSystem> collisionSystem)
    : Entity(name), m_tileMap(tileMap), m_collisionSystem(collisionSystem),
    m_subX(0.5f), m_subY(0.5f), m_moveSpeed(0.05f), m_dX(0.0f), m_dY(0.0f),
    m_collisionSize(0.2f), m_currentState(nullptr) { // Уменьшенный размер коллизии с 0.35f до 0.2f

    // Устанавливаем цвет игрока
    m_color = { 255, 50, 50, 255 }; // Ярко-красный
}

Player::~Player() {
    // Состояния очистятся автоматически благодаря unique_ptr
    LOG_DEBUG("Player destroyed: " + m_name);
}

bool Player::initialize() {
    // Инициализация состояний
    initializeStates();

    // Установка начального состояния
    if (m_states.find("Idle") != m_states.end()) {
        m_currentState = m_states["Idle"].get();
        m_currentState->enter();
    }

    LOG_INFO("Player initialized: " + m_name);
    return true;
}

void Player::handleEvent(const SDL_Event& event) {
    // Передаем событие текущему состоянию
    if (m_currentState) {
        m_currentState->handleEvent(event);
    }
}

void Player::update(float deltaTime) {
    // 1. Обновляем текущее состояние
    if (m_currentState) {
        m_currentState->update(deltaTime);
    }

    // 2. Перемещаем игрока с учетом коллизий
    moveWithCollision(deltaTime);

    // 3. Обновляем Z-координату для корректного отображения
    m_position.z = 0.5f; // Высота персонажа
}

void Player::render(SDL_Renderer* renderer) {
    // В базовой версии не делаем ничего - отрисовка осуществляется в MapScene
    // В дальнейшем здесь будет код для отрисовки спрайта персонажа
}

void Player::setPlayerPosition(float x, float y, float subX, float subY) {
    m_position.x = x;
    m_position.y = y;
    m_subX = subX;
    m_subY = subY;
}

bool Player::canMoveDiagonally(int fromX, int fromY, int toX, int toY) {
    // Убедимся, что это действительно диагональное перемещение
    int dx = toX - fromX;
    int dy = toY - fromY;

    if (abs(dx) != 1 || abs(dy) != 1) {
        // Не диагональное перемещение
        return m_tileMap->isValidCoordinate(toX, toY) && m_tileMap->isTileWalkable(toX, toY);
    }

    // Проверяем целевой тайл
    if (!m_tileMap->isValidCoordinate(toX, toY) || !m_tileMap->isTileWalkable(toX, toY)) {
        return false;
    }

    // Проверка обоих промежуточных тайлов для избежания "срезания углов"
    // Оба тайла должны быть проходимыми
    bool canPassX = m_tileMap->isValidCoordinate(toX, fromY) && m_tileMap->isTileWalkable(toX, fromY);
    bool canPassY = m_tileMap->isValidCoordinate(fromX, toY) && m_tileMap->isTileWalkable(fromX, toY);

    return canPassX && canPassY;
}

float Player::calculateZOrderPriority() {
    // 1. Базовый приоритет на основе положения в пространстве
    float baseDepth = (m_position.x + m_position.y) * 10.0f;

    // 2. Высота (Z) влияет на приоритет отображения
    float heightFactor = m_position.z * 5.0f;

    // 3. Для персонажа учитываем положение относительно границ тайлов
    float boundaryFactor = 0.0f;

    // Проверяем близость к границе тайла
    if (m_subX < 0.1f || m_subX > 0.9f ||
        m_subY < 0.1f || m_subY > 0.9f) {
        boundaryFactor = 0.5f;

        // Корректировка в зависимости от направления движения
        if (m_subX < 0.1f && m_dX < 0.0f) {
            boundaryFactor = 1.0f;  // Движение влево
        }
        else if (m_subX > 0.9f && m_dX > 0.0f) {
            boundaryFactor = 1.0f;  // Движение вправо
        }

        if (m_subY < 0.1f && m_dY < 0.0f) {
            boundaryFactor = 1.0f;  // Движение вверх
        }
        else if (m_subY > 0.9f && m_dY > 0.0f) {
            boundaryFactor = 1.0f;  // Движение вниз
        }
    }

    // 4. Финальный приоритет
    return baseDepth + heightFactor + boundaryFactor;
}

std::string Player::getCurrentStateName() const {
    if (m_currentState) {
        return m_currentState->getName();
    }
    return "None";
}

bool Player::changeState(const std::string& stateName) {
    // Проверяем, существует ли такое состояние
    auto it = m_states.find(stateName);
    if (it == m_states.end()) {
        LOG_ERROR("Player state '" + stateName + "' not found!");
        return false;
    }

    // Если это то же самое состояние, ничего не делаем
    if (m_currentState == it->second.get()) {
        return true;
    }

    // Выходим из текущего состояния
    if (m_currentState) {
        m_currentState->exit();
    }

    // Устанавливаем новое состояние
    m_currentState = it->second.get();

    // Входим в новое состояние
    m_currentState->enter();

    LOG_DEBUG("Player state changed to: " + stateName);
    return true;
}

void Player::handleCurrentStateEvent(const SDL_Event& event) {
    if (m_currentState) {
        m_currentState->handleEvent(event);
    }
}

void Player::setMoveDirection(float dx, float dy) {
    m_dX = dx;
    m_dY = dy;
}

void Player::moveWithCollision(float deltaTime) {
    // Если нет направления движения, ничего не делаем
    if (m_dX == 0.0f && m_dY == 0.0f) {
        return;
    }

    // 1. Нормализация скорости для разных частот кадров
    float normalizedSpeed = m_moveSpeed * deltaTime * 60.0f;

    // 2. Применяем сглаживание при движении - постепенно уменьшаем скорость вблизи стен
    // Сначала проверяем, не столкнемся ли мы со стеной при полной скорости
    float fullDeltaX = m_dX * normalizedSpeed;
    float fullDeltaY = m_dY * normalizedSpeed;

    // Получаем форму коллизии игрока
    CollisionShape playerShape(m_collisionSize * 0.8f); // Временно уменьшаем размер для проверки

    // Координаты после полного перемещения
    float fullNextX = m_position.x + fullDeltaX;
    float fullNextY = m_position.y + fullDeltaY;

    // Проверяем наличие коллизии при полном перемещении
    bool willCollide = !m_collisionSystem->canMove(this, playerShape, fullNextX, fullNextY);

    // 3. Если будет коллизия, применяем более осторожное движение с несколькими шагами
    if (willCollide) {
        // Количество шагов для плавного движения
        const int numSteps = 5;
        float stepDeltaX = fullDeltaX / numSteps;
        float stepDeltaY = fullDeltaY / numSteps;

        // Перемещаемся пошагово до обнаружения коллизии
        for (int i = 0; i < numSteps; i++) {
            float nextX = m_position.x + stepDeltaX;
            float nextY = m_position.y + stepDeltaY;

            // Проверяем коллизию на этом шаге
            if (m_collisionSystem->canMove(this, playerShape, nextX, nextY)) {
                // Если коллизии нет, выполняем этот шаг
                m_position.x = nextX;
                m_position.y = nextY;
            }
            else {
                // Если обнаружена коллизия, пробуем скользить вдоль стены
                bool canMoveX = m_collisionSystem->canMove(this, playerShape, m_position.x + stepDeltaX, m_position.y);
                bool canMoveY = m_collisionSystem->canMove(this, playerShape, m_position.x, m_position.y + stepDeltaY);

                if (canMoveX) {
                    m_position.x += stepDeltaX;
                }

                if (canMoveY) {
                    m_position.y += stepDeltaY;
                }

                // Если не можем двигаться ни по X, ни по Y - останавливаемся
                if (!canMoveX && !canMoveY) {
                    break;
                }
            }
        }
    }
    else {
        // Если коллизии не будет, просто выполняем полное перемещение
        m_position.x = fullNextX;
        m_position.y = fullNextY;
    }

    // 4. Обновляем субкоординаты в зависимости от новой позиции
    m_subX = m_position.x - std::floor(m_position.x);
    m_subY = m_position.y - std::floor(m_position.y);

    // 5. Гарантируем, что субкоординаты остаются в диапазоне [0, 1)
    if (m_subX >= 1.0f) {
        m_subX -= 1.0f;
    }
    else if (m_subX < 0.0f) {
        m_subX += 1.0f;
    }

    if (m_subY >= 1.0f) {
        m_subY -= 1.0f;
    }
    else if (m_subY < 0.0f) {
        m_subY += 1.0f;
    }
}

CollisionShape Player::getCollisionShape() const {
    // Возвращаем круговую форму коллизии с заданным радиусом
    return CollisionShape(m_collisionSize);
}

void Player::initializeStates() {
    // Создаем и добавляем состояния в словарь
    m_states["Idle"] = std::make_unique<IdleState>(this);
    m_states["Moving"] = std::make_unique<MovingState>(this);
    m_states["Action"] = std::make_unique<ActionState>(this);

    LOG_DEBUG("Player states initialized");
}
