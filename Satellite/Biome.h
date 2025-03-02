#pragma once

#include "TileType.h"
#include "MapTile.h"
#include <string>
#include <vector>
#include <map>
#include <random>

/**
 * @brief Класс для описания биома планеты
 */
class Biome {
public:
    /**
     * @brief Структура для описания декоративного элемента биома
     */
    struct BiomeDecoration {
        int id;                 ///< ID декорации
        std::string name;       ///< Название
        float probability;      ///< Вероятность появления (0.0 - 1.0)
        float minScale;         ///< Минимальный масштаб
        float maxScale;         ///< Максимальный масштаб
        bool animated;          ///< Признак анимированной декорации

        BiomeDecoration(int _id = 0, const std::string& _name = "", float _prob = 0.1f,
            float _minScale = 0.8f, float _maxScale = 1.2f, bool _animated = false)
            : id(_id), name(_name), probability(_prob), minScale(_minScale), maxScale(_maxScale), animated(_animated) {
        }
    };

    /**
     * @brief Конструктор
     * @param id Идентификатор биома
     * @param name Название биома
     */
    Biome(int id, const std::string& name);

    /**
     * @brief Деструктор
     */
    ~Biome();

    /**
     * @brief Получение ID биома
     * @return ID биома
     */
    int getId() const { return m_id; }

    /**
     * @brief Получение названия биома
     * @return Название биома
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Установка описания биома
     * @param description Описание биома
     */
    void setDescription(const std::string& description) { m_description = description; }

    /**
     * @brief Получение описания биома
     * @return Описание биома
     */
    const std::string& getDescription() const { return m_description; }

    /**
     * @brief Установка температурного диапазона
     * @param min Минимальная температура
     * @param max Максимальная температура
     */
    void setTemperatureRange(float min, float max);

    /**
     * @brief Получение минимальной температуры
     * @return Минимальная температура биома
     */
    float getMinTemperature() const { return m_minTemperature; }

    /**
     * @brief Получение максимальной температуры
     * @return Максимальная температура биома
     */
    float getMaxTemperature() const { return m_maxTemperature; }

    /**
     * @brief Установка диапазона влажности
     * @param min Минимальная влажность (0.0 - 1.0)
     * @param max Максимальная влажность (0.0 - 1.0)
     */
    void setHumidityRange(float min, float max);

    /**
     * @brief Получение минимальной влажности
     * @return Минимальная влажность биома
     */
    float getMinHumidity() const { return m_minHumidity; }

    /**
     * @brief Получение максимальной влажности
     * @return Максимальная влажность биома
     */
    float getMaxHumidity() const { return m_maxHumidity; }

    /**
     * @brief Установка диапазона высот
     * @param min Минимальная высота (0.0 - 1.0)
     * @param max Максимальная высота (0.0 - 1.0)
     */
    void setElevationRange(float min, float max);

    /**
     * @brief Получение минимальной высоты
     * @return Минимальная высота биома
     */
    float getMinElevation() const { return m_minElevation; }

    /**
     * @brief Получение максимальной высоты
     * @return Максимальная высота биома
     */
    float getMaxElevation() const { return m_maxElevation; }

    /**
     * @brief Проверка соответствия биома параметрам среды
     * @param temperature Температура
     * @param humidity Влажность (0.0 - 1.0)
     * @param elevation Высота (0.0 - 1.0)
     * @return true, если биом соответствует указанным параметрам
     */
    bool matches(float temperature, float humidity, float elevation) const;

    /**
     * @brief Добавление типа тайла с указанной вероятностью
     * @param type Тип тайла
     * @param probability Вероятность (0.0 - 1.0)
     */
    void addTileType(TileType type, float probability);

    /**
     * @brief Получение случайного типа тайла, характерного для биома
     * @return Тип тайла
     */
    TileType getRandomTileType() const;

    /**
     * @brief Добавление декоративного элемента, характерного для биома
     * @param decoration Информация о декоративном элементе
     */
    void addDecoration(const BiomeDecoration& decoration);

    /**
     * @brief Получение списка всех возможных декораций биома
     * @return Список декораций
     */
    const std::vector<BiomeDecoration>& getDecorations() const { return m_decorations; }

    /**
     * @brief Получение случайных декораций для тайла
     * @param count Максимальное количество декораций
     * @return Список сгенерированных декораций
     */
    std::vector<MapTile::Decoration> generateRandomDecorations(int count = 3) const;

    /**
     * @brief Проверка опасности биома
     * @return true, если биом считается опасным
     */
    bool isHazardous() const { return m_hazardLevel > 0.5f; }

    /**
     * @brief Установка уровня опасности биома
     * @param level Уровень опасности (0.0 - 1.0)
     */
    void setHazardLevel(float level) { m_hazardLevel = level; }

    /**
     * @brief Получение уровня опасности биома
     * @return Уровень опасности (0.0 - 1.0)
     */
    float getHazardLevel() const { return m_hazardLevel; }

    /**
     * @brief Установка уровня ресурсов биома
     * @param level Уровень ресурсов (0.0 - 1.0)
     */
    void setResourceLevel(float level) { m_resourceLevel = level; }

    /**
     * @brief Получение уровня ресурсов биома
     * @return Уровень ресурсов (0.0 - 1.0)
     */
    float getResourceLevel() const { return m_resourceLevel; }

    /**
     * @brief Сериализация биома в поток
     * @param stream Выходной поток
     */
    void serialize(std::ostream& stream) const;

    /**
     * @brief Десериализация биома из потока
     * @param stream Входной поток
     * @return true в случае успеха, false при ошибке
     */
    bool deserialize(std::istream& stream);

private:
    int m_id;                                          ///< ID биома
    std::string m_name;                                ///< Название биома
    std::string m_description;                         ///< Описание биома

    float m_minTemperature = -100.0f;                  ///< Минимальная температура
    float m_maxTemperature = 100.0f;                   ///< Максимальная температура
    float m_minHumidity = 0.0f;                        ///< Минимальная влажность
    float m_maxHumidity = 1.0f;                        ///< Максимальная влажность
    float m_minElevation = 0.0f;                       ///< Минимальная высота
    float m_maxElevation = 1.0f;                       ///< Максимальная высота

    float m_hazardLevel = 0.0f;                        ///< Уровень опасности биома (0.0 - 1.0)
    float m_resourceLevel = 0.1f;                      ///< Уровень ресурсов биома (0.0 - 1.0)

    std::map<TileType, float> m_tileDistribution;      ///< Распределение типов тайлов
    std::vector<BiomeDecoration> m_decorations;        ///< Декоративные элементы

    mutable std::random_device m_rd;                   ///< Генератор случайных чисел
    mutable std::mt19937 m_gen;                        ///< Мерсенн Твистер
};