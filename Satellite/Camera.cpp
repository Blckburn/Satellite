#include "Camera.h"
#include <algorithm>
#include <iostream>

Camera::Camera(int screenWidth, int screenHeight)
    : m_x(0.0f), m_y(0.0f), m_zoom(1.0f),
    m_screenWidth(screenWidth), m_screenHeight(screenHeight),
    m_moveSpeed(5.0f), m_zoomSpeed(0.1f),
    m_targetX(nullptr), m_targetY(nullptr),
    m_isDragging(false), m_dragStartX(0), m_dragStartY(0),
    m_dragStartCamX(0.0f), m_dragStartCamY(0.0f) {
}

Camera::~Camera() {
}

void Camera::update(float deltaTime) {
    // Если есть целевой объект для слежения
    if (m_targetX && m_targetY) {
        // Проверка на NaN
        float targetX = *m_targetX;
        float targetY = *m_targetY;

        if (std::isnan(targetX) || std::isnan(targetY)) {
            return;
        }

        // ИЗМЕНЕННЫЙ КОД: Используем более плавную интерполяцию
        // Ранее тут было значение 1.0f * deltaTime, что может давать резкие движения

        // 1. Плавная интерполяция для стабильности камеры
        // Используем константную скорость интерполяции вместо множителя deltaTime
        float interp = std::min(5.0f * deltaTime, 1.0f);

        // 2. Применяем интерполяцию к позиции
        m_x += (targetX - m_x) * interp;
        m_y += (targetY - m_y) * interp;
    }
}

void Camera::handleEvent(const SDL_Event& event) {
    // Обработка колеса мыши для масштабирования
    if (event.type == SDL_MOUSEWHEEL) {
        float zoomDelta = m_zoomSpeed;
        // Уменьшаем шаг масштабирования для более плавного изменения
        if (event.wheel.y > 0) {
            zoom(zoomDelta);
        }
        else if (event.wheel.y < 0) {
            zoom(-zoomDelta);
        }
    }

    // Перемещение камеры с помощью перетаскивания мышью (средняя кнопка)
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_MIDDLE) {
        m_isDragging = true;
        m_dragStartX = event.button.x;
        m_dragStartY = event.button.y;
        m_dragStartCamX = m_x;
        m_dragStartCamY = m_y;
    }
    else if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_MIDDLE) {
        m_isDragging = false;
    }
    else if (event.type == SDL_MOUSEMOTION && m_isDragging) {
        // Вычисляем смещение мыши
        int dx = event.motion.x - m_dragStartX;
        int dy = event.motion.y - m_dragStartY;

        // Конвертируем смещение мыши в мировые координаты
        // Для изометрического вида нужно учесть преобразование
        float worldDx = (dx - dy) / (m_zoom * 32.0f); // 32 - примерно половина ширины тайла
        float worldDy = (dx + dy) / (m_zoom * 32.0f); // 32 - примерно половина высоты тайла

        // Устанавливаем новую позицию камеры
        m_x = m_dragStartCamX - worldDx;
        m_y = m_dragStartCamY - worldDy;
    }
}

void Camera::setPosition(float x, float y) {
    m_x = x;
    m_y = y;
}

void Camera::setZoom(float scale) {
    // Ограничиваем масштаб
    if (scale < 0.1f) scale = 0.1f;
    if (scale > 5.0f) scale = 5.0f;

    m_zoom = scale;
}

void Camera::setTarget(const float* targetX, const float* targetY) {
    m_targetX = targetX;
    m_targetY = targetY;
}

void Camera::move(float dx, float dy) {
    m_x += dx;
    m_y += dy;
}

void Camera::zoom(float amount) {
    setZoom(m_zoom + amount);
}

void Camera::setScreenSize(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;
}