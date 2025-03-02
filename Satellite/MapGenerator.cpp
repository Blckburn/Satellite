#include "MapGenerator.h"
#include <algorithm>
#include <iostream>
#include <cmath>

// Константы для шума Перлина
const int PERMUTATION_SIZE = 256;
const int PERMUTATION_MASK = PERMUTATION_SIZE - 1;

MapGenerator::MapGenerator(unsigned int seed)
    : m_seed(seed) {
    // Инициализация генератора случайных чисел
    resetGenerator();
}

MapGenerator::~MapGenerator() {
    // Очистка ресурсов при необходимости
}

void MapGenerator::resetGenerator() {
    // Если сид равен 0, генерируем случайный сид
    if (m_seed == 0) {
        std::random_device rd;
        m_seed = rd();
    }

    // Инициализация генератора с заданным сидом
    m_rng.seed(m_seed);
}

void MapGenerator::setSeed(unsigned int seed) {
    m_seed = seed;
    resetGenerator();
}

void MapGenerator::setParameters(float temperature, float humidity, float roughness,
    float waterLevel, float resourceRichness) {
    m_baseTemperature = temperature;
    m_baseHumidity = std::max(0.0f, std::min(1.0f, humidity));
    m_terrainRoughness = std::max(0.1f, std::min(1.0f, roughness));
    m_waterLevel = std::max(0.0f, std::min(0.8f, waterLevel)); // Максимум 0.8 чтобы избежать полного затопления
    m_resourceRichness = std::max(0.0f, std::min(1.0f, resourceRichness));
}

void MapGenerator::addBiome(std::shared_ptr<Biome> biome) {
    if (biome) {
        m_biomes.push_back(biome);
    }
}

void MapGenerator::clearBiomes() {
    m_biomes.clear();
}

bool MapGenerator::generate(TileMap* tileMap, GenerationType genType) {
    if (!tileMap) {
        std::cerr << "Error: TileMap is nullptr" << std::endl;
        return false;
    }

    // Проверяем, есть ли биомы
    if (m_biomes.empty() && m_forcedBiomeId == -1) {
        setupDefaultBiomes();
    }

    // Выбираем метод генерации в зависимости от типа
    switch (genType) {
    case GenerationType::ARCHIPELAGO:
        generateArchipelago(tileMap);
        break;
    case GenerationType::MOUNTAINOUS:
        generateMountains(tileMap);
        break;
    case GenerationType::CRATER:
        generateCraters(tileMap);
        break;
    case GenerationType::VOLCANIC:
        generateVolcanic(tileMap);
        break;
    case GenerationType::ALIEN:
        generateAlien(tileMap);
        break;
    case GenerationType::DEFAULT:
    default:
        generateTerrain(tileMap);
        break;
    }

    // Распределяем биомы на карте
    distributeBiomes(tileMap);

    // Сглаживаем границы биомов
    smoothBiomeBorders(tileMap);

    // Размещаем ресурсы в зависимости от биомов
    placeResources(tileMap);

    // Размещаем особые точки интереса
    placePointsOfInterest(tileMap);

    // Добавляем декоративные элементы
    placeDecorations(tileMap);

    return true;
}

void MapGenerator::setupDefaultBiomes() {
    // Очищаем текущие биомы
    clearBiomes();

    // 1. Создаем пустынный биом
    auto desert = std::make_shared<Biome>(1, "Desert");
    desert->setDescription("Пустынная местность с высокими температурами и низкой влажностью.");
    desert->setTemperatureRange(30.0f, 50.0f);
    desert->setHumidityRange(0.0f, 0.2f);
    desert->setElevationRange(0.2f, 0.6f);
    desert->setHazardLevel(0.4f);
    desert->setResourceLevel(0.3f);

    desert->addTileType(TileType::SAND, 0.8f);
    desert->addTileType(TileType::STONE, 0.15f);
    desert->addTileType(TileType::ROCK_FORMATION, 0.05f);

    desert->addDecoration(Biome::BiomeDecoration(1, "Cactus", 0.05f, 0.8f, 1.2f, false));
    desert->addDecoration(Biome::BiomeDecoration(2, "DeadTree", 0.02f, 0.9f, 1.5f, false));
    desert->addDecoration(Biome::BiomeDecoration(3, "BoneRemains", 0.01f, 0.7f, 1.0f, false));

    addBiome(desert);

    // 2. Создаем тропический лес
    auto jungle = std::make_shared<Biome>(2, "Jungle");
    jungle->setDescription("Густой тропический лес с высокой влажностью и богатой растительностью.");
    jungle->setTemperatureRange(25.0f, 40.0f);
    jungle->setHumidityRange(0.7f, 1.0f);
    jungle->setElevationRange(0.3f, 0.7f);
    jungle->setHazardLevel(0.6f);
    jungle->setResourceLevel(0.8f);

    jungle->addTileType(TileType::GRASS, 0.65f);
    jungle->addTileType(TileType::MUD, 0.2f);
    jungle->addTileType(TileType::SHALLOW_WATER, 0.1f);
    jungle->addTileType(TileType::ALIEN_GROWTH, 0.05f);

    jungle->addDecoration(Biome::BiomeDecoration(4, "TropicalTree", 0.2f, 0.8f, 2.0f, true));
    jungle->addDecoration(Biome::BiomeDecoration(5, "Fern", 0.15f, 0.5f, 1.0f, true));
    jungle->addDecoration(Biome::BiomeDecoration(6, "ColorfulFlowers", 0.1f, 0.3f, 0.7f, true));

    addBiome(jungle);

    // 3. Создаем тундру
    auto tundra = std::make_shared<Biome>(3, "Tundra");
    tundra->setDescription("Холодная местность с вечной мерзлотой и скудной растительностью.");
    tundra->setTemperatureRange(-20.0f, 5.0f);
    tundra->setHumidityRange(0.3f, 0.6f);
    tundra->setElevationRange(0.2f, 0.5f);
    tundra->setHazardLevel(0.5f);
    tundra->setResourceLevel(0.4f);

    tundra->addTileType(TileType::SNOW, 0.6f);
    tundra->addTileType(TileType::ICE, 0.2f);
    tundra->addTileType(TileType::STONE, 0.15f);
    tundra->addTileType(TileType::GRASS, 0.05f);

    tundra->addDecoration(Biome::BiomeDecoration(7, "SnowPile", 0.08f, 0.5f, 1.0f, false));
    tundra->addDecoration(Biome::BiomeDecoration(8, "FrozenWaterfall", 0.02f, 1.0f, 2.0f, false));

    addBiome(tundra);

    // 4. Создаем вулканическую местность
    auto volcanic = std::make_shared<Biome>(4, "Volcanic");
    volcanic->setDescription("Активная вулканическая местность с лавовыми потоками и выжженной землей.");
    volcanic->setTemperatureRange(40.0f, 90.0f);
    volcanic->setHumidityRange(0.0f, 0.2f);
    volcanic->setElevationRange(0.4f, 0.9f);
    volcanic->setHazardLevel(0.9f);
    volcanic->setResourceLevel(0.7f);

    volcanic->addTileType(TileType::STONE, 0.5f);
    volcanic->addTileType(TileType::LAVA, 0.25f);
    volcanic->addTileType(TileType::ROCK_FORMATION, 0.15f);
    volcanic->addTileType(TileType::MOUNTAIN, 0.1f);

    volcanic->addDecoration(Biome::BiomeDecoration(9, "LavaFountain", 0.05f, 0.8f, 1.2f, true));
    volcanic->addDecoration(Biome::BiomeDecoration(10, "SmokeVent", 0.1f, 0.6f, 1.0f, true));

    addBiome(volcanic);

    // 5. Создаем умеренную зону
    auto temperate = std::make_shared<Biome>(5, "Temperate");
    temperate->setDescription("Умеренный климат с разнообразной растительностью.");
    temperate->setTemperatureRange(5.0f, 25.0f);
    temperate->setHumidityRange(0.4f, 0.7f);
    temperate->setElevationRange(0.3f, 0.6f);
    temperate->setHazardLevel(0.2f);
    temperate->setResourceLevel(0.6f);

    temperate->addTileType(TileType::GRASS, 0.7f);
    temperate->addTileType(TileType::FOREST, 0.15f);
    temperate->addTileType(TileType::WATER, 0.1f);
    temperate->addTileType(TileType::STONE, 0.05f);

    temperate->addDecoration(Biome::BiomeDecoration(11, "Tree", 0.1f, 0.8f, 1.5f, false));
    temperate->addDecoration(Biome::BiomeDecoration(12, "Flowers", 0.15f, 0.3f, 0.7f, true));
    temperate->addDecoration(Biome::BiomeDecoration(13, "Bush", 0.1f, 0.4f, 0.8f, false));

    addBiome(temperate);
}

void MapGenerator::generateTerrain(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем карты высот, температуры и влажности
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));
    std::vector<std::vector<float>> temperatureMap(height, std::vector<float>(width, 0.0f));
    std::vector<std::vector<float>> humidityMap(height, std::vector<float>(width, 0.0f));

    // Генерация карты высот с использованием шума Перлина
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 1. Генерация базовой высоты с шумом Перлина
            float baseHeight = perlinNoise(x, y, m_noiseScale, 6, 0.5f, 2.0f, m_seed);

            // 2. Применяем экспоненциальную кривую для создания более естественного рельефа
            // Уменьшаем вероятность очень высоких значений
            baseHeight = powf(baseHeight, 1.5f);

            // 3. Скалирование значения с учетом настройки неровности
            baseHeight = baseHeight * m_terrainRoughness;

            // 4. Дополнительные модификации для создания различных типов ландшафтов могут быть добавлены здесь

            // Сохраняем значение высоты
            heightMap[y][x] = baseHeight;
        }
    }

    // Генерация температурной карты с градиентом север-юг
    // и вариациями в зависимости от высоты и случайных факторов
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 1. Базовая температура планеты
            float baseTemp = m_baseTemperature;

            // 2. Градиент север-юг (теплее на экваторе, холоднее у полюсов)
            float latitudeEffect = 20.0f * (0.5f - fabsf(static_cast<float>(y) / height - 0.5f) * 2.0f);

            // 3. Высота влияет на температуру (выше = холоднее)
            float heightEffect = -30.0f * heightMap[y][x];

            // 4. Случайные вариации для создания микроклиматов
            float randomVariation = 5.0f * (perlinNoise(x, y, m_noiseScale * 2.0f, 3, 0.5f, 2.0f, m_seed + 100) - 0.5f);

            // 5. Итоговая температура
            float temperature = baseTemp + latitudeEffect + heightEffect + randomVariation;

            // Сохраняем значение температуры
            temperatureMap[y][x] = temperature;
        }
    }

    // Генерация карты влажности с учетом высоты, температуры и случайных факторов
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 1. Базовая влажность
            float baseHumidity = m_baseHumidity;

            // 2. Высота влияет на влажность (ниже = влажнее)
            float heightEffect = -0.3f * heightMap[y][x];

            // 3. Температура влияет на влажность (жарче = суше, но не линейно)
            float tempEffect = -0.01f * (temperatureMap[y][x] - 20.0f);

            // 4. Случайные вариации
            float randomVariation = 0.2f * (perlinNoise(x, y, m_noiseScale * 3.0f, 4, 0.5f, 2.0f, m_seed + 200) - 0.5f);

            // 5. Итоговая влажность
            float humidity = baseHumidity + heightEffect + tempEffect + randomVariation;
            humidity = std::max(0.0f, std::min(1.0f, humidity)); // Ограничиваем диапазоном [0,1]

            // Сохраняем значение влажности
            humidityMap[y][x] = humidity;
        }
    }

    // Применяем сгенерированные карты к тайлам
    applyHeightMap(tileMap, heightMap);
    applyTemperatureMap(tileMap, temperatureMap);
    applyHumidityMap(tileMap, humidityMap);

    // Генерируем водные объекты
    generateWaterBodies(tileMap, heightMap);
}

void MapGenerator::generateArchipelago(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем карты высот и другие карты
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));

    // Генерация карты высот для архипелага - много воды и разрозненные острова
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 1. Используем шум Перлина для получения значения
            float baseNoise = perlinNoise(x, y, m_noiseScale * 1.5f, 6, 0.5f, 2.0f, m_seed);

            // 2. Добавляем воронои шум для создания более изолированных форм островов
            float voronoiValue = voronoiNoise(x, y, 0.02f, m_seed + 1000);

            // 3. Комбинируем шумы для получения интересных островных форм
            float combinedNoise = 0.5f * baseNoise + 0.5f * voronoiValue;

            // 4. Применяем степенную функцию для выделения островов
            float islandValue = powf(combinedNoise, 3.0f);

            // 5. Устанавливаем базовый уровень, чтобы большая часть была под водой
            float heightValue = islandValue - (0.7f - m_waterLevel);

            // Сохраняем результат
            heightMap[y][x] = std::max(0.0f, heightValue);
        }
    }

    // Применяем сгенерированную карту высот к тайлам
    applyHeightMap(tileMap, heightMap);

    // Генерируем водные объекты с очень высоким уровнем воды
    m_waterLevel = std::max(m_waterLevel, 0.6f); // Гарантируем высокий уровень воды
    generateWaterBodies(tileMap, heightMap);

    // Затем создаем карты температуры и влажности, и применяем их
    std::vector<std::vector<float>> temperatureMap(height, std::vector<float>(width, m_baseTemperature));
    std::vector<std::vector<float>> humidityMap(height, std::vector<float>(width, m_baseHumidity + 0.2f)); // Острова более влажные

    applyTemperatureMap(tileMap, temperatureMap);
    applyHumidityMap(tileMap, humidityMap);
}

void MapGenerator::generateMountains(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем карту высот для горного ландшафта
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));

    // Параметры для создания горных хребтов
    float ridgeScale = m_noiseScale * 0.7f;
    float ridgeHeight = 0.6f + m_terrainRoughness * 0.4f;

    // Генерация карты высот для гор
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 1. Базовый шум Перлина
            float baseNoise = perlinNoise(x, y, m_noiseScale, 6, 0.5f, 2.0f, m_seed);

            // 2. Шум для создания горных хребтов (с другим сидом)
            float ridgeNoise = 1.0f - fabsf(2.0f * perlinNoise(x, y, ridgeScale, 4, 0.7f, 2.0f, m_seed + 500) - 1.0f);
            ridgeNoise = powf(ridgeNoise, 2.0f) * ridgeHeight;

            // 3. Комбинируем шумы с доминированием горных хребтов
            float combinedHeight = 0.3f * baseNoise + 0.7f * ridgeNoise;

            // 4. Применяем нелинейную функцию для усиления контраста
            float finalHeight = powf(combinedHeight, 1.5f);

            // Сохраняем результат
            heightMap[y][x] = finalHeight;
        }
    }

    // Применяем сгенерированную карту высот к тайлам
    applyHeightMap(tileMap, heightMap);

    // Сначала генерируем карты температуры и влажности, так как они зависят от высоты
    std::vector<std::vector<float>> temperatureMap(height, std::vector<float>(width, 0.0f));
    std::vector<std::vector<float>> humidityMap(height, std::vector<float>(width, 0.0f));

    // Температура уменьшается с высотой
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float elevation = heightMap[y][x];
            temperatureMap[y][x] = m_baseTemperature - 20.0f * elevation;

            // На одной стороне гор больше осадков
            float directionalFactor = perlinNoise(x, y, 0.01f, 1, 0.5f, 2.0f, m_seed + 300);
            humidityMap[y][x] = m_baseHumidity - 0.3f * elevation + 0.4f * directionalFactor;
            humidityMap[y][x] = std::max(0.0f, std::min(1.0f, humidityMap[y][x]));
        }
    }

    applyTemperatureMap(tileMap, temperatureMap);
    applyHumidityMap(tileMap, humidityMap);

    // Наконец, генерируем водные объекты (реки, водопады в горах)
    // Используем низкий уровень воды для гор
    m_waterLevel = std::min(m_waterLevel, 0.3f);
    generateWaterBodies(tileMap, heightMap);
}

void MapGenerator::generateCraters(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем карту высот
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));

    // Генерация базовой карты высот с небольшими вариациями
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Используем шум Перлина с меньшей амплитудой
            heightMap[y][x] = 0.3f + 0.2f * perlinNoise(x, y, m_noiseScale * 2.0f, 4, 0.5f, 2.0f, m_seed);
        }
    }

    // Определяем количество кратеров в зависимости от размера карты
    int numCraters = static_cast<int>(sqrt(width * height) * 0.05f * m_terrainRoughness);

    // Минимум несколько кратеров
    numCraters = std::max(3, numCraters);

    // Создаем кратеры в случайных местах
    std::uniform_int_distribution<int> xDist(0, width - 1);
    std::uniform_int_distribution<int> yDist(0, height - 1);
    std::uniform_real_distribution<float> sizeDist(0.05f, 0.15f); // Размер кратера в % от карты
    std::uniform_real_distribution<float> depthDist(0.1f, 0.5f);  // Глубина кратера

    for (int i = 0; i < numCraters; ++i) {
        int centerX = xDist(m_rng);
        int centerY = yDist(m_rng);
        int radius = static_cast<int>(std::min(width, height) * sizeDist(m_rng));
        float depth = depthDist(m_rng);

        // Создаем кратер в карте высот
        for (int y = centerY - radius; y <= centerY + radius; ++y) {
            for (int x = centerX - radius; x <= centerX + radius; ++x) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    // Расстояние от центра кратера
                    float dx = static_cast<float>(x - centerX);
                    float dy = static_cast<float>(y - centerY);
                    float distance = sqrtf(dx * dx + dy * dy);

                    if (distance <= radius) {
                        // Вычисляем форму кратера с учетом случайной "шероховатости"
                        float craterShape = 1.0f - powf(distance / radius, 2.0f);

                        // Добавляем шероховатость по краям кратера
                        float noise = 0.1f * perlinNoise(x * 0.5f, y * 0.5f, 0.1f, 3, 0.5f, 2.0f, m_seed + i * 100);
                        craterShape *= (1.0f + noise);

                        // Уменьшаем высоту для создания впадины кратера
                        heightMap[y][x] -= depth * craterShape;

                        // Добавляем вал вокруг кратера (на краю)
                        if (distance > radius * 0.7f && distance < radius) {
                            float rimShape = (distance - radius * 0.7f) / (radius * 0.3f);
                            rimShape = sinf(rimShape * 3.14159f);
                            heightMap[y][x] += 0.1f * rimShape;
                        }
                    }
                }
            }
        }
    }

    // Нормализуем карту высот
    float minHeight = 1.0f;
    float maxHeight = 0.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            minHeight = std::min(minHeight, heightMap[y][x]);
            maxHeight = std::max(maxHeight, heightMap[y][x]);
        }
    }

    float heightRange = maxHeight - minHeight;
    if (heightRange > 0.001f) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                heightMap[y][x] = (heightMap[y][x] - minHeight) / heightRange;
            }
        }
    }

    // Применяем сгенерированную карту высот
    applyHeightMap(tileMap, heightMap);

    // Генерируем карты температуры и влажности для кратерного ландшафта
    std::vector<std::vector<float>> temperatureMap(height, std::vector<float>(width, 0.0f));
    std::vector<std::vector<float>> humidityMap(height, std::vector<float>(width, 0.0f));

    // Присваиваем базовые значения с небольшими вариациями
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            temperatureMap[y][x] = m_baseTemperature - 10.0f + 20.0f * perlinNoise(x, y, 0.05f, 2, 0.5f, 2.0f, m_seed + 400);
            humidityMap[y][x] = std::max(0.0f, std::min(1.0f, m_baseHumidity * 0.5f + 0.2f * perlinNoise(x, y, 0.05f, 2, 0.5f, 2.0f, m_seed + 500)));
        }
    }

    applyTemperatureMap(tileMap, temperatureMap);
    applyHumidityMap(tileMap, humidityMap);

    // В кратерах может быть вода, если она достаточно глубокая
    m_waterLevel = std::min(m_waterLevel, 0.2f);
    generateWaterBodies(tileMap, heightMap);
}

void MapGenerator::generateVolcanic(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем карту высот
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));

    // Дополнительные карты для лавовых потоков и тепловых зон
    std::vector<std::vector<float>> lavaMap(height, std::vector<float>(width, 0.0f));
    std::vector<std::vector<float>> heatMap(height, std::vector<float>(width, 0.0f));

    // Генерация базовой карты высот с большими вариациями
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Используем шум Перлина с высокой амплитудой для создания более резких форм
            heightMap[y][x] = perlinNoise(x, y, m_noiseScale, 5, 0.6f, 2.2f, m_seed);

            // Применяем нелинейную функцию для создания более резких гор
            heightMap[y][x] = powf(heightMap[y][x], 1.5f);
        }
    }

    // Определяем количество вулканов в зависимости от размера карты
    int numVolcanoes = static_cast<int>(sqrt(width * height) * 0.02f * m_terrainRoughness);

    // Минимум хотя бы один вулкан
    numVolcanoes = std::max(1, numVolcanoes);

    // Создаем вулканы в случайных местах
    std::uniform_int_distribution<int> xDist(width / 5, width * 4 / 5);
    std::uniform_int_distribution<int> yDist(height / 5, height * 4 / 5);
    std::uniform_real_distribution<float> sizeDist(0.05f, 0.15f); // Размер вулкана в % от карты
    std::uniform_real_distribution<float> heightDist(0.6f, 1.0f); // Высота вулкана

    // Структура для хранения информации о вулканах
    struct Volcano {
        int x, y;
        int radius;
        float height;
    };

    std::vector<Volcano> volcanoes;

    for (int i = 0; i < numVolcanoes; ++i) {
        Volcano v;
        v.x = xDist(m_rng);
        v.y = yDist(m_rng);
        v.radius = static_cast<int>(std::min(width, height) * sizeDist(m_rng));
        v.height = heightDist(m_rng);

        volcanoes.push_back(v);

        // Создаем вулкан в карте высот
        for (int y = v.y - v.radius * 2; y <= v.y + v.radius * 2; ++y) {
            for (int x = v.x - v.radius * 2; x <= v.x + v.radius * 2; ++x) {
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    // Расстояние от центра вулкана
                    float dx = static_cast<float>(x - v.x);
                    float dy = static_cast<float>(y - v.y);
                    float distance = sqrtf(dx * dx + dy * dy);

                    // Форма склона вулкана
                    if (distance <= v.radius * 2) {
                        // Расчет высоты вулкана (конусообразная форма с кратером)
                        float normalizedDist = distance / v.radius;
                        float volcano_height = 0.0f;

                        if (normalizedDist <= 0.5f) {
                            // Кратер вулкана (впадина на вершине)
                            volcano_height = v.height * (0.7f + 0.3f * normalizedDist / 0.5f);
                        }
                        else if (normalizedDist <= 1.0f) {
                            // Склон вулкана
                            volcano_height = v.height * (1.0f - 0.2f * (normalizedDist - 0.5f) / 0.5f);
                        }
                        else {
                            // Подножие вулкана
                            volcano_height = v.height * 0.8f * (1.0f - (normalizedDist - 1.0f));
                            volcano_height = std::max(0.0f, volcano_height);
                        }

                        // Добавляем шероховатость склона
                        float noise = 0.1f * perlinNoise(x * 0.2f, y * 0.2f, 0.1f, 3, 0.5f, 2.0f, m_seed + i * 100);
                        volcano_height *= (1.0f + noise);

                        // Объединяем с существующей высотой, выбирая максимальное значение
                        heightMap[y][x] = std::max(heightMap[y][x], volcano_height);

                        // Создаем лавовые потоки в кратере и на склонах
                        if (normalizedDist <= 0.3f) {
                            // В кратере всегда есть лава
                            lavaMap[y][x] = 1.0f;
                        }
                        else if (normalizedDist <= 1.0f) {
                            // На склонах лава может течь по определенным путям
                            float lavaPathNoise = perlinNoise(x * 0.5f, y * 0.5f, 0.1f, 2, 0.5f, 2.0f, m_seed + i * 200);
                            if (lavaPathNoise > 0.6f) {
                                lavaMap[y][x] = 0.7f - 0.7f * (normalizedDist - 0.3f) / 0.7f;
                            }
                        }

                        // Создаем тепловые зоны вокруг вулкана
                        float heatValue = 1.0f - normalizedDist / 2.0f;
                        heatValue = std::max(0.0f, heatValue);
                        heatMap[y][x] = std::max(heatMap[y][x], heatValue);
                    }
                }
            }
        }
    }

    // Применяем сгенерированную карту высот
    applyHeightMap(tileMap, heightMap);

    // Генерируем карты температуры и влажности для вулканического ландшафта
    std::vector<std::vector<float>> temperatureMap(height, std::vector<float>(width, 0.0f));
    std::vector<std::vector<float>> humidityMap(height, std::vector<float>(width, 0.0f));

    // Устанавливаем температуру и влажность с учетом тепловых зон
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Высокая базовая температура + дополнительное тепло от вулканов
            temperatureMap[y][x] = m_baseTemperature + 20.0f + 40.0f * heatMap[y][x];

            // Низкая влажность из-за высоких температур
            humidityMap[y][x] = std::max(0.0f, std::min(1.0f, m_baseHumidity * 0.3f - 0.2f * heatMap[y][x]));

            // Но места с лавой маркируем как особые тайлы
            if (lavaMap[y][x] > 0.5f) {
                // Устанавливаем тип тайла как лава
                tileMap->setTileType(x, y, TileType::LAVA);

                // Очень высокая температура и нулевая влажность для лавы
                temperatureMap[y][x] = 800.0f; // Температура лавы
                humidityMap[y][x] = 0.0f;
            }
        }
    }

    applyTemperatureMap(tileMap, temperatureMap);
    applyHumidityMap(tileMap, humidityMap);

    // Вулканические зоны с низким уровнем воды
    m_waterLevel = std::min(m_waterLevel, 0.2f);
    generateWaterBodies(tileMap, heightMap);
}

void MapGenerator::generateAlien(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем карты для различных характеристик
    std::vector<std::vector<float>> heightMap(height, std::vector<float>(width, 0.0f));
    std::vector<std::vector<float>> strangenessMap(height, std::vector<float>(width, 0.0f));

    // Генерация базовой карты высот с необычными формациями
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 1. Базовый шум Перлина
            float baseNoise = perlinNoise(x, y, m_noiseScale, 5, 0.5f, 2.0f, m_seed);

            // 2. Деформационный шум для создания странных форм
            float distortX = 10.0f * perlinNoise(x * 0.5f, y * 0.5f, 0.05f, 2, 0.5f, 2.0f, m_seed + 100);
            float distortY = 10.0f * perlinNoise(x * 0.5f, y * 0.5f, 0.05f, 2, 0.5f, 2.0f, m_seed + 200);

            // 3. Деформируем координаты для создания искаженных форм
            float alienNoise = perlinNoise(x + distortX, y + distortY, m_noiseScale * 1.5f, 3, 0.7f, 2.0f, m_seed + 300);

            // 4. Комбинируем шумы для получения инопланетного ландшафта
            float combinedHeight = 0.5f * baseNoise + 0.5f * alienNoise;

            // 5. Добавляем экстремальные формации
            float anomalies = powf(perlinNoise(x, y, m_noiseScale * 3.0f, 2, 0.5f, 2.0f, m_seed + 400), 8.0f) * 2.0f;
            combinedHeight += anomalies;

            // 6. Ограничиваем результат
            combinedHeight = std::max(0.0f, std::min(1.0f, combinedHeight));

            // Сохраняем результат
            heightMap[y][x] = combinedHeight;

            // Карта "странности" для определения инопланетных участков
            strangenessMap[y][x] = perlinNoise(x, y, m_noiseScale * 2.0f, 4, 0.7f, 2.0f, m_seed + 500);
        }
    }

    // Применяем сгенерированную карту высот
    applyHeightMap(tileMap, heightMap);

    // Генерируем температуру и влажность с странными распределениями
    std::vector<std::vector<float>> temperatureMap(height, std::vector<float>(width, 0.0f));
    std::vector<std::vector<float>> humidityMap(height, std::vector<float>(width, 0.0f));

    // Устанавливаем температуру и влажность с необычными паттернами
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Странный температурный паттерн - резкие изменения
            float tempPattern = voronoiNoise(x, y, 0.01f, m_seed + 600);
            temperatureMap[y][x] = m_baseTemperature - 10.0f + 80.0f * tempPattern;

            // Влажность тоже следует необычному паттерну
            float humidPattern = 1.0f - fabsf(2.0f * perlinNoise(x, y, 0.03f, 3, 0.5f, 2.0f, m_seed + 700) - 1.0f);
            humidityMap[y][x] = std::max(0.0f, std::min(1.0f, humidPattern));

            // Специальные зоны с инопланетной растительностью
            if (strangenessMap[y][x] > 0.6f && heightMap[y][x] > 0.3f && heightMap[y][x] < 0.8f) {
                // Заменяем обычные тайлы на инопланетную растительность
                tileMap->setTileType(x, y, TileType::ALIEN_GROWTH);

                // Устанавливаем уровень радиации для этих зон
                tileMap->getTile(x, y)->setRadiationLevel(0.3f + 0.5f * (strangenessMap[y][x] - 0.6f) / 0.4f);
            }
        }
    }

    applyTemperatureMap(tileMap, temperatureMap);
    applyHumidityMap(tileMap, humidityMap);

    // Водные объекты с необычными свойствами
    m_waterLevel = m_waterLevel;
    generateWaterBodies(tileMap, heightMap);
}

void MapGenerator::applyHeightMap(TileMap* tileMap, const std::vector<std::vector<float>>& heightMap) {
    int height = heightMap.size();
    if (height == 0) return;

    int width = heightMap[0].size();
    if (width == 0) return;

    // Применяем значения высоты к тайлам
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Получаем тайл, если он существует
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                // Устанавливаем высоту для тайла
                tile->setElevation(heightMap[y][x]);

                // Определяем базовый тип тайла в зависимости от высоты
                // Это будет переопределено позже на основе биомов
                TileType baseTileType;

                if (heightMap[y][x] < m_waterLevel) {
                    // Водные зоны
                    baseTileType = TileType::WATER;

                    // Устанавливаем высоту для тайла (вода имеет определенную глубину)
                    tile->setHeight(0.1f);
                }
                else if (heightMap[y][x] < m_waterLevel + 0.05f) {
                    // Берега и отмели
                    baseTileType = TileType::SHALLOW_WATER;
                    tile->setHeight(0.05f);
                }
                else if (heightMap[y][x] < 0.4f) {
                    // Низменности
                    baseTileType = TileType::GRASS;
                    tile->setHeight(0.0f);
                }
                else if (heightMap[y][x] < 0.7f) {
                    // Холмы
                    baseTileType = TileType::HILL;

                    // Высота пропорциональна elevation
                    float heightScale = (heightMap[y][x] - 0.4f) / 0.3f; // 0-1 для диапазона 0.4-0.7
                    tile->setHeight(0.3f + 0.2f * heightScale);
                }
                else {
                    // Горы
                    baseTileType = TileType::MOUNTAIN;

                    // Высота пропорциональна elevation
                    float heightScale = (heightMap[y][x] - 0.7f) / 0.3f; // 0-1 для диапазона 0.7-1.0
                    tile->setHeight(0.5f + 0.5f * heightScale);
                }

                // Устанавливаем начальный тип тайла
                tile->setType(baseTileType);
            }
        }
    }
}

void MapGenerator::applyTemperatureMap(TileMap* tileMap, const std::vector<std::vector<float>>& tempMap) {
    int height = tempMap.size();
    if (height == 0) return;

    int width = tempMap[0].size();
    if (width == 0) return;

    // Применяем значения температуры к тайлам
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Получаем тайл, если он существует
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                // Устанавливаем температуру для тайла
                tile->setTemperature(tempMap[y][x]);

                // Модификация типа тайла в зависимости от температуры
                if (tile->getType() != TileType::WATER && tile->getType() != TileType::SHALLOW_WATER) {
                    if (tempMap[y][x] < -10.0f && tile->getElevation() > m_waterLevel) {
                        // Очень холодные участки покрыты снегом или льдом
                        if (tile->getType() == TileType::MOUNTAIN) {
                            // Снежные вершины гор
                            // Оставляем тип, но меняем цвет
                            SDL_Color snowColor = { 240, 240, 250, 255 };
                            tile->setColor(snowColor);
                        }
                        else {
                            // Снежный покров на обычных участках
                            tile->setType(TileType::SNOW);
                        }
                    }
                    else if (tempMap[y][x] > 80.0f) {
                        // Очень горячие участки могут быть лавой или выжженной землей
                        if (m_rng() % 100 < 30) { // 30% шанс на лаву
                            tile->setType(TileType::LAVA);
                        }
                        else {
                            // Оставляем тип, но меняем цвет на "выжженный"
                            SDL_Color burnedColor = { 100, 70, 30, 255 };
                            tile->setColor(burnedColor);
                        }
                    }
                }
            }
        }
    }
}

void MapGenerator::applyHumidityMap(TileMap* tileMap, const std::vector<std::vector<float>>& humidityMap) {
    int height = humidityMap.size();
    if (height == 0) return;

    int width = humidityMap[0].size();
    if (width == 0) return;

    // Применяем значения влажности к тайлам
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Получаем тайл, если он существует
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                // Устанавливаем влажность для тайла
                tile->setHumidity(humidityMap[y][x]);

                // Модификация типа тайла в зависимости от влажности
                if (tile->getType() != TileType::WATER &&
                    tile->getType() != TileType::SHALLOW_WATER &&
                    tile->getType() != TileType::SNOW &&
                    tile->getType() != TileType::LAVA) {

                    float temp = tile->getTemperature();

                    if (tile->getType() == TileType::GRASS) {
                        if (humidityMap[y][x] < 0.2f && temp > 15.0f) {
                            // Сухая трава превращается в песок в теплом климате
                            tile->setType(TileType::SAND);
                        }
                        else if (humidityMap[y][x] > 0.7f && temp > 20.0f) {
                            // Влажная трава в теплом климате становится болотом
                            tile->setType(TileType::MUD);
                        }
                    }
                }
            }
        }
    }
}

void MapGenerator::distributeBiomes(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Если нет биомов, нечего распределять
    if (m_biomes.empty()) return;

    // Распределяем биомы на карте в зависимости от высоты, температуры и влажности
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Получаем тайл, если он существует
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                // Если это принудительный биом, используем его для всех тайлов
                if (m_forcedBiomeId >= 0) {
                    // Ищем биом с указанным ID
                    for (const auto& biome : m_biomes) {
                        if (biome->getId() == m_forcedBiomeId) {
                            tile->setBiomeId(biome->getId());

                            // Только если это не вода, обновляем тип тайла на основе биома
                            if (tile->getType() != TileType::WATER && tile->getType() != TileType::SHALLOW_WATER) {
                                TileType newType = biome->getRandomTileType();
                                tile->setType(newType);
                            }

                            // Устанавливаем уровень ресурсов на основе биома
                            tile->setResourceDensity(biome->getResourceLevel());
                            break;
                        }
                    }
                }
                else {
                    // Выбираем подходящий биом на основе характеристик тайла
                    auto selectedBiome = selectBiome(
                        tile->getTemperature(),
                        tile->getHumidity(),
                        tile->getElevation()
                    );

                    if (selectedBiome) {
                        tile->setBiomeId(selectedBiome->getId());

                        // Только если это не вода, обновляем тип тайла на основе биома
                        if (tile->getType() != TileType::WATER && tile->getType() != TileType::SHALLOW_WATER) {
                            TileType newType = selectedBiome->getRandomTileType();
                            tile->setType(newType);
                        }

                        // Устанавливаем уровень ресурсов на основе биома
                        tile->setResourceDensity(selectedBiome->getResourceLevel());
                    }
                }
            }
        }
    }
}

std::shared_ptr<Biome> MapGenerator::selectBiome(float temperature, float humidity, float elevation) {
    // Если у нас нет биомов, возвращаем nullptr
    if (m_biomes.empty()) return nullptr;

    // Сначала проверяем, какие биомы соответствуют условиям
    std::vector<std::shared_ptr<Biome>> matchingBiomes;

    for (const auto& biome : m_biomes) {
        if (biome->matches(temperature, humidity, elevation)) {
            matchingBiomes.push_back(biome);
        }
    }

    // Если нет соответствующих биомов, выбираем ближайший по параметрам
    if (matchingBiomes.empty()) {
        std::shared_ptr<Biome> closestBiome = nullptr;
        float minDistance = std::numeric_limits<float>::max();

        for (const auto& biome : m_biomes) {
            // Вычисляем "расстояние" до биома по трем параметрам
            float tempDiff = std::min(
                fabsf(temperature - biome->getMinTemperature()),
                fabsf(temperature - biome->getMaxTemperature())
            );

            float humidDiff = std::min(
                fabsf(humidity - biome->getMinHumidity()),
                fabsf(humidity - biome->getMaxHumidity())
            );

            float elevDiff = std::min(
                fabsf(elevation - biome->getMinElevation()),
                fabsf(elevation - biome->getMaxElevation())
            );

            // Взвешенное расстояние
            float distance = tempDiff * 0.5f + humidDiff * 0.3f + elevDiff * 0.2f;

            if (distance < minDistance) {
                minDistance = distance;
                closestBiome = biome;
            }
        }

        return closestBiome;
    }
    else if (matchingBiomes.size() == 1) {
        // Если подходит только один биом, возвращаем его
        return matchingBiomes[0];
    }
    else {
        // Если подходит несколько биомов, выбираем случайный, но с вероятностью, пропорциональной "близости" его параметров
        std::vector<float> weights;
        float totalWeight = 0.0f;

        for (const auto& biome : matchingBiomes) {
            // Вычисляем "вес" биома на основе того, насколько хорошо он соответствует параметрам
            float tempWeight = 1.0f - fabsf(temperature - (biome->getMinTemperature() + biome->getMaxTemperature()) / 2.0f) / 50.0f;
            float humidWeight = 1.0f - fabsf(humidity - (biome->getMinHumidity() + biome->getMaxHumidity()) / 2.0f);
            float elevWeight = 1.0f - fabsf(elevation - (biome->getMinElevation() + biome->getMaxElevation()) / 2.0f);

            float weight = tempWeight * humidWeight * elevWeight;
            weight = std::max(0.1f, weight); // Минимальный вес

            weights.push_back(weight);
            totalWeight += weight;
        }

        // Выбираем биом с учетом весов
        std::uniform_real_distribution<float> dist(0.0f, totalWeight);
        float randValue = dist(m_rng);

        float cumulativeWeight = 0.0f;
        for (size_t i = 0; i < matchingBiomes.size(); ++i) {
            cumulativeWeight += weights[i];
            if (randValue <= cumulativeWeight) {
                return matchingBiomes[i];
            }
        }

        // На всякий случай, если что-то пошло не так, возвращаем последний биом
        return matchingBiomes.back();
    }
}

void MapGenerator::placeResources(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Распределяем ресурсы на карте
    std::uniform_real_distribution<float> resourceDist(0.0f, 1.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Получаем тайл, если он существует
            MapTile* tile = tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::WATER && tile->getType() != TileType::SHALLOW_WATER) {
                // Базовый уровень ресурсов уже установлен при распределении биомов
                float resourceLevel = tile->getResourceDensity();

                // Увеличиваем уровень ресурсов на основе общей настройки богатства ресурсами
                resourceLevel *= (0.5f + m_resourceRichness);

                // Добавляем немного случайности
                resourceLevel *= (0.8f + 0.4f * resourceDist(m_rng));

                // Если уровень ресурсов превышает порог, создаем месторождение ресурсов
                if (resourceLevel > 0.7f) {
                    // 10% шанс на месторождение ресурсов, если значение высокое
                    if (resourceDist(m_rng) < 0.1f) {
                        tile->setType(TileType::MINERAL_DEPOSIT);

                        // Устанавливаем повышенную плотность ресурсов
                        tile->setResourceDensity(resourceLevel);
                    }
                }
            }
        }
    }
}

void MapGenerator::placePointsOfInterest(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Определяем количество точек интереса в зависимости от размера карты
    int numPoints = static_cast<int>(sqrt(width * height) * 0.01f);

    // Минимум хотя бы одна точка интереса
    numPoints = std::max(1, numPoints);

    // Создаем точки интереса (руины, уникальные формации и т.д.)
    std::uniform_int_distribution<int> xDist(5, width - 6);
    std::uniform_int_distribution<int> yDist(5, height - 6);
    std::uniform_real_distribution<float> poiTypeDist(0.0f, 1.0f);

    for (int i = 0; i < numPoints; ++i) {
        // Выбираем случайную позицию для точки интереса
        int centerX = xDist(m_rng);
        int centerY = yDist(m_rng);

        // Проверяем, что точка находится на суше, а не в воде
        MapTile* centerTile = tileMap->getTile(centerX, centerY);
        if (centerTile && centerTile->getType() != TileType::WATER && centerTile->getType() != TileType::SHALLOW_WATER) {
            // Определяем тип точки интереса
            float poiType = poiTypeDist(m_rng);

            if (poiType < 0.4f) {
                // Руины - 40% шанс
                placePOIStructure(tileMap, centerX, centerY, TileType::RUINS, 3 + m_rng() % 3);
            }
            else if (poiType < 0.7f) {
                // Кратер - 30% шанс
                placePOIStructure(tileMap, centerX, centerY, TileType::CRATER, 4 + m_rng() % 4);
            }
            else {
                // Скальное образование - 30% шанс
                placePOIStructure(tileMap, centerX, centerY, TileType::ROCK_FORMATION, 2 + m_rng() % 3);
            }
        }
    }
}

void MapGenerator::placePOIStructure(TileMap* tileMap, int centerX, int centerY, TileType poiType, int size) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем точку интереса в виде структуры заданного типа и размера
    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                // Расстояние от центра
                float dx = static_cast<float>(x - centerX);
                float dy = static_cast<float>(y - centerY);
                float distance = sqrtf(dx * dx + dy * dy);

                // Базовая форма структуры зависит от ее типа
                if (distance <= size) {
                    MapTile* tile = tileMap->getTile(x, y);
                    if (tile) {
                        // Вероятность размещения зависит от расстояния
                        float probability = 1.0f - distance / size;

                        // Добавляем случайность к вероятности
                        probability = std::min(1.0f, probability * 1.2f);

                        // Решаем, размещать ли точку интереса в этом месте
                        float rand = static_cast<float>(m_rng()) / static_cast<float>(m_rng.max());

                        if (rand < probability) {
                            // Размещаем структуру
                            tile->setType(poiType);

                            // Устанавливаем свойства в зависимости от типа
                            if (poiType == TileType::RUINS) {
                                // Руины могут содержать ресурсы
                                tile->setResourceDensity(std::min(1.0f, tile->getResourceDensity() + 0.3f));

                                // Устанавливаем высоту руин
                                tile->setHeight(0.3f + 0.5f * rand);
                            }
                            else if (poiType == TileType::CRATER) {
                                // Кратеры имеют отрицательную высоту (впадина)
                                tile->setHeight(-0.2f * (1.0f - distance / size));

                                // Может быть радиация в кратере
                                if (rand < 0.3f) {
                                    tile->setRadiationLevel(0.3f + 0.3f * rand);
                                }
                            }
                            else if (poiType == TileType::ROCK_FORMATION) {
                                // Скальные образования имеют значительную высоту
                                tile->setHeight(0.5f + 0.5f * rand);

                                // Могут содержать ресурсы
                                if (rand < 0.5f) {
                                    tile->setResourceDensity(std::min(1.0f, tile->getResourceDensity() + 0.2f));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void MapGenerator::placeDecorations(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Размещаем декорации на карте на основе биомов
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Получаем тайл, если он существует
            MapTile* tile = tileMap->getTile(x, y);
            if (tile && tile->getType() != TileType::WATER && tile->getType() != TileType::SHALLOW_WATER) {
                // Находим биом этого тайла
                int biomeId = tile->getBiomeId();

                for (const auto& biome : m_biomes) {
                    if (biome->getId() == biomeId) {
                        // Генерируем случайное количество декораций для этого тайла
                        std::uniform_int_distribution<int> countDist(0, 3);
                        int decorCount = countDist(m_rng);

                        // Получаем случайные декорации для данного биома
                        auto decorations = biome->generateRandomDecorations(decorCount);

                        // Очищаем текущие декорации тайла и добавляем новые
                        tile->clearDecorations();
                        for (const auto& decor : decorations) {
                            tile->addDecoration(decor);
                        }

                        break;
                    }
                }
            }
        }
    }
}

void MapGenerator::generateWaterBodies(TileMap* tileMap, const std::vector<std::vector<float>>& heightMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Размещаем водные объекты (озера, реки) на основе карты высот
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Получаем тайл, если он существует
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                // Устанавливаем воду в низинах
                if (heightMap[y][x] < m_waterLevel) {
                    tile->setType(TileType::WATER);
                    tile->setHeight(0.1f);
                }
                else if (heightMap[y][x] < m_waterLevel + 0.05f) {
                    // Берега и отмели
                    tile->setType(TileType::SHALLOW_WATER);
                    tile->setHeight(0.05f);
                }
            }
        }
    }

    // Теперь создаем реки, текущие от высоких точек к низким
    createRivers(tileMap, heightMap);
}

void MapGenerator::createRivers(TileMap* tileMap, const std::vector<std::vector<float>>& heightMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Количество рек зависит от размера карты
    int numRivers = std::max(1, static_cast<int>(sqrt(width * height) * 0.01f));

    // Ищем высокие точки для начала рек
    std::vector<std::pair<int, int>> highPoints;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (heightMap[y][x] > 0.7f) {
                // Проверяем, является ли эта точка локальным максимумом
                bool isLocalMax = true;

                for (int dy = -1; dy <= 1 && isLocalMax; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int nx = x + dx;
                        int ny = y + dy;

                        if (nx >= 0 && nx < width && ny >= 0 && ny < height && (dx != 0 || dy != 0)) {
                            if (heightMap[ny][nx] > heightMap[y][x]) {
                                isLocalMax = false;
                                break;
                            }
                        }
                    }
                }

                if (isLocalMax) {
                    highPoints.push_back(std::make_pair(x, y));
                }
            }
        }
    }

    // Сортируем точки по высоте (от наибольшей к наименьшей)
    std::sort(highPoints.begin(), highPoints.end(), [&heightMap](const auto& a, const auto& b) {
        return heightMap[a.second][a.first] > heightMap[b.second][b.first];
        });

    // Создаем реки, начиная с самых высоких точек
    for (int i = 0; i < std::min(numRivers, static_cast<int>(highPoints.size())); ++i) {
        int x = highPoints[i].first;
        int y = highPoints[i].second;

        // Создаем реку, текущую от этой точки
        createRiverFromPoint(tileMap, heightMap, x, y);
    }
}

void MapGenerator::createRiverFromPoint(TileMap* tileMap, const std::vector<std::vector<float>>& heightMap, int startX, int startY) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Максимальная длина реки
    int maxLength = std::max(width, height) * 2;

    // Текущая позиция
    int x = startX;
    int y = startY;

    // Флаг достижения конца реки
    bool reachedEnd = false;

    // Создаем реку, длиной не более maxLength ячеек
    for (int step = 0; step < maxLength && !reachedEnd; ++step) {
        // Устанавливаем текущую ячейку как воду (если она еще не вода)
        MapTile* tile = tileMap->getTile(x, y);
        if (tile && tile->getType() != TileType::WATER) {
            // Если это не первый шаг, делаем ячейку водой
            if (step > 0) {
                tile->setType(TileType::WATER);
                tile->setHeight(0.05f);
            }
        }

        // Находим соседа с наименьшей высотой
        int lowestX = x;
        int lowestY = y;
        float lowestHeight = heightMap[y][x];

        bool foundLower = false;

        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (dx == 0 && dy == 0) continue;

                int nx = x + dx;
                int ny = y + dy;

                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    if (heightMap[ny][nx] < lowestHeight) {
                        lowestX = nx;
                        lowestY = ny;
                        lowestHeight = heightMap[ny][nx];
                        foundLower = true;
                    }
                }
            }
        }

        // Если не найдена ячейка ниже, река завершена
        if (!foundLower) {
            reachedEnd = true;
        }
        else {
            // Иначе переходим к следующей ячейке
            x = lowestX;
            y = lowestY;

            // Если достигли существующего водоема, также завершаем
            tile = tileMap->getTile(x, y);
            if (tile && (tile->getType() == TileType::WATER || heightMap[y][x] < m_waterLevel)) {
                reachedEnd = true;
            }
        }
    }
}

void MapGenerator::smoothBiomeBorders(TileMap* tileMap) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Временная копия карты биомов
    std::vector<std::vector<int>> biomeCopy(height, std::vector<int>(width, 0));

    // Сначала копируем ID биомов
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                biomeCopy[y][x] = tile->getBiomeId();
            }
        }
    }

    // Сглаживаем границы биомов
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                // Пропускаем водные тайлы
                if (tile->getType() == TileType::WATER || tile->getType() == TileType::SHALLOW_WATER) {
                    continue;
                }

                // Подсчитываем, сколько соседей имеют другой тип биома
                std::map<int, int> biomeCount;

                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int nx = x + dx;
                        int ny = y + dy;

                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            biomeCount[biomeCopy[ny][nx]]++;
                        }
                    }
                }

                // Если текущий биом не является большинством, заменяем его на наиболее распространенный
                int currentBiomeCount = biomeCount[biomeCopy[y][x]];
                int mostCommonBiome = biomeCopy[y][x];
                int maxCount = currentBiomeCount;

                for (const auto& pair : biomeCount) {
                    if (pair.second > maxCount) {
                        maxCount = pair.second;
                        mostCommonBiome = pair.first;
                    }
                }

                // Если большинство соседей имеют другой биом, меняем текущий тайл
                if (mostCommonBiome != biomeCopy[y][x]) {
                    tile->setBiomeId(mostCommonBiome);

                    // Также обновляем тип тайла для соответствия новому биому
                    for (const auto& biome : m_biomes) {
                        if (biome->getId() == mostCommonBiome) {
                            tile->setType(biome->getRandomTileType());
                            break;
                        }
                    }
                }
            }
        }
    }
}

float MapGenerator::perlinNoise(float x, float y, float scale, int octaves, float persistence, float lacunarity, int seed) const {
    x *= scale;
    y *= scale;

    float amplitude = 1.0f;
    float frequency = 1.0f;
    float total = 0.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        // Простой хеш-функция для создания уникального сида для каждой октавы
        unsigned int octaveSeed = seed + i * 1013;

        // Используем фрактальный шум (сумма нескольких шумов Perlin с разными частотами)
        float noise = generateSimpleNoise(x * frequency, y * frequency, octaveSeed);

        total += noise * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    // Нормализация результата в диапазон [0, 1]
    return (total / maxValue) * 0.5f + 0.5f;
}

float MapGenerator::generateSimpleNoise(float x, float y, unsigned int seed) const {
    // Простая хеш-функция для генерации случайного градиента
    auto hash = [seed](int ix, int iy) {
        unsigned int a = 1664525U * (ix ^ (iy << 16)) + 1013904223U + seed;
        a ^= (a >> 13);
        a = a * 196314165U;
        return a;
        };

    // Функция для скалярного произведения
    auto dot = [](float x, float y, unsigned int hash) {
        // Создаем векторы на основе хеша
        unsigned int h = hash & 3;
        float u = (h < 2) ? x : y;
        float v = (h < 2) ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
        };

    // Функция сглаживания
    auto fade = [](float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
        };

    // Функция интерполяции
    auto lerp = [](float a, float b, float t) {
        return a + t * (b - a);
        };

    // Целочисленные координаты
    int ix = static_cast<int>(floorf(x));
    int iy = static_cast<int>(floorf(y));

    // Дробные части координат
    float fx = x - ix;
    float fy = y - iy;

    // Хеши для четырех углов ячейки
    unsigned int h00 = hash(ix, iy);
    unsigned int h10 = hash(ix + 1, iy);
    unsigned int h01 = hash(ix, iy + 1);
    unsigned int h11 = hash(ix + 1, iy + 1);

    // Скалярные произведения для четырех углов
    float d00 = dot(fx, fy, h00);
    float d10 = dot(fx - 1, fy, h10);
    float d01 = dot(fx, fy - 1, h01);
    float d11 = dot(fx - 1, fy - 1, h11);

    // Сглаженные значения для интерполяции
    float sx = fade(fx);
    float sy = fade(fy);

    // Билинейная интерполяция
    float x1 = lerp(d00, d10, sx);
    float x2 = lerp(d01, d11, sx);
    float value = lerp(x1, x2, sy);

    return value;
}

float MapGenerator::voronoiNoise(float x, float y, float scale, int seed) const {
    x *= scale;
    y *= scale;

    // Целочисленные координаты ячейки
    int ix = static_cast<int>(floorf(x));
    int iy = static_cast<int>(floorf(y));

    float minDist = 10.0f;

    // Проверяем точки в 3x3 ячейках вокруг текущей
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            int cellX = ix + dx;
            int cellY = iy + dy;

            // Хеш-функция для создания случайных координат точки в ячейке
            unsigned int cellSeed = seed + cellX * 1013 + cellY * 1619;
            std::mt19937 rng(cellSeed);
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            // Координаты случайной точки внутри ячейки
            float pointX = static_cast<float>(cellX) + dist(rng);
            float pointY = static_cast<float>(cellY) + dist(rng);

            // Расстояние до точки
            float dx = x - pointX;
            float dy = y - pointY;
            float dist = sqrtf(dx * dx + dy * dy);

            // Обновляем минимальное расстояние
            minDist = std::min(minDist, dist);
        }
    }

    // Нормализуем результат (примерно в диапазон [0, 1])
    return std::min(1.0f, minDist / 0.7f);
}

void MapGenerator::createCrater(TileMap* tileMap, int centerX, int centerY, int radius, float depth) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем кратер в указанной позиции
    for (int y = centerY - radius; y <= centerY + radius; ++y) {
        for (int x = centerX - radius; x <= centerX + radius; ++x) {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                // Расстояние от центра кратера
                float dx = static_cast<float>(x - centerX);
                float dy = static_cast<float>(y - centerY);
                float distance = sqrtf(dx * dx + dy * dy);

                if (distance <= radius) {
                    MapTile* tile = tileMap->getTile(x, y);
                    if (tile) {
                        // Вычисляем новую высоту для тайла внутри кратера
                        // Форма кратера - параболическая
                        float craterFactor = 1.0f - (distance / radius) * (distance / radius);
                        float newHeight = -depth * craterFactor;

                        // Устанавливаем новую высоту
                        tile->setHeight(newHeight);

                        // Устанавливаем тип тайла как кратер
                        tile->setType(TileType::CRATER);
                    }
                }
            }
        }
    }
}

void MapGenerator::placePOIStructure(TileMap* tileMap, int centerX, int centerY, TileType poiType, int size) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Создаем точку интереса в виде структуры заданного типа и размера
    for (int y = centerY - size; y <= centerY + size; ++y) {
        for (int x = centerX - size; x <= centerX + size; ++x) {
            if (x >= 0 && x < width && y >= 0 && y < height) {
                // Расстояние от центра
                float dx = static_cast<float>(x - centerX);
                float dy = static_cast<float>(y - centerY);
                float distance = sqrtf(dx * dx + dy * dy);

                // Базовая форма структуры зависит от ее типа
                if (distance <= size) {
                    MapTile* tile = tileMap->getTile(x, y);
                    if (tile) {
                        // Вероятность размещения зависит от расстояния
                        float probability = 1.0f - distance / size;

                        // Добавляем случайность к вероятности
                        probability = std::min(1.0f, probability * 1.2f);

                        // Решаем, размещать ли точку интереса в этом месте
                        float rand = static_cast<float>(m_rng()) / static_cast<float>(m_rng.max());

                        if (rand < probability) {
                            // Размещаем структуру
                            tile->setType(poiType);

                            // Устанавливаем свойства в зависимости от типа
                            if (poiType == TileType::RUINS) {
                                // Руины могут содержать ресурсы
                                tile->setResourceDensity(std::min(1.0f, tile->getResourceDensity() + 0.3f));

                                // Устанавливаем высоту руин
                                tile->setHeight(0.3f + 0.5f * rand);

                                // Иногда добавляем декоративные элементы
                                if (rand > 0.7f) {
                                    MapTile::Decoration ruinDecor(
                                        100 + (m_rng() % 3),  // ID декорации: 100-102
                                        "AncientRuin",        // Имя декорации
                                        0.8f + 0.4f * rand,   // Случайный масштаб
                                        false                 // Не анимированная
                                    );
                                    tile->addDecoration(ruinDecor);
                                }
                            }
                            else if (poiType == TileType::CRATER) {
                                // Кратеры имеют отрицательную высоту (впадина)
                                tile->setHeight(-0.2f * (1.0f - distance / size));

                                // Может быть радиация в кратере
                                if (rand < 0.3f) {
                                    tile->setRadiationLevel(0.3f + 0.3f * rand);

                                    // Если есть радиация, добавляем декоративный эффект
                                    if (rand < 0.15f) {
                                        MapTile::Decoration radDecor(
                                            110,                 // ID декорации
                                            "RadiationEffect",   // Имя декорации
                                            0.6f + 0.8f * rand,  // Случайный масштаб
                                            true                 // Анимированная
                                        );
                                        tile->addDecoration(radDecor);
                                    }
                                }
                            }
                            else if (poiType == TileType::ROCK_FORMATION) {
                                // Скальные образования имеют значительную высоту
                                tile->setHeight(0.5f + 0.5f * rand);

                                // Могут содержать ресурсы
                                if (rand < 0.5f) {
                                    tile->setResourceDensity(std::min(1.0f, tile->getResourceDensity() + 0.2f));

                                    // Если есть ресурсы, иногда добавляем декоративный кристалл
                                    if (rand < 0.25f) {
                                        MapTile::Decoration crystalDecor(
                                            120,                  // ID декорации
                                            "CrystalFormation",   // Имя декорации
                                            0.3f + 0.6f * rand,   // Случайный масштаб
                                            rand < 0.1f           // Иногда анимированный
                                        );
                                        tile->addDecoration(crystalDecor);
                                    }
                                }
                            }
                            else if (poiType == TileType::ALIEN_GROWTH) {
                                // Инопланетная растительность может быть опасной
                                if (rand < 0.4f) {
                                    tile->setRadiationLevel(0.1f + 0.2f * rand);
                                }

                                // Всегда добавляем декоративный элемент
                                MapTile::Decoration alienDecor(
                                    130 + (m_rng() % 5),    // ID декорации: 130-134
                                    "AlienFlora",           // Имя декорации
                                    0.6f + 0.8f * rand,     // Случайный масштаб
                                    rand < 0.6f             // Часто анимированная
                                );
                                tile->addDecoration(alienDecor);
                            }
                        }
                    }
                }
            }
        }
    }
}