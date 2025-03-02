#include "Biome.h"
#include <algorithm>
#include <iostream>
#include <numeric>

Biome::Biome(int id, const std::string& name, const SDL_Color& color, const std::string& description)
    : m_id(id), m_name(name), m_color(color), m_description(description), m_gen(m_rd()) {
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
    return (temperature >= m_minTemperature && temperature <= m_maxTemperature) &&
        (humidity >= m_minHumidity && humidity <= m_maxHumidity) &&
        (elevation >= m_minElevation && elevation <= m_maxElevation);
}

void Biome::addTileType(TileType type, float probability) {
    if (probability <= 0.0f) {
        if (m_tileDistribution.find(type) != m_tileDistribution.end()) {
            m_tileDistribution.erase(type);
        }
        return;
    }
    m_tileDistribution[type] = probability;
}

TileType Biome::getRandomTileType() const {
    if (m_tileDistribution.empty()) {
        return TileType::EMPTY;
    }

    float totalProbability = 0.0f;
    for (const auto& pair : m_tileDistribution) {
        totalProbability += pair.second;
    }

    if (totalProbability < 0.001f) {
        return m_tileDistribution.begin()->first;
    }

    std::uniform_real_distribution<float> dist(0.0f, totalProbability);
    float randValue = dist(m_gen);

    float cumulative = 0.0f;
    for (const auto& pair : m_tileDistribution) {
        cumulative += pair.second;
        if (randValue < cumulative) {
            return pair.first;
        }
    }

    return m_tileDistribution.rbegin()->first;
}

void Biome::addDecoration(const BiomeDecoration& decoration) {
    m_decorations.push_back(decoration);
}

std::vector<MapTile::Decoration> Biome::generateRandomDecorations(int count) const {
    std::vector<MapTile::Decoration> result;

    if (m_decorations.empty() || count <= 0) {
        return result;
    }

    std::uniform_real_distribution<float> probDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> scaleDist(0.0f, 1.0f);

    int maxDecorations = std::min(count, static_cast<int>(m_decorations.size()));

    for (const auto& bioDecor : m_decorations) {
        if (probDist(m_gen) < bioDecor.probability) {
            float scale = bioDecor.minScale + (bioDecor.maxScale - bioDecor.minScale) * scaleDist(m_gen);
            MapTile::Decoration decor(bioDecor.id, bioDecor.name, scale, bioDecor.animated);
            result.push_back(decor);

            if (static_cast<int>(result.size()) >= maxDecorations) {
                break;
            }
        }
    }

    return result;
}

void Biome::serialize(std::ostream& stream) const {
    stream << m_id << " " << m_name << "\n";
    stream << m_description << "\n";

    stream << m_minTemperature << " " << m_maxTemperature << "\n";
    stream << m_minHumidity << " " << m_maxHumidity << "\n";
    stream << m_minElevation << " " << m_maxElevation << "\n";

    stream << m_hazardLevel << " " << m_resourceLevel << "\n";

    stream << m_tileDistribution.size() << "\n";
    for (const auto& pair : m_tileDistribution) {
        stream << static_cast<int>(pair.first) << " " << pair.second << "\n";
    }

    stream << m_decorations.size() << "\n";
    for (const auto& decor : m_decorations) {
        stream << decor.id << " " << decor.name << " " << decor.probability << " "
            << decor.minScale << " " << decor.maxScale << " " << decor.animated << "\n";
    }

    // Сериализуем цвет
    stream << m_color.r << " " << m_color.g << " " << m_color.b << " " << m_color.a << "\n";
}

bool Biome::deserialize(std::istream& stream) {
    stream >> m_id;

    std::getline(stream, m_name);
    if (m_name.empty()) {
        std::getline(stream, m_name);
    }

    std::getline(stream, m_description);

    stream >> m_minTemperature >> m_maxTemperature;
    stream >> m_minHumidity >> m_maxHumidity;
    stream >> m_minElevation >> m_maxElevation;

    stream >> m_hazardLevel >> m_resourceLevel;

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

    size_t decorCount;
    stream >> decorCount;
    m_decorations.clear();

    for (size_t i = 0; i < decorCount; ++i) {
        BiomeDecoration decor;
        stream >> decor.id;

        std::string line;
        std::getline(stream, line);

        std::istringstream iss(line);
        iss >> decor.name >> decor.probability >> decor.minScale >> decor.maxScale >> decor.animated;

        m_decorations.push_back(decor);
    }

    // Десериализуем цвет
    stream >> m_color.r >> m_color.g >> m_color.b >> m_color.a;

    return !stream.fail();
}