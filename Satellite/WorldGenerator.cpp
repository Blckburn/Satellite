#include "WorldGenerator.h"
#include "Logger.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <functional>
#include <SDL_image.h>

// Конструктор по умолчанию для PlanetData
WorldGenerator::PlanetData::PlanetData()
    : name("Unnamed Planet"),
    description("A mysterious world."),
    averageTemperature(20.0f),
    atmosphereDensity(1.0f),
    gravityMultiplier(1.0f),
    waterCoverage(0.3f),
    radiationLevel(0.0f),
    resourceRichness(0.5f),
    mainTerrainType(MapGenerator::GenerationType::DEFAULT),
    hasLife(false),
    seed(0) {
}

// Конструктор для WorldGenerator
WorldGenerator::WorldGenerator(unsigned int seed)
    : m_seed(seed ? seed : static_cast<unsigned int>(std::time(nullptr))),
    m_rng(m_seed),
    m_mapGenerator(std::make_shared<MapGenerator>()) {
    setupDefaultBiomes(); // Инициализация биомов по умолчанию
}

// Деструктор
WorldGenerator::~WorldGenerator() {
    // Очистка ресурсов не требуется благодаря shared_ptr
}

// Генерация планеты
bool WorldGenerator::generatePlanet(TileMap* tileMap, const PlanetData& planetData) {
    if (!tileMap) {
        LOG_ERROR("TileMap is null in generatePlanet");
        return false;
    }

    LOG_DEBUG("Generating planet with seed: " + std::to_string(planetData.seed));

    // Устанавливаем сид для генератора
    setSeed(planetData.seed);

    // Генерируем базовый рельеф
    m_mapGenerator->setGenerationType(planetData.mainTerrainType);
    m_mapGenerator->generateMap(tileMap);

    // Устанавливаем биомы
    if (!setupBiomesForPlanet(planetData)) {
        LOG_ERROR("Failed to setup biomes for planet");
        return false;
    }

    // Применяем особенности планеты
    applyPlanetaryFeatures(tileMap, planetData);

    // Создаём эрозию, переходы и достопримечательности
    createErosionPatterns(tileMap, 0.5f); // Средняя интенсивность эрозии
    enhanceBiomeTransitions(tileMap);
    createLandmarks(tileMap, 3); // 3 достопримечательности для примера

    return true;
}

// Генерация случайной планеты
WorldGenerator::PlanetData WorldGenerator::generateRandomPlanet(TileMap* tileMap) {
    PlanetData planetData;

    // Случайные параметры
    std::uniform_int_distribution<unsigned int> seedDist(0, UINT_MAX);
    planetData.seed = seedDist(m_rng);
    setSeed(planetData.seed);

    std::uniform_real_distribution<float> tempDist(-50.0f, 100.0f);
    std::uniform_real_distribution<float> coverageDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> typeDist(0, static_cast<int>(MapGenerator::GenerationType::COUNT) - 1);

    planetData.averageTemperature = tempDist(m_rng);
    planetData.waterCoverage = coverageDist(m_rng);
    planetData.mainTerrainType = static_cast<MapGenerator::GenerationType>(typeDist(m_rng));
    planetData.atmosphereDensity = coverageDist(m_rng);
    planetData.gravityMultiplier = 0.5f + coverageDist(m_rng) * 1.5f; // 0.5-2.0
    planetData.radiationLevel = coverageDist(m_rng) * 0.5f; // 0.0-0.5
    planetData.resourceRichness = coverageDist(m_rng);
    planetData.hasLife = m_rng() % 100 < 30; // 30% шанс на жизнь
    planetData.name = generatePlanetName();
    planetData.description = generatePlanetDescription(planetData);

    if (tileMap) {
        generatePlanet(tileMap, planetData);
    }

    return planetData;
}

// Генерация планеты с заданными параметрами
WorldGenerator::PlanetData WorldGenerator::generateCustomPlanet(TileMap* tileMap, float averageTemperature, float waterCoverage, MapGenerator::GenerationType terrainType) {
    PlanetData planetData;

    // Задаём пользовательские параметры
    planetData.averageTemperature = averageTemperature;
    planetData.waterCoverage = std::max(0.0f, std::min(1.0f, waterCoverage));
    planetData.mainTerrainType = terrainType;

    // Остальные параметры генерируем случайным образом
    std::uniform_int_distribution<unsigned int> seedDist(0, UINT_MAX);
    planetData.seed = seedDist(m_rng);
    setSeed(planetData.seed);

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    planetData.atmosphereDensity = dist(m_rng);
    planetData.gravityMultiplier = 0.5f + dist(m_rng) * 1.5f; // 0.5-2.0
    planetData.radiationLevel = dist(m_rng) * 0.5f; // 0.0-0.5
    planetData.resourceRichness = dist(m_rng);
    planetData.hasLife = m_rng() % 100 < 20; // 20% шанс на жизнь
    planetData.name = generatePlanetName();
    planetData.description = generatePlanetDescription(planetData);

    if (tileMap) {
        generatePlanet(tileMap, planetData);
    }

    return planetData;
}

// Генерация планеты с кэшем
bool WorldGenerator::generatePlanetWithCache(TileMap* tileMap, const PlanetData& planetData) {
    if (!tileMap) return false;

    // Проверяем, генерировали ли мы уже такую же планету
    if (m_lastPlanetData.seed == planetData.seed &&
        m_lastPlanetData.mainTerrainType == planetData.mainTerrainType &&
        std::abs(m_lastPlanetData.averageTemperature - planetData.averageTemperature) < 0.001f &&
        std::abs(m_lastPlanetData.waterCoverage - planetData.waterCoverage) < 0.001f) {

        LOG_DEBUG("Using cached planet with seed: " + std::to_string(planetData.seed));

        if (m_cachedTileMap && m_cachedTileMap->getWidth() == tileMap->getWidth() &&
            m_cachedTileMap->getHeight() == tileMap->getHeight()) {

            for (int y = 0; y < tileMap->getHeight(); ++y) {
                for (int x = 0; x < tileMap->getWidth(); ++x) {
                    MapTile* srcTile = m_cachedTileMap->getTile(x, y);
                    MapTile* dstTile = tileMap->getTile(x, y);

                    if (srcTile && dstTile) {
                        *dstTile = *srcTile;
                    }
                }
            }
            return true;
        }
    }

    LOG_DEBUG("Generating new planet with seed: " + std::to_string(planetData.seed));

    bool result = generatePlanet(tileMap, planetData);

    if (result) {
        m_lastPlanetData = planetData;

        if (!m_cachedTileMap || m_cachedTileMap->getWidth() != tileMap->getWidth() ||
            m_cachedTileMap->getHeight() != tileMap->getHeight()) {

            m_cachedTileMap = std::make_shared<TileMap>(tileMap->getWidth(), tileMap->getHeight());
            m_cachedTileMap->initialize();
        }

        for (int y = 0; y < tileMap->getHeight(); ++y) {
            for (int x = 0; x < tileMap->getWidth(); ++x) {
                MapTile* srcTile = tileMap->getTile(x, y);
                MapTile* dstTile = m_cachedTileMap->getTile(x, y);

                if (srcTile && dstTile) {
                    *dstTile = *srcTile;
                }
            }
        }
    }

    return result;
}

// Создание эрозионных узоров
void WorldGenerator::createErosionPatterns(TileMap* tileMap, float intensity) {
    if (!tileMap) return;

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                heightMap[y][x] = tile->getElevation();
            }
        }
    }

    int dropCount = static_cast<int>(width * height * 0.05f * intensity);
    const float inertia = 0.1f;
    const float erosionFactor = 0.3f;
    const float depositionFactor = 0.3f;
    const float evaporationFactor = 0.02f;
    const int maxSteps = 100;

    std::uniform_int_distribution<int> dist_x(0, width - 1);
    std::uniform_int_distribution<int> dist_y(0, height - 1);
    std::uniform_real_distribution<float> dist_01(0.0f, 1.0f);

    LOG_INFO("Creating erosion patterns with " + std::to_string(dropCount) + " water drops");

    for (int d = 0; d < dropCount; ++d) {
        int x = dist_x(m_rng);
        int y = dist_y(m_rng);

        MapTile* tile = tileMap->getTile(x, y);
        if (!tile || tile->isWater()) continue;

        float posX = static_cast<float>(x);
        float posY = static_cast<float>(y);
        float dirX = 0.0f;
        float dirY = 0.0f;
        float speed = 0.0f;
        float water = 1.0f;
        float sediment = 0.0f;

        for (int step = 0; step < maxSteps; ++step) {
            int cellX = std::min(width - 1, std::max(0, static_cast<int>(posX)));
            int cellY = std::min(height - 1, std::max(0, static_cast<int>(posY)));

            float lowestHeight = heightMap[cellY][cellX];
            int bestX = cellX;
            int bestY = cellY;

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    int nx = cellX + dx;
                    int ny = cellY + dy;

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        if (heightMap[ny][nx] < lowestHeight) {
                            lowestHeight = heightMap[ny][nx];
                            bestX = nx;
                            bestY = ny;
                        }
                    }
                }
            }

            if (bestX == cellX && bestY == cellY) break;

            float newDirX = static_cast<float>(bestX - cellX);
            float newDirY = static_cast<float>(bestY - cellY);

            float length = std::sqrt(newDirX * newDirX + newDirY * newDirY);
            if (length > 0) {
                newDirX /= length;
                newDirY /= length;
            }

            dirX = dirX * inertia + newDirX * (1 - inertia);
            dirY = dirY * inertia + newDirY * (1 - inertia);

            length = std::sqrt(dirX * dirX + dirY * dirY);
            if (length > 0) {
                dirX /= length;
                dirY /= length;
            }

            float newPosX = posX + dirX;
            float newPosY = posY + dirY;

            if (newPosX < 0 || newPosX >= width || newPosY < 0 || newPosY >= height) break;

            int newCellX = static_cast<int>(newPosX);
            int newCellY = static_cast<int>(newPosY);

            if (newCellX < 0 || newCellX >= width || newCellY < 0 || newCellY >= height) break;

            float heightDiff = heightMap[cellY][cellX] - heightMap[newCellY][newCellX];

            speed = std::sqrt(speed * speed + heightDiff);

            float erosion = speed * water * erosionFactor;

            if (heightDiff < 0) erosion *= 0.1f;

            erosion = std::min(erosion, heightMap[cellY][cellX] * 0.1f);

            heightMap[cellY][cellX] -= erosion;
            sediment += erosion;

            float deposition = sediment * depositionFactor;
            heightMap[newCellY][newCellX] += deposition;
            sediment -= deposition;

            water *= (1.0f - evaporationFactor);

            posX = newPosX;
            posY = newPosY;

            if (water < 0.01f) break;
        }
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile && !tile->isWater()) {
                tile->setElevation(heightMap[y][x]);
                if (heightMap[y][x] < m_mapGenerator->getWaterLevel()) {
                    tile->setType(TileType::WATER);
                    tile->setHeight(0.1f);
                }
                else if (heightMap[y][x] < m_mapGenerator->getWaterLevel() + 0.05f) {
                    tile->setType(TileType::SHALLOW_WATER);
                    tile->setHeight(0.05f);
                }
            }
        }
    }

    LOG_INFO("Erosion patterns created successfully");
}

// Улучшение переходов между биомами
void WorldGenerator::enhanceBiomeTransitions(TileMap* tileMap) {
    if (!tileMap) return;

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    std::vector<std::vector<int>> biomeCopy(height, std::vector<int>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                biomeCopy[y][x] = tile->getBiomeId();
            }
        }
    }

    LOG_INFO("Enhancing biome transitions");

    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            int currentBiome = biomeCopy[y][x];

            std::map<int, int> biomeCount;
            bool onBorder = false;

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if (dx == 0 && dy == 0) continue;

                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        int neighborBiome = biomeCopy[ny][nx];
                        biomeCount[neighborBiome]++;

                        if (neighborBiome != currentBiome) {
                            onBorder = true;
                        }
                    }
                }
            }

            if (onBorder) {
                int mostCommonBiome = currentBiome;
                int maxCount = 0;

                for (const auto& [biomeId, count] : biomeCount) {
                    if (biomeId != currentBiome && count > maxCount) {
                        mostCommonBiome = biomeId;
                        maxCount = count;
                    }
                }

                if (mostCommonBiome != currentBiome) {
                    std::shared_ptr<Biome> currentBiomeObj = nullptr;
                    std::shared_ptr<Biome> neighborBiomeObj = nullptr;

                    for (const auto& biome : m_biomes) {
                        if (biome->getId() == currentBiome) {
                            currentBiomeObj = biome;
                        }
                        else if (biome->getId() == mostCommonBiome) {
                            neighborBiomeObj = biome;
                        }
                    }

                    if (currentBiomeObj && neighborBiomeObj) {
                        std::uniform_real_distribution<float> transitionDist(0.0f, 1.0f);
                        float transitionRoll = transitionDist(m_rng);

                        if (transitionRoll < 0.3f) {
                            TileType newType = neighborBiomeObj->getRandomTileType();
                            tile->setType(newType);

                            SDL_Color currentColor = tile->getColor();
                            SDL_Color newColor = { currentColor.r, currentColor.g, currentColor.b, currentColor.a };

                            MapTile sampleTile(newType);
                            SDL_Color neighborColor = sampleTile.getColor();

                            newColor.r = static_cast<Uint8>(currentColor.r * 0.7f + neighborColor.r * 0.3f);
                            newColor.g = static_cast<Uint8>(currentColor.g * 0.7f + neighborColor.g * 0.3f);
                            newColor.b = static_cast<Uint8>(currentColor.b * 0.7f + neighborColor.b * 0.3f);

                            tile->setColor(newColor);
                        }
                        else if (transitionRoll < 0.5f) {
                            auto neighborDecorations = neighborBiomeObj->generateRandomDecorations(1);
                            if (!neighborDecorations.empty()) {
                                tile->addDecoration(neighborDecorations[0]);
                            }
                        }
                    }
                }
            }
        }
    }

    LOG_INFO("Biome transitions enhanced");
}

// Создание достопримечательностей
void WorldGenerator::createLandmarks(TileMap* tileMap, int count) {
    if (!tileMap) return;

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    count = std::max(1, count);

    LOG_INFO("Creating " + std::to_string(count) + " landmarks");

    enum class LandmarkType {
        MOUNTAIN_PEAK,
        CRATER_FORMATION,
        ROCK_SPIRE,
        ANCIENT_RUINS,
        STRANGE_MONUMENT,
        OASIS,
        GEYSER_FIELD,
        CRYSTAL_FORMATION,
        ALIEN_STRUCTURE
    };

    std::uniform_int_distribution<int> typeRange(0, 8);
    std::uniform_int_distribution<int> xDist(width / 10, width * 9 / 10);
    std::uniform_int_distribution<int> yDist(height / 10, height * 9 / 10);
    std::uniform_int_distribution<int> sizeRange(5, 15);

    for (int i = 0; i < count; ++i) {
        LandmarkType type = static_cast<LandmarkType>(typeRange(m_rng));
        int centerX = xDist(m_rng);
        int centerY = yDist(m_rng);
        int size = sizeRange(m_rng);

        MapTile* centerTile = tileMap->getTile(centerX, centerY);
        if (!centerTile || centerTile->isWater()) {
            if (type != LandmarkType::OASIS) continue;
        }

        switch (type) {
        case LandmarkType::MOUNTAIN_PEAK:
            createMountainPeak(tileMap, centerX, centerY, size);
            break;
        case LandmarkType::CRATER_FORMATION:
            createCraterFormation(tileMap, centerX, centerY, size);
            break;
        case LandmarkType::ROCK_SPIRE:
            createRockSpire(tileMap, centerX, centerY, size);
            break;
        case LandmarkType::ANCIENT_RUINS:
            createAncientRuins(tileMap, centerX, centerY, size);
            break;
        case LandmarkType::STRANGE_MONUMENT:
            createStrangeMonument(tileMap, centerX, centerY, size / 2);
            break;
        case LandmarkType::OASIS:
            createOasis(tileMap, centerX, centerY, size);
            break;
        case LandmarkType::GEYSER_FIELD:
            createGeyserField(tileMap, centerX, centerY, size);
            break;
        case LandmarkType::CRYSTAL_FORMATION:
            createCrystalFormation(tileMap, centerX, centerY, size);
            break;
        case LandmarkType::ALIEN_STRUCTURE:
            createAlienStructure(tileMap, centerX, centerY, size);
            break;
        }
    }

    LOG_INFO("Landmarks created successfully");
}

// Создание горной вершины
void WorldGenerator::createMountainPeak(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating mountain peak at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    float peakHeight = 0.9f + static_cast<float>(m_rng() % 20) / 100.0f;

    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 1.0f) continue;

            float heightFactor = std::pow(1.0f - distance, 2.0f);

            std::uniform_real_distribution<float> noiseDist(-0.1f, 0.1f);
            heightFactor += noiseDist(m_rng);
            heightFactor = std::max(0.0f, std::min(1.0f, heightFactor));

            float tileHeight = heightFactor * peakHeight;

            if (tileHeight > 0.7f) {
                tile->setType(TileType::SNOW);
                tile->setHeight(tileHeight);
                tile->setElevation(0.9f + 0.1f * heightFactor);
            }
            else if (tileHeight > 0.4f) {
                tile->setType(TileType::ROCK_FORMATION);
                tile->setHeight(tileHeight);
                tile->setElevation(0.7f + 0.2f * heightFactor);
            }
            else if (tileHeight > 0.2f) {
                tile->setType(TileType::STONE);
                tile->setHeight(tileHeight);
                tile->setElevation(0.5f + 0.2f * heightFactor);
            }
            else {
                tile->setHeight(tileHeight);
                tile->setElevation(0.4f + 0.1f * heightFactor);
            }
        }
    }
}

// Создание кратера
void WorldGenerator::createCraterFormation(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating crater formation at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    float craterDepth = 0.3f + static_cast<float>(m_rng() % 20) / 100.0f;
    float rimHeight = 0.4f + static_cast<float>(m_rng() % 20) / 100.0f;

    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile) continue;

            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 1.0f) continue;

            float tileHeight = 0.0f;
            float elevation = 0.5f;

            if (distance < 0.7f) {
                tileHeight = -craterDepth * (1.0f - distance / 0.7f);
                elevation = 0.3f - 0.2f * (1.0f - distance / 0.7f);

                if (distance < 0.3f && m_rng() % 100 < 30) {
                    tile->setType(TileType::SHALLOW_WATER);
                    tile->setElevation(0.1f);
                    continue;
                }

                tile->setType(TileType::CRATER);
            }
            else if (distance < 0.9f) {
                float rimFactor = (distance - 0.7f) / 0.2f;
                float normalizedFactor = std::sin(rimFactor * 3.14159f);
                tileHeight = rimHeight * normalizedFactor;
                elevation = 0.5f + 0.2f * normalizedFactor;

                tile->setType(TileType::ROCK_FORMATION);
            }
            else {
                float outerFactor = (distance - 0.9f) / 0.1f;
                tileHeight = rimHeight * (1.0f - outerFactor);
                elevation = 0.5f + 0.1f * (1.0f - outerFactor);
            }

            std::uniform_real_distribution<float> noiseDist(-0.05f, 0.05f);
            tileHeight += noiseDist(m_rng);

            tile->setHeight(std::max(0.0f, tileHeight));
            tile->setElevation(std::max(0.0f, std::min(1.0f, elevation)));
        }
    }
}

// Создание каменного шпиля
void WorldGenerator::createRockSpire(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating rock spire at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    int spireCount = 2 + m_rng() % 5;

    for (int i = 0; i < spireCount; ++i) {
        std::uniform_int_distribution<int> offsetDist(-size / 2, size / 2);
        int spireX = centerX + offsetDist(m_rng);
        int spireY = centerY + offsetDist(m_rng);

        int spireSize = 1 + m_rng() % (size / 3);
        float spireHeight = 0.6f + static_cast<float>(m_rng() % 40) / 100.0f;

        for (int y = spireY - spireSize; y <= spireY + spireSize; ++y) {
            for (int x = spireX - spireSize; x <= spireX + spireSize; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                float dx = static_cast<float>(x - spireX) / spireSize;
                float dy = static_cast<float>(y - spireY) / spireSize;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance > 1.0f) continue;

                float heightFactor = std::pow(1.0f - distance, 3.0f);

                float tileHeight = heightFactor * spireHeight;

                if (tileHeight > 0.0f) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(tileHeight);
                    tile->setElevation(0.6f + 0.3f * heightFactor);

                    if (distance < 0.3f && m_rng() % 100 < 30) {
                        MapTile::Decoration spireDecor(
                            200 + (m_rng() % 10),
                            "RockSpireFormation",
                            0.8f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
                            false
                        );
                        tile->addDecoration(spireDecor);
                    }
                }
            }
        }
    }
}

// Создание древних руин
void WorldGenerator::createAncientRuins(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating ancient ruins at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    int plazaSize = size / 3;

    for (int y = centerY - plazaSize; y <= centerY + plazaSize; ++y) {
        for (int x = centerX - plazaSize; x <= centerX + plazaSize; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            tile->setType(TileType::RUINS);
            tile->setHeight(0.1f);

            if (m_rng() % 100 < 10) {
                MapTile::Decoration ruinDecor(
                    300 + (m_rng() % 5),
                    "AncientRuinsDecoration",
                    0.6f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
                    false
                );
                tile->addDecoration(ruinDecor);
            }
        }
    }

    int buildingCount = 4 + m_rng() % 5;

    for (int i = 0; i < buildingCount; ++i) {
        std::uniform_int_distribution<int> offsetDist(-size + plazaSize, size - plazaSize);
        int buildingX = centerX + offsetDist(m_rng);
        int buildingY = centerY + offsetDist(m_rng);

        int buildingSize = 1 + m_rng() % 3;

        for (int y = buildingY - buildingSize; y <= buildingY + buildingSize; ++y) {
            for (int x = buildingX - buildingSize; x <= buildingX + buildingSize; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                bool isWall = (x == buildingX - buildingSize || x == buildingX + buildingSize ||
                    y == buildingY - buildingSize || y == buildingY + buildingSize);

                if (isWall) {
                    if (m_rng() % 100 < 70) {
                        tile->setType(TileType::RUINS);
                        tile->setHeight(0.3f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f);

                        if (m_rng() % 100 < 30) {
                            MapTile::Decoration wallDecor(
                                310 + (m_rng() % 3),
                                "RuinWall",
                                0.7f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                                false
                            );
                            tile->addDecoration(wallDecor);
                        }
                    }
                    else {
                        tile->setType(TileType::RUINS);
                        tile->setHeight(0.1f);
                    }
                }
                else {
                    tile->setType(TileType::RUINS);
                    tile->setHeight(0.05f);

                    if (m_rng() % 100 < 15) {
                        MapTile::Decoration itemDecor(
                            320 + (m_rng() % 10),
                            "AncientArtifact",
                            0.5f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                            m_rng() % 100 < 20
                        );
                        tile->addDecoration(itemDecor);
                    }
                }
            }
        }
    }
}

// Создание странного монумента
void WorldGenerator::createStrangeMonument(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating strange monument at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    enum class MonumentType {
        OBELISK,
        CIRCLE,
        ARCH,
        PYRAMID
    };

    MonumentType type = static_cast<MonumentType>(m_rng() % 4);

    switch (type) {
    case MonumentType::OBELISK:
    {
        for (int y = centerY - 1; y <= centerY + 1; ++y) {
            for (int x = centerX - 1; x <= centerX + 1; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                if (x == centerX && y == centerY) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(1.0f);

                    MapTile::Decoration obeliskDecor(
                        400,
                        "StrangeObelisk",
                        1.5f,
                        m_rng() % 100 < 30
                    );
                    tile->addDecoration(obeliskDecor);
                }
                else {
                    tile->setType(TileType::STONE);
                    tile->setHeight(0.2f);
                }
            }
        }

        for (int y = centerY - size; y <= centerY + size; ++y) {
            for (int x = centerX - size; x <= centerX + size; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;
                if (std::abs(x - centerX) <= 1 && std::abs(y - centerY) <= 1) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                float dx = static_cast<float>(x - centerX) / size;
                float dy = static_cast<float>(y - centerY) / size;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance <= 1.0f) {
                    tile->setType(TileType::STONE);
                    tile->setHeight(0.05f);

                    if (m_rng() % 100 < 10 && distance > 0.3f) {
                        MapTile::Decoration symbolDecor(
                            410 + (m_rng() % 5),
                            "StrangeSymbol",
                            0.5f + 0.2f * static_cast<float>(m_rng() % 100) / 100.0f,
                            false
                        );
                        tile->addDecoration(symbolDecor);
                    }
                }
            }
        }
        break;
    }
    case MonumentType::CIRCLE:
    {
        for (int y = centerY - size; y <= centerY + size; ++y) {
            for (int x = centerX - size; x <= centerX + size; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                float dx = static_cast<float>(x - centerX) / size;
                float dy = static_cast<float>(y - centerY) / size;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance > 0.8f && distance < 1.0f) {
                    float angle = std::atan2(dy, dx);
                    float normAngle = angle / (2.0f * 3.14159f) + 0.5f;

                    int stoneCount = 8 + m_rng() % 5;
                    float stoneInterval = 1.0f / stoneCount;

                    float fractionalPart = normAngle - static_cast<int>(normAngle / stoneInterval) * stoneInterval;
                    if (fractionalPart < stoneInterval * 0.4f) {
                        tile->setType(TileType::ROCK_FORMATION);
                        tile->setHeight(0.4f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f);

                        MapTile::Decoration stoneDecor(
                            420 + (m_rng() % 3),
                            "StrangeStone",
                            1.0f + 0.5f * static_cast<float>(m_rng() % 100) / 100.0f,
                            false
                        );
                        tile->addDecoration(stoneDecor);
                    }
                }
                else if (distance < 0.7f) {
                    tile->setType(TileType::STONE);
                    tile->setHeight(0.05f);

                    if (distance < 0.2f) {
                        if (x == centerX && y == centerY) {
                            tile->setType(TileType::STONE);
                            tile->setHeight(0.2f);

                            MapTile::Decoration centerDecor(
                                430,
                                "StrangeAltar",
                                1.0f,
                                m_rng() % 100 < 50
                            );
                            tile->addDecoration(centerDecor);
                        }
                    }
                }
            }
        }
        break;
    }
    case MonumentType::ARCH:
    {
        int archWidth = size;
        int archHeight = size / 2;

        for (int y = centerY - archHeight; y <= centerY + archHeight; ++y) {
            int leftX = centerX - archWidth;

            if (leftX >= 0 && leftX < width && y >= 0 && y < height) {
                MapTile* tile = tileMap->getTile(leftX, y);
                if (tile && !tile->isWater()) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(0.7f);
                }
            }

            int rightX = centerX + archWidth;

            if (rightX >= 0 && rightX < width && y >= 0 && y < height) {
                MapTile* tile = tileMap->getTile(rightX, y);
                if (tile && !tile->isWater()) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(0.7f);
                }
            }
        }

        for (int x = centerX - archWidth + 1; x < centerX + archWidth; ++x) {
            if (x < 0 || x >= width) continue;

            int topY = centerY - archHeight;

            if (topY >= 0 && topY < height) {
                MapTile* tile = tileMap->getTile(x, topY);
                if (tile && !tile->isWater()) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(0.5f);

                    if (m_rng() % 100 < 30) {
                        MapTile::Decoration archDecor(
                            440 + (m_rng() % 2),
                            "ArchDecoration",
                            0.6f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
                            false
                        );
                        tile->addDecoration(archDecor);
                    }
                }
            }
        }
        break;
    }
    case MonumentType::PYRAMID:
    {
        int pyramidHeight = 5;

        for (int level = 0; level < pyramidHeight; ++level) {
            int levelSize = size - level;
            float tileHeight = 0.1f + 0.8f * static_cast<float>(level) / pyramidHeight;

            for (int y = centerY - levelSize; y <= centerY + levelSize; ++y) {
                for (int x = centerX - levelSize; x <= centerX + levelSize; ++x) {
                    bool onBorder = (x == centerX - levelSize || x == centerX + levelSize ||
                        y == centerY - levelSize || y == centerY + levelSize);

                    if (onBorder && x >= 0 && x < width && y >= 0 && y < height) {
                        MapTile* tile = tileMap->getTile(x, y);
                        if (tile && !tile->isWater()) {
                            tile->setType(TileType::STONE);
                            tile->setHeight(tileHeight);

                            if (level > 0 && m_rng() % 100 < 20) {
                                MapTile::Decoration pyramidDecor(
                                    450 + (m_rng() % 3),
                                    "PyramidSymbol",
                                    0.5f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                                    false
                                );
                                tile->addDecoration(pyramidDecor);
                            }
                        }
                    }
                }
            }
        }

        MapTile* topTile = tileMap->getTile(centerX, centerY);
        if (topTile && !topTile->isWater()) {
            topTile->setType(TileType::STONE);
            topTile->setHeight(1.0f);

            MapTile::Decoration topDecor(
                460,
                "PyramidTop",
                1.0f,
                m_rng() % 100 < 70
            );
            topTile->addDecoration(topDecor);
        }
        break;
    }
    }
}

// Создание оазиса
void WorldGenerator::createOasis(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating oasis at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    bool isSurroundingDesert = false;
    int desertCount = 0;
    int totalCount = 0;

    for (int y = centerY - size * 2; y <= centerY + size * 2; ++y) {
        for (int x = centerX - size * 2; x <= centerX + size * 2; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile) continue;

            totalCount++;
            if (tile->getType() == TileType::SAND) desertCount++;
        }
    }

    isSurroundingDesert = (totalCount > 0 && static_cast<float>(desertCount) / totalCount > 0.5f);

    if (!isSurroundingDesert) {
        for (int y = centerY - size * 2; y <= centerY + size * 2; ++y) {
            for (int x = centerX - size * 2; x <= centerX + size * 2; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                tile->setType(TileType::SAND);
                tile->setTemperature(tile->getTemperature() + 10.0f);
                tile->setHumidity(std::max(0.0f, tile->getHumidity() - 0.3f));
            }
        }
    }

    float pondSize = size * 0.6f;

    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile) continue;

            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 1.0f) continue;

            if (distance < pondSize / size) {
                tile->setType(TileType::WATER);
                tile->setHeight(0.1f);
                tile->setElevation(0.3f);
                tile->setHumidity(1.0f);
            }
            else if (distance < (pondSize + 0.2f) / size) {
                tile->setType(TileType::SHALLOW_WATER);
                tile->setHeight(0.05f);
                tile->setElevation(0.35f);
                tile->setHumidity(0.9f);
            }
            else {
                float humidityFactor = 1.0f - (distance - pondSize / size) / (1.0f - pondSize / size);

                tile->setType(TileType::GRASS);
                tile->setHeight(0.0f);
                tile->setElevation(0.4f);
                tile->setHumidity(0.5f + 0.4f * humidityFactor);

                if (m_rng() % 100 < 70 * humidityFactor) {
                    MapTile::Decoration vegDecor(
                        470 + (m_rng() % 5),
                        "OasisVegetation",
                        0.6f + 0.6f * static_cast<float>(m_rng() % 100) / 100.0f,
                        m_rng() % 100 < 30
                    );
                    tile->addDecoration(vegDecor);
                }
            }
        }
    }
}

// Создание поля гейзеров
void WorldGenerator::createGeyserField(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating geyser field at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    int geyserCount = 5 + m_rng() % 8;

    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 1.0f) continue;

            tile->setType(TileType::STONE);
            tile->setHeight(0.0f);

            tile->setTemperature(tile->getTemperature() + 20.0f * (1.0f - distance));

            if (m_rng() % 100 < 20 && distance > 0.3f) {
                MapTile::Decoration thermalDecor(
                    480 + (m_rng() % 3),
                    "ThermalVegetation",
                    0.4f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                    false
                );
                tile->addDecoration(thermalDecor);
            }
        }
    }

    for (int i = 0; i < geyserCount; ++i) {
        std::uniform_real_distribution<float> posDist(0.0f, 1.0f);
        float angle = posDist(m_rng) * 6.28318f;
        float radius = posDist(m_rng) * 0.9f * size;

        int geyserX = centerX + static_cast<int>(std::cos(angle) * radius);
        int geyserY = centerY + static_cast<int>(std::sin(angle) * radius);

        if (geyserX < 0 || geyserX >= width || geyserY < 0 || geyserY >= height) continue;

        MapTile* tile = tileMap->getTile(geyserX, geyserY);
        if (!tile || tile->isWater()) continue;

        tile->setType(TileType::LAVA);
        tile->setHeight(0.05f);

        MapTile::Decoration geyserDecor(
            490,
            "Geyser",
            1.0f + 0.5f * static_cast<float>(m_rng() % 100) / 100.0f,
            true
        );
        tile->addDecoration(geyserDecor);

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;

                int poolX = geyserX + dx;
                int poolY = geyserY + dy;

                if (poolX < 0 || poolX >= width || poolY < 0 || poolY >= height) continue;

                MapTile* poolTile = tileMap->getTile(poolX, poolY);
                if (!poolTile || poolTile->isWater()) continue;

                if (m_rng() % 100 < 70) {
                    poolTile->setType(TileType::SHALLOW_WATER);
                    poolTile->setHeight(0.02f);
                    poolTile->setTemperature(poolTile->getTemperature() + 30.0f);
                }
            }
        }
    }
}

// Создание кристаллической формации
void WorldGenerator::createCrystalFormation(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating crystal formation at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    int crystalCount = 15 + m_rng() % 10;

    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > 1.0f) continue;

            if (distance < 0.3f) {
                if (m_rng() % 100 < 80) {
                    tile->setType(TileType::MINERAL_DEPOSIT);
                    tile->setHeight(0.1f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f);
                    tile->setResourceDensity(0.8f + 0.2f * static_cast<float>(m_rng() % 100) / 100.0f);

                    MapTile::Decoration crystalDecor(
                        500 + (m_rng() % 5),
                        "LargeCrystal",
                        0.8f + 0.5f * static_cast<float>(m_rng() % 100) / 100.0f,
                        m_rng() % 100 < 40
                    );
                    tile->addDecoration(crystalDecor);
                }
            }
            else {
                tile->setType(TileType::STONE);
                tile->setHeight(0.0f);

                if (m_rng() % 100 < 30) {
                    SDL_Color stoneColor = tile->getColor();
                    stoneColor.r = static_cast<Uint8>(stoneColor.r * 0.7f + 0.3f * (m_rng() % 50 + 150));
                    stoneColor.g = static_cast<Uint8>(stoneColor.g * 0.7f + 0.3f * (m_rng() % 50 + 150));
                    stoneColor.b = static_cast<Uint8>(stoneColor.b * 0.7f + 0.3f * (m_rng() % 50 + 150));
                    tile->setColor(stoneColor);
                }
            }
        }
    }

    for (int i = 0; i < crystalCount; ++i) {
        std::uniform_real_distribution<float> distFactor(0.3f, 0.9f);
        float angle = static_cast<float>(m_rng() % 360) * 3.14159f / 180.0f;
        float distance = distFactor(m_rng) * size;

        int crystalX = centerX + static_cast<int>(std::cos(angle) * distance);
        int crystalY = centerY + static_cast<int>(std::sin(angle) * distance);

        if (crystalX < 0 || crystalX >= width || crystalY < 0 || crystalY >= height) continue;

        MapTile* tile = tileMap->getTile(crystalX, crystalY);
        if (!tile || tile->isWater()) continue;

        tile->setType(TileType::MINERAL_DEPOSIT);
        tile->setHeight(0.2f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f);
        tile->setResourceDensity(0.7f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f);

        MapTile::Decoration crystalDecor(
            510 + (m_rng() % 3),
            "SingleCrystal",
            0.9f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
            m_rng() % 100 < 20
        );
        tile->addDecoration(crystalDecor);

        SDL_Color crystalColor;
        int colorType = m_rng() % 5;
        switch (colorType) {
        case 0: crystalColor = { 50, 100, 255, 255 }; break;
        case 1: crystalColor = { 180, 50, 255, 255 }; break;
        case 2: crystalColor = { 50, 220, 100, 255 }; break;
        case 3: crystalColor = { 255, 50, 80, 255 }; break;
        case 4: crystalColor = { 255, 240, 40, 255 }; break;
        }
        tile->setColor(crystalColor);
    }
}

// Создание инопланетной структуры
void WorldGenerator::createAlienStructure(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating alien structure at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    enum class AlienStructureType {
        LANDING_PAD,
        RESEARCH_FACILITY,
        POWER_SOURCE,
        GEOMETRIC_ANOMALY
    };

    AlienStructureType type = static_cast<AlienStructureType>(m_rng() % 4);

    switch (type) {
    case AlienStructureType::LANDING_PAD:
    {
        int padRadius = size * 3 / 4;

        for (int y = centerY - padRadius; y <= centerY + padRadius; ++y) {
            for (int x = centerX - padRadius; x <= centerX + padRadius; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                float dx = static_cast<float>(x - centerX) / padRadius;
                float dy = static_cast<float>(y - centerY) / padRadius;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance <= 1.0f) {
                    tile->setType(TileType::METAL);
                    tile->setHeight(0.1f);

                    SDL_Color metalColor = { 180, 180, 200, 255 };
                    tile->setColor(metalColor);

                    if (m_rng() % 100 < 30) {
                        MapTile::Decoration padDecor(
                            520 + (m_rng() % 3),
                            "LandingPadPattern",
                            0.7f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                            m_rng() % 100 < 50
                        );
                        tile->addDecoration(padDecor);
                    }

                    if (distance < 0.3f) {
                        float angle = std::atan2(dy, dx);
                        float normAngle = angle / (2.0f * 3.14159f) + 0.5f;

                        int markerCount = 8;
                        float markerInterval = 1.0f / markerCount;

                        float fractionalPart = normAngle - static_cast<int>(normAngle / markerInterval) * markerInterval;
                        if (fractionalPart < markerInterval * 0.2f && distance > 0.1f) {
                            MapTile::Decoration markerDecor(
                                523,
                                "LandingMarker",
                                0.5f,
                                true
                            );
                            tile->addDecoration(markerDecor);
                        }
                    }
                }
            }
        }

        for (int y = centerY - 2; y <= centerY + 2; ++y) {
            for (int x = centerX - 2; x <= centerX + 2; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile) continue;

                if (std::abs(x - centerX) <= 1 && std::abs(y - centerY) <= 1) {
                    tile->setType(TileType::ALIEN_GROWTH);
                    tile->setHeight(0.4f);

                    if (x == centerX && y == centerY) {
                        MapTile::Decoration shipDecor(
                            530,
                            "AlienShip",
                            1.5f,
                            true
                        );
                        tile->addDecoration(shipDecor);
                    }
                }
            }
        }
        break;
    }
    case AlienStructureType::RESEARCH_FACILITY:
    {
        int mainSize = size / 2;

        for (int y = centerY - mainSize; y <= centerY + mainSize; ++y) {
            for (int x = centerX - mainSize; x <= centerX + mainSize; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                bool isWall = (x == centerX - mainSize || x == centerX + mainSize ||
                    y == centerY - mainSize || y == centerY + mainSize);

                if (isWall) {
                    tile->setType(TileType::METAL);
                    tile->setHeight(0.5f);

                    if (m_rng() % 100 < 30) {
                        MapTile::Decoration wallDecor(
                            540 + (m_rng() % 2),
                            "FacilityWall",
                            0.8f,
                            m_rng() % 100 < 20
                        );
                        tile->addDecoration(wallDecor);
                    }
                }
                else {
                    tile->setType(TileType::METAL);
                    tile->setHeight(0.1f);

                    if (m_rng() % 100 < 20) {
                        MapTile::Decoration equipDecor(
                            545 + (m_rng() % 5),
                            "AlienEquipment",
                            0.7f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
                            m_rng() % 100 < 60
                        );
                        tile->addDecoration(equipDecor);
                    }
                }
            }
        }

        int moduleCount = 2 + m_rng() % 3;

        for (int i = 0; i < moduleCount; ++i) {
            int direction = m_rng() % 4;
            int moduleSize = mainSize / 2;

            int moduleX = centerX;
            int moduleY = centerY;

            switch (direction) {
            case 0: moduleY -= mainSize + moduleSize; break;
            case 1: moduleX += mainSize + moduleSize; break;
            case 2: moduleY += mainSize + moduleSize; break;
            case 3: moduleX -= mainSize + moduleSize; break;
            }

            for (int y = moduleY - moduleSize; y <= moduleY + moduleSize; ++y) {
                for (int x = moduleX - moduleSize; x <= moduleX + moduleSize; ++x) {
                    if (x < 0 || x >= width || y < 0 || y >= height) continue;

                    MapTile* tile = tileMap->getTile(x, y);
                    if (!tile || tile->isWater()) continue;

                    bool isWall = (x == moduleX - moduleSize || x == moduleX + moduleSize ||
                        y == moduleY - moduleSize || y == moduleY + moduleSize);

                    if (isWall) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.4f);
                    }
                    else {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.1f);

                        if (m_rng() % 100 < 40) {
                            MapTile::Decoration moduleDecor(
                                550 + (m_rng() % 5),
                                "ModuleEquipment",
                                0.6f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
                                m_rng() % 100 < 50
                            );
                            tile->addDecoration(moduleDecor);
                        }
                    }
                }
            }

            int corridorX1, corridorY1, corridorX2, corridorY2;

            switch (direction) {
            case 0:
                corridorX1 = centerX;
                corridorY1 = centerY - mainSize;
                corridorX2 = moduleX;
                corridorY2 = moduleY + moduleSize;

                for (int y = corridorY2; y <= corridorY1; ++y) {
                    if (y < 0 || y >= height) continue;

                    MapTile* tile = tileMap->getTile(corridorX1, y);
                    if (tile && !tile->isWater()) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.2f);
                    }
                }
                break;

            case 1:
                corridorX1 = centerX + mainSize;
                corridorY1 = centerY;
                corridorX2 = moduleX - moduleSize;
                corridorY2 = moduleY;

                for (int x = corridorX1; x <= corridorX2; ++x) {
                    if (x < 0 || x >= width) continue;

                    MapTile* tile = tileMap->getTile(x, corridorY1);
                    if (tile && !tile->isWater()) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.2f);
                    }
                }
                break;

            case 2:
                corridorX1 = centerX;
                corridorY1 = centerY + mainSize;
                corridorX2 = moduleX;
                corridorY2 = moduleY - moduleSize;

                for (int y = corridorY1; y <= corridorY2; ++y) {
                    if (y < 0 || y >= height) continue;

                    MapTile* tile = tileMap->getTile(corridorX1, y);
                    if (tile && !tile->isWater()) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.2f);
                    }
                }
                break;

            case 3:
                corridorX1 = centerX - mainSize;
                corridorY1 = centerY;
                corridorX2 = moduleX + moduleSize;
                corridorY2 = moduleY;

                for (int x = corridorX2; x <= corridorX1; ++x) {
                    if (x < 0 || x >= width) continue;

                    MapTile* tile = tileMap->getTile(x, corridorY1);
                    if (tile && !tile->isWater()) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.2f);
                    }
                }
                break;
            }
        }
        break;
    }
    case AlienStructureType::POWER_SOURCE:
    {
        for (int y = centerY - 2; y <= centerY + 2; ++y) {
            for (int x = centerX - 2; x <= centerX + 2; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                float dx = static_cast<float>(x - centerX);
                float dy = static_cast<float>(y - centerY);
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance <= 2.0f) {
                    tile->setType(TileType::ALIEN_GROWTH);
                    tile->setHeight(0.3f);

                    if (distance <= 1.0f) {
                        tile->setHeight(0.5f);
                    }

                    if (x == centerX && y == centerY) {
                        tile->setHeight(0.7f);

                        MapTile::Decoration coreDecor(
                            560,
                            "EnergyCore",
                            1.2f,
                            true
                        );
                        tile->addDecoration(coreDecor);
                    }
                }
            }
        }

        int rayCount = (m_rng() % 100 < 50) ? 4 : 8;

        for (int i = 0; i < rayCount; ++i) {
            float angle = 2.0f * 3.14159f * i / rayCount;
            int rayLength = size - 2;

            for (int dist = 3; dist <= rayLength; ++dist) {
                int rayX = centerX + static_cast<int>(std::cos(angle) * dist);
                int rayY = centerY + static_cast<int>(std::sin(angle) * dist);

                if (rayX < 0 || rayX >= width || rayY < 0 || rayY >= height) continue;

                MapTile* tile = tileMap->getTile(rayX, rayY);
                if (!tile || tile->isWater()) continue;

                float heightFactor = 1.0f - static_cast<float>(dist) / rayLength;

                tile->setType(TileType::ALIEN_GROWTH);
                tile->setHeight(0.2f * heightFactor);

                MapTile::Decoration rayDecor(
                    565 + (i % 4),
                    "EnergyBeam",
                    0.7f + 0.3f * heightFactor,
                    true
                );
                tile->addDecoration(rayDecor);

                tile->setRadiationLevel(0.5f + 0.5f * heightFactor);
            }
        }
        break;
    }
    case AlienStructureType::GEOMETRIC_ANOMALY:
    {
        enum class GeometryType {
            SPIRAL,
            FRACTAL,
            SYMMETRIC
        };

        GeometryType geoType = static_cast<GeometryType>(m_rng() % 3);

        switch (geoType) {
        case GeometryType::SPIRAL:
        {
            float growthFactor = 0.2f + 0.1f * static_cast<float>(m_rng() % 6);
            float angleStep = 0.2f + 0.1f * static_cast<float>(m_rng() % 5);
            int numSteps = size * 4;

            for (int i = 0; i < numSteps; ++i) {
                float angle = angleStep * i;
                float radius = growthFactor * angle;

                int spiralX = centerX + static_cast<int>(std::cos(angle) * radius);
                int spiralY = centerY + static_cast<int>(std::sin(angle) * radius);

                if (spiralX < 0 || spiralX >= width || spiralY < 0 || spiralY >= height) continue;

                MapTile* tile = tileMap->getTile(spiralX, spiralY);
                if (!tile || tile->isWater()) continue;

                tile->setType(TileType::ALIEN_GROWTH);
                tile->setHeight(0.3f);

                float hue = fmod(angle * 30.0f, 360.0f);
                SDL_Color spiralColor;

                if (hue < 60.0f) {
                    spiralColor = { 255, static_cast<Uint8>(255 * hue / 60.0f), 0, 255 };
                }
                else if (hue < 120.0f) {
                    spiralColor = { static_cast<Uint8>(255 * (120.0f - hue) / 60.0f), 255, 0, 255 };
                }
                else if (hue < 180.0f) {
                    spiralColor = { 0, 255, static_cast<Uint8>(255 * (hue - 120.0f) / 60.0f), 255 };
                }
                else if (hue < 240.0f) {
                    spiralColor = { 0, static_cast<Uint8>(255 * (240.0f - hue) / 60.0f), 255, 255 };
                }
                else if (hue < 300.0f) {
                    spiralColor = { static_cast<Uint8>(255 * (hue - 240.0f) / 60.0f), 0, 255, 255 };
                }
                else {
                    spiralColor = { 255, 0, static_cast<Uint8>(255 * (360.0f - hue) / 60.0f), 255 };
                }

                tile->setColor(spiralColor);

                if (i % 5 == 0) {
                    MapTile::Decoration spiralDecor(
                        570 + (i % 5),
                        "SpiralNode",
                        0.5f + 0.5f * static_cast<float>(m_rng() % 100) / 100.0f,
                        true
                    );
                    tile->addDecoration(spiralDecor);
                }
            }
            break;
        }
        case GeometryType::FRACTAL:
        {
            std::function<void(int, int, int, int, int, int, int)> createFractal;
            createFractal = [&](int x1, int y1, int x2, int y2, int x3, int y3, int depth) {
                if (depth <= 0) return;

                std::vector<std::pair<int, int>> points = { {x1, y1}, {x2, y2}, {x3, y3} };

                for (size_t i = 0; i < points.size(); ++i) {
                    size_t j = (i + 1) % points.size();

                    int startX = points[i].first;
                    int startY = points[i].second;
                    int endX = points[j].first;
                    int endY = points[j].second;

                    int dx = std::abs(endX - startX);
                    int dy = std::abs(endY - startY);
                    int sx = startX < endX ? 1 : -1;
                    int sy = startY < endY ? 1 : -1;
                    int err = dx - dy;

                    int x = startX;
                    int y = startY;

                    while (true) {
                        if (x >= 0 && x < width && y >= 0 && y < height) {
                            MapTile* tile = tileMap->getTile(x, y);
                            if (tile && !tile->isWater()) {
                                tile->setType(TileType::ALIEN_GROWTH);
                                tile->setHeight(0.2f);

                                SDL_Color fractalColor;
                                switch (depth % 6) {
                                case 0: fractalColor = { 255, 50, 50, 255 }; break;
                                case 1: fractalColor = { 50, 255, 50, 255 }; break;
                                case 2: fractalColor = { 50, 50, 255, 255 }; break;
                                case 3: fractalColor = { 255, 255, 50, 255 }; break;
                                case 4: fractalColor = { 255, 50, 255, 255 }; break;
                                case 5: fractalColor = { 50, 255, 255, 255 }; break;
                                }
                                tile->setColor(fractalColor);
                            }
                        }

                        if (x == endX && y == endY) break;
                        int e2 = 2 * err;
                        if (e2 > -dy) {
                            err -= dy;
                            x += sx;
                        }
                        if (e2 < dx) {
                            err += dx;
                            y += sy;
                        }
                    }
                }

                // Рекурсивные вызовы для подтреугольников
                int mx1 = (x1 + x2) / 2;
                int my1 = (y1 + y2) / 2;
                int mx2 = (x2 + x3) / 2;
                int my2 = (y2 + y3) / 2;
                int mx3 = (x3 + x1) / 2;
                int my3 = (y3 + y1) / 2;

                createFractal(x1, y1, mx1, my1, mx3, my3, depth - 1);
                createFractal(mx1, my1, x2, y2, mx2, my2, depth - 1);
                createFractal(mx3, my3, mx2, my2, x3, y3, depth - 1);
                };

            // Создаем начальный треугольник
            int triangleSize = size;
            int x1 = centerX;
            int y1 = centerY - triangleSize;
            int x2 = centerX - triangleSize;
            int y2 = centerY + triangleSize;
            int x3 = centerX + triangleSize;
            int y3 = centerY + triangleSize;

            // Запускаем рекурсию
            int depth = 4; // Глубина рекурсии
            createFractal(x1, y1, x2, y2, x3, y3, depth);
            break;
        }
        case GeometryType::SYMMETRIC:
        {
            // Симметричная структура (мандала)
            int sectors = 8 + m_rng() % 8; // 8-15 секторов
            int rings = 3 + m_rng() % 4;   // 3-6 колец

            for (int ring = 1; ring <= rings; ++ring) {
                float radius = static_cast<float>(ring) * size / rings;

                for (int sector = 0; sector < sectors; ++sector) {
                    float startAngle = 2.0f * 3.14159f * sector / sectors;
                    float endAngle = 2.0f * 3.14159f * (sector + 1) / sectors;
                    float angleStep = 0.1f;

                    for (float angle = startAngle; angle < endAngle; angle += angleStep) {
                        int arcX = centerX + static_cast<int>(std::cos(angle) * radius);
                        int arcY = centerY + static_cast<int>(std::sin(angle) * radius);

                        if (arcX < 0 || arcX >= width || arcY < 0 || arcY >= height) continue;

                        MapTile* tile = tileMap->getTile(arcX, arcY);
                        if (!tile || tile->isWater()) continue;

                        tile->setType(TileType::ALIEN_GROWTH);
                        tile->setHeight(0.1f);

                        SDL_Color ringColor;
                        switch (ring % 6) {
                        case 0: ringColor = { 255, 100, 100, 255 }; break;
                        case 1: ringColor = { 100, 255, 100, 255 }; break;
                        case 2: ringColor = { 100, 100, 255, 255 }; break;
                        case 3: ringColor = { 255, 255, 100, 255 }; break;
                        case 4: ringColor = { 255, 100, 255, 255 }; break;
                        case 5: ringColor = { 100, 255, 255, 255 }; break;
                        }
                        tile->setColor(ringColor);
                    }

                    if (ring == rings) {
                        float midAngle = (startAngle + endAngle) / 2.0f;

                        for (float dist = 0; dist <= radius; dist += 0.5f) {
                            int lineX = centerX + static_cast<int>(std::cos(midAngle) * dist);
                            int lineY = centerY + static_cast<int>(std::sin(midAngle) * dist);

                            if (lineX < 0 || lineX >= width || lineY < 0 || lineY >= height) continue;

                            MapTile* tile = tileMap->getTile(lineX, lineY);
                            if (!tile || tile->isWater()) continue;

                            tile->setType(TileType::ALIEN_GROWTH);
                            tile->setHeight(0.2f);

                            if (static_cast<int>(dist) % 2 == 0) {
                                MapTile::Decoration lineDecor(
                                    580 + (sector % 5),
                                    "SymmetryNode",
                                    0.4f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                                    true
                                );
                                tile->addDecoration(lineDecor);
                            }
                        }
                    }
                }
            }

            MapTile* centerTile = tileMap->getTile(centerX, centerY);
            if (centerTile && !centerTile->isWater()) {
                centerTile->setType(TileType::ALIEN_GROWTH);
                centerTile->setHeight(0.4f);

                MapTile::Decoration centerDecor(
                    590,
                    "SymmetryCenter",
                    1.2f,
                    true
                );
                centerTile->addDecoration(centerDecor);
            }
            break;
        }
    }
    break;
}
}
}

// Остальные методы WorldGenerator

void WorldGenerator::setupPlanetParameters(MapGenerator::GenerationType terrainType, PlanetData& planetData) {
    // Пока пустая реализация, можно расширить позже
}

void WorldGenerator::setupDefaultBiomes() {
    m_biomes.push_back(std::make_shared<Biome>(0, "Plains", SDL_Color{ 100, 200, 100, 255 }, "A temperate grassy area."));
    auto desert = std::make_shared<Biome>(1, "Desert", SDL_Color{ 200, 200, 100, 255 }, "A dry desert with high temperatures and low humidity.");
}

bool WorldGenerator::setupBiomesForPlanet(const PlanetData& planetData) {
    clearBiomes();
    setupDefaultBiomes();
    return true;
}

void WorldGenerator::createUniqueBiomes(const PlanetData& planetData, int count) {
    // Пока пустая реализация
}

std::string WorldGenerator::generatePlanetName() {
    return "Planet_" + std::to_string(m_seed); // Простое имя для примера
}

std::string WorldGenerator::generatePlanetDescription(const PlanetData& planetData) {
    return "A planet with temperature " + std::to_string(planetData.averageTemperature) + "°C.";
}

void WorldGenerator::setSeed(unsigned int seed) {
    m_seed = seed ? seed : static_cast<unsigned int>(std::time(nullptr));
    m_rng.seed(m_seed);
}

void WorldGenerator::addBiome(std::shared_ptr<Biome> biome) {
    m_biomes.push_back(biome);
}

void WorldGenerator::clearBiomes() {
    m_biomes.clear();
}

void WorldGenerator::applyPlanetaryFeatures(TileMap* tileMap, const PlanetData& planetData) {
    if (tileMap) {
        for (int y = 0; y < tileMap->getHeight(); ++y) {
            for (int x = 0; x < tileMap->getWidth(); ++x) {
                MapTile* tile = tileMap->getTile(x, y);
                if (tile) {
                    tile->setTemperature(planetData.averageTemperature);
                    tile->setHumidity(planetData.waterCoverage);
                }
            }
        }
    }
}

bool WorldGenerator::generateRegion(TileMap* tileMap, const RegionData& regionData, const PlanetData& planetData) {
    // Заглушка для метода
    return false;
}

void WorldGenerator::applyRegionalFeatures(TileMap* tileMap, const RegionData& regionData, const PlanetData& planetData) {
    // Пока пустая реализация
}

std::string WorldGenerator::generateThematicName(int theme) {
    return "ThematicPlanet_" + std::to_string(theme); // Простая заглушка
}