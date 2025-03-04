#include "Player.h"
#include <cmath>
#include <iostream>
#include "CollisionSystem.h"  // Добавить этот include
#include "IsometricRenderer.h"

Player::Player(const std::string& name, TileMap* tileMap)
    : Entity(name), m_tileMap(tileMap), m_currentDirection(Direction::SOUTH),
    m_subX(0.5f), m_subY(0.5f), m_moveSpeed(0.05f), m_dX(0.0f), m_dY(0.0f),
    m_collisionSize(0.35f), m_height(0.5f)
{
    // Устанавливаем начальный цвет игрока (красный)
    m_color = { 255, 50, 50, 255 };

    // Инициализируем цвета граней
    updateFaceColors();
}

Player::~Player()
{
    // Здесь освобождаются ресурсы, если они были выделены
}

bool Player::initialize()
{
    // Начальные настройки игрока
    setPosition(25.0f, 25.0f, 0.0f); // Установка позиции по умолчанию
    return true;
}

void Player::handleEvent(const SDL_Event& event)
{
    // Базовая обработка событий из Entity
    Entity::handleEvent(event);

    // Специальная обработка для игрока может быть добавлена здесь
    // Например, реакция на клавиши взаимодействия, использования предметов и т.д.
}

void Player::detectKeyInput()
{
    // Получаем текущее состояние клавиатуры
    const Uint8* keyState = SDL_GetKeyboardState(NULL);

    // Определяем нажатие клавиш направления
    bool upPressed = keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP];
    bool downPressed = keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN];
    bool leftPressed = keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT];
    bool rightPressed = keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT];

    // Сбрасываем направление перед установкой нового
    m_dX = 0.0f;
    m_dY = 0.0f;

    // Определение направления (изометрическая система координат)
    if (upPressed && !downPressed) {
        m_dY = -1.0f; // Север в изометрии
    }
    else if (!upPressed && downPressed) {
        m_dY = 1.0f;  // Юг в изометрии
    }

    if (rightPressed && !leftPressed) {
        m_dX = 1.0f;  // Восток в изометрии
    }
    else if (!rightPressed && leftPressed) {
        m_dX = -1.0f; // Запад в изометрии
    }

    // Нормализация для диагонального движения
    if (m_dX != 0.0f && m_dY != 0.0f) {
        float length = std::sqrt(m_dX * m_dX + m_dY * m_dY);
        m_dX /= length;
        m_dY /= length;
    }

    // Обновляем направление персонажа на основе векторов движения
    updateDirection();
}

void Player::update(float deltaTime)
{
    // Если нет движения, то выходим
    if (m_dX == 0.0f && m_dY == 0.0f) {
        return;
    }

    // Получаем текущую позицию персонажа в тайлах
    int currentTileX = static_cast<int>(m_position.x);
    int currentTileY = static_cast<int>(m_position.y);

    // Нормализация скорости для разных частот кадров
    float normalizedSpeed = m_moveSpeed * deltaTime * 60.0f;

    // Рассчитываем дельту передвижения
    float deltaX = m_dX * normalizedSpeed;
    float deltaY = m_dY * normalizedSpeed;

    // Используем систему коллизий для обработки перемещения
    if (m_collisionSystem) {
        // Используем улучшенную систему с учётом скольжения вдоль стен
        CollisionResult collisionResult = m_collisionSystem->handleCollisionWithSliding(
            currentTileX, currentTileY,
            m_subX, m_subY,
            deltaX, deltaY,
            m_collisionSize
        );

        // Применяем результаты обработки коллизии
        m_subX = collisionResult.adjustedX;
        m_subY = collisionResult.adjustedY;

        // ВАЖНО: Только здесь обрабатываем переход на новый тайл
        NormalizeSubCoordinates();
    }
    else {
        // Запасной вариант, если система коллизий недоступна
        // Рассчитываем следующую позицию
        float nextSubX = m_subX + deltaX;
        float nextSubY = m_subY + deltaY;

        // Определяем, выходит ли персонаж за пределы текущего тайла
        bool crossingTileBoundaryX = nextSubX >= 1.0f || nextSubX < 0.0f;
        bool crossingTileBoundaryY = nextSubY >= 1.0f || nextSubY < 0.0f;

        // Координаты следующего тайла
        int nextTileX = currentTileX + (nextSubX >= 1.0f ? 1 : (nextSubX < 0.0f ? -1 : 0));
        int nextTileY = currentTileY + (nextSubY >= 1.0f ? 1 : (nextSubY < 0.0f ? -1 : 0));

        // Проверка возможности движения по X
        bool canMoveX = true;
        if (crossingTileBoundaryX) {
            canMoveX = m_tileMap->isValidCoordinate(nextTileX, currentTileY) &&
                m_tileMap->isTileWalkable(nextTileX, currentTileY);
        }

        // Проверка возможности движения по Y
        bool canMoveY = true;
        if (crossingTileBoundaryY) {
            canMoveY = m_tileMap->isValidCoordinate(currentTileX, nextTileY) &&
                m_tileMap->isTileWalkable(currentTileX, nextTileY);
        }

        // Проверка диагонального движения
        bool diagonalMove = crossingTileBoundaryX && crossingTileBoundaryY;
        bool canMoveDiag = true;

        if (diagonalMove) {
            canMoveDiag = canMoveDiagonally(currentTileX, currentTileY, nextTileX, nextTileY);
        }

        // Применяем движение с учетом коллизий
        if (canMoveX && (!diagonalMove || canMoveDiag)) {
            m_subX = nextSubX;
        }
        else if (crossingTileBoundaryX) {
            // Останавливаемся у границы тайла
            m_subX = nextSubX >= 1.0f ? 0.99f : 0.01f;
        }
        else {
            m_subX = nextSubX; // Двигаемся в пределах текущего тайла
        }

        if (canMoveY && (!diagonalMove || canMoveDiag)) {
            m_subY = nextSubY;
        }
        else if (crossingTileBoundaryY) {
            // Останавливаемся у границы тайла
            m_subY = nextSubY >= 1.0f ? 0.99f : 0.01f;
        }
        else {
            m_subY = nextSubY; // Двигаемся в пределах текущего тайла
        }

        // ВАЖНО: Обрабатываем переход на новый тайл
        NormalizeSubCoordinates();
    }
}

void Player::render(SDL_Renderer* renderer)
{
    // Отрисовка будет осуществляться через TileRenderer в MapScene
    // Этот метод может быть использован для дополнительных элементов
    // связанных с игроком, но не с отрисовкой его основного тела
}

bool Player::canMoveDiagonally(int fromX, int fromY, int toX, int toY)
{
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

void Player::updateDirection()
{
    // Определяем направление на основе вектора движения
    if (m_dX == 0.0f && m_dY == 0.0f) {
        // Направление не меняется, если нет движения
        return;
    }

    // Используем угол вектора движения для определения направления
    float angle = atan2(m_dY, m_dX) * 180.0f / 3.14159f;

    // Нормализуем угол в диапазон [0, 360)
    if (angle < 0.0f) {
        angle += 360.0f;
    }

    // Определяем направление на основе угла
    if (angle >= 337.5f || angle < 22.5f) {
        m_currentDirection = Direction::EAST;
    }
    else if (angle >= 22.5f && angle < 67.5f) {
        m_currentDirection = Direction::SOUTHEAST;
    }
    else if (angle >= 67.5f && angle < 112.5f) {
        m_currentDirection = Direction::SOUTH;
    }
    else if (angle >= 112.5f && angle < 157.5f) {
        m_currentDirection = Direction::SOUTHWEST;
    }
    else if (angle >= 157.5f && angle < 202.5f) {
        m_currentDirection = Direction::WEST;
    }
    else if (angle >= 202.5f && angle < 247.5f) {
        m_currentDirection = Direction::NORTHWEST;
    }
    else if (angle >= 247.5f && angle < 292.5f) {
        m_currentDirection = Direction::NORTH;
    }
    else if (angle >= 292.5f && angle < 337.5f) {
        m_currentDirection = Direction::NORTHEAST;
    }
}

void Player::updateFaceColors()
{
    // Создаем оттенки для граней на основе основного цвета
    m_leftFaceColor = {
        static_cast<Uint8>(m_color.r * 0.7f),
        static_cast<Uint8>(m_color.g * 0.7f),
        static_cast<Uint8>(m_color.b * 0.7f),
        m_color.a
    };

    m_rightFaceColor = {
        static_cast<Uint8>(m_color.r * 0.5f),
        static_cast<Uint8>(m_color.g * 0.5f),
        static_cast<Uint8>(m_color.b * 0.5f),
        m_color.a
    };
}

void Player::setCollisionSystem(CollisionSystem* collisionSystem) {
    m_collisionSystem = collisionSystem;
}

void Player::NormalizeSubCoordinates() {
    // Гарантирует, что субкоординаты всегда в пределах [0, 1)
    while (m_subX >= 1.0f) {
        m_position.x += 1.0f;
        m_subX -= 1.0f;
    }
    while (m_subX < 0.0f) {
        m_position.x -= 1.0f;
        m_subX += 1.0f;
    }

    while (m_subY >= 1.0f) {
        m_position.y += 1.0f;
        m_subY -= 1.0f;
    }
    while (m_subY < 0.0f) {
        m_position.y -= 1.0f;
        m_subY += 1.0f;
    }
}

void Player::renderDirectionIndicator(SDL_Renderer* renderer, IsometricRenderer* isoRenderer, int centerX, int centerY) const {
    // Получаем координаты игрока
    float playerX = getFullX();
    float playerY = getFullY();
    float playerZ = getHeight();

    // Вычисляем направляющий вектор в зависимости от текущего направления
    float dirX = 0.0f, dirY = 0.0f;
    const float arrowLength = 0.4f; // Длина стрелки

    // Используем switch с enum class
    switch (m_currentDirection) {
    case Direction::NORTH:
        dirX = 0.0f;
        dirY = -1.0f;
        break;
    case Direction::NORTHEAST:
        dirX = 0.7071f;  // cos(45°)
        dirY = -0.7071f; // sin(45°)
        break;
    case Direction::EAST:
        dirX = 1.0f;
        dirY = 0.0f;
        break;
    case Direction::SOUTHEAST:
        dirX = 0.7071f;
        dirY = 0.7071f;
        break;
    case Direction::SOUTH:
        dirX = 0.0f;
        dirY = 1.0f;
        break;
    case Direction::SOUTHWEST:
        dirX = -0.7071f;
        dirY = 0.7071f;
        break;
    case Direction::WEST:
        dirX = -1.0f;
        dirY = 0.0f;
        break;
    case Direction::NORTHWEST:
        dirX = -0.7071f;
        dirY = -0.7071f;
        break;
    default:
        // Для безопасности добавляем default
        dirX = 0.0f;
        dirY = 0.0f;
        break;
    }

    // Вычисляем конечную точку стрелки
    float endX = playerX + dirX * arrowLength;
    float endY = playerY + dirY * arrowLength;

    // Преобразуем мировые координаты в экранные
    int screenStartX, screenStartY;
    int screenEndX, screenEndY;

    // Точка начала (центр игрока)
    isoRenderer->worldToDisplay(
        playerX, playerY, playerZ + 0.01f, // Слегка выше игрока
        centerX, centerY, screenStartX, screenStartY
    );

    // Конечная точка стрелки
    isoRenderer->worldToDisplay(
        endX, endY, playerZ + 0.01f,
        centerX, centerY, screenEndX, screenEndY
    );

    // Устанавливаем цвет линии
    SDL_SetRenderDrawColor(renderer,
        m_directionIndicatorColor.r,
        m_directionIndicatorColor.g,
        m_directionIndicatorColor.b,
        m_directionIndicatorColor.a);

    // Рисуем основную линию стрелки
    SDL_RenderDrawLine(renderer, screenStartX, screenStartY, screenEndX, screenEndY);

    // Рисуем наконечник стрелки
    // Угол наконечника от основной линии
    const float arrowAngle = 20.0f * 3.14159f / 180.0f; // 20 градусов в радианах
    const float arrowSize = 10.0f; // Размер наконечника стрелки в пикселях

    // Вычисляем вектор основной линии
    float dx = static_cast<float>(screenEndX - screenStartX);
    float dy = static_cast<float>(screenEndY - screenStartY);

    // Нормализуем вектор
    float length = sqrt(dx * dx + dy * dy);
    if (length > 0) {
        dx /= length;
        dy /= length;
    }

    // Вычисляем векторы для двух линий наконечника
    float arrowX1 = screenEndX - (dx * cos(arrowAngle) - dy * sin(arrowAngle)) * arrowSize;
    float arrowY1 = screenEndY - (dx * sin(arrowAngle) + dy * cos(arrowAngle)) * arrowSize;

    float arrowX2 = screenEndX - (dx * cos(-arrowAngle) - dy * sin(-arrowAngle)) * arrowSize;
    float arrowY2 = screenEndY - (dx * sin(-arrowAngle) + dy * cos(-arrowAngle)) * arrowSize;

    // Рисуем две линии наконечника
    SDL_RenderDrawLine(renderer, screenEndX, screenEndY, static_cast<int>(arrowX1), static_cast<int>(arrowY1));
    SDL_RenderDrawLine(renderer, screenEndX, screenEndY, static_cast<int>(arrowX2), static_cast<int>(arrowY2));
}