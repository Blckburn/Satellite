#pragma once

#include "MapGenerator.h"
#include "TileMap.h"
#include "WorldGenerator.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <queue>
#include <atomic>

/**
 * @brief Класс для генерации больших карт по частям
 */
class ChunkedMapGenerator {
public:
    /**
     * @brief Структура для хранения координат чанка
     */
    struct ChunkCoord {
        int x;
        int y;

        // Хеш-функция для использования в unordered_map
        struct Hash {
            std::size_t operator()(const ChunkCoord& coord) const {
                return std::hash<int>()(coord.x) ^ (std::hash<int>()(coord.y) << 1);
            }
        };

        // Оператор сравнения для использования в unordered_map
        bool operator==(const ChunkCoord& other) const {
            return x == other.x && y == other.y;
        }
    };

    /**
     * @brief Конструктор
     * @param chunkSize Размер чанка в тайлах
     * @param seed Сид для генератора
     */
    ChunkedMapGenerator(int chunkSize = 16, unsigned int seed = 0);

    /**
     * @brief Деструктор
     */
    ~ChunkedMapGenerator();

    /**
     * @brief Инициализация генератора
     * @return true в случае успеха, false при ошибке
     */
    bool initialize();

    /**
     * @brief Установка параметров планеты
     * @param planetData Данные о планете
     */
    void setPlanetData(const WorldGenerator::PlanetData& planetData);

    /**
     * @brief Обновление видимой области карты
     * @param tileMap Указатель на карту
     * @param centerX X координата центра области в тайлах
     * @param centerY Y координата центра области в тайлах
     * @param radius Радиус видимой области в тайлах
     */
    void updateVisibleArea(TileMap* tileMap, int centerX, int centerY, int radius);

    /**
     * @brief Генерация чанка
     * @param tileMap Указатель на карту
     * @param chunkX X координата чанка
     * @param chunkY Y координата чанка
     * @param immediate Флаг немедленной генерации (true) или в отдельном потоке (false)
     */
    void generateChunk(TileMap* tileMap, int chunkX, int chunkY, bool immediate = false);

    /**
     * @brief Проверка, сгенерирован ли чанк
     * @param chunkX X координата чанка
     * @param chunkY Y координата чанка
     * @return true, если чанк сгенерирован, false в противном случае
     */
    bool isChunkGenerated(int chunkX, int chunkY) const;

    /**
     * @brief Очистка всех сгенерированных чанков
     */
    void clearChunks();

    /**
     * @brief Получение размера чанка
     * @return Размер чанка в тайлах
     */
    int getChunkSize() const { return m_chunkSize; }

    /**
     * @brief Установка размера чанка
     * @param size Размер чанка в тайлах
     */
    void setChunkSize(int size) { m_chunkSize = size; }

    /**
     * @brief Получение количества потоков для генерации
     * @return Количество потоков
     */
    int getThreadCount() const { return m_threadCount; }

    /**
     * @brief Установка количества потоков для генерации
     * @param count Количество потоков
     */
    void setThreadCount(int count);

    /**
     * @brief Получение количества сгенерированных чанков
     * @return Количество чанков
     */
    size_t getGeneratedChunkCount() const { return m_generatedChunks.size(); }

    /**
     * @brief Установка сида генератора
     * @param seed Сид
     */
    void setSeed(unsigned int seed);

private:
    /**
     * @brief Функция рабочего потока для генерации чанков
     */
    void workerThread();

    /**
     * @brief Преобразование координат тайла в координаты чанка
     * @param tileX X координата тайла
     * @param tileY Y координата тайла
     * @param chunkX X координата чанка (выходной параметр)
     * @param chunkY Y координата чанка (выходной параметр)
     */
    void tileToChunk(int tileX, int tileY, int& chunkX, int& chunkY) const;

    /**
     * @brief Преобразование координат чанка в координаты начального тайла чанка
     * @param chunkX X координата чанка
     * @param chunkY Y координата чанка
     * @param tileX X координата начального тайла (выходной параметр)
     * @param tileY Y координата начального тайла (выходной параметр)
     */
    void chunkToTile(int chunkX, int chunkY, int& tileX, int& tileY) const;

    /**
     * @brief Внутренняя функция генерации чанка
     * @param tileMap Указатель на карту
     * @param chunkX X координата чанка
     * @param chunkY Y координата чанка
     */
    void generateChunkInternal(TileMap* tileMap, int chunkX, int chunkY);

private:
    int m_chunkSize;                       ///< Размер чанка в тайлах
    int m_threadCount;                     ///< Количество потоков для генерации
    std::shared_ptr<MapGenerator> m_mapGenerator;  ///< Генератор карт
    WorldGenerator::PlanetData m_planetData;      ///< Данные о планете

    // Информация о сгенерированных чанках
    std::unordered_map<ChunkCoord, bool, ChunkCoord::Hash> m_generatedChunks;
    mutable std::mutex m_chunksMutex;

    // Механизм для многопоточной генерации
    std::vector<std::thread> m_workerThreads;
    std::queue<std::pair<ChunkCoord, TileMap*>> m_chunkQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCondition;
    std::atomic<bool> m_stopThreads;
};