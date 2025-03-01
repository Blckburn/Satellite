#include "TileRenderer.h"
#include <iostream>

TileRenderer::TileRenderer(IsometricRenderer* isoRenderer)
    : m_isoRenderer(isoRenderer) {
}

TileRenderer::~TileRenderer() {
    clear();
}

void TileRenderer::clear() {
    m_tiles.clear();
}

void TileRenderer::addFlatTile(float x, float y, SDL_Texture* texture, SDL_Color color, int priority) {
    m_tiles.emplace_back(x, y, texture, color, priority);
}

void TileRenderer::addVolumetricTile(float x, float y, float z,
    SDL_Texture* topTexture,
    SDL_Texture* leftTexture,
    SDL_Texture* rightTexture,
    SDL_Color topColor,
    SDL_Color leftColor,
    SDL_Color rightColor,
    int priority) {
    m_tiles.emplace_back(x, y, z,
        topTexture, leftTexture, rightTexture,
        topColor, leftColor, rightColor,
        priority);
}

void TileRenderer::render(SDL_Renderer* renderer, int centerX, int centerY) {
    // 1. Сортируем тайлы по Z-порядку с учетом приоритета для правильного перекрытия
    std::sort(m_tiles.begin(), m_tiles.end(),
        [](const RenderableTile& a, const RenderableTile& b) {
            // Если приоритеты разные, они имеют превосходство над всеми остальными факторами
            if (a.renderPriority != b.renderPriority) {
                return a.renderPriority < b.renderPriority;
            }

            // Сортируем по сумме X + Y (для корректного изометрического порядка)
            float sumA = a.worldX + a.worldY;
            float sumB = b.worldX + b.worldY;

            if (sumA != sumB) {
                return sumA < sumB;
            }

            // Затем по высоте (Z)
            return a.worldZ < b.worldZ;
        });

    // 2. Рендерим тайлы в отсортированном порядке
    for (const auto& tile : m_tiles) {
        if (tile.type == RenderableTile::TileType::FLAT) {
            // Рендеринг плоского тайла с использованием цвета, игнорируем текстуры
            m_isoRenderer->renderTile(
                renderer,
                tile.worldX, tile.worldY, tile.worldZ,
                tile.topColor,
                centerX, centerY
            );
        }
        else {
            // Рендеринг объемного тайла с использованием цветов
            m_isoRenderer->renderVolumetricTile(
                renderer,
                tile.worldX, tile.worldY, tile.worldZ,
                tile.topColor, tile.leftColor, tile.rightColor,
                centerX, centerY
            );
        }
    }
}