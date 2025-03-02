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

void TileRenderer::addFlatTile(float x, float y, SDL_Texture* texture, SDL_Color color, float priority) {
    m_tiles.emplace_back(x, y, texture, color, priority);
}

void TileRenderer::addVolumetricTile(float x, float y, float z,
    SDL_Texture* topTexture,
    SDL_Texture* leftTexture,
    SDL_Texture* rightTexture,
    SDL_Color topColor,
    SDL_Color leftColor,
    SDL_Color rightColor,
    float priority) {
    m_tiles.emplace_back(x, y, z,
        topTexture, leftTexture, rightTexture,
        topColor, leftColor, rightColor,
        priority);
}

void TileRenderer::render(SDL_Renderer* renderer, int centerX, int centerY) {
    // Проверка наличия тайлов для рендеринга
    if (m_tiles.empty()) {
        return;
    }

    // Z-СОРТИРОВКА для правильного порядка отображения тайлов
    std::sort(m_tiles.begin(), m_tiles.end(),
        [](const RenderableTile& a, const RenderableTile& b) {
            // 1. Сортировка по приоритету
            if (std::abs(a.renderPriority - b.renderPriority) > 0.001f) {
                return a.renderPriority < b.renderPriority;
            }

            // 2. НОВОЕ: Специальная обработка для воды - вода всегда отображается под объектами
            // Этот код можно определить по цвету, но безопаснее передавать флаг "isWater"
            bool aIsWater = (a.topColor.r < 100 && a.topColor.g > 150 && a.topColor.b > 200);
            bool bIsWater = (b.topColor.r < 100 && b.topColor.g > 150 && b.topColor.b > 200);

            if (aIsWater && !bIsWater) {
                return true; // Вода рисуется под другими объектами
            }
            if (!aIsWater && bIsWater) {
                return false; // Другие объекты рисуются поверх воды
            }

            // 3. Если приоритеты равны, используем сумму координат (X+Y)
            float sumA = a.worldX + a.worldY;
            float sumB = b.worldX + b.worldY;

            if (std::abs(sumA - sumB) > 0.01f) {
                return sumA < sumB;
            }

            // 4. При равных (X+Y) сортируем по высоте (Z)
            if (std::abs(a.worldZ - b.worldZ) > 0.001f) {
                return a.worldZ < b.worldZ;
            }

            // 5. Крайний случай - объемные объекты поверх плоских
            if (a.type != b.type) {
                return a.type == RenderableTile::TileType::FLAT &&
                    b.type == RenderableTile::TileType::VOLUMETRIC;
            }

            // 6. Детерминированный порядок для избежания "мерцания"
            if (std::abs(a.worldX - b.worldX) > 0.001f) {
                return a.worldX < b.worldX;
            }

            return a.worldY < b.worldY;
        });

    // Рендерим тайлы в отсортированном порядке
    for (const auto& tile : m_tiles) {
        if (tile.type == RenderableTile::TileType::FLAT) {
            // Рендеринг плоского тайла
            m_isoRenderer->renderTile(
                renderer,
                tile.worldX, tile.worldY, tile.worldZ,
                tile.topColor,
                centerX, centerY
            );
        }
        else {
            // Рендеринг объемного тайла
            m_isoRenderer->renderVolumetricTile(
                renderer,
                tile.worldX, tile.worldY, tile.worldZ,
                tile.topColor, tile.leftColor, tile.rightColor,
                centerX, centerY
            );
        }
    }
}