#include "Biome.h"
#include <algorithm>
#include <iostream>
#include <numeric>

Biome::Biome(int id, const std::string& name)
    : m_id(id), m_name(name), m_gen(m_rd()) {
}

Biome::~Biome() {
}

void Biome::setTemperatureRange(float min, float max) {
    if (min > max) {
        std::swap(min, max);
    }
    m_minTemperature = min;
    m_maxTemperature = max;
}

void Biome::setHumidityRange(float min, float max) {
    if (min > max) {
        std::swap(min, max);
    }
    m_minHumidity = std::max(0.0f, min);
    m_maxHumidity = std::min(1.0f, max);
}

void Biome::setElevationRange(float min, float max) {
    if (min > max) {
        std::swap(min, max);
    }
    m_minElevation = std::max(0.0f, min);
    m_maxElevation = std::min(1.0f, max);
}

bool Biome::matches(float temperature, float humidity, float elevation) const {
    // Проверяем, что значения находятся в допустимых диапазонах для биома
    return (temperature >= m_minTemperature && temperature <= m_maxTemperature) &&
        (humidity >= m_minHumidity && humidity <= m_maxHumidity) &&
        (elevation >= m_minElevation && elevation <= m_maxElevation);
}

void Biome::addTileType(TileType type, float probability) {
    if (probability <= 0.0f) {
        // Удаляем тип тайла, если вероятность <= 0
        if (m_tileDistribution.find(type) != m_tileDistribution.end()) {
            m_tileDistribution.erase(type);
        }
        return;
    }

    // Добавляем или обновляем вероятность
    m_tileDistribution[type] = probability;
}

TileType Biome::getRandomTileType() const {
    // Если нет типов тайлов, возвращаем пустой тайл
    if (m_tileDistribution.empty()) {
        return TileType::EMPTY;
    }

    // Считаем сумму всех вероятностей
    float totalProbability = 0.0f;
    for (const auto& pair : m_tileDistribution) {
        totalProbability += pair.second;
    }

    // Если сумма слишком мала, возвращаем первый тип тайла
    if (totalProbability < 0.001f) {
        return m_tileDistribution.begin()->first;
    }

    // Генерируем случайное число в диапазоне [0, totalProbability)
    std::uniform_real_distribution<float> dist(0.0f, totalProbability);
    float randValue = dist(m_gen);

    // Выбираем тип тайла на основе вероятности
    float cumulative = 0.0f;
    for (const auto& pair : m_tileDistribution) {
        cumulative += pair.second;
        if (randValue < cumulative) {
            return pair.first;
        }
    }

    // Если по какой-то причине мы не выбрали тип, возвращаем последний в списке
    return m_tileDistribution.rbegin()->first;
}

void Biome::addDecoration(const BiomeDecoration& decoration) {
    m_decorations.push_back(decoration);
}

std::vector<MapTile::Decoration> Biome::generateRandomDecorations(int count) const {
    std::vector<MapTile::Decoration> result;

    // Если нет декораций или запрошено 0 декораций, возвращаем пустой список
    if (m_decorations.empty() || count <= 0) {
        return result;
    }

    // Для каждой декорации проверяем, должна ли она быть добавлена
    std::uniform_real_distribution<float> probDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> scaleDist(0.0f, 1.0f);

    // Ограничиваем максимальное количество декораций
    int maxDecorations = std::min(count, static_cast<int>(m_decorations.size()));

    // Пробуем добавить декорации в случайном порядке
    for (const auto& bioDecor : m_decorations) {
        // Генерируем случайное число и сравниваем с вероятностью декорации
        if (probDist(m_gen) < bioDecor.probability) {
            // Генерируем случайный масштаб в указанном диапазоне
            float scale = bioDecor.minScale +
                (bioDecor.maxScale - bioDecor.minScale) * scaleDist(m_gen);

            // Создаем объект декорации и добавляем в результат
            MapTile::Decoration decor(bioDecor.id, bioDecor.name, scale, bioDecor.animated);
            result.push_back(decor);

            // Если достигли максимального количества, прекращаем
            if (static_cast<int>(result.size()) >= maxDecorations) {
                break;
            }
        }
    }

    return result;
}

void Biome::serialize(std::ostream& stream) const {
    // Записываем основные свойства биома
    stream << m_id << " " << m_name << "\n";
    stream << m_description << "\n";

    // Записываем диапазоны значений
    stream << m_minTemperature << " " << m_maxTemperature << "\n";
    stream << m_minHumidity << " " << m_maxHumidity << "\n";
    stream << m_minElevation << " " << m_maxElevation << "\n";

    // Записываем уровни опасности и ресурсов
    stream << m_hazardLevel << " " << m_resourceLevel << "\n";

    // Записываем распределение типов тайлов
    stream << m_tileDistribution.size() << "\n";
    for (const auto& pair : m_tileDistribution) {
        stream << static_cast<int>(pair.first) << " " << pair.second << "\n";
    }

    // Записываем декоративные элементы
    stream << m_decorations.size() << "\n";
    for (const auto& decor : m_decorations) {
        stream << decor.id << " " << decor.name << " " << decor.probability << " "
            << decor.minScale << " " << decor.maxScale << " " << decor.animated << "\n";
    }
}

bool Biome::deserialize(std::istream& stream) {
    // Читаем основные свойства биома
    stream >> m_id;

    // Имя может содержать пробелы, поэтому читаем до конца строки
    std::getline(stream, m_name);
    if (m_name.empty()) {
        std::getline(stream, m_name); // Повторное чтение, если первое чтение было пустым
    }

    std::getline(stream, m_description);

    // Читаем диапазоны значений
    stream >> m_minTemperature >> m_maxTemperature;
    stream >> m_minHumidity >> m_maxHumidity;
    stream >> m_minElevation >> m_maxElevation;

    // Читаем уровни опасности и ресурсов
    stream >> m_hazardLevel >> m_resourceLevel;

    // Читаем распределение типов тайлов
    size_t tileCount;
    stream >> tileCount;
    m_tileDistribution.clear();

    for (size_t i = 0; i < tileCount; ++i) {
        int typeInt;
        float probability;
        stream >> typeInt >> probability;

        TileType type = static_cast<TileType>(typeInt);
        m_tileDistribution[type] = probability;
    }

    // Читаем декоративные элементы
    size_t decorCount;
    stream >> decorCount;
    m_decorations.clear();

    for (size_t i = 0; i < decorCount; ++i) {
        BiomeDecoration decor;
        stream >> decor.id;

        // Имя может содержать пробелы
        std::string line;
        std::getline(stream, line);

        // Парсим строку для получения остальных параметров
        std::istringstream iss(line);
        iss >> decor.name >> decor.probability >> decor.minScale >> decor.maxScale >> decor.animated;

        m_decorations.push_back(decor);
    }

    return !stream.fail();
}