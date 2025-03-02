#pragma once

#include <SDL.h>
#include <vector>

/**
 * @brief Класс для изометрического рендеринга
 */
class IsometricRenderer {
public:
    // Константа для масштабирования высоты
    static constexpr float HEIGHT_SCALE = 30.0f;  // Увеличено с 20.0f для более выраженного 3D-эффекта

    /**
     * @brief Конструктор
     * @param tileWidth Ширина изометрического тайла
     * @param tileHeight Высота изометрического тайла
     */
    IsometricRenderer(int tileWidth, int tileHeight);

    /**
     * @brief Деструктор
     */
    ~IsometricRenderer();

    /**
     * @brief Преобразование из мировых координат в экранные
     * @param worldX X координата в мировом пространстве
     * @param worldY Y координата в мировом пространстве
     * @param screenX X координата на экране (выходной параметр)
     * @param screenY Y координата на экране (выходной параметр)
     */
    void worldToScreen(float worldX, float worldY, int& screenX, int& screenY) const;

    /**
     * @brief Преобразование из экранных координат в мировые
     * @param screenX X координата на экране
     * @param screenY Y координата на экране
     * @param worldX X координата в мировом пространстве (выходной параметр)
     * @param worldY Y координата в мировом пространстве (выходной параметр)
     */
    void screenToWorld(int screenX, int screenY, float& worldX, float& worldY) const;

    /**
     * @brief Преобразование из мировых координат в экранные с учетом высоты и центра экрана
     * @param worldX X координата в мировом пространстве
     * @param worldY Y координата в мировом пространстве
     * @param worldZ Высота (Z координата) объекта
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     * @param displayX X координата на экране (выходной параметр)
     * @param displayY Y координата на экране (выходной параметр)
     */
    void worldToDisplay(float worldX, float worldY, float worldZ,
        int centerX, int centerY,
        int& displayX, int& displayY) const;

    /**
     * @brief Получение высоты объекта в пикселях
     * @param worldHeight Высота объекта в мировых единицах
     * @return Высота в пикселях
     */
    int getHeightInPixels(float worldHeight) const;

    /**
     * @brief Получение размера с учетом масштаба камеры
     * @param size Исходный размер
     * @return Размер с учетом масштаба
     */
    int getScaledSize(int size) const;

    /**
     * @brief Отрисовка изометрического тайла
     * @param renderer SDL рендерер
     * @param worldX X координата в мировом пространстве
     * @param worldY Y координата в мировом пространстве
     * @param height Высота тайла (для объемных тайлов)
     * @param color Цвет тайла
     * @param centerX X координата центра экрана (по умолчанию 0)
     * @param centerY Y координата центра экрана (по умолчанию 0)
     */
    void renderTile(SDL_Renderer* renderer, float worldX, float worldY, float height, SDL_Color color,
        int centerX = 0, int centerY = 0) const;

    /**
     * @brief Отрисовка изометрического тайла с текстурой
     * @param renderer SDL рендерер
     * @param texture Текстура для отрисовки
     * @param worldX X координата в мировом пространстве
     * @param worldY Y координата в мировом пространстве
     * @param height Высота тайла (для объемных тайлов)
     * @param centerX X координата центра экрана (по умолчанию 0)
     * @param centerY Y координата центра экрана (по умолчанию 0)
     */
    void renderTileWithTexture(SDL_Renderer* renderer, SDL_Texture* texture,
        float worldX, float worldY, float height,
        int centerX = 0, int centerY = 0) const;

    /**
     * @brief Отрисовка изометрического объемного тайла
     * @param renderer SDL рендерер
     * @param worldX X координата в мировом пространстве
     * @param worldY Y координата в мировом пространстве
     * @param height Высота тайла (для объемных тайлов)
     * @param topColor Цвет верхней грани
     * @param leftColor Цвет левой грани
     * @param rightColor Цвет правой грани
     * @param centerX X координата центра экрана (по умолчанию 0)
     * @param centerY Y координата центра экрана (по умолчанию 0)
     */
    void renderVolumetricTile(SDL_Renderer* renderer, float worldX, float worldY, float height,
        SDL_Color topColor, SDL_Color leftColor, SDL_Color rightColor,
        int centerX = 0, int centerY = 0) const;

    /**
     * @brief Отрисовка изометрического объемного тайла с текстурами
     * @param renderer SDL рендерер
     * @param topTexture Текстура для верхней грани
     * @param leftTexture Текстура для левой грани
     * @param rightTexture Текстура для правой грани
     * @param worldX X координата в мировом пространстве
     * @param worldY Y координата в мировом пространстве
     * @param height Высота тайла (для объемных тайлов)
     * @param centerX X координата центра экрана (по умолчанию 0)
     * @param centerY Y координата центра экрана (по умолчанию 0)
     */
    void renderVolumetricTileWithTextures(SDL_Renderer* renderer,
        SDL_Texture* topTexture,
        SDL_Texture* leftTexture,
        SDL_Texture* rightTexture,
        float worldX, float worldY, float height,
        int centerX = 0, int centerY = 0) const;

    /**
     * @brief Отрисовка объемного тайла с улучшенной поддержкой текстур
     * @param renderer SDL рендерер
     * @param worldX X координата в мировом пространстве
     * @param worldY Y координата в мировом пространстве
     * @param height Высота тайла
     * @param topTexture Текстура для верхней грани (или nullptr)
     * @param leftTexture Текстура для левой грани (или nullptr)
     * @param rightTexture Текстура для правой грани (или nullptr)
     * @param centerX X координата центра экрана (по умолчанию 0)
     * @param centerY Y координата центра экрана (по умолчанию 0)
     */
    void renderEnhancedVolumetricTile(SDL_Renderer* renderer,
        float worldX, float worldY, float height,
        SDL_Texture* topTexture,
        SDL_Texture* leftTexture,
        SDL_Texture* rightTexture,
        int centerX = 0, int centerY = 0) const;

    /**
     * @brief Отрисовка изометрической сетки
     * @param renderer SDL рендерер
     * @param centerX X координата центра сетки на экране
     * @param centerY Y координата центра сетки на экране
     * @param gridSize Размер сетки (количество тайлов от центра)
     * @param color Цвет линий сетки
     */
    void renderGrid(SDL_Renderer* renderer, int centerX, int centerY, int gridSize, SDL_Color color) const;

    /**
     * @brief Отрисовка точки в мировых координатах для отладки
     * @param renderer SDL рендерер
     * @param worldX X координата в мировом пространстве
     * @param worldY Y координата в мировом пространстве
     * @param worldZ Z координата (высота) в мировом пространстве
     * @param color Цвет точки
     * @param centerX X координата центра экрана (по умолчанию 0)
     * @param centerY Y координата центра экрана (по умолчанию 0)
     */
    void renderDebugPoint(SDL_Renderer* renderer, float worldX, float worldY, float worldZ,
        SDL_Color color, int centerX, int centerY) const;

    /**
     * @brief Получение ширины тайла
     * @return Ширина тайла в пикселях
     */
    int getTileWidth() const { return m_tileWidth; }

    /**
     * @brief Получение высоты тайла
     * @return Высота тайла в пикселях
     */
    int getTileHeight() const { return m_tileHeight; }

    /**
     * @brief Установка позиции камеры
     * @param x X координата камеры в мировом пространстве
     * @param y Y координата камеры в мировом пространстве
     */
    void setCameraPosition(float x, float y);

    /**
     * @brief Получение X координаты камеры
     * @return X координата камеры в мировом пространстве
     */
    float getCameraX() const { return m_cameraX; }

    /**
     * @brief Получение Y координаты камеры
     * @return Y координата камеры в мировом пространстве
     */
    float getCameraY() const { return m_cameraY; }

    /**
     * @brief Установка масштаба камеры
     * @param scale Масштаб (1.0 - нормальный размер)
     */
    void setCameraZoom(float scale);

    /**
     * @brief Получение текущего масштаба камеры
     * @return Текущий масштаб камеры
     */
    float getCameraZoom() const { return m_cameraZoom; }

    void renderTexturedDiamond(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Point* points) const;

    /**
 * @brief Отрисовка плоского тайла (обертка для renderTile)
 * @param renderer SDL рендерер
 * @param x X координата в мировом пространстве
 * @param y Y координата в мировом пространстве
 * @param texture Текстура для отрисовки (может быть nullptr)
 * @param color Цвет для отрисовки, если текстура отсутствует
 * @param centerX X координата центра экрана (по умолчанию 0)
 * @param centerY Y координата центра экрана (по умолчанию 0)
 */
    void renderFlatTile(SDL_Renderer* renderer, float x, float y,
        SDL_Texture* texture, SDL_Color color,
        int centerX = 0, int centerY = 0);

private:
    /**
     * @brief Отрисовка заполненного полигона
     * @param renderer SDL рендерер
     * @param points Массив точек полигона
     * @param count Количество точек
     */
    void fillPolygon(SDL_Renderer* renderer, const SDL_Point* points, int count) const;

    /**
     * @brief Отрисовка текстурированного полигона
     * @param renderer SDL рендерер
     * @param texture Текстура для отрисовки
     * @param points Массив точек полигона на экране
     * @param texCoords Массив текстурных координат (от 0.0 до 1.0)
     * @param count Количество точек
     */
    void renderTexturedPolygon(SDL_Renderer* renderer, SDL_Texture* texture,
        const SDL_Point* points, const SDL_Point* texCoords,
        int count) const;

private:
    int m_tileWidth;    ///< Ширина изометрического тайла
    int m_tileHeight;   ///< Высота изометрического тайла

    float m_cameraX;    ///< X координата камеры в мировом пространстве
    float m_cameraY;    ///< Y координата камеры в мировом пространстве
    float m_cameraZoom; ///< Масштаб камеры (1.0 - нормальный размер)
};