#include "Player.h"
#include "PlayerState.h"
#include <cmath>
#include <iostream>
#include "Logger.h"

Player::Player(const std::string& name, std::shared_ptr<TileMap> tileMap,
    std::shared_ptr<CollisionSystem> collisionSystem)
    : Entity(name), m_tileMap(tileMap), m_collisionSystem(collisionSystem),
    m_subX(0.5f), m_subY(0.5f), m_moveSpeed(0.05f), m_dX(0.0f), m_dY(0.0f),
    m_collisionSize(0.2f), m_currentState(nullptr), m_targetX(0.0f), m_targetY(0.0f), m_isMoving(false),
    m_velocityX(0.0f), m_velocityY(0.0f), m_collisionOccurred(false),
    m_lastValidX(0.0f), m_lastValidY(0.0f), m_visualX(0.0f), m_visualY(0.0f), m_visualSubX(0.5f), m_visualSubY(0.5f), m_historyIndex(0), m_positionTime(0.0f), m_usingHistory(false), m_stabilityTime(0.0f), m_accumulatedTime(0.0f) { // Добавлена инициализация m_accumulatedTime

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

    // Инициализация истории позиций
    for (int i = 0; i < HISTORY_SIZE; i++) {
        m_positionHistory[i].x = m_position.x;
        m_positionHistory[i].y = m_position.y;
        m_positionHistory[i].time = 0.0f;
    }

    // Синхронизация визуальной и логической позиции в начале
    m_visualX = m_position.x;
    m_visualY = m_position.y;
    m_visualSubX = m_subX;
    m_visualSubY = m_subY;

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
    // Обновляем общее время
    m_positionTime += deltaTime;

    // После нестабильности даем системе время на восстановление
    if (m_usingHistory) {
        m_stabilityTime += deltaTime;
        if (m_stabilityTime > 0.5f) {
            m_usingHistory = false;
            m_stabilityTime = 0.0f;
        }
    }

    // 1. Обновление состояния и обработка ввода
    if (m_currentState) {
        m_currentState->update(deltaTime);
    }

    // 2. Обновление логической позиции
    updateMovement(deltaTime);

    // 3. Обновление визуальной позиции (с интерполяцией)
    updateVisualPosition(deltaTime);
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

float Player::moveTowards(float current, float target, float maxDelta) {
    if (std::abs(target - current) <= maxDelta) {
        return target;
    }
    return current + std::copysign(maxDelta, target - current);
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
    // 1. Если нет направления движения, ничего не делаем
    if (m_dX == 0.0f && m_dY == 0.0f) {
        return;
    }

    // 2. Нормализация скорости для разных частот кадров с ограничением максимального перемещения
    // Используем меньший множитель и добавляем ограничение максимального перемещения за кадр
    float normalizedSpeed = m_moveSpeed * deltaTime * 30.0f;
    normalizedSpeed = std::min(normalizedSpeed, 0.1f); // Ограничиваем максимальное перемещение за один кадр

    // 3. Вычисляем целевое перемещение
    float targetDeltaX = m_dX * normalizedSpeed;
    float targetDeltaY = m_dY * normalizedSpeed;

    // 4. Пробуем сначала полное перемещение
    float nextX = m_position.x + targetDeltaX;
    float nextY = m_position.y + targetDeltaY;

    // Создаем форму коллизии с небольшим уменьшением для лучшего скольжения
    CollisionShape playerShape(m_collisionSize * 0.9f);

    // 5. Если полное перемещение возможно, выполняем его
    if (m_collisionSystem->canMove(this, playerShape, nextX, nextY)) {
        m_position.x = nextX;
        m_position.y = nextY;
    }
    // 6. Иначе пробуем движение по отдельным осям для скольжения
    else {
        // Попытка движения только по X
        if (targetDeltaX != 0.0f) {
            nextX = m_position.x + targetDeltaX;
            if (m_collisionSystem->canMove(this, playerShape, nextX, m_position.y)) {
                m_position.x = nextX;
            }
        }

        // Попытка движения только по Y
        if (targetDeltaY != 0.0f) {
            nextY = m_position.y + targetDeltaY;
            if (m_collisionSystem->canMove(this, playerShape, m_position.x, nextY)) {
                m_position.y = nextY;
            }
        }
    }

    // 7. Обновляем субкоординаты в зависимости от новой позиции
    m_subX = m_position.x - std::floor(m_position.x);
    m_subY = m_position.y - std::floor(m_position.y);

    // 8. Гарантируем, что субкоординаты остаются в диапазоне [0, 1)
    if (m_subX >= 1.0f) m_subX -= 1.0f;
    else if (m_subX < 0.0f) m_subX += 1.0f;

    if (m_subY >= 1.0f) m_subY -= 1.0f;
    else if (m_subY < 0.0f) m_subY += 1.0f;
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

void Player::updateSmoothDirection(float deltaTime) {
    // Применяем плавную интерполяцию к направлению движения
    if (m_currentState && m_currentState->getName() == "Moving") {
        // В состоянии движения используем направление из State
        MovingState* movingState = dynamic_cast<MovingState*>(m_currentState);
        if (movingState) {
            // Целевое направление уже установлено в m_dX и m_dY из состояния
            // Но мы можем применить интерполяцию для плавности
        }
    }
    else if (m_dX != 0.0f || m_dY != 0.0f) {
        // Если мы не в состоянии движения, но у нас есть направление, плавно уменьшаем его
        float slowdownFactor = 5.0f * deltaTime;

        if (std::abs(m_dX) < 0.01f) m_dX = 0.0f;
        else m_dX -= m_dX * slowdownFactor;

        if (std::abs(m_dY) < 0.01f) m_dY = 0.0f;
        else m_dY -= m_dY * slowdownFactor;
    }
}

void Player::moveWithFixedStep(float fixedDeltaTime) {
    // Если нет направления движения, ничего не делаем
    if (m_dX == 0.0f && m_dY == 0.0f) {
        return;
    }

    // Базовая скорость для этого фрейма - уменьшенная для большей плавности
    float baseSpeed = m_moveSpeed * fixedDeltaTime * 10.0f; // Меньший коэффициент

    // Рассчитываем целевое перемещение
    float deltaX = m_dX * baseSpeed;
    float deltaY = m_dY * baseSpeed;

    // Используем уменьшенную форму коллизии для лучшего скольжения
    CollisionShape playerShape(m_collisionSize * 0.8f);

    // Проверяем возможность полного перемещения (диагонального)
    bool canMoveFully = m_collisionSystem->canMove(this, playerShape,
        m_position.x + deltaX,
        m_position.y + deltaY);

    if (canMoveFully) {
        // Если полное перемещение возможно, просто выполняем его
        m_position.x += deltaX;
        m_position.y += deltaY;
    }
    else {
        // Пробуем движение по отдельным осям
        bool canMoveX = m_collisionSystem->canMove(this, playerShape,
            m_position.x + deltaX,
            m_position.y);

        bool canMoveY = m_collisionSystem->canMove(this, playerShape,
            m_position.x,
            m_position.y + deltaY);

        // Применяем перемещение по доступным направлениям
        if (canMoveX) m_position.x += deltaX;
        if (canMoveY) m_position.y += deltaY;
    }

    // Обновляем субкоординаты
    m_subX = m_position.x - std::floor(m_position.x);
    m_subY = m_position.y - std::floor(m_position.y);

    // Нормализуем субкоординаты
    if (m_subX >= 1.0f) m_subX -= 1.0f;
    else if (m_subX < 0.0f) m_subX += 1.0f;

    if (m_subY >= 1.0f) m_subY -= 1.0f;
    else if (m_subY < 0.0f) m_subY += 1.0f;
}

void Player::initPhysics() {
    m_targetX = m_position.x;
    m_targetY = m_position.y;
    m_lastValidX = m_position.x;
    m_lastValidY = m_position.y;
    m_velocityX = 0.0f;
    m_velocityY = 0.0f;
    m_isMoving = false;
    m_collisionOccurred = false;
}

void Player::updatePhysics(float deltaTime) {
    // 1. Обновление целевой позиции на основе ввода
    if (m_dX != 0.0f || m_dY != 0.0f) {
        m_isMoving = true;
    }
    else {
        m_isMoving = false;
    }

    // 2. Применение ускорения/замедления к скорости
    float targetVelocityX = m_dX * MAX_VELOCITY;
    float targetVelocityY = m_dY * MAX_VELOCITY;

    if (m_isMoving) {
        // Ускорение при движении
        m_velocityX = moveTowards(m_velocityX, targetVelocityX, ACCELERATION * deltaTime);
        m_velocityY = moveTowards(m_velocityY, targetVelocityY, ACCELERATION * deltaTime);
    }
    else {
        // Замедление при остановке
        m_velocityX = moveTowards(m_velocityX, 0.0f, DECELERATION * deltaTime);
        m_velocityY = moveTowards(m_velocityY, 0.0f, DECELERATION * deltaTime);
    }

    // 3. Рассчитываем новую позицию
    float newX = m_position.x + m_velocityX * deltaTime;
    float newY = m_position.y + m_velocityY * deltaTime;

    // 4. Проверка на коллизии
    CollisionShape playerShape(m_collisionSize * 0.8f);  // Уменьшенная форма для лучшего скольжения
    bool canMove = m_collisionSystem->canMove(this, playerShape, newX, newY);

    if (canMove) {
        // Сохраняем текущую позицию как последнюю валидную
        m_lastValidX = m_position.x;
        m_lastValidY = m_position.y;

        // Обновляем позицию
        m_position.x = newX;
        m_position.y = newY;
        m_collisionOccurred = false;
    }
    else {
        m_collisionOccurred = true;

        // Пробуем перемещение по отдельным осям с измененными скоростями
        bool canMoveX = m_collisionSystem->canMove(this, playerShape, newX, m_position.y);
        bool canMoveY = m_collisionSystem->canMove(this, playerShape, m_position.x, newY);

        if (canMoveX) {
            m_position.x = newX;
            // Уменьшаем скорость по оси X для более плавного замедления у стены
            m_velocityX *= 0.8f;
        }
        else {
            // Останавливаем движение по X при столкновении
            m_velocityX = 0.0f;
        }

        if (canMoveY) {
            m_position.y = newY;
            // Уменьшаем скорость по оси Y для более плавного замедления у стены
            m_velocityY *= 0.8f;
        }
        else {
            // Останавливаем движение по Y при столкновении
            m_velocityY = 0.0f;
        }
    }
}

void Player::updateMovement(float deltaTime) {
    // Если нет направления движения, сохраняем позицию и выходим
    if (m_dX == 0.0f && m_dY == 0.0f) {
        // Когда персонаж стоит, это хорошее время для сохранения позиции
        if (!m_usingHistory) {
            savePositionToHistory();
        }
        return;
    }

    // Если мы восстанавливаемся после нестабильности, не двигаемся
    if (m_usingHistory) {
        return;
    }

    // Сохраняем текущую позицию перед движением
    float oldX = m_position.x;
    float oldY = m_position.y;

    // Используем очень низкую скорость для стабильности логики
    const float moveSpeed = 2.0f * deltaTime;

    // Рассчитываем перемещение
    float deltaX = m_dX * moveSpeed;
    float deltaY = m_dY * moveSpeed;

    // Коллизия с уменьшенным размером для лучшей проходимости
    CollisionShape smallerShape(m_collisionSize * 0.7f);

    // Проверяем возможность полного перемещения
    bool canMove = m_collisionSystem->canMove(this, smallerShape,
        m_position.x + deltaX,
        m_position.y + deltaY);

    if (canMove) {
        // Перемещаем персонажа в новую позицию
        m_position.x += deltaX;
        m_position.y += deltaY;
    }
    else {
        // Раздельная проверка по осям для скольжения вдоль стен
        bool canMoveX = m_collisionSystem->canMove(this, smallerShape,
            m_position.x + deltaX,
            m_position.y);

        bool canMoveY = m_collisionSystem->canMove(this, smallerShape,
            m_position.x,
            m_position.y + deltaY);

        // Применяем допустимые смещения
        if (canMoveX) m_position.x += deltaX;
        if (canMoveY) m_position.y += deltaY;
    }

    // Обновляем субкоординаты логической позиции
    m_subX = m_position.x - std::floor(m_position.x);
    m_subY = m_position.y - std::floor(m_position.y);

    // Принудительно нормализуем субкоординаты
    if (m_subX >= 1.0f) {
        m_subX -= 1.0f;
        m_position.x = std::floor(m_position.x) + 1.0f + m_subX;
    }
    else if (m_subX < 0.0f) {
        m_subX += 1.0f;
        m_position.x = std::floor(m_position.x) - 1.0f + m_subX;
    }

    if (m_subY >= 1.0f) {
        m_subY -= 1.0f;
        m_position.y = std::floor(m_position.y) + 1.0f + m_subY;
    }
    else if (m_subY < 0.0f) {
        m_subY += 1.0f;
        m_position.y = std::floor(m_position.y) - 1.0f + m_subY;
    }

    // Проверяем, валидна ли новая позиция
    bool positionIsValid = m_collisionSystem->canMove(this, smallerShape, m_position.x, m_position.y);

    if (!positionIsValid) {
        // Если новая позиция недопустима, пытаемся восстановиться
        bool recovered = findStablePosition(0.3f);

        if (!recovered) {
            // Если не удалось восстановиться из истории, возвращаемся к предыдущей позиции
            m_position.x = oldX;
            m_position.y = oldY;
            m_subX = m_position.x - std::floor(m_position.x);
            m_subY = m_position.y - std::floor(m_position.y);

            // Если и предыдущая позиция невалидна, принудительно стабилизируем
            if (!m_collisionSystem->canMove(this, smallerShape, m_position.x, m_position.y)) {
                forceStabilizePosition();
            }
        }

        // Устанавливаем флаг использования истории
        m_usingHistory = true;
        m_stabilityTime = 0.0f;
    }
    else {
        // Если позиция валидна, сохраняем её в историю
        savePositionToHistory();
    }
}

// Новый метод для обновления визуальной позиции
void Player::updateVisualPosition(float deltaTime) {
    // Вычисляем целевую полную позицию
    float targetX = m_position.x;
    float targetY = m_position.y;

    // Рассчитываем текущее расстояние между визуальной и логической позицией
    float distanceX = targetX - m_visualX;
    float distanceY = targetY - m_visualY;
    float distance = std::sqrt(distanceX * distanceX + distanceY * distanceY);

    // Если расстояние больше порогового значения, применяем плавное перемещение
    if (distance > 0.001f) {
        // Вычисляем максимальное перемещение за этот кадр
        float maxMove = VISUAL_SMOOTHING * deltaTime;

        // Если расстояние слишком большое, ограничиваем скорость
        if (distance > 1.0f) {
            maxMove = std::min(maxMove, distance * 0.2f); // Ограничение до 20% расстояния
        }

        // Ограничиваем maxMove для слишком больших значений deltaTime
        maxMove = std::min(maxMove, 0.5f);

        // Если расстояние меньше maxMove, просто устанавливаем визуальную позицию равной логической
        if (distance <= maxMove) {
            m_visualX = targetX;
            m_visualY = targetY;
        }
        else {
            // Иначе перемещаем визуальную позицию в направлении логической на maxMove
            float moveX = distanceX * (maxMove / distance);
            float moveY = distanceY * (maxMove / distance);

            m_visualX += moveX;
            m_visualY += moveY;
        }
    }
    else {
        // Если расстояние очень маленькое, просто устанавливаем визуальную позицию равной логической
        m_visualX = targetX;
        m_visualY = targetY;
    }

    // Обновляем визуальные субкоординаты
    m_visualSubX = m_visualX - std::floor(m_visualX);
    m_visualSubY = m_visualY - std::floor(m_visualY);
}

void Player::savePositionToHistory() {
    // Проверяем, валидна ли текущая позиция
    CollisionShape shape(m_collisionSize * 0.7f);
    bool isValidPosition = m_collisionSystem->canMove(this, shape, m_position.x, m_position.y);

    // Сохраняем только валидные позиции
    if (isValidPosition) {
        m_historyIndex = (m_historyIndex + 1) % HISTORY_SIZE;
        m_positionHistory[m_historyIndex].x = m_position.x;
        m_positionHistory[m_historyIndex].y = m_position.y;
        m_positionHistory[m_historyIndex].time = m_positionTime;
    }
}

bool Player::findStablePosition(float maxAge) {
    // Если буфер истории пуст, возвращаем false
    if (m_positionTime < 0.1f) return false;

    // Минимальное допустимое время для позиции
    float minTime = m_positionTime - maxAge;

    // Ищем самую свежую валидную позицию
    for (int i = 0; i < HISTORY_SIZE; i++) {
        // Проверяем в обратном порядке, начиная с последней сохраненной позиции
        int index = (m_historyIndex - i + HISTORY_SIZE) % HISTORY_SIZE;

        // Проверяем, что позиция достаточно свежая
        if (m_positionHistory[index].time < minTime) continue;

        // Проверяем, валидна ли позиция
        CollisionShape shape(m_collisionSize * 0.7f);
        bool isValidPosition = m_collisionSystem->canMove(
            this, shape,
            m_positionHistory[index].x,
            m_positionHistory[index].y
        );

        if (isValidPosition) {
            // Нашли валидную позицию, возвращаемся к ней
            m_position.x = m_positionHistory[index].x;
            m_position.y = m_positionHistory[index].y;

            // Обновляем субкоординаты
            m_subX = m_position.x - std::floor(m_position.x);
            m_subY = m_position.y - std::floor(m_position.y);

            return true;
        }
    }

    // Не нашли валидную позицию в истории
    return false;
}

// Метод для принудительной стабилизации позиции
void Player::forceStabilizePosition() {
    // Получаем текущий тайл
    int currentTileX = static_cast<int>(floor(m_position.x));
    int currentTileY = static_cast<int>(floor(m_position.y));

    // Проверяем, проходим ли текущий тайл
    bool currentTileWalkable = m_tileMap->isValidCoordinate(currentTileX, currentTileY) &&
        m_tileMap->isTileWalkable(currentTileX, currentTileY);

    if (currentTileWalkable) {
        // Если текущий тайл проходим, устанавливаем позицию в его центр
        m_position.x = currentTileX + 0.5f;
        m_position.y = currentTileY + 0.5f;
        m_subX = 0.5f;
        m_subY = 0.5f;
        return;
    }

    // Если текущий тайл непроходим, ищем ближайший проходимый тайл
    const int searchRadius = 3;
    float nearestDistance = FLT_MAX;
    int nearestX = currentTileX;
    int nearestY = currentTileY;

    for (int y = currentTileY - searchRadius; y <= currentTileY + searchRadius; y++) {
        for (int x = currentTileX - searchRadius; x <= currentTileX + searchRadius; x++) {
            if (m_tileMap->isValidCoordinate(x, y) && m_tileMap->isTileWalkable(x, y)) {
                float distance = (x - m_position.x) * (x - m_position.x) +
                    (y - m_position.y) * (y - m_position.y);

                if (distance < nearestDistance) {
                    nearestDistance = distance;
                    nearestX = x;
                    nearestY = y;
                }
            }
        }
    }

    // Устанавливаем позицию в центр ближайшего проходимого тайла
    m_position.x = nearestX + 0.5f;
    m_position.y = nearestY + 0.5f;
    m_subX = 0.5f;
    m_subY = 0.5f;
}