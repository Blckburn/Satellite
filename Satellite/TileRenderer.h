#pragma once

#include "RenderableTile.h"
#include "IsometricRenderer.h"
#include "ResourceManager.h"
#include <vector>
#include <algorithm>

/**
 * @brief Класс для управления рендерингом тайлов
 */
class TileRenderer {
public:
    /**
     * @brief Конструктор
     * @param isoRenderer Указатель на изометрический рендерер
     */
    TileRenderer(IsometricRenderer* isoRenderer);

    /**
     * @brief Деструктор
     */
    ~TileRenderer();

    /**
     * @brief Очистка всех тайлов
     */
    void clear();

    /**
     * @brief Добавление плоского тайла
     * @param x X координата в мировом пространстве
     * @param y Y координата в мировом пространстве
     * @param texture Текстура для отрисовки (может быть nullptr)
     * @param color Цвет для отрисовки, если текстура отсутствует
     * @param priority Приоритет отрисовки (выше значение = отображается поверх)
     */
    void addFlatTile(float x, float y, SDL_Texture* texture, SDL_Color color, int priority = 0);

    /**
     * @brief Добавление объемного тайла
     * @param x X координата в мировом пространстве
     * @param y Y координата в мировом пространстве
     * @param z Z координата (высота)
     * @param topTexture Текстура для верхней грани (может быть nullptr)
     * @param leftTexture Текстура для левой грани (может быть nullptr)
     * @param rightTexture Текстура для правой грани (может быть nullptr)
     * @param topColor Цвет верхней грани
     * @param leftColor Цвет левой грани
     * @param rightColor Цвет правой грани
     * @param priority Приоритет отрисовки (выше значение = отображается поверх)
     */
    void addVolumetricTile(float x, float y, float z,
        SDL_Texture* topTexture,
        SDL_Texture* leftTexture,
        SDL_Texture* rightTexture,
        SDL_Color topColor,
        SDL_Color leftColor,
        SDL_Color rightColor,
        int priority = 0);

    /**
     * @brief Отрисовка всех тайлов
     * @param renderer SDL рендерер
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void render(SDL_Renderer* renderer, int centerX, int centerY);

private:
    std::vector<RenderableTile> m_tiles;  ///< Вектор тайлов для отрисовки
    IsometricRenderer* m_isoRenderer;     ///< Указатель на изометрический рендерер
};