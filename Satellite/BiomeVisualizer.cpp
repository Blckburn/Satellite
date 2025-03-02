#include "BiomeVisualizer.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

BiomeVisualizer::BiomeVisualizer(const std::vector<std::shared_ptr<Biome>>& biomes, IsometricRenderer* isoRenderer)
    : m_biomes(biomes), m_isoRenderer(isoRenderer), m_activeBiomeId(-1),
    m_biomeColumns(3), m_sampleSize(64) {
}

BiomeVisualizer::~BiomeVisualizer() {
}

bool BiomeVisualizer::initialize() {
    // Проверяем, что у нас есть биомы для визуализации
    if (m_biomes.empty()) {
        return false;
    }

    // Проверяем, что у нас есть рендерер
    if (!m_isoRenderer) {
        return false;
    }

    return true;
}

void BiomeVisualizer::render(SDL_Renderer* renderer, int centerX, int centerY,
    int panelX, int panelY, int panelWidth, int panelHeight) {
    // Проверяем, что у нас есть данные для визуализации
    if (m_biomes.empty() || !m_isoRenderer || !renderer) {
        return;
    }

    // Рисуем панель с полупрозрачным фоном
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_Rect panelRect = { panelX, panelY, panelWidth, panelHeight };
    SDL_RenderFillRect(renderer, &panelRect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &panelRect);

    // Определяем количество строк, необходимых для отображения всех биомов
    int totalBiomes = static_cast<int>(m_biomes.size());
    int biomeRows = (totalBiomes + m_biomeColumns - 1) / m_biomeColumns; // Округление вверх

    // Определяем размер ячейки для каждого биома
    int cellWidth = panelWidth / m_biomeColumns;
    int cellHeight = std::min(cellWidth, panelHeight / biomeRows);

    // Создаем рендерер тайлов для примеров биомов
    TileRenderer tileRenderer(m_isoRenderer);

    // Сохраняем текущие настройки рендерера
    float originalZoom = m_isoRenderer->getCameraZoom();
    float originalCameraX = m_isoRenderer->getCameraX();
    float originalCameraY = m_isoRenderer->getCameraY();

    // Устанавливаем масштаб для рендеринга примеров
    float scale = static_cast<float>(cellWidth) / m_sampleSize;
    m_isoRenderer->setCameraZoom(scale);
    m_isoRenderer->setCameraPosition(0.0f, 0.0f);

    // Рендерим примеры биомов
    for (int i = 0; i < totalBiomes; ++i) {
        // Определяем позицию ячейки
        int row = i / m_biomeColumns;
        int col = i % m_biomeColumns;

        int cellX = panelX + col * cellWidth;
        int cellY = panelY + row * cellHeight;

        // Рисуем ячейку
        SDL_Rect cellRect = { cellX, cellY, cellWidth, cellHeight };

        // Если это активный биом, выделяем его
        if (m_biomes[i]->getId() == m_activeBiomeId) {
            SDL_SetRenderDrawColor(renderer, 100, 255, 100, 180);
            SDL_RenderFillRect(renderer, &cellRect);
        }

        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &cellRect);

        // Рендерим пример биома
        tileRenderer.clear();
        createBiomeSample(m_biomes[i], &tileRenderer, 0.0f, 0.0f);
        tileRenderer.render(renderer, cellX + cellWidth / 2, cellY + cellHeight / 2);

        // Отображаем название биома
        // Здесь должен быть код для отображения текста
        // В рамках этого примера просто выводим в консоль
        if (m_biomes[i]->getId() == m_activeBiomeId) {
            std::cout << "Active biome: " << m_biomes[i]->getName() << std::endl;
        }
    }

    // Восстанавливаем настройки рендерера
    m_isoRenderer->setCameraZoom(originalZoom);
    m_isoRenderer->setCameraPosition(originalCameraX, originalCameraY);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

int BiomeVisualizer::handleClick(int x, int y, int panelX, int panelY, int panelWidth, int panelHeight) {
    // Проверяем, что клик был внутри панели
    if (x < panelX || x >= panelX + panelWidth || y < panelY || y >= panelY + panelHeight) {
        return -1;
    }

    // Определяем количество строк и размер ячейки
    int totalBiomes = static_cast<int>(m_biomes.size());
    int biomeRows = (totalBiomes + m_biomeColumns - 1) / m_biomeColumns;
    int cellWidth = panelWidth / m_biomeColumns;
    int cellHeight = std::min(cellWidth, panelHeight / biomeRows);

    // Определяем, по какой ячейке кликнули
    int col = (x - panelX) / cellWidth;
    int row = (y - panelY) / cellHeight;
    int index = row * m_biomeColumns + col;

    // Проверяем, что индекс в пределах массива биомов
    if (index >= 0 && index < totalBiomes) {
        // Устанавливаем активный биом
        setActiveBiome(m_biomes[index]->getId());
        return m_biomes[index]->getId();
    }

    return -1;
}

void BiomeVisualizer::setActiveBiome(int biomeId) {
    m_activeBiomeId = biomeId;
}

std::string BiomeVisualizer::getBiomeInfo(int biomeId) const {
    // Ищем биом по ID
    for (const auto& biome : m_biomes) {
        if (biome->getId() == biomeId) {
            // Формируем строку с информацией
            std::ostringstream info;

            info << "Biome: " << biome->getName() << "\n";
            info << biome->getDescription() << "\n\n";

            info << "Temperature range: " << std::fixed << std::setprecision(1)
                << biome->getMinTemperature() << "°C to "
                << biome->getMaxTemperature() << "°C\n";

            info << "Humidity range: " << std::fixed << std::setprecision(2)
                << biome->getMinHumidity() << " to "
                << biome->getMaxHumidity() << "\n";

            info << "Elevation range: " << std::fixed << std::setprecision(2)
                << biome->getMinElevation() << " to "
                << biome->getMaxElevation() << "\n";

            info << "Hazard level: " << std::fixed << std::setprecision(2)
                << biome->getHazardLevel() << "\n";

            info << "Resource level: " << std::fixed << std::setprecision(2)
                << biome->getResourceLevel() << "\n\n";

            // Добавляем информацию о доступных декорациях
            const auto& decorations = biome->getDecorations();
            if (!decorations.empty()) {
                info << "Available decorations:\n";
                for (const auto& decor : decorations) {
                    info << "  - " << decor.name << " (ID: " << decor.id
                        << ", Probability: " << std::fixed << std::setprecision(2) << decor.probability
                        << ", Scale: " << decor.minScale << "-" << decor.maxScale
                        << (decor.animated ? ", Animated" : "") << ")\n";
                }
            }

            return info.str();
        }
    }

    return "Biome not found";
}

void BiomeVisualizer::createBiomeSample(const std::shared_ptr<Biome>& biome, TileRenderer* tileRenderer, float x, float y) {
    if (!biome || !tileRenderer) return;

    // Получаем случайный тип тайла для этого биома
    TileType tileType = biome->getRandomTileType();

    // Определяем цвет на основе типа тайла
    SDL_Color color;
    float height = 0.0f;

    switch (tileType) {
    case TileType::GRASS:
        color = { 30, 150, 30, 255 }; // Зеленый
        break;
    case TileType::SAND:
        color = { 240, 240, 100, 255 }; // Жёлтый
        break;
    case TileType::STONE:
        color = { 150, 150, 150, 255 }; // Серый
        break;
    case TileType::SNOW:
        color = { 240, 240, 250, 255 }; // Белый
        break;
    case TileType::WATER:
        color = { 64, 164, 223, 255 }; // Синий
        height = 0.1f;
        break;
    case TileType::LAVA:
        color = { 255, 100, 0, 255 }; // Оранжевый
        height = 0.1f;
        break;
    case TileType::MOUNTAIN:
        color = { 120, 100, 80, 255 }; // Коричневый
        height = 0.7f;
        break;
    case TileType::HILL:
        color = { 150, 120, 90, 255 }; // Светло-коричневый
        height = 0.4f;
        break;
    case TileType::ROCK_FORMATION:
        color = { 180, 150, 120, 255 }; // Бежевый
        height = 0.5f;
        break;
    case TileType::ALIEN_GROWTH:
        color = { 200, 50, 200, 255 }; // Фиолетовый
        height = 0.3f;
        break;
    case TileType::ICE:
        color = { 200, 230, 255, 200 }; // Голубой полупрозрачный
        height = 0.05f;
        break;
    case TileType::MUD:
        color = { 120, 100, 70, 255 }; // Грязно-коричневый
        break;
    case TileType::SHALLOW_WATER:
        color = { 120, 200, 230, 255 }; // Светло-синий
        height = 0.05f;
        break;
    case TileType::CRATER:
        color = { 100, 100, 100, 255 }; // Тёмно-серый
        height = -0.1f;
        break;
    case TileType::RUINS:
        color = { 220, 220, 180, 255 }; // Песочный
        height = 0.3f;
        break;
    case TileType::MINERAL_DEPOSIT:
        color = { 200, 150, 250, 255 }; // Светло-фиолетовый
        break;
    default:
        color = { 150, 150, 150, 255 }; // Стандартный серый
    }

    // Добавляем тайл в рендерер
    if (height > 0.0f) {
        // Объемный тайл
        SDL_Color topColor = color;
        SDL_Color leftColor = {
            static_cast<Uint8>(color.r * 0.7f),
            static_cast<Uint8>(color.g * 0.7f),
            static_cast<Uint8>(color.b * 0.7f),
            color.a
        };
        SDL_Color rightColor = {
            static_cast<Uint8>(color.r * 0.5f),
            static_cast<Uint8>(color.g * 0.5f),
            static_cast<Uint8>(color.b * 0.5f),
            color.a
        };

        tileRenderer->addVolumetricTile(
            x, y, height,
            nullptr, nullptr, nullptr,
            topColor, leftColor, rightColor,
            0
        );
    }
    else {
        // Плоский тайл
        tileRenderer->addFlatTile(
            x, y,
            nullptr, // Без текстуры
            color,
            0
        );
    }
}