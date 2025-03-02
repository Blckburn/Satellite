#pragma once

#include <SDL.h>

/**
 * @brief Класс для хранения информации о тайле для рендеринга
 */
class RenderableTile {
public:
    // Координаты в мировом пространстве
    float worldX = 0.0f;
    float worldY = 0.0f;
    float worldZ = 0.0f;   // Высота

    // Тип тайла
    enum class TileType {
        FLAT,       // Плоский тайл
        VOLUMETRIC  // Объемный тайл
    } type = TileType::FLAT;

    // Приоритет рендеринга (более высокое значение означает, что объект будет отрисован поверх других)
    float renderPriority = 0.0f;  // Изменено с int на float для более точной сортировки

    // Текстуры (могут быть nullptr)
    SDL_Texture* topTexture = nullptr;    // Верхняя грань
    SDL_Texture* leftTexture = nullptr;   // Левая грань (для объемных)
    SDL_Texture* rightTexture = nullptr;  // Правая грань (для объемных)

    // Цвета для отрисовки без текстур
    SDL_Color topColor = { 255, 255, 255, 255 };
    SDL_Color leftColor = { 200, 200, 200, 255 };
    SDL_Color rightColor = { 150, 150, 150, 255 };

    // Конструктор для плоского тайла
    RenderableTile(float x, float y, SDL_Texture* texture, SDL_Color color, float priority = 0.0f)
        : worldX(x), worldY(y), worldZ(0.0f), type(TileType::FLAT),
        renderPriority(priority), topTexture(texture), topColor(color) {
    }

    // Конструктор для объемного тайла
    RenderableTile(float x, float y, float z,
        SDL_Texture* top, SDL_Texture* left, SDL_Texture* right,
        SDL_Color topCol, SDL_Color leftCol, SDL_Color rightCol,
        float priority = 0.0f)
        : worldX(x), worldY(y), worldZ(z), type(TileType::VOLUMETRIC),
        renderPriority(priority), topTexture(top), leftTexture(left), rightTexture(right),
        topColor(topCol), leftColor(leftCol), rightColor(rightCol) {
    }
};