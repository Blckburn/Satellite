#include "TileRenderer.h"
#include "Logger.h"
#include <algorithm>

TileRenderer::TileRenderer(IsometricRenderer* isoRenderer)
    : m_isoRenderer(isoRenderer), m_useZSorting(true), m_useBatchRendering(true) {
}

TileRenderer::~TileRenderer() {
    clear();
}

void TileRenderer::addFlatTile(float x, float y, SDL_Texture* texture, const SDL_Color& color, float priority) {
    if (!m_isoRenderer) return;

    auto tile = new TileRenderData();
    tile->type = TileRenderType::FLAT;
    tile->x = x;
    tile->y = y;
    tile->height = 0.0f;
    tile->texture = texture;
    tile->topColor = color;
    tile->priority = priority;

    m_tiles.push_back(tile);
}

void TileRenderer::addVolumetricTile(float x, float y, float height,
    SDL_Texture* topTexture, SDL_Texture* leftTexture, SDL_Texture* rightTexture,
    const SDL_Color& topColor, const SDL_Color& leftColor, const SDL_Color& rightColor,
    float priority) {
    if (!m_isoRenderer) return;

    auto tile = new TileRenderData();
    tile->type = TileRenderType::VOLUMETRIC;
    tile->x = x;
    tile->y = y;
    tile->height = height;
    tile->texture = topTexture;
    tile->leftTexture = leftTexture;
    tile->rightTexture = rightTexture;
    tile->topColor = topColor;
    tile->leftColor = leftColor;
    tile->rightColor = rightColor;
    tile->priority = priority;

    m_tiles.push_back(tile);
}

void TileRenderer::clear() {
    // Очищаем все данные для рендеринга
    for (auto& tile : m_tiles) {
        delete tile;
    }
    m_tiles.clear();
    
    // Очищаем кэши рендеринга
    m_tileBatchMap.clear();
}

void TileRenderer::setZSorting(bool enable) {
    m_useZSorting = enable;
}

void TileRenderer::setBatchRendering(bool enable) {
    m_useBatchRendering = enable;
}

void TileRenderer::render(SDL_Renderer* renderer, int centerX, int centerY) {
    if (!m_isoRenderer || !renderer || m_tiles.empty()) return;

    // Если включена сортировка по глубине, сортируем тайлы
    if (m_useZSorting) {
        sortTiles();
    }

    // Получаем размер окна для определения видимой области
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Определяем границы видимой области с небольшим запасом
    // (добавляем размер тайла, чтобы отрисовать тайлы, частично видимые)
    int tileWidth = m_isoRenderer->getTileWidth();
    int tileHeight = m_isoRenderer->getTileHeight();
    SDL_Rect viewport = {
        -tileWidth,
        -tileHeight,
        windowWidth + tileWidth * 2,
        windowHeight + tileHeight * 2
    };

    // Подготавливаем батчи для оптимизированного рендеринга, если включено
    if (m_useBatchRendering) {
        prepareBatches();
    }

    // Счетчики для статистики
    int totalTiles = m_tiles.size();
    int renderedTiles = 0;

    // Отрисовываем тайлы
    for (auto& tile : m_tiles) {
        // Проверяем, находится ли тайл в видимой области
        int screenX, screenY;
        m_isoRenderer->worldToDisplay(
            tile->x, tile->y, tile->height,
            centerX, centerY, screenX, screenY
        );

        // Проверяем, видим ли тайл
        bool isVisible = (screenX >= viewport.x && screenX < viewport.x + viewport.w &&
                         screenY >= viewport.y && screenY < viewport.y + viewport.h);

        if (!isVisible) {
            continue; // Пропускаем отрисовку невидимых тайлов
        }

        // Отрисовываем тайл
        if (tile->type == TileRenderType::FLAT) {
            m_isoRenderer->renderFlatTile(renderer, tile->x, tile->y,
                tile->texture, tile->topColor, centerX, centerY);
        }
        else if (tile->type == TileRenderType::VOLUMETRIC) {
            m_isoRenderer->renderVolumetricTile(renderer, tile->x, tile->y, tile->height,
                centerX, centerY, tile->texture, tile->leftTexture, tile->rightTexture,
                tile->topColor, tile->leftColor, tile->rightColor);
        }

        renderedTiles++;
    }

    // Выводим статистику, если включен режим отладки
    if (Logger::getInstance().getConsoleLogLevel() <= LogLevel::DEBUG && totalTiles > 0) {
        static int frameCounter = 0;
        static int statInterval = 60; // Выводим статистику каждые 60 кадров
        
        if (++frameCounter >= statInterval) {
            float cullRatio = 100.0f * (1.0f - static_cast<float>(renderedTiles) / totalTiles);
            LOG_DEBUG("Rendering stats: " + std::to_string(renderedTiles) + "/" + 
                     std::to_string(totalTiles) + " tiles rendered (" + 
                     std::to_string(cullRatio) + "% culled)");
            frameCounter = 0;
        }
    }
}

void TileRenderer::sortTiles() {
    // Сортировка тайлов для корректного отображения в изометрии
    std::sort(m_tiles.begin(), m_tiles.end(), [](const TileRenderData* a, const TileRenderData* b) {
        // 1. Сначала проверяем явный приоритет
        if (a->priority != b->priority) {
            return a->priority < b->priority;
        }

        // 2. Если приоритеты равны, сортируем по глубине (y + x)
        float depthA = a->y + a->x;
        float depthB = b->y + b->x;
        
        if (depthA != depthB) {
            return depthA < depthB;
        }

        // 3. Для тайлов на одной глубине, сортируем по высоте
        return a->height < b->height;
    });
}

void TileRenderer::prepareBatches() {
    // Очищаем предыдущие батчи
    m_tileBatchMap.clear();
    
    // Группируем тайлы по текстурам для уменьшения переключений состояния рендерера
    for (auto& tile : m_tiles) {
        // Создаем ключ батча на основе текстур и типа тайла
        BatchKey key = { 
            tile->type, 
            tile->texture, 
            tile->leftTexture, 
            tile->rightTexture 
        };
        
        // Добавляем тайл в соответствующий батч
        m_tileBatchMap[key].push_back(tile);
    }
}