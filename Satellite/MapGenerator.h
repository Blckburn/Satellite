#pragma once

#include "TileMap.h"
#include "Biome.h"
#include <random>
#include <vector>
#include <memory>
#include <functional>

/**
 * @brief Класс для процедурной генерации планетарных поверхностей
 */
class MapGenerator {
public:
    /**
     * @brief Перечисление типов генерации
     */
    enum class GenerationType {
        DEFAULT,        ///< Стандартная генерация с биомами
        ARCHIPELAGO,    ///< Архипелаг островов
        MOUNTAINOUS,    ///< Горный рельеф
        CRATER,         ///< Поверхность с кратерами
        VOLCANIC,       ///< Вулканическая поверхность
        ALIEN           ///< Инопланетный ландшафт
    };

    /**
     * @brief Конструктор
     * @param seed Сид для генератора случайных чисел (0 = случайный)
     */
    MapGenerator(unsigned int seed = 0);

    /**
     * @brief Деструктор
     */
    ~MapGenerator();

    /**
     * @brief Генерация карты
     * @param tileMap Указатель на карту для заполнения
     * @param genType Тип генерации
     * @return true в случае успеха, false при ошибке
     */
    bool generate(TileMap* tileMap, GenerationType genType = GenerationType::DEFAULT);

    /**
     * @brief Установка параметров генерации
     * @param temperature Средняя температура планеты (в градусах Цельсия)
     * @param humidity Средняя влажность (0.0 - 1.0)
     * @param roughness Неровность поверхности (0.0 - 1.0)
     * @param waterLevel Уровень воды (0.0 - 1.0)
     * @param resourceRichness Богатство ресурсами (0.0 - 1.0)
     */
    void setParameters(float temperature, float humidity, float roughness,
        float waterLevel, float resourceRichness);

    /**
     * @brief Добавление биома в генератор
     * @param biome Указатель на биом
     */
    void addBiome(std::shared_ptr<Biome> biome);

    /**
     * @brief Установка биомов по умолчанию
     */
    void setupDefaultBiomes();

    /**
     * @brief Очистка списка биомов
     */
    void clearBiomes();

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
     * @brief Установка масштаба шума
     * @param scale Масштаб (меньшие значения = более плавные изменения)
     */
    void setNoiseScale(float scale) { m_noiseScale = scale; }

    /**
     * @brief Генерация только определенного биома
     * @param biomeId ID биома для генерации (-1 = случайное распределение)
     */
    void setForcedBiome(int biomeId) { m_forcedBiomeId = biomeId; }

    /**
     * @brief Пересоздает генератор случайных чисел с текущим сидом
     */
    void resetGenerator();

    /**
     * @brief Получение списка доступных биомов
     * @return Вектор биомов
     */
    const std::vector<std::shared_ptr<Biome>>& getBiomes() const { return m_biomes; }

    /**
     * @brief Создание точки интереса на карте
     * @param tileMap Указатель на карту тайлов
     * @param centerX Центр по X
     * @param centerY Центр по Y
     * @param poiType Тип точки интереса
     * @param size Размер структуры
     */
    void placePOIStructure(TileMap* tileMap, int centerX, int centerY, TileType poiType, int size);

private:
    // Внутренние методы генерации

    /**
     * @brief Генерация базового рельефа
     * @param tileMap Указатель на карту
     */
    void generateTerrain(TileMap* tileMap);

    /**
     * @brief Генерация рельефа типа "архипелаг"
     * @param tileMap Указатель на карту
     */
    void generateArchipelago(TileMap* tileMap);

    /**
     * @brief Генерация горного рельефа
     * @param tileMap Указатель на карту
     */
    void generateMountains(TileMap* tileMap);

    /**
     * @brief Генерация рельефа с кратерами
     * @param tileMap Указатель на карту
     */
    void generateCraters(TileMap* tileMap);

    /**
     * @brief Генерация вулканического рельефа
     * @param tileMap Указатель на карту
     */
    void generateVolcanic(TileMap* tileMap);

    /**
     * @brief Генерация инопланетного рельефа
     * @param tileMap Указатель на карту
     */
    void generateAlien(TileMap* tileMap);

    /**
     * @brief Применение значений высоты к тайлам
     * @param tileMap Указатель на карту
     * @param heightMap Карта высот
     */
    void applyHeightMap(TileMap* tileMap, const std::vector<std::vector<float>>& heightMap);

    /**
     * @brief Применение значений температуры к тайлам
     * @param tileMap Указатель на карту
     * @param tempMap Карта температур
     */
    void applyTemperatureMap(TileMap* tileMap, const std::vector<std::vector<float>>& tempMap);

    /**
     * @brief Применение значений влажности к тайлам
     * @param tileMap Указатель на карту
     * @param humidityMap Карта влажности
     */
    void applyHumidityMap(TileMap* tileMap, const std::vector<std::vector<float>>& humidityMap);

    /**
     * @brief Распределение биомов на карте
     * @param tileMap Указатель на карту
     */
    void distributeBiomes(TileMap* tileMap);

    /**
     * @brief Размещение ресурсов на карте
     * @param tileMap Указатель на карту
     */
    void placeResources(TileMap* tileMap);

    /**
     * @brief Размещение особых точек интереса
     * @param tileMap Указатель на карту
     */
    void placePointsOfInterest(TileMap* tileMap);

    /**
     * @brief Размещение декоративных элементов
     * @param tileMap Указатель на карту
     */
    void placeDecorations(TileMap* tileMap);

    /**
     * @brief Генерация водных объектов (реки, озера)
     * @param tileMap Указатель на карту
     * @param heightMap Карта высот
     */
    void generateWaterBodies(TileMap* tileMap, const std::vector<std::vector<float>>& heightMap);

    /**
     * @brief Создание рек
     * @param tileMap Указатель на карту
     * @param heightMap Карта высот
     */
    void createRivers(TileMap* tileMap, const std::vector<std::vector<float>>& heightMap);

    /**
     * @brief Создание реки от заданной точки
     * @param tileMap Указатель на карту
     * @param heightMap Карта высот
     * @param startX Начальная X координата
     * @param startY Начальная Y координата
     */
    void createRiverFromPoint(TileMap* tileMap, const std::vector<std::vector<float>>& heightMap, int startX, int startY);

    /**
     * @brief Сглаживание границ биомов
     * @param tileMap Указатель на карту
     */
    void smoothBiomeBorders(TileMap* tileMap);

    /**
     * @brief Выбор подходящего биома для точки
     * @param temperature Температура
     * @param humidity Влажность
     * @param elevation Высота
     * @return Указатель на выбранный биом или nullptr
     */
    std::shared_ptr<Biome> selectBiome(float temperature, float humidity, float elevation);

    /**
     * @brief Генерация шума Перлина
     * @param x X координата
     * @param y Y координата
     * @param scale Масштаб шума
     * @param octaves Количество октав
     * @param persistence Персистенция
     * @param lacunarity Лакунарность
     * @param seed Сид генератора
     * @return Значение шума в диапазоне [0, 1]
     */
    float perlinNoise(float x, float y, float scale, int octaves, float persistence, float lacunarity, int seed) const;

    /**
     * @brief Генерация простого шума для Perlin
     * @param x X координата
     * @param y Y координата
     * @param seed Сид для генератора
     * @return Значение шума
     */
    float generateSimpleNoise(float x, float y, unsigned int seed) const;

    /**
     * @brief Генерация шума ворнои
     * @param x X координата
     * @param y Y координата
     * @param scale Масштаб шума
     * @param seed Сид генератора
     * @return Значение шума в диапазоне [0, 1]
     */
    float voronoiNoise(float x, float y, float scale, int seed) const;

    /**
     * @brief Создание случайного кратера
     * @param tileMap Указатель на карту
     * @param centerX Центр кратера по X
     * @param centerY Центр кратера по Y
     * @param radius Радиус кратера
     * @param depth Глубина кратера
     */
    void createCrater(TileMap* tileMap, int centerX, int centerY, int radius, float depth);

private:
    unsigned int m_seed;                                ///< Сид генератора
    std::mt19937 m_rng;                                 ///< Генератор случайных чисел
    std::vector<std::shared_ptr<Biome>> m_biomes;       ///< Список доступных биомов

    // Параметры генерации
    float m_baseTemperature = 20.0f;                    ///< Базовая температура
    float m_baseHumidity = 0.5f;                        ///< Базовая влажность
    float m_terrainRoughness = 0.5f;                    ///< Неровность рельефа
    float m_waterLevel = 0.3f;                          ///< Уровень воды
    float m_resourceRichness = 0.5f;                    ///< Богатство ресурсами
    float m_noiseScale = 0.1f;                          ///< Масштаб шума
    int m_forcedBiomeId = -1;                           ///< ID биома для принудительной генерации
};