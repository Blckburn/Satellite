#pragma once

#include "MapTile.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>

/**
 * @brief Класс для управления картой из тайлов
 */
class TileMap {
public:
    /**
     * @brief Конструктор
     * @param width Ширина карты в тайлах
     * @param height Высота карты в тайлах
     */
    TileMap(int width, int height);

    /**
     * @brief Деструктор
     */
    ~TileMap();

    /**
     * @brief Инициализация карты
     * @return true в случае успеха, false при ошибке
     */
    bool initialize();

    /**
     * @brief Получение ширины карты
     * @return Ширина карты в тайлах
     */
    int getWidth() const { return m_width; }

    /**
     * @brief Получение высоты карты
     * @return Высота карты в тайлах
     */
    int getHeight() const { return m_height; }

    /**
     * @brief Проверка, находятся ли координаты в пределах карты
     * @param x X координата
     * @param y Y координата
     * @return true, если координаты в пределах карты, false в противном случае
     */
    bool isValidCoordinate(int x, int y) const;

    /**
     * @brief Получение тайла по координатам
     * @param x X координата
     * @param y Y координата
     * @return Указатель на тайл или nullptr, если координаты вне карты
     */
    MapTile* getTile(int x, int y);

    /**
     * @brief Получение тайла по координатам (константная версия)
     * @param x X координата
     * @param y Y координата
     * @return Константный указатель на тайл или nullptr, если координаты вне карты
     */
    const MapTile* getTile(int x, int y) const;

    /**
     * @brief Установка типа тайла по координатам
     * @param x X координата
     * @param y Y координата
     * @param type Тип тайла
     * @return true, если операция успешна, false в противном случае
     */
    bool setTileType(int x, int y, TileType type);

    /**
     * @brief Установка проходимости тайла по координатам
     * @param x X координата
     * @param y Y координата
     * @param walkable Признак проходимости
     * @return true, если операция успешна, false в противном случае
     */
    bool setTileWalkable(int x, int y, bool walkable);

    /**
     * @brief Установка прозрачности тайла по координатам
     * @param x X координата
     * @param y Y координата
     * @param transparent Признак прозрачности
     * @return true, если операция успешна, false в противном случае
     */
    bool setTileTransparent(int x, int y, bool transparent);

    /**
     * @brief Установка высоты тайла по координатам
     * @param x X координата
     * @param y Y координата
     * @param height Высота тайла
     * @return true, если операция успешна, false в противном случае
     */
    bool setTileHeight(int x, int y, float height);

    /**
     * @brief Заполнение прямоугольной области карты одним типом тайлов
     * @param startX Начальная X координата
     * @param startY Начальная Y координата
     * @param endX Конечная X координата
     * @param endY Конечная Y координата
     * @param type Тип тайла
     */
    void fillRect(int startX, int startY, int endX, int endY, TileType type);

    /**
     * @brief Создание прямоугольной комнаты с полом и стенами
     * @param startX Начальная X координата
     * @param startY Начальная Y координата
     * @param endX Конечная X координата
     * @param endY Конечная Y координата
     * @param floorType Тип тайла для пола
     * @param wallType Тип тайла для стен
     */
    void createRoom(int startX, int startY, int endX, int endY, TileType floorType, TileType wallType);

    /**
     * @brief Создание горизонтального коридора
     * @param startX Начальная X координата
     * @param endX Конечная X координата
     * @param y Y координата
     * @param floorType Тип тайла для пола
     */
    void createHorizontalCorridor(int startX, int endX, int y, TileType floorType);

    /**
     * @brief Создание вертикального коридора
     * @param x X координата
     * @param startY Начальная Y координата
     * @param endY Конечная Y координата
     * @param floorType Тип тайла для пола
     */
    void createVerticalCorridor(int x, int startY, int endY, TileType floorType);

    /**
     * @brief Создание двери
     * @param x X координата
     * @param y Y координата
     * @param doorType Тип тайла для двери (по умолчанию TileType::DOOR)
     */
    void createDoor(int x, int y, TileType doorType = TileType::DOOR);

    /**
     * @brief Проверка проходимости тайла по координатам
     * @param x X координата
     * @param y Y координата
     * @return true, если тайл проходим, false в противном случае
     */
    bool isTileWalkable(int x, int y) const;

    /**
     * @brief Проверка прозрачности тайла по координатам
     * @param x X координата
     * @param y Y координата
     * @return true, если тайл прозрачен, false в противном случае
     */
    bool isTileTransparent(int x, int y) const;

    /**
     * @brief Очистка карты (заполнение пустыми тайлами)
     */
    void clear();

    /**
     * @brief Сохранение карты в файл
     * @param filename Имя файла
     * @return true в случае успеха, false при ошибке
     */
    bool saveToFile(const std::string& filename) const;

    /**
     * @brief Загрузка карты из файла
     * @param filename Имя файла
     * @return true в случае успеха, false при ошибке
     */
    bool loadFromFile(const std::string& filename);

private:
    int m_width;                           ///< Ширина карты в тайлах
    int m_height;                          ///< Высота карты в тайлах
    std::vector<std::vector<MapTile>> m_tiles; ///< Двумерный массив тайлов
};