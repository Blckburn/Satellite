#include "TileRenderer.h"
#include <iostream>
#include "Logger.h"

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

// Минимальная диагностическая версия метода render в TileRenderer.cpp

void TileRenderer::render(SDL_Renderer* renderer, int centerX, int centerY) {
    LOG_INFO("===== TileRenderer::render - DIAGNOSTICS =====");

    // Проверка наличия тайлов для рендеринга
    if (m_tiles.empty()) {
        LOG_INFO("No tiles to render");
        return;
    }

    LOG_INFO("Tiles to render: " + std::to_string(m_tiles.size()));

    // Проверяем валидность рендерера
    if (!renderer) {
        LOG_ERROR("Renderer is nullptr");
        return;
    }

    // Проверяем наличие изометрического рендерера
    if (!m_isoRenderer) {
        LOG_ERROR("IsometricRenderer is nullptr");
        return;
    }

    LOG_INFO("Rendering first tile...");

    // Рендерим только первый тайл для тестирования
    if (!m_tiles.empty()) {
        const RenderableTile& tile = m_tiles[0];

        // Выводим информацию о тайле для отладки
        LOG_INFO("Tile info: x=" + std::to_string(tile.worldX) +
            ", y=" + std::to_string(tile.worldY) +
            ", z=" + std::to_string(tile.worldZ) +
            ", type=" + std::to_string(static_cast<int>(tile.type)));

        // Проверяем наличие текстуры
        if (tile.topTexture) {
            LOG_INFO("Tile has texture");
        }
        else {
            LOG_INFO("Tile has no texture, using color");
        }

        try {
            if (tile.type == RenderableTile::TileType::FLAT) {
                // Простой рендеринг одного тайла с цветом вместо текстуры для диагностики
                LOG_INFO("Rendering flat tile with color only (ignoring texture for testing)");
                m_isoRenderer->renderTile(
                    renderer,
                    tile.worldX, tile.worldY, tile.worldZ,
                    tile.topColor,
                    centerX, centerY
                );
                LOG_INFO("Flat tile rendered successfully");
            }
            else {
                // Рендеринг объемного тайла цветом для диагностики
                LOG_INFO("Rendering volumetric tile with color only (ignoring texture for testing)");
                m_isoRenderer->renderVolumetricTile(
                    renderer,
                    tile.worldX, tile.worldY, tile.worldZ,
                    tile.topColor, tile.leftColor, tile.rightColor,
                    centerX, centerY
                );
                LOG_INFO("Volumetric tile rendered successfully");
            }
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception during tile rendering: " + std::string(e.what()));
        }
    }

    LOG_INFO("===== TileRenderer::render - COMPLETED =====");
}