#include "WorldGenerator.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// Структуры с исходными данными для генерации названий планет
namespace {
    const std::vector<std::string> prefix = {
        "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta", "Iota",
        "Kappa", "Lambda", "Mu", "Nu", "Xi", "Omicron", "Pi", "Rho", "Sigma", "Tau",
        "Upsilon", "Phi", "Chi", "Psi", "Omega"
    };

    const std::vector<std::string> scifiRoots = {
        "tron", "cor", "bos", "zon", "plex", "dor", "thal", "neb", "prax", "vor",
        "nix", "rix", "tris", "lex", "dex", "tec", "sar", "tan", "bor", "xar"
    };

    const std::vector<std::string> naturalRoots = {
        "terra", "aqua", "ignis", "aer", "sol", "luna", "stella", "mons", "flumen",
        "silva", "orb", "gaia", "astra", "vita", "mar", "vent", "glac", "des", "vol"
    };

    const std::vector<std::string> suffix = {
        "ia", "is", "us", "um", "on", "ar", "or", "ix", "ax", "a",
        "Prime", "Alpha", "Beta", "Proxima", "Major", "Minor", "Ultima", "Secundus"
    };

    const std::vector<std::vector<std::string>> temperatureDescriptors = {
        // Экстремальный холод
        {
            "ледяной", "замерзший", "арктический", "морозный", "криогенный"
        },
        // Холодный
        {
            "холодный", "промозглый", "прохладный", "мягкий холодный", "свежий"
        },
        // Умеренный
        {
            "умеренный", "мягкий", "приятный", "благоприятный", "сбалансированный"
        },
        // Теплый
        {
            "теплый", "жаркий", "знойный", "душный", "паровой"
        },
        // Экстремальный жар
        {
            "раскаленный", "вулканический", "испепеляющий", "адский", "палящий"
        }
    };

    const std::vector<std::vector<std::string>> atmosphereDescriptors = {
        // Отсутствие атмосферы
        {
            "безвоздушный", "лишенный атмосферы", "вакуумный", "мертвый"
        },
        // Разреженная атмосфера
        {
            "разреженный", "истонченный", "негостеприимный", "тонкий"
        },
        // Средняя атмосфера
        {
            "вполне дышащий", "пригодный для дыхания", "приемлемый", "достаточный"
        },
        // Плотная атмосфера
        {
            "плотный", "давящий", "тяжелый", "насыщенный", "густой"
        },
        // Экстремальная плотность
        {
            "удушающий", "токсичный", "коррозийный", "смертельный"
        }
    };

    const std::vector<std::vector<std::string>> waterDescriptors = {
        // Безводный
        {
            "безводный", "иссушенный", "пустынный", "засушливый", "обезвоженный"
        },
        // Немного воды
        {
            "полупустынный", "сухой", "с редкими водоемами", "маловодный"
        },
        // Средний уровень воды
        {
            "с умеренными водоемами", "с реками и озерами", "с водными ресурсами"
        },
        // Много воды
        {
            "изобилующий водой", "влажный", "океанический", "архипелаговый"
        },
        // Водный мир
        {
            "водный мир", "океанический мир", "затопленный", "подводный"
        }
    };

    const std::vector<std::vector<std::string>> gravityDescriptors = {
        // Низкая гравитация
        {
            "с низкой гравитацией", "практически невесомый", "с легким притяжением"
        },
        // Стандартная гравитация
        {
            "с нормальной гравитацией", "с привычным притяжением", "со стандартной тяжестью"
        },
        // Высокая гравитация
        {
            "тяжелый", "с повышенной гравитацией", "давящий", "с мощным притяжением"
        }
    };

    const std::vector<std::vector<std::string>> resourceDescriptors = {
        // Бедные ресурсы
        {
            "скудный", "пустой", "ресурсно-бедный", "истощенный"
        },
        // Средние ресурсы
        {
            "с умеренными ресурсами", "с некоторыми залежами", "частично богатый"
        },
        // Богатые ресурсы
        {
            "богатый ресурсами", "изобильный", "щедрый", "процветающий", "ценный"
        }
    };

    const std::vector<std::vector<std::string>> radiationDescriptors = {
        // Нет радиации
        {
            "свободный от радиации", "радиационно-безопасный", "чистый"
        },
        // Небольшая радиация
        {
            "с фоновой радиацией", "слегка радиоактивный", "с умеренным излучением"
        },
        // Высокая радиация
        {
            "сильно радиоактивный", "излучающий", "опасный", "смертельно радиационный"
        }
    };

    const std::vector<std::vector<std::string>> lifeDescriptors = {
        // Нет жизни
        {
            "безжизненный", "стерильный", "мертвый", "необитаемый"
        },
        // Простая жизнь
        {
            "с примитивной жизнью", "с бактериальной жизнью", "с простейшими организмами"
        },
        // Развитая жизнь
        {
            "с развитой жизнью", "населенный", "с богатой экосистемой", "с биоразнообразием", "живой"
        }
    };
}

// Конструкторы структур данных

WorldGenerator::PlanetData::PlanetData()
    : name("Unnamed Planet"), description(""), averageTemperature(20.0f),
    atmosphereDensity(0.5f), gravityMultiplier(1.0f), waterCoverage(0.5f),
    radiationLevel(0.0f), resourceRichness(0.5f),
    mainTerrainType(MapGenerator::GenerationType::DEFAULT),
    hasLife(false), seed(0) {
}

WorldGenerator::RegionData::RegionData()
    : name("Unnamed Region"), biomePriority(-1), dangerLevel(0.3f),
    resourceLevel(0.5f), seed(0) {
}

// Реализация класса WorldGenerator

WorldGenerator::WorldGenerator(unsigned int seed)
    : m_seed(seed) {
    // Инициализируем генератор случайных чисел
    if (m_seed == 0) {
        std::random_device rd;
        m_seed = rd();
    }
    m_rng.seed(m_seed);

    // Создаем генератор карт
    m_mapGenerator = std::make_shared<MapGenerator>(m_seed);

    // Устанавливаем биомы по умолчанию
    setupDefaultBiomes();
}

WorldGenerator::~WorldGenerator() {
    // Освобождаем ресурсы при необходимости
}

void WorldGenerator::setSeed(unsigned int seed) {
    m_seed = seed;
    if (m_seed == 0) {
        std::random_device rd;
        m_seed = rd();
    }
    m_rng.seed(m_seed);

    // Обновляем сид для генератора карт
    if (m_mapGenerator) {
        m_mapGenerator->setSeed(m_seed);
    }
}

void WorldGenerator::addBiome(std::shared_ptr<Biome> biome) {
    if (biome) {
        m_biomes.push_back(biome);
        // Добавляем биом и в генератор карт
        if (m_mapGenerator) {
            m_mapGenerator->addBiome(biome);
        }
    }
}

void WorldGenerator::clearBiomes() {
    m_biomes.clear();
    // Очищаем биомы и в генераторе карт
    if (m_mapGenerator) {
        m_mapGenerator->clearBiomes();
    }
}

void WorldGenerator::setupDefaultBiomes() {
    clearBiomes();

    // Базовые биомы уже настроены в MapGenerator, не нужно дублировать
    if (m_mapGenerator) {
        m_mapGenerator->setupDefaultBiomes();

        // Копируем биомы из генератора карт
        for (const auto& biome : m_mapGenerator->getBiomes()) {
            m_biomes.push_back(biome);
        }
    }
}

bool WorldGenerator::generatePlanet(TileMap* tileMap, const PlanetData& planetData) {
    if (!tileMap || !m_mapGenerator) return false;

    // Применяем сид планеты
    m_mapGenerator->setSeed(planetData.seed);

    // Устанавливаем параметры генерации в соответствии с планетой
    m_mapGenerator->setParameters(
        planetData.averageTemperature,
        planetData.hasLife ? 0.5f : 0.3f, // Влажность зависит от наличия жизни
        0.5f, // Средняя неровность рельефа
        planetData.waterCoverage,
        planetData.resourceRichness
    );

    // Устанавливаем биомы, подходящие для этой планеты
    setupBiomesForPlanet(planetData);

    // Генерируем карту с заданным типом ландшафта
    bool result = m_mapGenerator->generate(tileMap, planetData.mainTerrainType);

    // После генерации карты добавляем планетарные особенности
    if (result) {
        applyPlanetaryFeatures(tileMap, planetData);
    }

    return result;
}

bool WorldGenerator::generateRegion(TileMap* tileMap, const RegionData& regionData, const PlanetData& planetData) {
    if (!tileMap || !m_mapGenerator) return false;

    // Комбинируем сиды планеты и региона
    unsigned int combinedSeed = planetData.seed ^ (regionData.seed << 16);
    m_mapGenerator->setSeed(combinedSeed);

    // Устанавливаем параметры генерации в соответствии с планетой и регионом
    m_mapGenerator->setParameters(
        planetData.averageTemperature,
        planetData.hasLife ? 0.5f : 0.3f, // Влажность зависит от наличия жизни
        0.5f, // Средняя неровность рельефа
        planetData.waterCoverage,
        regionData.resourceLevel // Используем уровень ресурсов региона
    );

    // Если указан приоритетный биом, устанавливаем его
    if (regionData.biomePriority >= 0) {
        m_mapGenerator->setForcedBiome(regionData.biomePriority);
    }
    else {
        m_mapGenerator->setForcedBiome(-1); // Сбрасываем принудительный биом
    }

    // Устанавливаем биомы, подходящие для этой планеты
    setupBiomesForPlanet(planetData);

    // Генерируем карту с заданным типом ландшафта
    bool result = m_mapGenerator->generate(tileMap, planetData.mainTerrainType);

    // После генерации карты добавляем региональные особенности
    if (result) {
        applyRegionalFeatures(tileMap, regionData, planetData);
    }

    return result;
}

void WorldGenerator::applyPlanetaryFeatures(TileMap* tileMap, const PlanetData& planetData) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Применяем планетарные свойства ко всем тайлам
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            MapTile* tile = tileMap->getTile(x, y);
            if (tile) {
                // Учитываем гравитацию при установке высоты объектов
                float height = tile->getHeight();
                float gravitationalHeight = height * (2.0f - planetData.gravityMultiplier);
                gravitationalHeight = std::max(0.0f, std::min(1.0f, gravitationalHeight));
                tile->setHeight(gravitationalHeight);

                // Учитываем атмосферу при установке прозрачности
                if (planetData.atmosphereDensity < 0.2f) {
                    // Разреженная атмосфера - более четкая видимость
                    tile->setTransparent(true);
                }
                else if (planetData.atmosphereDensity > 0.8f) {
                    // Плотная атмосфера - может мешать видимости
                    if (tile->isTransparent() && m_rng() % 100 < 30) {
                        tile->setTransparent(false);
                    }
                }

                // Учитываем общий уровень радиации планеты
                float baseRadiation = tile->getRadiationLevel();
                float finalRadiation = baseRadiation + planetData.radiationLevel * 0.5f;
                finalRadiation = std::max(0.0f, std::min(1.0f, finalRadiation));
                tile->setRadiationLevel(finalRadiation);
            }
        }
    }
}

void WorldGenerator::applyRegionalFeatures(TileMap* tileMap, const RegionData& regionData, const PlanetData& planetData) {
    int width = tileMap->getWidth();
    int height = tileMap->getHeight();

    // Уровень опасности влияет на количество опасных объектов в регионе
    int dangerObjects = static_cast<int>(width * height * 0.01f * regionData.dangerLevel);

    // Распределяем опасные объекты
    std::uniform_int_distribution<int> xDist(0, width - 1);
    std::uniform_int_distribution<int> yDist(0, height - 1);
    std::uniform_real_distribution<float> typeDist(0.0f, 1.0f);

    for (int i = 0; i < dangerObjects; ++i) {
        int x = xDist(m_rng);
        int y = yDist(m_rng);
        MapTile* tile = tileMap->getTile(x, y);

        if (tile && !tile->isWater()) {
            float rand = typeDist(m_rng);

            if (rand < 0.3f) {
                // Радиация
                tile->setRadiationLevel(0.5f + 0.5f * typeDist(m_rng));
            }
            else if (rand < 0.6f) {
                // Опасные образования
                tile->setType(TileType::ALIEN_GROWTH);

                // Добавляем декоративный элемент
                MapTile::Decoration dangerDecor(
                    150 + (m_rng() % 5),
                    "DangerousFormation",
                    0.7f + 0.6f * typeDist(m_rng),
                    true
                );
                tile->addDecoration(dangerDecor);
            }
            else {
                // Опасная зона (лава, ядовитые вещества)
                if (rand < 0.8f) {
                    tile->setType(TileType::LAVA);
                }
                else {
                    // Меняем цвет на токсично-зеленый
                    tile->setColor({ 100, 200, 50, 255 });

                    // Добавляем декоративный эффект
                    MapTile::Decoration toxicDecor(
                        160,
                        "ToxicEffect",
                        0.6f + 0.8f * typeDist(m_rng),
                        true
                    );
                    tile->addDecoration(toxicDecor);
                }
            }
        }
    }
}

WorldGenerator::PlanetData WorldGenerator::generateRandomPlanet(TileMap* tileMap) {
    PlanetData planetData;

    // Генерируем случайные параметры планеты
    std::uniform_real_distribution<float> tempDist(-50.0f, 100.0f);
    std::uniform_real_distribution<float> valueDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> terrainTypeDist(0, 5); // Соответствует enum GenerationType
    std::uniform_int_distribution<int> nameDist(0, 3); // Тема названия

    // Устанавливаем параметры
    planetData.seed = m_rng();
    planetData.averageTemperature = tempDist(m_rng);
    planetData.atmosphereDensity = valueDist(m_rng);
    planetData.gravityMultiplier = 0.2f + 1.6f * valueDist(m_rng);
    planetData.waterCoverage = valueDist(m_rng);
    planetData.radiationLevel = 0.7f * valueDist(m_rng); // Максимум 0.7 для случайных планет
    planetData.resourceRichness = valueDist(m_rng);

    // Устанавливаем тип ландшафта
    planetData.mainTerrainType = static_cast<MapGenerator::GenerationType>(terrainTypeDist(m_rng));

    // Устанавливаем наличие жизни с учетом условий
    // Жизнь более вероятна при благоприятных условиях
    float lifeChance = 0.5f;

    // Экстремальные температуры уменьшают шанс
    if (planetData.averageTemperature < -20.0f || planetData.averageTemperature > 60.0f) {
        lifeChance *= 0.3f;
    }

    // Отсутствие атмосферы делает жизнь маловероятной
    if (planetData.atmosphereDensity < 0.2f) {
        lifeChance *= 0.1f;
    }

    // Высокая радиация уменьшает шанс
    if (planetData.radiationLevel > 0.5f) {
        lifeChance *= 0.2f;
    }

    // Отсутствие воды делает жизнь маловероятной
    if (planetData.waterCoverage < 0.1f) {
        lifeChance *= 0.2f;
    }

    // Генерируем случайное значение и сравниваем с шансом на жизнь
    float lifeRoll = valueDist(m_rng);
    planetData.hasLife = (lifeRoll < lifeChance);

    // Генерируем тематическое название
    planetData.name = generateThematicName(nameDist(m_rng));

    // Генерируем описание
    planetData.description = generatePlanetDescription(planetData);

    // Генерируем саму планету, если указана карта
    if (tileMap) {
        generatePlanet(tileMap, planetData);
    }

    return planetData;
}

WorldGenerator::PlanetData WorldGenerator::generateCustomPlanet(TileMap* tileMap, float averageTemperature, float waterCoverage, MapGenerator::GenerationType terrainType) {
    PlanetData planetData;

    // Устанавливаем заданные параметры
    planetData.seed = m_rng();
    planetData.averageTemperature = averageTemperature;
    planetData.waterCoverage = std::max(0.0f, std::min(1.0f, waterCoverage));
    planetData.mainTerrainType = terrainType;

    // Генерируем остальные параметры с учетом типа ландшафта
    setupPlanetParameters(terrainType, planetData);

    // Генерируем название и описание
    planetData.name = generatePlanetName();
    planetData.description = generatePlanetDescription(planetData);

    // Генерируем саму планету, если указана карта
    if (tileMap) {
        generatePlanet(tileMap, planetData);
    }

    return planetData;
}

void WorldGenerator::setupPlanetParameters(MapGenerator::GenerationType terrainType, PlanetData& planetData) {
    std::uniform_real_distribution<float> valueDist(0.0f, 1.0f);

    // Настраиваем параметры в зависимости от типа ландшафта
    switch (terrainType) {
    case MapGenerator::GenerationType::ARCHIPELAGO:
        // Архипелаг - теплый, высокое содержание воды
        planetData.atmosphereDensity = 0.6f + 0.3f * valueDist(m_rng);
        planetData.gravityMultiplier = 0.7f + 0.6f * valueDist(m_rng);
        planetData.waterCoverage = std::max(0.6f, planetData.waterCoverage);
        planetData.radiationLevel = 0.1f * valueDist(m_rng);
        planetData.resourceRichness = 0.3f + 0.4f * valueDist(m_rng);
        planetData.hasLife = valueDist(m_rng) < 0.7f; // Высокий шанс на жизнь
        break;

    case MapGenerator::GenerationType::MOUNTAINOUS:
        // Горный - прохладный, низкое содержание воды
        planetData.atmosphereDensity = 0.4f + 0.4f * valueDist(m_rng);
        planetData.gravityMultiplier = 0.8f + 0.8f * valueDist(m_rng);
        planetData.waterCoverage = std::min(0.4f, planetData.waterCoverage);
        planetData.radiationLevel = 0.2f + 0.3f * valueDist(m_rng);
        planetData.resourceRichness = 0.6f + 0.4f * valueDist(m_rng); // Богатый ресурсами
        planetData.hasLife = valueDist(m_rng) < 0.4f; // Средний шанс на жизнь
        break;

    case MapGenerator::GenerationType::CRATER:
        // Кратерный - часто холодный, низкое содержание воды
        planetData.atmosphereDensity = 0.1f + 0.3f * valueDist(m_rng);
        planetData.gravityMultiplier = 0.5f + 0.5f * valueDist(m_rng);
        planetData.waterCoverage = std::min(0.3f, planetData.waterCoverage);
        planetData.radiationLevel = 0.3f + 0.4f * valueDist(m_rng); // Повышенная радиация
        planetData.resourceRichness = 0.4f + 0.6f * valueDist(m_rng);
        planetData.hasLife = valueDist(m_rng) < 0.2f; // Низкий шанс на жизнь
        break;

    case MapGenerator::GenerationType::VOLCANIC:
        // Вулканический - горячий, среднее содержание воды
        planetData.atmosphereDensity = 0.5f + 0.5f * valueDist(m_rng);
        planetData.gravityMultiplier = 1.0f + 0.5f * valueDist(m_rng);
        // Обеспечиваем минимальную температуру для вулканического мира
        planetData.averageTemperature = std::max(40.0f, planetData.averageTemperature);
        planetData.radiationLevel = 0.4f + 0.4f * valueDist(m_rng); // Высокая радиация
        planetData.resourceRichness = 0.7f + 0.3f * valueDist(m_rng); // Очень богатый ресурсами
        planetData.hasLife = valueDist(m_rng) < 0.3f; // Низкий шанс на жизнь
        break;

    case MapGenerator::GenerationType::ALIEN:
        // Инопланетный - экстремальные условия
        planetData.atmosphereDensity = 0.2f + 0.8f * valueDist(m_rng); // Случайная атмосфера
        planetData.gravityMultiplier = 0.3f + 1.4f * valueDist(m_rng); // Случайная гравитация
        planetData.radiationLevel = 0.5f + 0.5f * valueDist(m_rng); // Высокая радиация
        planetData.resourceRichness = 0.5f + 0.5f * valueDist(m_rng); // Потенциально богатые ресурсы
        planetData.hasLife = valueDist(m_rng) < 0.8f; // Высокий шанс на (странную) жизнь
        break;

    case MapGenerator::GenerationType::DEFAULT:
    default:
        // Стандартный - умеренные условия
        planetData.atmosphereDensity = 0.4f + 0.3f * valueDist(m_rng);
        planetData.gravityMultiplier = 0.8f + 0.4f * valueDist(m_rng);
        planetData.radiationLevel = 0.3f * valueDist(m_rng);
        planetData.resourceRichness = 0.4f + 0.3f * valueDist(m_rng);
        planetData.hasLife = valueDist(m_rng) < 0.5f; // 50% шанс на жизнь
        break;
    }
}

bool WorldGenerator::setupBiomesForPlanet(const PlanetData& planetData) {
    // Сбрасываем текущие биомы
    clearBiomes();

    // Устанавливаем биомы в генератор карт
    if (m_mapGenerator) {
        m_mapGenerator->setupDefaultBiomes();

        // Создаем дополнительные уникальные биомы для планеты
        createUniqueBiomes(planetData, 2 + (m_rng() % 3)); // 2-4 уникальных биома

        // Копируем биомы из генератора карт для синхронизации
        for (const auto& biome : m_mapGenerator->getBiomes()) {
            bool alreadyExists = false;
            for (const auto& existingBiome : m_biomes) {
                if (existingBiome->getId() == biome->getId()) {
                    alreadyExists = true;
                    break;
                }
            }

            if (!alreadyExists) {
                m_biomes.push_back(biome);
            }
        }

        return true;
    }

    return false;
}

void WorldGenerator::createUniqueBiomes(const PlanetData& planetData, int count) {
    if (!m_mapGenerator) return;

    // Генерируем уникальные биомы для планеты
    std::uniform_real_distribution<float> valueDist(0.0f, 1.0f);

    // Определяем базовый ID для новых биомов (избегаем конфликтов с существующими)
    int baseId = 1000;
    for (const auto& biome : m_biomes) {
        baseId = std::max(baseId, biome->getId() + 1);
    }

    for (int i = 0; i < count; ++i) {
        // Создаем новый биом
        std::string biomeName;
        std::string biomeDesc;

        // Определяем название и описание биома на основе типа планеты
        switch (planetData.mainTerrainType) {
        case MapGenerator::GenerationType::ARCHIPELAGO:
        {
            std::vector<std::string> names = {
                "Коралловый риф", "Мангровые заросли", "Прибрежная лагуна", "Тропический архипелаг"
            };
            std::vector<std::string> descs = {
                "Богатая морская экосистема с коралловыми рифами и разнообразной подводной жизнью.",
                "Затопленные леса, служащие домом для множества адаптированных к солоноватой воде видов.",
                "Мелководная экосистема, защищенная от открытого океана.",
                "Группа тропических островов с уникальной изолированной флорой и фауной."
            };

            int index = m_rng() % names.size();
            biomeName = names[index];
            biomeDesc = descs[index];
            break;
        }
        case MapGenerator::GenerationType::MOUNTAINOUS:
        {
            std::vector<std::string> names = {
                "Альпийские луга", "Скалистые пики", "Горное плато", "Ледниковая долина"
            };
            std::vector<std::string> descs = {
                "Высокогорные луга с редкими, адаптированными к экстремальным условиям растениями.",
                "Голые скалистые вершины, подверженные постоянным ветрам и низким температурам.",
                "Высокогорное плато с особой микроклиматической зоной.",
                "Долина, сформированная древними ледниками, с характерной U-образной формой."
            };

            int index = m_rng() % names.size();
            biomeName = names[index];
            biomeDesc = descs[index];
            break;
        }
        case MapGenerator::GenerationType::CRATER:
        {
            std::vector<std::string> names = {
                "Метеоритная воронка", "Ударный бассейн", "Кратерное озеро", "Засыпанный кратер"
            };
            std::vector<std::string> descs = {
                "Глубокая впадина, образованная метеоритным ударом, с измененной геологией и микроклиматом.",
                "Огромная ударная зона с особой геологической структурой и высоким уровнем минералов.",
                "Кратер, заполненный водой, создающий уникальную изолированную экосистему.",
                "Кратер, частично заполненный осадочными породами за тысячелетия эрозии."
            };

            int index = m_rng() % names.size();
            biomeName = names[index];
            biomeDesc = descs[index];
            break;
        }
        case MapGenerator::GenerationType::VOLCANIC:
        {
            std::vector<std::string> names = {
                "Термальные источники", "Вулканический пепел", "Застывшая лава", "Геотермальная зона"
            };
            std::vector<std::string> descs = {
                "Зона с горячими источниками, богатыми минералами и экстремофильными микроорганизмами.",
                "Территория, покрытая вулканическим пеплом, создающим особые условия для почвы.",
                "Ландшафт из застывшей лавы, медленно трансформирующийся под воздействием эрозии и жизни.",
                "Активная геотермальная зона с гейзерами, грязевыми вулканами и горячими источниками."
            };

            int index = m_rng() % names.size();
            biomeName = names[index];
            biomeDesc = descs[index];
            break;
        }
        case MapGenerator::GenerationType::ALIEN:
        {
            std::vector<std::string> names = {
                "Кристаллический лес", "Биолюминесцентная равнина", "Пульсирующие холмы", "Живой ландшафт"
            };
            std::vector<std::string> descs = {
                "Странные кристаллические образования, напоминающие земные леса, но состоящие из минералов.",
                "Плоская территория, покрытая биолюминесцентными организмами, создающими невероятные световые шоу.",
                "Холмистая местность с необычной пульсирующей поверхностью, напоминающей дыхание живого существа.",
                "Территория, где граница между живым и неживым размыта, и сам ландшафт проявляет признаки жизни."
            };

            int index = m_rng() % names.size();
            biomeName = names[index];
            biomeDesc = descs[index];
            break;
        }
        default:
        {
            std::vector<std::string> names = {
                "Смешанный лес", "Равнина", "Предгорье", "Холмистая местность"
            };
            std::vector<std::string> descs = {
                "Разнообразный лесной биом с умеренным климатом и богатым биоразнообразием.",
                "Обширные равнины с травянистой растительностью и умеренными осадками.",
                "Переходная зона между равнинами и горами со смешанным ландшафтом.",
                "Холмистая территория с разнообразным ландшафтом и микроклиматами."
            };

            int index = m_rng() % names.size();
            biomeName = names[index];
            biomeDesc = descs[index];
            break;
        }
        }

        auto biome = std::make_shared<Biome>(baseId + i, biomeName);
        biome->setDescription(biomeDesc);

        // Настраиваем параметры биома с учетом планеты
        float tempOffset = 20.0f * (valueDist(m_rng) - 0.5f); // Случайное отклонение от средней температуры
        biome->setTemperatureRange(
            planetData.averageTemperature - 10.0f + tempOffset,
            planetData.averageTemperature + 10.0f + tempOffset
        );

        float humidityMin = std::max(0.1f, valueDist(m_rng) * 0.5f);
        float humidityMax = std::min(1.0f, humidityMin + 0.2f + valueDist(m_rng) * 0.3f);
        biome->setHumidityRange(humidityMin, humidityMax);

        float elevationMin = valueDist(m_rng) * 0.7f;
        float elevationMax = std::min(1.0f, elevationMin + 0.1f + valueDist(m_rng) * 0.2f);
        biome->setElevationRange(elevationMin, elevationMax);

        // Генерируем типы тайлов для биома
        std::vector<TileType> possibleTypes;

        // Выбираем подходящие типы тайлов на основе условий планеты и биома
        if (planetData.averageTemperature < 0.0f) {
            possibleTypes.push_back(TileType::SNOW);
            possibleTypes.push_back(TileType::ICE);
            possibleTypes.push_back(TileType::STONE);
        }
        else if (planetData.averageTemperature < 20.0f) {
            possibleTypes.push_back(TileType::GRASS);
            possibleTypes.push_back(TileType::STONE);
            possibleTypes.push_back(TileType::FOREST);
            possibleTypes.push_back(TileType::HILL);
        }
        else if (planetData.averageTemperature < 40.0f) {
            possibleTypes.push_back(TileType::GRASS);
            possibleTypes.push_back(TileType::SAND);
            possibleTypes.push_back(TileType::FOREST);
            possibleTypes.push_back(TileType::MUD);
        }
        else {
            possibleTypes.push_back(TileType::SAND);
            possibleTypes.push_back(TileType::STONE);
            possibleTypes.push_back(TileType::LAVA);
        }

        // Если планета имеет жизнь, добавляем соответствующие типы
        if (planetData.hasLife) {
            possibleTypes.push_back(TileType::ALIEN_GROWTH);
            possibleTypes.push_back(TileType::FOREST);
        }

        // Добавляем случайные типы тайлов с разными вероятностями
        for (int j = 0; j < std::min(4, static_cast<int>(possibleTypes.size())); ++j) {
            TileType type = possibleTypes[m_rng() % possibleTypes.size()];
            float probability = 0.1f + 0.8f * valueDist(m_rng);
            biome->addTileType(type, probability);
        }

        // Добавляем декорации
        for (int j = 0; j < 3; ++j) {
            std::string decorName = "Decor" + std::to_string(baseId + i * 10 + j);
            float probability = 0.05f + 0.2f * valueDist(m_rng);

            Biome::BiomeDecoration decor(
                baseId + i * 10 + j,
                decorName,
                probability,
                0.7f + 0.6f * valueDist(m_rng),
                1.0f + 1.0f * valueDist(m_rng),
                valueDist(m_rng) < 0.3f
            );

            biome->addDecoration(decor);
        }

        // Устанавливаем уровни опасности и ресурсов
        biome->setHazardLevel(0.1f + 0.8f * valueDist(m_rng));
        biome->setResourceLevel(0.1f + 0.8f * valueDist(m_rng));

        // Добавляем биом
        m_mapGenerator->addBiome(biome);
    }
}

std::string WorldGenerator::generatePlanetName() {
    // Генерируем случайное название планеты
    std::uniform_int_distribution<int> dist(0, 3);
    return generateThematicName(dist(m_rng));
}

std::string WorldGenerator::generateThematicName(int theme) {
    // Если тема не указана или указан случайный выбор, выбираем случайную тему
    if (theme < 0 || theme > 3) {
        std::uniform_int_distribution<int> themeDist(0, 3);
        theme = themeDist(m_rng);
    }

    std::string result;

    // Генераторы случайных индексов
    std::uniform_int_distribution<int> prefixDist(0, prefix.size() - 1);
    std::uniform_int_distribution<int> scifiRootDist(0, scifiRoots.size() - 1);
    std::uniform_int_distribution<int> naturalRootDist(0, naturalRoots.size() - 1);
    std::uniform_int_distribution<int> suffixDist(0, suffix.size() - 1);
    std::uniform_int_distribution<int> numberDist(1, 999);

    // Решаем, будет ли название содержать номер
    std::uniform_int_distribution<int> hasNumberDist(0, 5); // 1/6 шанс
    bool hasNumber = (hasNumberDist(m_rng) == 0);

    // Генерируем название в зависимости от темы
    switch (theme) {
    case 0: // Научно-фантастическая тема
    {
        result = prefix[prefixDist(m_rng)] + "-" + scifiRoots[scifiRootDist(m_rng)];

        // Добавляем суффикс с 50% вероятностью
        if (m_rng() % 2 == 0) {
            result += suffix[suffixDist(m_rng)];
        }

        // Добавляем номер, если нужно
        if (hasNumber) {
            result += " " + std::to_string(numberDist(m_rng));
        }
        break;
    }
    case 1: // Натуралистическая тема
    {
        result = naturalRoots[naturalRootDist(m_rng)];

        // Добавляем суффикс
        result += suffix[suffixDist(m_rng)];

        // Добавляем номер, если нужно
        if (hasNumber) {
            result += " " + std::to_string(numberDist(m_rng));
        }
        break;
    }
    case 2: // Смешанная тема
    {
        // Случайно выбираем prefix + naturalRoot или scifiRoot + suffix
        if (m_rng() % 2 == 0) {
            result = prefix[prefixDist(m_rng)] + " " + naturalRoots[naturalRootDist(m_rng)];
        }
        else {
            result = scifiRoots[scifiRootDist(m_rng)] + suffix[suffixDist(m_rng)];
        }

        // Добавляем номер, если нужно
        if (hasNumber) {
            result += " " + std::to_string(numberDist(m_rng));
        }
        break;
    }
    case 3: // Экзотическая тема
    {
        // Используем 2-3 корня вместе
        int rootCount = 2 + (m_rng() % 2);

        for (int i = 0; i < rootCount; ++i) {
            // Выбираем случайный корень
            if (m_rng() % 2 == 0) {
                result += scifiRoots[scifiRootDist(m_rng)];
            }
            else {
                result += naturalRoots[naturalRootDist(m_rng)].substr(0, 3); // Берем только первые 3 буквы
            }
        }

        // Делаем первую букву заглавной
        if (!result.empty()) {
            result[0] = std::toupper(result[0]);
        }

        // Добавляем номер с 50% вероятностью
        if (m_rng() % 2 == 0) {
            result += "-" + std::to_string(numberDist(m_rng));
        }
        break;
    }
    default: // Запасной вариант
        result = "Planet " + std::to_string(numberDist(m_rng));
        break;
    }

    return result;
}

std::string WorldGenerator::generatePlanetDescription(const PlanetData& planetData) {
    // Определяем индексы для дескрипторов в зависимости от параметров планеты
    int tempIndex = 0;
    if (planetData.averageTemperature < -20.0f) {
        tempIndex = 0; // Экстремальный холод
    }
    else if (planetData.averageTemperature < 10.0f) {
        tempIndex = 1; // Холодный
    }
    else if (planetData.averageTemperature < 30.0f) {
        tempIndex = 2; // Умеренный
    }
    else if (planetData.averageTemperature < 60.0f) {
        tempIndex = 3; // Теплый
    }
    else {
        tempIndex = 4; // Экстремальный жар
    }

    int atmosIndex = 0;
    if (planetData.atmosphereDensity < 0.1f) {
        atmosIndex = 0; // Отсутствие атмосферы
    }
    else if (planetData.atmosphereDensity < 0.3f) {
        atmosIndex = 1; // Разреженная атмосфера
    }
    else if (planetData.atmosphereDensity < 0.7f) {
        atmosIndex = 2; // Средняя атмосфера
    }
    else if (planetData.atmosphereDensity < 0.9f) {
        atmosIndex = 3; // Плотная атмосфера
    }
    else {
        atmosIndex = 4; // Экстремальная плотность
    }

    int waterIndex = 0;
    if (planetData.waterCoverage < 0.1f) {
        waterIndex = 0; // Безводный
    }
    else if (planetData.waterCoverage < 0.3f) {
        waterIndex = 1; // Немного воды
    }
    else if (planetData.waterCoverage < 0.6f) {
        waterIndex = 2; // Средний уровень воды
    }
    else if (planetData.waterCoverage < 0.9f) {
        waterIndex = 3; // Много воды
    }
    else {
        waterIndex = 4; // Водный мир
    }

    int gravIndex = 0;
    if (planetData.gravityMultiplier < 0.7f) {
        gravIndex = 0; // Низкая гравитация
    }
    else if (planetData.gravityMultiplier < 1.3f) {
        gravIndex = 1; // Стандартная гравитация
    }
    else {
        gravIndex = 2; // Высокая гравитация
    }

    int resIndex = 0;
    if (planetData.resourceRichness < 0.3f) {
        resIndex = 0; // Бедные ресурсы
    }
    else if (planetData.resourceRichness < 0.7f) {
        resIndex = 1; // Средние ресурсы
    }
    else {
        resIndex = 2; // Богатые ресурсы
    }

    int radIndex = 0;
    if (planetData.radiationLevel < 0.2f) {
        radIndex = 0; // Нет радиации
    }
    else if (planetData.radiationLevel < 0.5f) {
        radIndex = 1; // Небольшая радиация
    }
    else {
        radIndex = 2; // Высокая радиация
    }

    int lifeIndex = 0;
    if (!planetData.hasLife) {
        lifeIndex = 0; // Нет жизни
    }
    else if (planetData.averageTemperature < -10.0f || planetData.averageTemperature > 50.0f ||
        planetData.radiationLevel > 0.6f || planetData.atmosphereDensity < 0.2f) {
        lifeIndex = 1; // Простая жизнь
    }
    else {
        lifeIndex = 2; // Развитая жизнь
    }

    // Выбираем случайные дескрипторы из соответствующих категорий
    std::uniform_int_distribution<int> tempDist(0, temperatureDescriptors[tempIndex].size() - 1);
    std::uniform_int_distribution<int> atmosDist(0, atmosphereDescriptors[atmosIndex].size() - 1);
    std::uniform_int_distribution<int> waterDist(0, waterDescriptors[waterIndex].size() - 1);
    std::uniform_int_distribution<int> gravDist(0, gravityDescriptors[gravIndex].size() - 1);
    std::uniform_int_distribution<int> resDist(0, resourceDescriptors[resIndex].size() - 1);
    std::uniform_int_distribution<int> radDist(0, radiationDescriptors[radIndex].size() - 1);
    std::uniform_int_distribution<int> lifeDist(0, lifeDescriptors[lifeIndex].size() - 1);

    // Создаем базовое описание
    std::string temp = temperatureDescriptors[tempIndex][tempDist(m_rng)];
    std::string atmos = atmosphereDescriptors[atmosIndex][atmosDist(m_rng)];
    std::string water = waterDescriptors[waterIndex][waterDist(m_rng)];

    // Составляем полное описание
    std::ostringstream desc;

    // Первое предложение - общее описание
    desc << planetData.name << " - " << temp << ", " << atmos << " и " << water << " мир ";
    desc << gravityDescriptors[gravIndex][gravDist(m_rng)] << ". ";

    // Второе предложение - о ресурсах и радиации
    desc << "Этот " << resourceDescriptors[resIndex][resDist(m_rng)] << " и "
        << radiationDescriptors[radIndex][radDist(m_rng)] << " мир ";

    // Третье предложение - о жизни
    desc << "является " << lifeDescriptors[lifeIndex][lifeDist(m_rng)] << ". ";

    // Добавляем информацию в зависимости от типа рельефа
    switch (planetData.mainTerrainType) {
    case MapGenerator::GenerationType::ARCHIPELAGO:
        desc << "Поверхность покрыта многочисленными островами, окруженными океанами.";
        break;
    case MapGenerator::GenerationType::MOUNTAINOUS:
        desc << "Поверхность преимущественно гористая, с высокими пиками и глубокими долинами.";
        break;
    case MapGenerator::GenerationType::CRATER:
        desc << "Поверхность покрыта многочисленными кратерами от метеоритных ударов.";
        break;
    case MapGenerator::GenerationType::VOLCANIC:
        desc << "Обширные вулканические образования покрывают большую часть поверхности.";
        break;
    case MapGenerator::GenerationType::ALIEN:
        desc << "Инопланетные формы рельефа создают причудливый ландшафт, не похожий на земной.";
        break;
    default:
        desc << "Поверхность включает разнообразные равнинные и холмистые ландшафты.";
    }

    return desc.str();
}