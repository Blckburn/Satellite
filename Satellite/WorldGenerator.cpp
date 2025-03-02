#include "WorldGenerator.h"
#include "Logger.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <functional>
#include <SDL_image.h>

// Добавляем новые методы в класс WorldGenerator

bool WorldGenerator::generatePlanetWithCache(TileMap* tileMap, const PlanetData& planetData) {
    if (!tileMap) return false;

    // Проверяем, генерировали ли мы уже такую же планету
    // (проверка по ключевым параметрам)
    if (m_lastPlanetData.seed == planetData.seed &&
        m_lastPlanetData.mainTerrainType == planetData.mainTerrainType &&
        std::abs(m_lastPlanetData.averageTemperature - planetData.averageTemperature) < 0.001f &&
        std::abs(m_lastPlanetData.waterCoverage - planetData.waterCoverage) < 0.001f) {

        LOG_DEBUG("Using cached planet with seed: " + std::to_string(planetData.seed));

        // Если карта того же размера, просто копируем данные из кэша
        if (m_cachedTileMap && m_cachedTileMap->getWidth() == tileMap->getWidth() &&
            m_cachedTileMap->getHeight() == tileMap->getHeight()) {

            // Копируем тайлы из кэша
            for (int y = 0; y < tileMap->getHeight(); ++y) {
                for (int x = 0; x < tileMap->getWidth(); ++x) {
                    MapTile* srcTile = m_cachedTileMap->getTile(x, y);
                    MapTile* dstTile = tileMap->getTile(x, y);

                    if (srcTile && dstTile) {
                        *dstTile = *srcTile; // Копируем данные тайла
                    }
                }
            }

            return true;
        }
    }

    // Если нет в кэше, генерируем новую планету
    LOG_DEBUG("Generating new planet with seed: " + std::to_string(planetData.seed));

    // Вызываем обычный метод генерации
    bool result = generatePlanet(tileMap, planetData);

    // Если успешно, обновляем кэш
    if (result) {
        m_lastPlanetData = planetData;

        // Если кэш еще не создан или имеет неправильный размер, создаем новый
        if (!m_cachedTileMap || m_cachedTileMap->getWidth() != tileMap->getWidth() ||
            m_cachedTileMap->getHeight() != tileMap->getHeight()) {

            m_cachedTileMap = std::make_shared<TileMap>(tileMap->getWidth(), tileMap->getHeight());
            m_cachedTileMap->initialize();
        }

        // Копируем тайлы в кэш
        for (int y = 0; y < tileMap->getHeight(); ++y) {
            for (int x = 0; x < tileMap->getWidth(); ++x) {
                MapTile* srcTile = tileMap->getTile(x, y);
                MapTile* dstTile = m_cachedTileMap->getTile(x, y);

                if (srcTile && dstTile) {
                    *dstTile = *srcTile; // Копируем данные тайла
                }
            }
        }
    }

    return result;
}

void WorldGenerator::createErosionPatterns(TileMap* tileMap, float intensity) {
    if (!tileMap) return;

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем временную карту высот для работы с эрозией
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));

    // Копируем текущие высоты из тайлов
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                heightMap[y][x] = tile->getElevation();
            }
        }
    }

    // Количество капель воды для симуляции
    int dropCount = static_cast<int>(width * height * 0.05f * intensity);

    // Параметры симуляции
    const float inertia = 0.1f;         // Инерция капли воды
    const float erosionFactor = 0.3f;   // Коэффициент эрозии
    const float depositionFactor = 0.3f;// Коэффициент отложения
    const float evaporationFactor = 0.02f; // Коэффициент испарения
    const int maxSteps = 100;           // Максимальное количество шагов для одной капли

    std::uniform_int_distribution<int> dist_x(0, width - 1);
    std::uniform_int_distribution<int> dist_y(0, height - 1);
    std::uniform_real_distribution<float> dist_01(0.0f, 1.0f);

    LOG_INFO("Creating erosion patterns with " + std::to_string(dropCount) + " water drops");

    // Симуляция капель воды
    for (int d = 0; d < dropCount; ++d) {
        // Начальная позиция капли
        int x = dist_x(m_rng);
        int y = dist_y(m_rng);

        // Проверяем, что начальная позиция не вода
        MapTile* tile = tileMap->getTile(x, y);
        if (!tile || tile->isWater()) continue;

        // Параметры капли
        float posX = static_cast<float>(x);
        float posY = static_cast<float>(y);
        float dirX = 0.0f;
        float dirY = 0.0f;
        float speed = 0.0f;
        float water = 1.0f;
        float sediment = 0.0f;

        // Симуляция движения капли
        for (int step = 0; step < maxSteps; ++step) {
            // Текущая целочисленная позиция
            int cellX = std::min(width - 1, std::max(0, static_cast<int>(posX)));
            int cellY = std::min(height - 1, std::max(0, static_cast<int>(posY)));

            // Вычисляем градиент (направление спуска)
            // Проверяем соседние клетки и ищем ту, где высота ниже всего
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

            // Если нет направления вниз, капля останавливается
            if (bestX == cellX && bestY == cellY) {
                break;
            }

            // Обновляем направление с учетом инерции
            float newDirX = static_cast<float>(bestX - cellX);
            float newDirY = static_cast<float>(bestY - cellY);

            // Нормализуем направление
            float length = std::sqrt(newDirX * newDirX + newDirY * newDirY);
            if (length > 0) {
                newDirX /= length;
                newDirY /= length;
            }

            // Применяем инерцию
            dirX = dirX * inertia + newDirX * (1 - inertia);
            dirY = dirY * inertia + newDirY * (1 - inertia);

            // Нормализуем направление снова
            length = std::sqrt(dirX * dirX + dirY * dirY);
            if (length > 0) {
                dirX /= length;
                dirY /= length;
            }

            // Перемещаем каплю
            float newPosX = posX + dirX;
            float newPosY = posY + dirY;

            // Проверяем, что новая позиция в пределах карты
            if (newPosX < 0 || newPosX >= width || newPosY < 0 || newPosY >= height) {
                break;
            }

            // Разница высот
            int newCellX = static_cast<int>(newPosX);
            int newCellY = static_cast<int>(newPosY);

            // Если вышли за границы карты, останавливаемся
            if (newCellX < 0 || newCellX >= width || newCellY < 0 || newCellY >= height) {
                break;
            }

            float heightDiff = heightMap[cellY][cellX] - heightMap[newCellY][newCellX];

            // Обновляем скорость и количество переносимого материала
            speed = std::sqrt(speed * speed + heightDiff);

            // Эрозия: больше скорость - больше эрозия
            float erosion = speed * water * erosionFactor;

            // Если капля движется вверх, отложение больше
            if (heightDiff < 0) {
                erosion *= 0.1f; // Уменьшаем эрозию при движении вверх
            }

            // Ограничиваем эрозию
            erosion = std::min(erosion, heightMap[cellY][cellX] * 0.1f);

            // Применяем эрозию
            heightMap[cellY][cellX] -= erosion;
            sediment += erosion;

            // Отложение: если скорость уменьшается, часть материала откладывается
            float deposition = sediment * depositionFactor;
            heightMap[newCellY][newCellX] += deposition;
            sediment -= deposition;

            // Испарение воды
            water *= (1.0f - evaporationFactor);

            // Перемещаем каплю
            posX = newPosX;
            posY = newPosY;

            // Если вода испарилась, капля останавливается
            if (water < 0.01f) {
                break;
            }
        }
    }

    // Применяем обновленную карту высот к тайлам
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile && !tile->isWater()) {
                // Обновляем высоту тайла
                tile->setElevation(heightMap[y][x]);

                // Если высота ниже уровня воды, превращаем в воду
                if (heightMap[y][x] < m_mapGenerator->getWaterLevel()) {
                    tile->setType(TileType::WATER);
                    tile->setHeight(0.1f);
                }
                else if (heightMap[y][x] < m_mapGenerator->getWaterLevel() + 0.05f) {
                    // Берега
                    tile->setType(TileType::SHALLOW_WATER);
                    tile->setHeight(0.05f);
                }
            }
        }
    }

    LOG_INFO("Erosion patterns created successfully");
}

void WorldGenerator::enhanceBiomeTransitions(TileMap* tileMap) {
    if (!tileMap) return;

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем временную копию карты биомов
    std::vector<std::vector<int>> biomeCopy(height, std::vector<int>(width, 0));

    // Копируем текущие биомы
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                biomeCopy[y][x] = tile->getBiomeId();
            }
        }
    }

    LOG_INFO("Enhancing biome transitions");

    // Проходим по всем тайлам и улучшаем переходы между биомами
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            // Получаем текущий биом
            int currentBiome = biomeCopy[y][x];

            // Подсчитываем количество соседей с другими биомами
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

                        // Проверяем, находится ли тайл на границе двух биомов
                        if (neighborBiome != currentBiome) {
                            onBorder = true;
                        }
                    }
                }
            }

            // Если тайл находится на границе биомов, создаем плавный переход
            if (onBorder) {
                // Находим самый распространенный соседний биом (кроме текущего)
                int mostCommonBiome = currentBiome;
                int maxCount = 0;

                for (const auto& [biomeId, count] : biomeCount) {
                    if (biomeId != currentBiome && count > maxCount) {
                        mostCommonBiome = biomeId;
                        maxCount = count;
                    }
                }

                // Если нашли другой биом, создаем переходный тайл
                if (mostCommonBiome != currentBiome) {
                    // Ищем биом с указанным ID
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

                    // Если нашли оба биома, создаем переходный тайл
                    if (currentBiomeObj && neighborBiomeObj) {
                        // Для переходных тайлов выбираем тип тайла из соседнего биома
                        // с некоторой вероятностью
                        std::uniform_real_distribution<float> transitionDist(0.0f, 1.0f);
                        float transitionRoll = transitionDist(m_rng);

                        if (transitionRoll < 0.3f) {
                            // 30% шанс взять тип тайла из соседнего биома
                            TileType newType = neighborBiomeObj->getRandomTileType();
                            tile->setType(newType);

                            // Смешиваем цвета для создания плавного перехода
                            SDL_Color currentColor = tile->getColor();
                            SDL_Color newColor = { currentColor.r, currentColor.g, currentColor.b, currentColor.a };

                            // Получаем тайл из соседнего биома для образца цвета
                            MapTile sampleTile(newType);
                            SDL_Color neighborColor = sampleTile.getColor();

                            // Смешиваем цвета (70% текущего, 30% соседнего)
                            newColor.r = static_cast<Uint8>(currentColor.r * 0.7f + neighborColor.r * 0.3f);
                            newColor.g = static_cast<Uint8>(currentColor.g * 0.7f + neighborColor.g * 0.3f);
                            newColor.b = static_cast<Uint8>(currentColor.b * 0.7f + neighborColor.b * 0.3f);

                            tile->setColor(newColor);
                        }
                        else if (transitionRoll < 0.5f) {
                            // 20% шанс добавить декорацию из соседнего биома
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

void WorldGenerator::createLandmarks(TileMap* tileMap, int count) {
    if (!tileMap) return;

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Гарантируем минимальное количество достопримечательностей
    count = std::max(1, count);

    LOG_INFO("Creating " + std::to_string(count) + " landmarks");

    // Типы достопримечательностей
    enum class LandmarkType {
        MOUNTAIN_PEAK,      // Высокая одиночная гора
        CRATER_FORMATION,   // Большой ударный кратер
        ROCK_SPIRE,         // Каменный шпиль или группа шпилей
        ANCIENT_RUINS,      // Древние руины
        STRANGE_MONUMENT,   // Странный инопланетный монумент
        OASIS,              // Оазис среди пустыни
        GEYSER_FIELD,       // Поле гейзеров
        CRYSTAL_FORMATION,  // Кристаллические образования
        ALIEN_STRUCTURE     // Инопланетная структура
    };

    std::uniform_int_distribution<int> typeRange(0, 8); // Диапазон для LandmarkType
    std::uniform_int_distribution<int> xDist(width / 10, width * 9 / 10); // Не по краям
    std::uniform_int_distribution<int> yDist(height / 10, height * 9 / 10); // Не по краям
    std::uniform_int_distribution<int> sizeRange(5, 15); // Размер достопримечательности

    // Создаем достопримечательности
    for (int i = 0; i < count; ++i) {
        LandmarkType type = static_cast<LandmarkType>(typeRange(m_rng));
        int centerX = xDist(m_rng);
        int centerY = yDist(m_rng);
        int size = sizeRange(m_rng);

        // Проверяем, что центральный тайл не вода
        MapTile* centerTile = tileMap->getTile(centerX, centerY);
        if (!centerTile || centerTile->isWater()) {
            // Если центр - вода, пропускаем (кроме некоторых типов)
            if (type != LandmarkType::OASIS) {
                continue;
            }
        }

        // Создаем достопримечательность в зависимости от типа
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

// Вспомогательные методы для создания различных достопримечательностей

void WorldGenerator::createMountainPeak(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating mountain peak at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Высота пика
    float peakHeight = 0.9f + static_cast<float>(m_rng() % 20) / 100.0f; // 0.9-1.1

    // Форма горы - экспоненциальное уменьшение высоты от центра
    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            // Расстояние от центра
            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Если выходит за размер, пропускаем
            if (distance > 1.0f) continue;

            // Высота в зависимости от расстояния (функция убывания)
            // distance = 0 -> 1.0, distance = 1 -> 0.0
            float heightFactor = std::pow(1.0f - distance, 2.0f);

            // Добавляем шум для естественности
            std::uniform_real_distribution<float> noiseDist(-0.1f, 0.1f);
            heightFactor += noiseDist(m_rng);
            heightFactor = std::max(0.0f, std::min(1.0f, heightFactor));

            // Итоговая высота
            float tileHeight = heightFactor * peakHeight;

            // Устанавливаем тип тайла и высоту
            if (tileHeight > 0.7f) {
                // Вершина горы (снежная)
                tile->setType(TileType::SNOW);
                tile->setHeight(tileHeight);
                tile->setElevation(0.9f + 0.1f * heightFactor);
            }
            else if (tileHeight > 0.4f) {
                // Скалистая часть
                tile->setType(TileType::ROCK_FORMATION);
                tile->setHeight(tileHeight);
                tile->setElevation(0.7f + 0.2f * heightFactor);
            }
            else if (tileHeight > 0.2f) {
                // Каменистая часть
                tile->setType(TileType::STONE);
                tile->setHeight(tileHeight);
                tile->setElevation(0.5f + 0.2f * heightFactor);
            }
            else {
                // Подножие (оставляем текущий тип)
                tile->setHeight(tileHeight);
                tile->setElevation(0.4f + 0.1f * heightFactor);
            }
        }
    }
}

void WorldGenerator::createCraterFormation(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating crater formation at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Глубина кратера
    float craterDepth = 0.3f + static_cast<float>(m_rng() % 20) / 100.0f; // 0.3-0.5

    // Высота вала кратера
    float rimHeight = 0.4f + static_cast<float>(m_rng() % 20) / 100.0f; // 0.4-0.6

    // Форма кратера - круговая впадина с валом по краям
    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile) continue;

            // Расстояние от центра
            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Если выходит за размер, пропускаем
            if (distance > 1.0f) continue;

            // Вычисляем высоту в зависимости от расстояния
            float tileHeight = 0.0f;
            float elevation = 0.5f; // Базовая высота рельефа

            if (distance < 0.7f) {
                // Внутренняя часть кратера - впадина
                tileHeight = -craterDepth * (1.0f - distance / 0.7f);
                elevation = 0.3f - 0.2f * (1.0f - distance / 0.7f);

                // Дно кратера может быть с водой
                if (distance < 0.3f && m_rng() % 100 < 30) {
                    tile->setType(TileType::SHALLOW_WATER);
                    tile->setElevation(0.1f);
                    continue;
                }

                // Тип тайла для кратера
                tile->setType(TileType::CRATER);
            }
            else if (distance < 0.9f) {
                // Вал кратера - возвышенность
                float rimFactor = (distance - 0.7f) / 0.2f; // 0.0-1.0 в пределах вала
                float normalizedFactor = std::sin(rimFactor * 3.14159f); // Форма вала
                tileHeight = rimHeight * normalizedFactor;
                elevation = 0.5f + 0.2f * normalizedFactor;

                // Тип тайла для вала
                tile->setType(TileType::ROCK_FORMATION);
            }
            else {
                // Внешняя часть - плавное снижение
                float outerFactor = (distance - 0.9f) / 0.1f; // 0.0-1.0 во внешней части
                tileHeight = rimHeight * (1.0f - outerFactor);
                elevation = 0.5f + 0.1f * (1.0f - outerFactor);

                // Оставляем текущий тип тайла
            }

            // Добавляем шум для естественности
            std::uniform_real_distribution<float> noiseDist(-0.05f, 0.05f);
            tileHeight += noiseDist(m_rng);

            // Устанавливаем высоту тайла
            tile->setHeight(std::max(0.0f, tileHeight));
            tile->setElevation(std::max(0.0f, std::min(1.0f, elevation)));
        }
    }
}

void WorldGenerator::createRockSpire(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating rock spire at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Количество шпилей
    int spireCount = 2 + m_rng() % 5; // 2-6 шпилей

    for (int i = 0; i < spireCount; ++i) {
        // Положение шпиля (случайное смещение от центра)
        std::uniform_int_distribution<int> offsetDist(-size / 2, size / 2);
        int spireX = centerX + offsetDist(m_rng);
        int spireY = centerY + offsetDist(m_rng);

        // Размер и высота шпиля
        int spireSize = 1 + m_rng() % (size / 3);
        float spireHeight = 0.6f + static_cast<float>(m_rng() % 40) / 100.0f; // 0.6-1.0

        // Создаем шпиль
        for (int y = spireY - spireSize; y <= spireY + spireSize; ++y) {
            for (int x = spireX - spireSize; x <= spireX + spireSize; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Расстояние от центра шпиля
                float dx = static_cast<float>(x - spireX) / spireSize;
                float dy = static_cast<float>(y - spireY) / spireSize;
                float distance = std::sqrt(dx * dx + dy * dy);

                // Если выходит за размер шпиля, пропускаем
                if (distance > 1.0f) continue;

                // Высота в зависимости от расстояния (резкое падение от центра)
                float heightFactor = std::pow(1.0f - distance, 3.0f);

                // Итоговая высота
                float tileHeight = heightFactor * spireHeight;

                // Устанавливаем тип тайла и высоту
                if (tileHeight > 0.0f) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(tileHeight);
                    tile->setElevation(0.6f + 0.3f * heightFactor);

                    // Добавляем декорацию для шпиля
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

void WorldGenerator::createAncientRuins(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating ancient ruins at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Форма руин - разбросанные структуры с центральной площадью

    // Сначала создаем центральную площадь
    int plazaSize = size / 3;

    for (int y = centerY - plazaSize; y <= centerY + plazaSize; ++y) {
        for (int x = centerX - plazaSize; x <= centerX + plazaSize; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            // Центральная площадь
            tile->setType(TileType::RUINS);
            tile->setHeight(0.1f);

            // Редкие декоративные элементы
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

    // Затем добавляем руины зданий
    int buildingCount = 4 + m_rng() % 5; // 4-8 зданий

    for (int i = 0; i < buildingCount; ++i) {
        // Положение здания
        std::uniform_int_distribution<int> offsetDist(-size + plazaSize, size - plazaSize);
        int buildingX = centerX + offsetDist(m_rng);
        int buildingY = centerY + offsetDist(m_rng);

        // Размер здания
        int buildingSize = 1 + m_rng() % 3;

        // Создаем руины здания
        for (int y = buildingY - buildingSize; y <= buildingY + buildingSize; ++y) {
            for (int x = buildingX - buildingSize; x <= buildingX + buildingSize; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Стены руин (по периметру)
                bool isWall = (x == buildingX - buildingSize || x == buildingX + buildingSize ||
                    y == buildingY - buildingSize || y == buildingY + buildingSize);

                if (isWall) {
                    // Стены не всегда целые
                    if (m_rng() % 100 < 70) {
                        tile->setType(TileType::RUINS);
                        tile->setHeight(0.3f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f);

                        // Декорации для стен
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
                        // Разрушенная стена
                        tile->setType(TileType::RUINS);
                        tile->setHeight(0.1f);
                    }
                }
                else {
                    // Пол внутри здания
                    tile->setType(TileType::RUINS);
                    tile->setHeight(0.05f);

                    // Редкие предметы внутри
                    if (m_rng() % 100 < 15) {
                        MapTile::Decoration itemDecor(
                            320 + (m_rng() % 10),
                            "AncientArtifact",
                            0.5f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                            m_rng() % 100 < 20 // 20% шанс анимации
                        );
                        tile->addDecoration(itemDecor);
                    }
                }
            }
        }
    }
}

void WorldGenerator::createStrangeMonument(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating strange monument at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Тип монумента (обелиск, круг, арка и т.д.)
    enum class MonumentType {
        OBELISK,   // Одиночный высокий обелиск
        CIRCLE,    // Круг из камней
        ARCH,      // Арочная структура
        PYRAMID    // Пирамидальная структура
    };

    MonumentType type = static_cast<MonumentType>(m_rng() % 4);

    // Создаем монумент в зависимости от типа
    switch (type) {
    case MonumentType::OBELISK:
    {
        // Обелиск - высокая тонкая конструкция в центре
        for (int y = centerY - 1; y <= centerY + 1; ++y) {
            for (int x = centerX - 1; x <= centerX + 1; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Центральный тайл - самый высокий
                if (x == centerX && y == centerY) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(1.0f);

                    // Декорация для обелиска
                    MapTile::Decoration obeliskDecor(
                        400,
                        "StrangeObelisk",
                        1.5f,
                        m_rng() % 100 < 30 // 30% шанс анимации
                    );
                    tile->addDecoration(obeliskDecor);
                }
                else {
                    // Основание обелиска
                    tile->setType(TileType::STONE);
                    tile->setHeight(0.2f);
                }
            }
        }

        // Площадка вокруг обелиска
        for (int y = centerY - size; y <= centerY + size; ++y) {
            for (int x = centerX - size; x <= centerX + size; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;
                if (std::abs(x - centerX) <= 1 && std::abs(y - centerY) <= 1) continue; // Пропускаем центр

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Расстояние от центра
                float dx = static_cast<float>(x - centerX) / size;
                float dy = static_cast<float>(y - centerY) / size;
                float distance = std::sqrt(dx * dx + dy * dy);

                // Если в пределах размера, создаем площадку
                if (distance <= 1.0f) {
                    tile->setType(TileType::STONE);
                    tile->setHeight(0.05f);

                    // Редкие символы на площадке
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
        // Круг из камней
        for (int y = centerY - size; y <= centerY + size; ++y) {
            for (int x = centerX - size; x <= centerX + size; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Расстояние от центра
                float dx = static_cast<float>(x - centerX) / size;
                float dy = static_cast<float>(y - centerY) / size;
                float distance = std::sqrt(dx * dx + dy * dy);

                // Камни по кругу
                if (distance > 0.8f && distance < 1.0f) {
                    // Камни ставим не везде, а с некоторым интервалом
                    float angle = std::atan2(dy, dx);
                    float normAngle = angle / (2.0f * 3.14159f) + 0.5f; // 0.0-1.0

                    // Количество камней
                    int stoneCount = 8 + m_rng() % 5; // 8-12 камней
                    float stoneInterval = 1.0f / stoneCount;

                    // Проверяем, нужно ли ставить камень в данной позиции
                    float fractionalPart = normAngle - static_cast<int>(normAngle / stoneInterval) * stoneInterval;
                    if (fractionalPart < stoneInterval * 0.4f) {
                        tile->setType(TileType::ROCK_FORMATION);
                        tile->setHeight(0.4f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f);

                        // Декорация для камня
                        MapTile::Decoration stoneDecor(
                            420 + (m_rng() % 3),
                            "StrangeStone",
                            1.0f + 0.5f * static_cast<float>(m_rng() % 100) / 100.0f,
                            false
                        );
                        tile->addDecoration(stoneDecor);
                    }
                }
                // Центральная площадка
                else if (distance < 0.7f) {
                    tile->setType(TileType::STONE);
                    tile->setHeight(0.05f);

                    // Центральный алтарь или объект
                    if (distance < 0.2f) {
                        if (x == centerX && y == centerY) {
                            tile->setType(TileType::STONE);
                            tile->setHeight(0.2f);

                            // Декорация для центрального объекта
                            MapTile::Decoration centerDecor(
                                430,
                                "StrangeAltar",
                                1.0f,
                                m_rng() % 100 < 50 // 50% шанс анимации
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
        // Арочная структура - вертикальные опоры и горизонтальная перекладина

        // Размер арки
        int archWidth = size;
        int archHeight = size / 2;

        // Опоры
        for (int y = centerY - archHeight; y <= centerY + archHeight; ++y) {
            // Левая опора
            int leftX = centerX - archWidth;

            if (leftX >= 0 && leftX < width && y >= 0 && y < height) {
                MapTile* tile = tileMap->getTile(leftX, y);
                if (tile && !tile->isWater()) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(0.7f);
                }
            }

            // Правая опора
            int rightX = centerX + archWidth;

            if (rightX >= 0 && rightX < width && y >= 0 && y < height) {
                MapTile* tile = tileMap->getTile(rightX, y);
                if (tile && !tile->isWater()) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(0.7f);
                }
            }
        }

        // Перекладина
        for (int x = centerX - archWidth + 1; x < centerX + archWidth; ++x) {
            if (x < 0 || x >= width) continue;

            // Верхняя граница арки - полукруг
            int topY = centerY - archHeight;

            if (topY >= 0 && topY < height) {
                MapTile* tile = tileMap->getTile(x, topY);
                if (tile && !tile->isWater()) {
                    tile->setType(TileType::ROCK_FORMATION);
                    tile->setHeight(0.5f);

                    // Декорация для перекладины
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
        // Пирамидальная структура - уменьшающиеся квадраты снизу вверх

        // Высота пирамиды
        int pyramidHeight = 5;

        // Создаем уровни пирамиды
        for (int level = 0; level < pyramidHeight; ++level) {
            // Размер текущего уровня
            int levelSize = size - level;

            // Высота тайлов на этом уровне
            float tileHeight = 0.1f + 0.8f * static_cast<float>(level) / pyramidHeight;

            // Создаем квадрат для текущего уровня
            for (int y = centerY - levelSize; y <= centerY + levelSize; ++y) {
                for (int x = centerX - levelSize; x <= centerX + levelSize; ++x) {
                    // Если точка на границе текущего уровня
                    bool onBorder = (x == centerX - levelSize || x == centerX + levelSize ||
                        y == centerY - levelSize || y == centerY + levelSize);

                    if (onBorder && x >= 0 && x < width && y >= 0 && y < height) {
                        MapTile* tile = tileMap->getTile(x, y);
                        if (tile && !tile->isWater()) {
                            tile->setType(TileType::STONE);
                            tile->setHeight(tileHeight);

                            // Декорация для пирамиды
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

        // Вершина пирамиды
        MapTile* topTile = tileMap->getTile(centerX, centerY);
        if (topTile && !topTile->isWater()) {
            topTile->setType(TileType::STONE);
            topTile->setHeight(1.0f);

            // Декорация для вершины
            MapTile::Decoration topDecor(
                460,
                "PyramidTop",
                1.0f,
                m_rng() % 100 < 70 // 70% шанс анимации
            );
            topTile->addDecoration(topDecor);
        }
        break;
    }
    }
}

void WorldGenerator::createOasis(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating oasis at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Проверяем, что вокруг пустыня или похожий сухой биом
    bool isSurroundingDesert = false;
    int desertCount = 0;
    int totalCount = 0;

    for (int y = centerY - size * 2; y <= centerY + size * 2; ++y) {
        for (int x = centerX - size * 2; x <= centerX + size * 2; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile) continue;

            totalCount++;

            if (tile->getType() == TileType::SAND) {
                desertCount++;
            }
        }
    }

    // Если большинство окружающих тайлов - пустыня, создаем оазис
    isSurroundingDesert = (totalCount > 0 && static_cast<float>(desertCount) / totalCount > 0.5f);

    if (!isSurroundingDesert) {
        // Если вокруг не пустыня, сначала создаем пустыню
        for (int y = centerY - size * 2; y <= centerY + size * 2; ++y) {
            for (int x = centerX - size * 2; x <= centerX + size * 2; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Превращаем в песок
                tile->setType(TileType::SAND);
                tile->setTemperature(tile->getTemperature() + 10.0f); // Увеличиваем температуру
                tile->setHumidity(std::max(0.0f, tile->getHumidity() - 0.3f)); // Уменьшаем влажность
            }
        }
    }

    // Создаем водоем в центре
    float pondSize = size * 0.6f;

    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile) continue;

            // Расстояние от центра
            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Если выходит за размер, пропускаем
            if (distance > 1.0f) continue;

            // Внутренняя часть - водоем
            if (distance < pondSize / size) {
                tile->setType(TileType::WATER);
                tile->setHeight(0.1f);
                tile->setElevation(0.3f);
                tile->setHumidity(1.0f);
            }
            // Берег - отмель
            else if (distance < (pondSize + 0.2f) / size) {
                tile->setType(TileType::SHALLOW_WATER);
                tile->setHeight(0.05f);
                tile->setElevation(0.35f);
                tile->setHumidity(0.9f);
            }
            // Область вокруг водоема - растительность
            else {
                // Градиент влажности от водоема
                float humidityFactor = 1.0f - (distance - pondSize / size) / (1.0f - pondSize / size);

                tile->setType(TileType::GRASS);
                tile->setHeight(0.0f);
                tile->setElevation(0.4f);
                tile->setHumidity(0.5f + 0.4f * humidityFactor);

                // Добавляем растительность
                if (m_rng() % 100 < 70 * humidityFactor) {
                    MapTile::Decoration vegDecor(
                        470 + (m_rng() % 5),
                        "OasisVegetation",
                        0.6f + 0.6f * static_cast<float>(m_rng() % 100) / 100.0f,
                        m_rng() % 100 < 30 // 30% шанс анимации
                    );
                    tile->addDecoration(vegDecor);
                }
            }
        }
    }
}

void WorldGenerator::createGeyserField(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating geyser field at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Количество гейзеров
    int geyserCount = 5 + m_rng() % 8; // 5-12 гейзеров

    // Создаем геотермальную область
    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            // Расстояние от центра
            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Если выходит за размер, пропускаем
            if (distance > 1.0f) continue;

            // Изменяем тип тайла и свойства
            tile->setType(TileType::STONE);
            tile->setHeight(0.0f);

            // Увеличиваем температуру для геотермальной области
            tile->setTemperature(tile->getTemperature() + 20.0f * (1.0f - distance));

            // Низкая растительность
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

    // Создаем гейзеры
    for (int i = 0; i < geyserCount; ++i) {
        // Положение гейзера (случайное в пределах поля)
        std::uniform_real_distribution<float> posDist(0.0f, 1.0f);
        float angle = posDist(m_rng) * 6.28318f; // 0-2π
        float radius = posDist(m_rng) * 0.9f * size; // Не слишком близко к краю

        int geyserX = centerX + static_cast<int>(std::cos(angle) * radius);
        int geyserY = centerY + static_cast<int>(std::sin(angle) * radius);

        // Проверяем границы
        if (geyserX < 0 || geyserX >= width || geyserY < 0 || geyserY >= height) {
            continue;
        }

        MapTile* tile = tileMap->getTile(geyserX, geyserY);
        if (!tile || tile->isWater()) continue;

        // Создаем гейзер
        tile->setType(TileType::LAVA); // Используем тип лавы для гейзера
        tile->setHeight(0.05f);

        // Декорация для гейзера
        MapTile::Decoration geyserDecor(
            490,
            "Geyser",
            1.0f + 0.5f * static_cast<float>(m_rng() % 100) / 100.0f,
            true // Гейзер всегда анимированный
        );
        tile->addDecoration(geyserDecor);

        // Небольшая лужа вокруг гейзера
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue; // Пропускаем центр

                int poolX = geyserX + dx;
                int poolY = geyserY + dy;

                if (poolX < 0 || poolX >= width || poolY < 0 || poolY >= height) continue;

                MapTile* poolTile = tileMap->getTile(poolX, poolY);
                if (!poolTile || poolTile->isWater()) continue;

                // С некоторой вероятностью создаем лужу
                if (m_rng() % 100 < 70) {
                    poolTile->setType(TileType::SHALLOW_WATER);
                    poolTile->setHeight(0.02f);

                    // Увеличиваем температуру воды
                    poolTile->setTemperature(poolTile->getTemperature() + 30.0f);
                }
            }
        }
    }
}

void WorldGenerator::createCrystalFormation(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating crystal formation at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Количество кристаллов
    int crystalCount = 15 + m_rng() % 10; // 15-24 кристалла

    // Создаем кристаллическую область
    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x < 0 || x >= width || y < 0 || y >= height) continue;

            MapTile* tile = tileMap->getTile(x, y);
            if (!tile || tile->isWater()) continue;

            // Расстояние от центра
            float dx = static_cast<float>(x - centerX) / size;
            float dy = static_cast<float>(y - centerY) / size;
            float distance = std::sqrt(dx * dx + dy * dy);

            // Если выходит за размер, пропускаем
            if (distance > 1.0f) continue;

            // Изменяем тип тайла и свойства
            if (distance < 0.3f) {
                // Центральная часть - плотное скопление кристаллов
                if (m_rng() % 100 < 80) {
                    tile->setType(TileType::MINERAL_DEPOSIT);
                    tile->setHeight(0.1f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f);
                    tile->setResourceDensity(0.8f + 0.2f * static_cast<float>(m_rng() % 100) / 100.0f);

                    // Декорация для кристалла
                    MapTile::Decoration crystalDecor(
                        500 + (m_rng() % 5),
                        "LargeCrystal",
                        0.8f + 0.5f * static_cast<float>(m_rng() % 100) / 100.0f,
                        m_rng() % 100 < 40 // 40% шанс анимации
                    );
                    tile->addDecoration(crystalDecor);
                }
            }
            else {
                // Внешняя часть - каменистая поверхность с редкими кристаллами
                tile->setType(TileType::STONE);
                tile->setHeight(0.0f);

                // Визуальный эффект для каменистой поверхности
                if (m_rng() % 100 < 30) {
                    SDL_Color stoneColor = tile->getColor();

                    // Случайный цвет для камня
                    stoneColor.r = static_cast<Uint8>(stoneColor.r * 0.7f + 0.3f * (m_rng() % 50 + 150));
                    stoneColor.g = static_cast<Uint8>(stoneColor.g * 0.7f + 0.3f * (m_rng() % 50 + 150));
                    stoneColor.b = static_cast<Uint8>(stoneColor.b * 0.7f + 0.3f * (m_rng() % 50 + 150));

                    tile->setColor(stoneColor);
                }
            }
        }
    }

    // Создаем отдельные большие кристаллы вокруг скопления
    for (int i = 0; i < crystalCount; ++i) {
        // Положение кристалла
        std::uniform_real_distribution<float> distFactor(0.3f, 0.9f);
        float angle = static_cast<float>(m_rng() % 360) * 3.14159f / 180.0f;
        float distance = distFactor(m_rng) * size;

        int crystalX = centerX + static_cast<int>(std::cos(angle) * distance);
        int crystalY = centerY + static_cast<int>(std::sin(angle) * distance);

        // Проверяем границы
        if (crystalX < 0 || crystalX >= width || crystalY < 0 || crystalY >= height) {
            continue;
        }

        MapTile* tile = tileMap->getTile(crystalX, crystalY);
        if (!tile || tile->isWater()) continue;

        // Создаем большой кристалл
        tile->setType(TileType::MINERAL_DEPOSIT);
        tile->setHeight(0.2f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f);
        tile->setResourceDensity(0.7f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f);

        // Декорация для кристалла
        MapTile::Decoration crystalDecor(
            510 + (m_rng() % 3),
            "SingleCrystal",
            0.9f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
            m_rng() % 100 < 20 // 20% шанс анимации
        );
        tile->addDecoration(crystalDecor);

        // Случайный цвет для кристалла
        SDL_Color crystalColor;

        // Выбираем случайный цвет кристалла
        int colorType = m_rng() % 5;
        switch (colorType) {
        case 0: // Синий
            crystalColor = { 50, 100, 255, 255 };
            break;
        case 1: // Фиолетовый
            crystalColor = { 180, 50, 255, 255 };
            break;
        case 2: // Зеленый
            crystalColor = { 50, 220, 100, 255 };
            break;
        case 3: // Красный
            crystalColor = { 255, 50, 80, 255 };
            break;
        case 4: // Желтый
            crystalColor = { 255, 240, 40, 255 };
            break;
        }

        // Устанавливаем цвет
        tile->setColor(crystalColor);
    }
}

void WorldGenerator::createAlienStructure(TileMap* tileMap, int centerX, int centerY, int size) {
    LOG_DEBUG("Creating alien structure at " + std::to_string(centerX) + ", " + std::to_string(centerY));

    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Тип инопланетной структуры
    enum class AlienStructureType {
        LANDING_PAD,    // Посадочная площадка
        RESEARCH_FACILITY, // Исследовательский комплекс
        POWER_SOURCE,    // Источник энергии
        GEOMETRIC_ANOMALY // Геометрическая аномалия
    };

    AlienStructureType type = static_cast<AlienStructureType>(m_rng() % 4);

    // Создаем структуру в зависимости от типа
    switch (type) {
    case AlienStructureType::LANDING_PAD:
    {
        // Посадочная площадка - круглая платформа с инопланетными объектами

        // Радиус площадки
        int padRadius = size * 3 / 4;

        // Создаем площадку
        for (int y = centerY - padRadius; y <= centerY + padRadius; ++y) {
            for (int x = centerX - padRadius; x <= centerX + padRadius; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Расстояние от центра
                float dx = static_cast<float>(x - centerX) / padRadius;
                float dy = static_cast<float>(y - centerY) / padRadius;
                float distance = std::sqrt(dx * dx + dy * dy);

                // Круглая площадка
                if (distance <= 1.0f) {
                    // Основа площадки
                    tile->setType(TileType::METAL);
                    tile->setHeight(0.1f);

                    // Металлический цвет
                    SDL_Color metalColor = { 180, 180, 200, 255 };
                    tile->setColor(metalColor);

                    // Узоры на площадке
                    if (m_rng() % 100 < 30) {
                        MapTile::Decoration padDecor(
                            520 + (m_rng() % 3),
                            "LandingPadPattern",
                            0.7f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                            m_rng() % 100 < 50 // 50% шанс анимации
                        );
                        tile->addDecoration(padDecor);
                    }

                    // Центральная часть - маркеры посадки
                    if (distance < 0.3f) {
                        // Световые индикаторы по кругу
                        float angle = std::atan2(dy, dx);
                        float normAngle = angle / (2.0f * 3.14159f) + 0.5f; // 0.0-1.0

                        int markerCount = 8;
                        float markerInterval = 1.0f / markerCount;

                        float fractionalPart = normAngle - static_cast<int>(normAngle / markerInterval) * markerInterval;
                        if (fractionalPart < markerInterval * 0.2f && distance > 0.1f) {
                            MapTile::Decoration markerDecor(
                                523,
                                "LandingMarker",
                                0.5f,
                                true // Всегда анимированный
                            );
                            tile->addDecoration(markerDecor);
                        }
                    }
                }
            }
        }

        // Создаем инопланетный корабль в центре
        for (int y = centerY - 2; y <= centerY + 2; ++y) {
            for (int x = centerX - 2; x <= centerX + 2; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile) continue;

                // Центральный корабль
                if (std::abs(x - centerX) <= 1 && std::abs(y - centerY) <= 1) {
                    tile->setType(TileType::ALIEN_GROWTH);
                    tile->setHeight(0.4f);

                    // Декорация для корабля
                    if (x == centerX && y == centerY) {
                        MapTile::Decoration shipDecor(
                            530,
                            "AlienShip",
                            1.5f,
                            true // Анимированный
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
        // Исследовательский комплекс - прямоугольные здания с соединениями

        // Создаем основное здание (центральный блок)
        int mainSize = size / 2;

        for (int y = centerY - mainSize; y <= centerY + mainSize; ++y) {
            for (int x = centerX - mainSize; x <= centerX + mainSize; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Стены здания (по периметру)
                bool isWall = (x == centerX - mainSize || x == centerX + mainSize ||
                    y == centerY - mainSize || y == centerY + mainSize);

                if (isWall) {
                    tile->setType(TileType::METAL);
                    tile->setHeight(0.5f);

                    // Декорации для стен
                    if (m_rng() % 100 < 30) {
                        MapTile::Decoration wallDecor(
                            540 + (m_rng() % 2),
                            "FacilityWall",
                            0.8f,
                            m_rng() % 100 < 20 // 20% шанс анимации
                        );
                        tile->addDecoration(wallDecor);
                    }
                }
                else {
                    // Пол внутри здания
                    tile->setType(TileType::METAL);
                    tile->setHeight(0.1f);

                    // Инопланетное оборудование внутри
                    if (m_rng() % 100 < 20) {
                        MapTile::Decoration equipDecor(
                            545 + (m_rng() % 5),
                            "AlienEquipment",
                            0.7f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
                            m_rng() % 100 < 60 // 60% шанс анимации
                        );
                        tile->addDecoration(equipDecor);
                    }
                }
            }
        }

        // Создаем дополнительные модули вокруг основного здания
        int moduleCount = 2 + m_rng() % 3; // 2-4 модуля

        for (int i = 0; i < moduleCount; ++i) {
            // Направление от центра (0 = север, 1 = восток, 2 = юг, 3 = запад)
            int direction = m_rng() % 4;

            // Размер модуля
            int moduleSize = mainSize / 2;

            // Положение модуля
            int moduleX = centerX;
            int moduleY = centerY;

            switch (direction) {
            case 0: // Север
                moduleY -= mainSize + moduleSize;
                break;
            case 1: // Восток
                moduleX += mainSize + moduleSize;
                break;
            case 2: // Юг
                moduleY += mainSize + moduleSize;
                break;
            case 3: // Запад
                moduleX -= mainSize + moduleSize;
                break;
            }

            // Создаем модуль
            for (int y = moduleY - moduleSize; y <= moduleY + moduleSize; ++y) {
                for (int x = moduleX - moduleSize; x <= moduleX + moduleSize; ++x) {
                    if (x < 0 || x >= width || y < 0 || y >= height) continue;

                    MapTile* tile = tileMap->getTile(x, y);
                    if (!tile || tile->isWater()) continue;

                    // Стены модуля (по периметру)
                    bool isWall = (x == moduleX - moduleSize || x == moduleX + moduleSize ||
                        y == moduleY - moduleSize || y == moduleY + moduleSize);

                    if (isWall) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.4f);
                    }
                    else {
                        // Пол внутри модуля
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.1f);

                        // Специализированное оборудование в модуле
                        if (m_rng() % 100 < 40) {
                            MapTile::Decoration moduleDecor(
                                550 + (m_rng() % 5),
                                "ModuleEquipment",
                                0.6f + 0.4f * static_cast<float>(m_rng() % 100) / 100.0f,
                                m_rng() % 100 < 50 // 50% шанс анимации
                            );
                            tile->addDecoration(moduleDecor);
                        }
                    }
                }
            }

            // Создаем коридор, соединяющий модуль с основным зданием
            int corridorX1, corridorY1, corridorX2, corridorY2;

            switch (direction) {
            case 0: // Север
                corridorX1 = centerX;
                corridorY1 = centerY - mainSize;
                corridorX2 = moduleX;
                corridorY2 = moduleY + moduleSize;

                // Вертикальный коридор
                for (int y = corridorY2; y <= corridorY1; ++y) {
                    if (y < 0 || y >= height) continue;

                    MapTile* tile = tileMap->getTile(corridorX1, y);
                    if (tile && !tile->isWater()) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.2f);
                    }
                }
                break;

            case 1: // Восток
                corridorX1 = centerX + mainSize;
                corridorY1 = centerY;
                corridorX2 = moduleX - moduleSize;
                corridorY2 = moduleY;

                // Горизонтальный коридор
                for (int x = corridorX1; x <= corridorX2; ++x) {
                    if (x < 0 || x >= width) continue;

                    MapTile* tile = tileMap->getTile(x, corridorY1);
                    if (tile && !tile->isWater()) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.2f);
                    }
                }
                break;

            case 2: // Юг
                corridorX1 = centerX;
                corridorY1 = centerY + mainSize;
                corridorX2 = moduleX;
                corridorY2 = moduleY - moduleSize;

                // Вертикальный коридор
                for (int y = corridorY1; y <= corridorY2; ++y) {
                    if (y < 0 || y >= height) continue;

                    MapTile* tile = tileMap->getTile(corridorX1, y);
                    if (tile && !tile->isWater()) {
                        tile->setType(TileType::METAL);
                        tile->setHeight(0.2f);
                    }
                }
                break;

            case 3: // Запад
                corridorX1 = centerX - mainSize;
                corridorY1 = centerY;
                corridorX2 = moduleX + moduleSize;
                corridorY2 = moduleY;

                // Горизонтальный коридор
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
        // Источник энергии - центральное ядро с энергетическими лучами

        // Создаем центральное ядро
        for (int y = centerY - 2; y <= centerY + 2; ++y) {
            for (int x = centerX - 2; x <= centerX + 2; ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;

                MapTile* tile = tileMap->getTile(x, y);
                if (!tile || tile->isWater()) continue;

                // Расстояние от центра
                float dx = static_cast<float>(x - centerX);
                float dy = static_cast<float>(y - centerY);
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance <= 2.0f) {
                    // Центральное ядро
                    tile->setType(TileType::ALIEN_GROWTH);
                    tile->setHeight(0.3f);

                    // Увеличиваем высоту к центру
                    if (distance <= 1.0f) {
                        tile->setHeight(0.5f);
                    }

                    // Центральный элемент
                    if (x == centerX && y == centerY) {
                        tile->setHeight(0.7f);

                        // Декорация для ядра
                        MapTile::Decoration coreDecor(
                            560,
                            "EnergyCore",
                            1.2f,
                            true // Всегда анимированный
                        );
                        tile->addDecoration(coreDecor);
                    }
                }
            }
        }

        // Создаем энергетические лучи в 4 или 8 направлениях
        int rayCount = (m_rng() % 100 < 50) ? 4 : 8; // 50% шанс на 4 или 8 лучей

        for (int i = 0; i < rayCount; ++i) {
            // Угол луча
            float angle = 2.0f * 3.14159f * i / rayCount;

            // Длина луча
            int rayLength = size - 2;

            // Создаем луч
            for (int dist = 3; dist <= rayLength; ++dist) {
                int rayX = centerX + static_cast<int>(std::cos(angle) * dist);
                int rayY = centerY + static_cast<int>(std::sin(angle) * dist);

                if (rayX < 0 || rayX >= width || rayY < 0 || rayY >= height) continue;

                MapTile* tile = tileMap->getTile(rayX, rayY);
                if (!tile || tile->isWater()) continue;

                // Чем дальше от центра, тем меньше высота
                float heightFactor = 1.0f - static_cast<float>(dist) / rayLength;

                tile->setType(TileType::ALIEN_GROWTH);
                tile->setHeight(0.2f * heightFactor);

                // Декорации для луча
                MapTile::Decoration rayDecor(
                    565 + (i % 4), // 4 варианта лучей
                    "EnergyBeam",
                    0.7f + 0.3f * heightFactor,
                    true // Всегда анимированный
                );
                tile->addDecoration(rayDecor);

                // Увеличиваем радиацию вдоль луча
                tile->setRadiationLevel(0.5f + 0.5f * heightFactor);
            }
        }
        break;
    }
    case AlienStructureType::GEOMETRIC_ANOMALY:
    {
        // Геометрическая аномалия - странные геометрические формы

        // Тип геометрической аномалии
        enum class GeometryType {
            SPIRAL,     // Спиральная структура
            FRACTAL,    // Фрактальная структура
            SYMMETRIC   // Симметричная структура
        };

        GeometryType geoType = static_cast<GeometryType>(m_rng() % 3);

        // Создаем аномалию в зависимости от типа
        switch (geoType) {
        case GeometryType::SPIRAL:
        {
            // Спиральная структура

            // Параметры спирали
            float growthFactor = 0.2f + 0.1f * static_cast<float>(m_rng() % 6);
            float angleStep = 0.2f + 0.1f * static_cast<float>(m_rng() % 5);
            int numSteps = size * 4;

            // Создаем спираль
            for (int i = 0; i < numSteps; ++i) {
                float angle = angleStep * i;
                float radius = growthFactor * angle;

                // Координаты точки спирали
                int spiralX = centerX + static_cast<int>(std::cos(angle) * radius);
                int spiralY = centerY + static_cast<int>(std::sin(angle) * radius);

                if (spiralX < 0 || spiralX >= width || spiralY < 0 || spiralY >= height) continue;

                MapTile* tile = tileMap->getTile(spiralX, spiralY);
                if (!tile || tile->isWater()) continue;

                // Изменяем тайл
                tile->setType(TileType::ALIEN_GROWTH);
                tile->setHeight(0.3f);

                // Случайный цвет для тайла
                float hue = fmod(angle * 30.0f, 360.0f);
                SDL_Color spiralColor;

                // HSV to RGB преобразование (упрощенно)
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

                // Декорации для спирали
                if (i % 5 == 0) { // Каждые 5 шагов
                    MapTile::Decoration spiralDecor(
                        570 + (i % 5),
                        "SpiralNode",
                        0.5f + 0.5f * static_cast<float>(m_rng() % 100) / 100.0f,
                        true // Всегда анимированный
                    );
                    tile->addDecoration(spiralDecor);
                }
            }
            break;
        }
        case GeometryType::FRACTAL:
        {
            // Фрактальная структура (треугольники Серпинского)

            // Рекурсивная функция для создания фрактала
            std::function<void(int, int, int, int, int, int, int)> createFractal;
            createFractal = [&](int x1, int y1, int x2, int y2, int x3, int y3, int depth) {
                // Базовый случай
                if (depth <= 0) return;

                // Создаем треугольник
                std::vector<std::pair<int, int>> points = {
                    {x1, y1}, {x2, y2}, {x3, y3}
                };

                // Рисуем линии треугольника
                for (size_t i = 0; i < points.size(); ++i) {
                    size_t j = (i + 1) % points.size();

                    int startX = points[i].first;
                    int startY = points[i].second;
                    int endX = points[j].first;
                    int endY = points[j].second;

                    // Рисуем линию (алгоритм Брезенхэма)
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

                                // Разные цвета для разных глубин
                                SDL_Color fractalColor;
                                switch (depth % 6) {
                                case 0: fractalColor = { 255, 50, 50, 255 }; break;  // Красный
                                case 1: fractalColor = { 50, 255, 50, 255 }; break;  // Зеленый
                                case 2: fractalColor = { 50, 50, 255, 255 }; break;  // Синий
                                case 3: fractalColor = { 255, 255, 50, 255 }; break; // Желтый
                                case 4: fractalColor = { 255, 50, 255, 255 }; break; // Пурпурный
                                case 5: fractalColor = { 50, 255, 255, 255 }; break; // Голубой
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

            // Параметры мандалы
            int sectors = 8 + m_rng() % 8; // 8-15 секторов
            int rings = 3 + m_rng() % 4;   // 3-6 колец

            // Создаем мандалу
            for (int ring = 1; ring <= rings; ++ring) {
                // Радиус кольца
                float radius = static_cast<float>(ring) * size / rings;

                // Создаем сектора
                for (int sector = 0; sector < sectors; ++sector) {
                    // Угол сектора
                    float startAngle = 2.0f * 3.14159f * sector / sectors;
                    float endAngle = 2.0f * 3.14159f * (sector + 1) / sectors;

                    // Шаг угла для рисования дуги
                    float angleStep = 0.1f;

                    // Рисуем дугу
                    for (float angle = startAngle; angle < endAngle; angle += angleStep) {
                        int arcX = centerX + static_cast<int>(std::cos(angle) * radius);
                        int arcY = centerY + static_cast<int>(std::sin(angle) * radius);

                        if (arcX < 0 || arcX >= width || arcY < 0 || arcY >= height) continue;

                        MapTile* tile = tileMap->getTile(arcX, arcY);
                        if (!tile || tile->isWater()) continue;

                        // Изменяем тайл
                        tile->setType(TileType::ALIEN_GROWTH);
                        tile->setHeight(0.1f);

                        // Цвет зависит от кольца
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

                    // Создаем линии от центра к периметру
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

                            // Декорация для линии
                            if (static_cast<int>(dist) % 2 == 0) {
                                MapTile::Decoration lineDecor(
                                    580 + (sector % 5),
                                    "SymmetryNode",
                                    0.4f + 0.3f * static_cast<float>(m_rng() % 100) / 100.0f,
                                    true // Анимированный
                                );
                                tile->addDecoration(lineDecor);
                            }
                        }
                    }
                }
            }

            // Центральная точка
            MapTile* centerTile = tileMap->getTile(centerX, centerY);
            if (centerTile && !centerTile->isWater()) {
                centerTile->setType(TileType::ALIEN_GROWTH);
                centerTile->setHeight(0.4f);

                // Декорация для центра
                MapTile::Decoration centerDecor(
                    590,
                    "SymmetryCenter",
                    1.2f,
                    true // Анимированный
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