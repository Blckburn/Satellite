#include "Player.h"
#include <cmath>
#include <iostream>

Player::Player(const std::string& name, std::shared_ptr<TileMap> tileMap)
    : Entity(name), m_tileMap(tileMap), m_subX(0.5f), m_subY(0.5f),
    m_moveSpeed(0.05f), m_dX(0.0f), m_dY(0.0f), m_collisionSize(0.35f) {

    // Устанавливаем цвет игрока
    m_color = { 255, 50, 50, 255 }; // Ярко-красный
}

Player::~Player() {
}

bool Player::initialize() {
    // Базовая инициализация, в будущем здесь может быть загрузка спрайтов и т.д.
    std::cout << "Player initialized: " << m_name << std::endl;
    return true;
}

void Player::handleEvent(const SDL_Event& event) {
    // Обработка событий, связанных с игроком
    // Основное управление осуществляется через keyboard state в методе detectKeyInput
}

void Player::update(float deltaTime) {
    // 1. Обнаружение нажатий клавиш
    detectKeyInput();

    // 2. Получаем текущую позицию персонажа
    int currentTileX = static_cast<int>(m_position.x);
    int currentTileY = static_cast<int>(m_position.y);

    // 3. Обработка перемещения при наличии направления
    if (m_dX != 0.0f || m_dY != 0.0f) {
        // Нормализация скорости для разных частот кадров
        float normalizedSpeed = m_moveSpeed * deltaTime * 60.0f;

        // Рассчитываем следующую позицию
        float nextSubX = m_subX + m_dX * normalizedSpeed;
        float nextSubY = m_subY + m_dY * normalizedSpeed;

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
            canMoveDiag = m_tileMap->isValidCoordinate(nextTileX, nextTileY) &&
                m_tileMap->isTileWalkable(nextTileX, nextTileY) &&
                canMoveX && canMoveY;
        }

        // Применяем движение с учетом коллизий
        if (canMoveX && (!diagonalMove || canMoveDiag)) {
            m_subX = nextSubX;

            // Если перешли в новый тайл, обновляем координаты
            if (nextSubX >= 1.0f) {
                m_position.x += 1.0f;
                m_subX -= 1.0f;
            }
            else if (nextSubX < 0.0f) {
                m_position.x -= 1.0f;
                m_subX += 1.0f;
            }
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

            // Если перешли в новый тайл, обновляем координаты
            if (nextSubY >= 1.0f) {
                m_position.y += 1.0f;
                m_subY -= 1.0f;
            }
            else if (nextSubY < 0.0f) {
                m_position.y -= 1.0f;
                m_subY += 1.0f;
            }
        }
        else if (crossingTileBoundaryY) {
            // Останавливаемся у границы тайла
            m_subY = nextSubY >= 1.0f ? 0.99f : 0.01f;
        }
        else {
            m_subY = nextSubY; // Двигаемся в пределах текущего тайла
        }

        // Гарантируем, что субкоординаты остаются в диапазоне [0, 1)
        if (m_subX >= 1.0f) {
            m_position.x += 1.0f;
            m_subX -= 1.0f;
        }
        else if (m_subX < 0.0f) {
            m_position.x -= 1.0f;
            m_subX += 1.0f;
        }

        if (m_subY >= 1.0f) {
            m_position.y += 1.0f;
            m_subY -= 1.0f;
        }
        else if (m_subY < 0.0f) {
            m_position.y -= 1.0f;
            m_subY += 1.0f;
        }
    }

    // Обновляем Z-координату для корректного отображения
    m_position.z = 0.5f; // Высота персонажа
}

void Player::render(SDL_Renderer* renderer) {
    // В базовой версии не делаем ничего - отрисовка будет осуществляться в MapScene
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

void Player::detectKeyInput() {
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
}