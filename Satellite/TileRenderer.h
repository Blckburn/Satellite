#pragma once

#include "IsometricRenderer.h"
#include <SDL.h>
#include <vector>
#include <map>
#include <tuple>

/**
 * @brief Класс для рендеринга множества тайлов с оптимизацией
 */
class TileRenderer {
private:
    // Типы тайлов для рендеринга
    enum class TileRenderType {
        FLAT,        ///< Плоский тайл (пол, трава и т.д.)
        VOLUMETRIC   ///< Объемный тайл (стены, объекты)
    };

    // Структура для хранения данных тайла
    struct TileRenderData {
        TileRenderType type;     ///< Тип тайла
        float x;                 ///< Координата X
        float y;                 ///< Координата Y
        float height;            ///< Высота (для объемных тайлов)
        SDL_Texture* texture;    ///< Текстура (верхняя для объемных тайлов)
        SDL_Texture* leftTexture;///< Текстура левой грани (для объемных тайлов)
        SDL_Texture* rightTexture;///< Текстура правой грани (для объемных тайлов)
        SDL_Color topColor;      ///< Цвет верхней грани
        SDL_Color leftColor;     ///< Цвет левой грани (для объемных тайлов)
        SDL_Color rightColor;    ///< Цвет правой грани (для объемных тайлов)
        float priority;          ///< Приоритет отрисовки (ниже = дальше)
    };

    // Структура ключа для батчей
    struct BatchKey {
        TileRenderType type;
        SDL_Texture* topTexture;
        SDL_Texture* leftTexture;
        SDL_Texture* rightTexture;

        // Оператор сравнения для использования в std::map
        bool operator<(const BatchKey& other) const {
            return std::tie(type, topTexture, leftTexture, rightTexture) <
                std::tie(other.type, other.topTexture, other.leftTexture, other.rightTexture);
        }
    };

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
     * @brief Добавление плоского тайла
     * @param x Координата X
     * @param y Координата Y
     * @param texture Текстура тайла (nullptr для цветного тайла)
     * @param color Цвет тайла (используется, если texture == nullptr или как оттенок)
     * @param priority Приоритет отрисовки (ниже = дальше)
     */
    void addFlatTile(float x, float y, SDL_Texture* texture, const SDL_Color& color, float priority = 0.0f);

    /**
     * @brief Добавление объемного тайла
     * @param x Координата X
     * @param y Координата Y
     * @param height Высота тайла
     * @param topTexture Текстура верхней грани (nullptr для цветной грани)
     * @param leftTexture Текстура левой грани (nullptr для цветной грани)
     * @param rightTexture Текстура правой грани (nullptr для цветной грани)
     * @param topColor Цвет верхней грани
     * @param leftColor Цвет левой грани
     * @param rightColor Цвет правой грани
     * @param priority Приоритет отрисовки (ниже = дальше)
     */
    void addVolumetricTile(float x, float y, float height,
        SDL_Texture* topTexture, SDL_Texture* leftTexture, SDL_Texture* rightTexture,
        const SDL_Color& topColor, const SDL_Color& leftColor, const SDL_Color& rightColor,
        float priority = 0.0f);

    /**
     * @brief Очистка всех данных о тайлах
     */
    void clear();

    /**
     * @brief Рендеринг всех добавленных тайлов
     * @param renderer Указатель на SDL_Renderer
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void render(SDL_Renderer* renderer, int centerX, int centerY);

    /**
     * @brief Включение/выключение сортировки по глубине
     * @param enable true для включения, false для отключения
     */
    void setZSorting(bool enable);

    /**
     * @brief Включение/выключение пакетного рендеринга
     * @param enable true для включения, false для отключения
     */
    void setBatchRendering(bool enable);

private:
    /**
     * @brief Сортировка тайлов для корректного отображения в изометрии
     */
    void sortTiles();

    /**
     * @brief Подготовка батчей для оптимизированного рендеринга
     */
    void prepareBatches();

private:
    IsometricRenderer* m_isoRenderer;                  ///< Указатель на изометрический рендерер
    std::vector<TileRenderData*> m_tiles;              ///< Список тайлов для отрисовки
    bool m_useZSorting;                                ///< Флаг использования сортировки по глубине
    bool m_useBatchRendering;                          ///< Флаг использования пакетного рендеринга
    std::map<BatchKey, std::vector<TileRenderData*>> m_tileBatchMap; ///< Карта батчей
};