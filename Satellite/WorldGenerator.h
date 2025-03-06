#pragma once

#include "TileMap.h"
#include "Door.h"
#include "Terminal.h"
#include "PickupItem.h"
#include <memory>
#include <vector>
#include <set>
#include "Switch.h"

class Engine;
class MapScene;
class Player;

/**
 * @brief Класс для процедурной генерации игрового мира и размещения объектов
 */
class WorldGenerator {
public:
    /**
     * @brief Конструктор
     * @param tileMap Указатель на карту тайлов
     * @param engine Указатель на движок
     * @param mapScene Указатель на сцену карты
     * @param player Указатель на игрока
     */
    WorldGenerator(std::shared_ptr<TileMap> tileMap, Engine* engine,
        MapScene* mapScene, std::shared_ptr<Player> player);

    /**
     * @brief Генерация тестовой карты
     * @param biomeType Тип биома для генерации
     * @return Координаты стартовой позиции игрока
     */
    std::pair<float, float> generateTestMap(int biomeType);

    /**
     * @brief Генерация дверей в коридорах и переходах
     * @param doorProbability Вероятность размещения двери в подходящем месте
     * @param maxDoors Максимальное количество дверей
     */
    void generateDoors(float doorProbability = 0.4f, int maxDoors = 8);

    /**
     * @brief Создание интерактивных предметов на карте
     */
    void createInteractiveItems();

    /**
     * @brief Создание терминалов на карте
     */
    void createTerminals();

    /**
     * @brief Создание тестового предмета для подбора
     * @param x X-координата
     * @param y Y-координата
     * @param name Имя предмета
     * @param type Тип предмета
     * @return Указатель на созданный предмет
     */
    std::shared_ptr<PickupItem> createTestPickupItem(float x, float y,
        const std::string& name,
        PickupItem::ItemType type);

    /**
     * @brief Создание тестовой двери
     * @param x X-координата
     * @param y Y-координата
     * @param name Имя двери
     * @return Указатель на созданную дверь
     */
    std::shared_ptr<Door> createTestDoor(float x, float y, const std::string& name);

    /**
     * @brief Создание тестового терминала
     * @param x X-координата
     * @param y Y-координата
     * @param name Имя терминала
     * @param type Тип терминала
     * @return Указатель на созданный терминал
     */
    std::shared_ptr<Terminal> createTestTerminal(float x, float y,
        const std::string& name,
        Terminal::TerminalType type);

    /**
 * @brief Создание переключателей на карте
 */
    void createSwitches();

    /**
 * @brief Создание тестового переключателя
 * @param x X-координата
 * @param y Y-координата
 * @param name Имя переключателя
 * @param type Тип переключателя
 * @return Указатель на созданный переключатель
 */
    std::shared_ptr<Switch> createTestSwitch(float x, float y,
        const std::string& name,
        Switch::SwitchType type);

private:
    /**
     * @brief Поиск углов комнат на карте для размещения терминалов
     * @return Вектор координат углов комнат
     */
    std::vector<std::pair<int, int>> findRoomCorners();

    /**
     * @brief Запасной метод для случайного размещения терминала
     * @param terminalType Тип терминала
     * @param terminalName Имя терминала
     */
    void placeTerminalRandomly(Terminal::TerminalType terminalType,
        const std::string& terminalName);

    std::shared_ptr<TileMap> m_tileMap;    ///< Указатель на карту тайлов
    Engine* m_engine;                       ///< Указатель на движок
    MapScene* m_mapScene;                   ///< Указатель на сцену карты
    std::shared_ptr<Player> m_player;       ///< Указатель на игрока
    int m_currentBiome;                     ///< Текущий биом
};