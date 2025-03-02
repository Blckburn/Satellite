#include "ChunkedMapGenerator.h"
#include <iostream>
#include <cmath>
#include <algorithm>

ChunkedMapGenerator::ChunkedMapGenerator(int chunkSize, unsigned int seed)
    : m_chunkSize(chunkSize), m_threadCount(std::thread::hardware_concurrency()),
    m_stopThreads(false) {

    // Создаем генератор карт с заданным сидом
    m_mapGenerator = std::make_shared<MapGenerator>(seed);
}

ChunkedMapGenerator::~ChunkedMapGenerator() {
    // Остановка всех рабочих потоков
    m_stopThreads = true;
    m_queueCondition.notify_all();

    // Ожидаем завершения всех потоков
    for (auto& thread : m_workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

bool ChunkedMapGenerator::initialize() {
    if (!m_mapGenerator) {
        return false;
    }

    // Запускаем рабочие потоки для генерации чанков
    for (int i = 0; i < m_threadCount; ++i) {
        m_workerThreads.emplace_back(&ChunkedMapGenerator::workerThread, this);
    }

    return true;
}

void ChunkedMapGenerator::setPlanetData(const WorldGenerator::PlanetData& planetData) {
    m_planetData = planetData;

    // Устанавливаем параметры генерации
    m_mapGenerator->setParameters(
        planetData.averageTemperature,
        planetData.hasLife ? 0.5f : 0.3f, // Влажность зависит от наличия жизни
        0.5f, // Средняя неровность рельефа
        planetData.waterCoverage,
        planetData.resourceRichness
    );

    // Устанавливаем сид
    m_mapGenerator->setSeed(planetData.seed);

    // Очищаем ранее сгенерированные чанки
    clearChunks();
}

void ChunkedMapGenerator::updateVisibleArea(TileMap* tileMap, int centerX, int centerY, int radius) {
    if (!tileMap) return;

    // Преобразуем координаты центра в координаты чанка
    int centerChunkX, centerChunkY;
    tileToChunk(centerX, centerY, centerChunkX, centerChunkY);

    // Определяем количество чанков, необходимых для покрытия видимой области
    int chunkRadius = (radius + m_chunkSize - 1) / m_chunkSize + 1;

    // Собираем список чанков, которые нужно сгенерировать
    std::vector<std::pair<int, int>> chunksToGenerate;

    for (int dy = -chunkRadius; dy <= chunkRadius; ++dy) {
        for (int dx = -chunkRadius; dx <= chunkRadius; ++dx) {
            int chunkX = centerChunkX + dx;
            int chunkY = centerChunkY + dy;

            // Проверяем, находится ли чанк в видимой области
            // Для простоты используем квадратную область вместо круглой

            if (!isChunkGenerated(chunkX, chunkY)) {
                // Если чанк не был сгенерирован, добавляем его в список
                chunksToGenerate.push_back(std::make_pair(chunkX, chunkY));
            }
        }
    }

    // Сортируем чанки по расстоянию от центра (ближайшие сначала)
    std::sort(chunksToGenerate.begin(), chunksToGenerate.end(),
        [centerChunkX, centerChunkY](const auto& a, const auto& b) {
            int distA = (a.first - centerChunkX) * (a.first - centerChunkX) +
                (a.second - centerChunkY) * (a.second - centerChunkY);
            int distB = (b.first - centerChunkX) * (b.first - centerChunkX) +
                (b.second - centerChunkY) * (b.second - centerChunkY);
            return distA < distB;
        });

    // Генерируем ближайшие чанки немедленно, остальные в отдельном потоке
    const int immediateChunks = 1; // Количество чанков для немедленной генерации

    for (size_t i = 0; i < chunksToGenerate.size(); ++i) {
        if (i < immediateChunks) {
            // Генерируем чанк немедленно
            generateChunk(tileMap, chunksToGenerate[i].first, chunksToGenerate[i].second, true);
        }
        else {
            // Генерируем чанк в отдельном потоке
            generateChunk(tileMap, chunksToGenerate[i].first, chunksToGenerate[i].second, false);
        }
    }
}

void ChunkedMapGenerator::generateChunk(TileMap* tileMap, int chunkX, int chunkY, bool immediate) {
    if (!tileMap) return;

    // Проверяем, не был ли этот чанк уже сгенерирован или поставлен в очередь
    {
        std::lock_guard<std::mutex> lock(m_chunksMutex);
        ChunkCoord coord = { chunkX, chunkY };

        // Если чанк уже сгенерирован или находится в процессе генерации, выходим
        if (m_generatedChunks.find(coord) != m_generatedChunks.end()) {
            return;
        }

        // Помечаем чанк как "в процессе генерации"
        m_generatedChunks[coord] = false;
    }

    if (immediate) {
        // Генерируем чанк немедленно
        generateChunkInternal(tileMap, chunkX, chunkY);
    }
    else {
        // Добавляем чанк в очередь на генерацию
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            ChunkCoord coord = { chunkX, chunkY };
            m_chunkQueue.push(std::make_pair(coord, tileMap));
        }

        // Уведомляем один из рабочих потоков
        m_queueCondition.notify_one();
    }
}

bool ChunkedMapGenerator::isChunkGenerated(int chunkX, int chunkY) const {
    std::lock_guard<std::mutex> lock(m_chunksMutex);
    ChunkCoord coord = { chunkX, chunkY };

    auto it = m_generatedChunks.find(coord);
    if (it != m_generatedChunks.end()) {
        return it->second; // true, если чанк полностью сгенерирован
    }

    return false;
}

void ChunkedMapGenerator::clearChunks() {
    std::lock_guard<std::mutex> lock(m_chunksMutex);
    m_generatedChunks.clear();

    // Очищаем очередь чанков
    {
        std::lock_guard<std::mutex> queueLock(m_queueMutex);
        std::queue<std::pair<ChunkCoord, TileMap*>> empty;
        std::swap(m_chunkQueue, empty);
    }
}

void ChunkedMapGenerator::setSeed(unsigned int seed) {
    // Устанавливаем новый сид для генератора
    m_mapGenerator->setSeed(seed);

    // Обновляем сид в данных планеты
    m_planetData.seed = seed;

    // Очищаем ранее сгенерированные чанки
    clearChunks();
}

void ChunkedMapGenerator::setThreadCount(int count) {
    // Проверяем, что количество потоков изменилось
    if (count == m_threadCount) {
        return;
    }

    // Остановка всех текущих рабочих потоков
    m_stopThreads = true;
    m_queueCondition.notify_all();

    // Ожидаем завершения всех потоков
    for (auto& thread : m_workerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Устанавливаем новое количество потоков
    m_threadCount = count;
    m_workerThreads.clear();
    m_stopThreads = false;

    // Запускаем новые рабочие потоки
    for (int i = 0; i < m_threadCount; ++i) {
        m_workerThreads.emplace_back(&ChunkedMapGenerator::workerThread, this);
    }
}

void ChunkedMapGenerator::workerThread() {
    while (!m_stopThreads) {
        std::pair<ChunkCoord, TileMap*> task;
        bool hasTask = false;

        // Ожидаем задачу из очереди
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCondition.wait(lock, [this] {
                return !m_chunkQueue.empty() || m_stopThreads;
                });

            if (m_stopThreads) {
                break;
            }

            if (!m_chunkQueue.empty()) {
                task = m_chunkQueue.front();
                m_chunkQueue.pop();
                hasTask = true;
            }
        }

        // Если есть задача, выполняем ее
        if (hasTask) {
            generateChunkInternal(task.second, task.first.x, task.first.y);
        }
    }
}

void ChunkedMapGenerator::tileToChunk(int tileX, int tileY, int& chunkX, int& chunkY) const {
    // Преобразование координат тайла в координаты чанка
    // Учитываем отрицательные координаты (используем целочисленное деление с округлением вниз)
    chunkX = (tileX < 0) ? ((tileX + 1) / m_chunkSize - 1) : (tileX / m_chunkSize);
    chunkY = (tileY < 0) ? ((tileY + 1) / m_chunkSize - 1) : (tileY / m_chunkSize);
}

void ChunkedMapGenerator::chunkToTile(int chunkX, int chunkY, int& tileX, int& tileY) const {
    // Преобразование координат чанка в координаты начального тайла чанка
    tileX = chunkX * m_chunkSize;
    tileY = chunkY * m_chunkSize;
}

void ChunkedMapGenerator::generateChunkInternal(TileMap* tileMap, int chunkX, int chunkY) {
    if (!tileMap) return;

    // Вычисляем координаты начального тайла чанка
    int startX, startY;
    chunkToTile(chunkX, chunkY, startX, startY);

    // Создаем временную карту только для этого чанка
    TileMap chunkMap(m_chunkSize, m_chunkSize);
    chunkMap.initialize();

    // Генерируем содержимое чанка
    // Настраиваем параметры генерации с учетом смещения чанка
    m_mapGenerator->setSeed(m_planetData.seed + static_cast<unsigned int>(chunkX * 1000 + chunkY));

    // Генерируем карту для чанка с заданным типом ландшафта
    m_mapGenerator->generate(&chunkMap, m_planetData.mainTerrainType);

    // Копируем сгенерированные тайлы в основную карту
    for (int y = 0; y < m_chunkSize; ++y) {
        for (int x = 0; x < m_chunkSize; ++x) {
            int globalX = startX + x;
            int globalY = startY + y;

            // Проверяем, что координаты находятся в пределах основной карты
            if (tileMap->isValidCoordinate(globalX, globalY)) {
                const MapTile* sourceTile = chunkMap.getTile(x, y);
                MapTile* targetTile = tileMap->getTile(globalX, globalY);

                if (sourceTile && targetTile) {
                    // Копируем свойства тайла
                    targetTile->setType(sourceTile->getType());
                    targetTile->setHeight(sourceTile->getHeight());
                    targetTile->setColor(sourceTile->getColor());
                    targetTile->setWalkable(sourceTile->isWalkable());
                    targetTile->setTransparent(sourceTile->isTransparent());

                    // Копируем дополнительные свойства для планетарных поверхностей
                    targetTile->setTemperature(sourceTile->getTemperature());
                    targetTile->setHumidity(sourceTile->getHumidity());
                    targetTile->setElevation(sourceTile->getElevation());
                    targetTile->setRadiationLevel(sourceTile->getRadiationLevel());
                    targetTile->setResourceDensity(sourceTile->getResourceDensity());
                    targetTile->setBiomeId(sourceTile->getBiomeId());

                    // Копируем декорации
                    targetTile->clearDecorations();
                    for (const auto& decor : sourceTile->getDecorations()) {
                        targetTile->addDecoration(decor);
                    }
                }
            }
        }
    }

    // Помечаем чанк как полностью сгенерированный
    {
        std::lock_guard<std::mutex> lock(m_chunksMutex);
        ChunkCoord coord = { chunkX, chunkY };
        m_generatedChunks[coord] = true;
    }
}