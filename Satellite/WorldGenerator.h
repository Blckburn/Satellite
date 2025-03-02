#pragma once

#include "MapGenerator.h"
#include "TileMap.h"
#include "Biome.h"
#include <memory>
#include <random>
#include <vector>
#include <string>

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

private:
    unsigned int m_seed;                               ///< Сид генератора
    std::mt19937 m_rng;                               ///< Генератор случайных чисел
    std::vector<std::shared_ptr<Biome>> m_biomes;     ///< Список доступных биомов
    std::shared_ptr<MapGenerator> m_mapGenerator;     ///< Генератор карт
};