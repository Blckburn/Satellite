#pragma once

#include <SDL.h>

/**
 * @brief Класс камеры для управления видом
 */
class Camera {
public:
    /**
     * @brief Конструктор
     * @param screenWidth Ширина экрана
     * @param screenHeight Высота экрана
     */
    Camera(int screenWidth, int screenHeight);

    /**
     * @brief Деструктор
     */
    ~Camera();

    /**
     * @brief Обновление камеры
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void update(float deltaTime);

    /**
     * @brief Обработка событий камеры (перемещение, масштабирование)
     * @param event SDL событие
     */
    void handleEvent(const SDL_Event& event);

    /**
     * @brief Установка позиции камеры
     * @param x X координата в мировом пространстве
     * @param y Y координата в мировом пространстве
     */
    void setPosition(float x, float y);

    /**
     * @brief Получение X координаты камеры
     * @return X координата в мировом пространстве
     */
    float getX() const { return m_x; }

    /**
     * @brief Получение Y координаты камеры
     * @return Y координата в мировом пространстве
     */
    float getY() const { return m_y; }

    /**
     * @brief Установка масштаба камеры
     * @param scale Масштаб (1.0 - нормальный размер)
     */
    void setZoom(float scale);

    /**
     * @brief Получение текущего масштаба камеры
     * @return Текущий масштаб
     */
    float getZoom() const { return m_zoom; }

    /**
     * @brief Установка целевого объекта для слежения
     * @param targetX Указатель на X координату цели
     * @param targetY Указатель на Y координату цели
     */
    void setTarget(const float* targetX, const float* targetY);

    /**
     * @brief Перемещение камеры
     * @param dx Изменение по X
     * @param dy Изменение по Y
     */
    void move(float dx, float dy);

    /**
     * @brief Масштабирование камеры
     * @param amount Величина изменения масштаба
     */
    void zoom(float amount);

    /**
     * @brief Получение ширины экрана
     * @return Ширина экрана
     */
    int getScreenWidth() const { return m_screenWidth; }

    /**
     * @brief Получение высоты экрана
     * @return Высота экрана
     */
    int getScreenHeight() const { return m_screenHeight; }

    /**
     * @brief Установка размеров экрана
     * @param width Ширина экрана
     * @param height Высота экрана
     */
    void setScreenSize(int width, int height);

    /**
     * @brief Установка скорости перемещения камеры
     * @param speed Скорость перемещения
     */
    void setMoveSpeed(float speed) { m_moveSpeed = speed; }

    /**
     * @brief Установка скорости масштабирования
     * @param speed Скорость масштабирования
     */
    void setZoomSpeed(float speed) { m_zoomSpeed = speed; }

private:
    float m_x;                  ///< X координата камеры в мировом пространстве
    float m_y;                  ///< Y координата камеры в мировом пространстве
    float m_zoom;               ///< Масштаб камеры

    int m_screenWidth;          ///< Ширина экрана
    int m_screenHeight;         ///< Высота экрана

    float m_moveSpeed;          ///< Скорость перемещения
    float m_zoomSpeed;          ///< Скорость масштабирования

    const float* m_targetX;     ///< Указатель на X координату целевого объекта
    const float* m_targetY;     ///< Указатель на Y координату целевого объекта

    bool m_isDragging;          ///< Флаг перемещения камеры мышью
    int m_dragStartX;           ///< Начальная X координата при перемещении мышью
    int m_dragStartY;           ///< Начальная Y координата при перемещении мышью
    float m_dragStartCamX;      ///< Начальная X координата камеры при перемещении мышью
    float m_dragStartCamY;      ///< Начальная Y координата камеры при перемещении мышью
};