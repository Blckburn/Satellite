#pragma once

#include "MapGenerator.h"
#include "TileMap.h"
#include "Biome.h"
#include <memory>
#include <random>
#include <vector>
#include <string>
#include <functional>

/**
 * @brief Класс для генерации планетарных миров
 */
class WorldGenerator {
public:
    /**
     * @brief Структура для описания планеты
     */
    struct PlanetData {
        std::string name;             ///< Название планеты
        std::string description;      ///< Описание планеты
        float averageTemperature;     ///< Средняя температура
        float atmosphereDensity;      ///< Плотность атмосферы (0.0 - 1.0)
        float gravityMultiplier;      ///< Множитель гравитации относительно Земли
        float waterCoverage;          ///< Процент поверхности, покрытой водой (0.0 - 1.0)
        float radiationLevel;         ///< Общий уровень радиации (0.0 - 1.0)
        float resourceRichness;       ///< Общее богатство ресурсами (0.0 - 1.0)
        MapGenerator::GenerationType mainTerrainType; ///< Основной тип ландшафта
        bool hasLife;                 ///< Наличие жизни
        unsigned int seed;            ///< Сид для генерации

        PlanetData();
    };

    /**
     * @brief Структура для описания региона планеты
     */
    struct RegionData {
        std::string name;             ///< Название региона
        int biomePriority;            ///< Приоритетный биом
        float dangerLevel;            ///< Уровень опасности (0.0 - 1.0)
        float resourceLevel;          ///< Уровень ресурсов (0.0 - 1.0)
        unsigned int seed;            ///< Сид для генерации

        RegionData();
    };

    /**
     * @brief Конструктор
     * @param seed Сид для генератора случайных чисел (0 = случайный)
     */
    WorldGenerator(unsigned int seed = 0);

    /**
     * @brief Деструктор
     */
    ~WorldGenerator();

    /**
     * @brief Генерация планеты
     * @param tileMap Указатель на карту для заполнения
     * @param planetData Данные о планете
     * @return true в случае успеха, false при ошибке
     */
    bool generatePlanet(TileMap* tileMap, const PlanetData& planetData);

    /**
     * @brief Генерация планеты с использованием кэша
     * @param tileMap Указатель на карту для заполнения
     * @param planetData Данные о планете
     * @return true в случае успеха, false при ошибке
     */
    bool generatePlanetWithCache(TileMap* tileMap, const PlanetData& planetData);

    /**
     * @brief Генерация региона планеты
     * @param tileMap Указатель на карту для заполнения
     * @param regionData Данные о регионе
     * @param planetData Данные о планете
     * @return true в случае успеха, false при ошибке
     */
    bool generateRegion(TileMap* tileMap, const RegionData& regionData, const PlanetData& planetData);

    /**
     * @brief Генерация случайной планеты
     * @param tileMap Указатель на карту для заполнения
     * @return Данные о сгенерированной планете
     */
    PlanetData generateRandomPlanet(TileMap* tileMap = nullptr);

    /**
     * @brief Генерация планеты с заданными параметрами
     * @param tileMap Указатель на карту для заполнения
     * @param averageTemperature Средняя температура
     * @param waterCoverage Процент поверхности, покрытой водой (0.0 - 1.0)
     * @param terrainType Основной тип рельефа
     * @return Данные о сгенерированной планете
     */
    PlanetData generateCustomPlanet(TileMap* tileMap, float averageTemperature, float waterCoverage, MapGenerator::GenerationType terrainType);

    /**
     * @brief Генерация названия планеты
     * @return Сгенерированное название
     */
    std::string generatePlanetName();

    /**
     * @brief Генерация описания планеты
     * @param planetData Данные о планете
     * @return Сгенерированное описание
     */
    std::string generatePlanetDescription(const PlanetData& planetData);

    /**
     * @brief Создание рельефа эрозии
     * @param tileMap Указатель на карту
     * @param intensity Интенсивность эрозии (0.0-1.0)
     */
    void createErosionPatterns(TileMap* tileMap, float intensity);

    /**
     * @brief Улучшение переходов между биомами
     * @param tileMap Указатель на карту
     */
    void enhanceBiomeTransitions(TileMap* tileMap);

    /**
     * @brief Создание достопримечательностей на карте
     * @param tileMap Указатель на карту
     * @param count Количество достопримечательностей
     */
    void createLandmarks(TileMap* tileMap, int count);

    /**
     * @brief Установка сида генератора
     * @param seed Сид (0 = случайный)
     */
    void setSeed(unsigned int seed);

    /**
     * @brief Получение текущего сида
     * @return Текущий сид
     */
    unsigned int getSeed() const { return m_seed; }

    /**
     * @brief Добавление биома
     * @param biome Указатель на биом
     */
    void addBiome(std::shared_ptr<Biome> biome);

    /**
     * @brief Получение списка биомов
     * @return Вектор с указателями на биомы
     */
    const std::vector<std::shared_ptr<Biome>>& getBiomes() const { return m_biomes; }

    /**
     * @brief Очистка списка биомов
     */
    void clearBiomes();

private:
    /**
     * @brief Генерация параметров планеты на основе типа рельефа
     * @param terrainType Тип рельефа
     * @param planetData Данные о планете (входной/выходной параметр)
     */
    void setupPlanetParameters(MapGenerator::GenerationType terrainType, PlanetData& planetData);

    /**
     * @brief Инициализация биомов по умолчанию
     */
    void setupDefaultBiomes();

    /**
     * @brief Создание биомов, соответствующих типу планеты
     * @param planetData Данные о планете
     * @return true, если биомы созданы успешно
     */
    bool setupBiomesForPlanet(const PlanetData& planetData);

    /**
     * @brief Создание уникальных биомов для планеты
     * @param planetData Данные о планете
     * @param count Количество биомов
     */
    void createUniqueBiomes(const PlanetData& planetData, int count);

    /**
     * @brief Генерация названия планеты по заданной теме
     * @param theme Тема для названия (0-3, где 0 = случайная)
     * @return Сгенерированное название
     */
    std::string generateThematicName(int theme);

    /**
     * @brief Применение планетарных особенностей к карте
     * @param tileMap Указатель на карту
     * @param planetData Данные о планете
     */
    void applyPlanetaryFeatures(TileMap* tileMap, const PlanetData& planetData);

    /**
     * @brief Применение региональных особенностей к карте
     * @param tileMap Указатель на карту
     * @param regionData Данные о регионе
     * @param planetData Данные о планете
     */
    void applyRegionalFeatures(TileMap* tileMap, const RegionData& regionData, const PlanetData& planetData);

    /**
     * @brief Создание горной вершины
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер вершины
     */
    void createMountainPeak(TileMap* tileMap, int centerX, int centerY, int size);

    /**
     * @brief Создание кратера
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер кратера
     */
    void createCraterFormation(TileMap* tileMap, int centerX, int centerY, int size);

    /**
     * @brief Создание каменного шпиля
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер структуры
     */
    void createRockSpire(TileMap* tileMap, int centerX, int centerY, int size);

    /**
     * @brief Создание древних руин
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер руин
     */
    void createAncientRuins(TileMap* tileMap, int centerX, int centerY, int size);

    /**
     * @brief Создание странного монумента
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер монумента
     */
    void createStrangeMonument(TileMap* tileMap, int centerX, int centerY, int size);

    /**
     * @brief Создание оазиса
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер оазиса
     */
    void createOasis(TileMap* tileMap, int centerX, int centerY, int size);

    /**
     * @brief Создание поля гейзеров
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер поля гейзеров
     */
    void createGeyserField(TileMap* tileMap, int centerX, int centerY, int size);

    /**
     * @brief Создание кристаллической формации
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер формации
     */
    void createCrystalFormation(TileMap* tileMap, int centerX, int centerY, int size);

    /**
     * @brief Создание инопланетной структуры
     * @param tileMap Указатель на карту
     * @param centerX Центр X
     * @param centerY Центр Y
     * @param size Размер структуры
     */
    void createAlienStructure(TileMap* tileMap, int centerX, int centerY, int size);

private:
    unsigned int m_seed;                               ///< Сид генератора
    std::mt19937 m_rng;                               ///< Генератор случайных чисел
    std::vector<std::shared_ptr<Biome>> m_biomes;     ///< Список доступных биомов
    std::shared_ptr<MapGenerator> m_mapGenerator;     ///< Генератор карт

    PlanetData m_lastPlanetData;                      ///< Данные последней сгенерированной планеты
    std::shared_ptr<TileMap> m_cachedTileMap;         ///< Кэш последней сгенерированной карты
};