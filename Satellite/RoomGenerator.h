#pragma once

#include "TileMap.h"
#include <vector>
#include <random>
#include <string>

/**
 * @brief Класс для генерации комнатных карт в изометрическом пространстве
 */
class RoomGenerator {
public:
    /**
     * @brief Типы биомов для генерации карт
     */
    enum class BiomeType {
        DEFAULT,    ///< Стандартный биом со стенами и полом
        FOREST,     ///< Лесной биом с деревьями и травой
        DESERT,     ///< Пустынный биом с песком и камнями
        TUNDRA,     ///< Тундра со снегом и льдом
        VOLCANIC    ///< Вулканический биом с камнями и лавой
    };

    /**
     * @brief Структура для хранения информации о комнате
     */
    struct Room {
        int x = 0;              ///< X координата левого верхнего угла
        int y = 0;              ///< Y координата левого верхнего угла
        int width = 0;          ///< Ширина комнаты
        int height = 0;         ///< Высота комнаты
        TileType floorType = TileType::FLOOR;  ///< Тип пола
        TileType wallType = TileType::WALL;    ///< Тип стен
        int biomeId = 0;        ///< ID биома для комнаты
    };

    /**
     * @brief Тип комнаты для упрощения инициализации
     */
    typedef Room RoomType;

    /**
     * @brief Конструктор
     * @param seed Сид для генератора случайных чисел (0 = случайный)
     */
    RoomGenerator(unsigned int seed = 0);

    /**
     * @brief Деструктор
     */
    ~RoomGenerator();

    /**
     * @brief Генерация карты с комнатами
     * @param tileMap Указатель на карту для заполнения
     * @param biomeType Тип биома для генерации
     * @return true в случае успеха, false при ошибке
     */
    bool generateMap(TileMap* tileMap, BiomeType biomeType = BiomeType::DEFAULT);

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
     * @brief Установка ограничений размера комнат
     * @param minSize Минимальный размер комнаты
     * @param maxSize Максимальный размер комнаты
     */
    void setRoomSizeLimits(int minSize, int maxSize);

private:
    /**
     * @brief Сброс и реинициализация генератора
     */
    void resetGenerator();

    /**
     * @brief Очистка карты перед генерацией
     * @param tileMap Указатель на карту
     */
    void clearMap(TileMap* tileMap);

    /**
     * @brief Создание комнаты на карте
     * @param tileMap Указатель на карту
     * @param room Информация о комнате
     */
    void createRoom(TileMap* tileMap, const Room& room);

    /**
     * @brief Создание коридора между комнатами
     * @param tileMap Указатель на карту
     * @param room1 Первая комната
     * @param room2 Вторая комната
     * @param floorType Тип пола для коридора
     * @param wallType Тип стен для коридора
     */
    void createCorridor(TileMap* tileMap, const Room& room1, const Room& room2,
        TileType floorType, TileType wallType);

    /**
     * @brief Добавление стен вокруг коридора
     * @param tileMap Указатель на карту
     * @param startX Начальная X координата
     * @param endX Конечная X координата
     * @param startY Начальная Y координата
     * @param endY Конечная Y координата
     * @param x1 X координата центра первой комнаты
     * @param y1 Y координата центра первой комнаты
     * @param x2 X координата центра второй комнаты
     * @param y2 Y координата центра второй комнаты
     * @param floorType Тип пола
     * @param wallType Тип стен
     */
    void addWallsAroundCorridor(TileMap* tileMap, int startX, int endX, int startY, int endY,
        int x1, int y1, int x2, int y2,
        TileType floorType, TileType wallType);

    /**
     * @brief Добавление новой комнаты, соединенной коридором с существующей
     * @param tileMap Указатель на карту
     * @param rooms Вектор существующих комнат
     * @param biomeType Тип биома
     */
    void addRoomWithCorridor(TileMap* tileMap, std::vector<Room>& rooms, BiomeType biomeType);

    /**
     * @brief Проверка пересечения комнат
     * @param room1 Первая комната
     * @param room2 Вторая комната
     * @param padding Отступ между комнатами
     * @return true, если комнаты пересекаются
     */
    bool checkRoomOverlap(const Room& room1, const Room& room2, int padding);

    /**
     * @brief Получение случайного размера комнаты
     * @return Случайный размер в пределах [m_minRoomSize, m_maxRoomSize]
     */
    int getRandomRoomSize();

    /**
     * @brief Получение типа пола в зависимости от биома
     * @param biomeType Тип биома
     * @return Тип пола
     */
    TileType getBiomeFloorType(BiomeType biomeType);

    /**
     * @brief Получение типа стен в зависимости от биома
     * @param biomeType Тип биома
     * @return Тип стен
     */
    TileType getBiomeWallType(BiomeType biomeType);

    /**
     * @brief Получение типа комнаты в зависимости от биома
     * @param biomeType Тип биома
     * @return Структура с данными комнаты
     */
    RoomType getBiomeRoomType(BiomeType biomeType);

private:
    unsigned int m_seed;            ///< Сид генератора
    std::mt19937 m_rng;             ///< Генератор случайных чисел
    int m_maxRoomSize;              ///< Максимальный размер комнаты
    int m_minRoomSize;              ///< Минимальный размер комнаты
    int m_maxCorridorLength;        ///< Максимальная длина коридора
};