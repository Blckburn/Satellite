#pragma once

#include "InteractiveObject.h"
#include "TileMap.h"
#include <memory>

// Forward declaration
class MapScene;

/**
 * @brief Класс для представления двери в игровом мире
 */
class Door : public InteractiveObject, public std::enable_shared_from_this<Door> {
public:
    /**
     * @brief Конструктор
     * @param name Имя двери
     * @param tileMap Указатель на карту для обновления проходимости
     * @param parentScene Указатель на родительскую сцену
     */
    Door(const std::string& name, TileMap* tileMap, MapScene* parentScene = nullptr);

    /**
     * @brief Инициализация двери
     * @return true в случае успеха, false при ошибке
     */
    bool initialize() override;

    /**
     * @brief Обновление состояния двери
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void update(float deltaTime) override;

    /**
     * @brief Взаимодействие с дверью (открытие/закрытие)
     * @param player Указатель на игрока
     * @return true, если взаимодействие было успешным
     */
    bool interact(Player* player) override;

    /**
     * @brief Проверка, открыта ли дверь
     * @return true, если дверь открыта, false если закрыта
     */
    bool isOpen() const;

    /**
     * @brief Открытие двери
     * @param open true для открытия, false для закрытия
     */
    void setOpen(bool open);

    /**
     * @brief Установка родительской сцены
     * @param scene Указатель на сцену
     */
    void setParentScene(MapScene* scene) { m_parentScene = scene; }

    /**
     * @brief Задает ориентацию двери (горизонтальная/вертикальная)
     * @param isVertical true для вертикальной двери, false для горизонтальной
     */
    void setVertical(bool isVertical) { m_isVertical = isVertical; }

    /**
     * @brief Возвращает ориентацию двери
     * @return true если дверь вертикальная, false если горизонтальная
     */
    bool isVertical() const { return m_isVertical; }

    /**
     * @brief Конструктор
     * @param name Имя двери
     * @param tileMap Указатель на карту для обновления проходимости
     * @param parentScene Указатель на родительскую сцену
     * @param biomeType Тип биома (1-Forest, 2-Desert, 3-Tundra, 4-Volcanic)
     */
    Door(const std::string& name, TileMap* tileMap, MapScene* parentScene = nullptr, int biomeType = 1);

    /**
     * @brief Получение подсказки для взаимодействия, специфичной для биома
     * @return Текст подсказки
     */
    const std::string& getInteractionHint() const;

private:
    /**
     * @brief Обновление состояния проходимости тайла
     */
    void updateTileWalkability();

    /**
     * @brief Обновляет подсказку для взаимодействия в зависимости от биома и состояния
     */
    void updateInteractionHint();

    bool m_isOpen;          ///< Флаг состояния двери (открыта/закрыта)
    TileMap* m_tileMap;     ///< Указатель на карту для обновления проходимости
    int m_tileX;            ///< X координата тайла, на котором находится дверь
    int m_tileY;            ///< Y координата тайла, на котором находится дверь
    SDL_Color m_openColor;  ///< Цвет двери в открытом состоянии
    SDL_Color m_closedColor; ///< Цвет двери в закрытом состоянии
    MapScene* m_parentScene; ///< Указатель на родительскую сцену
    bool m_isVertical;         ///< Ориентация двери (вертикальная/горизонтальная)
    int m_biomeType;           ///< Тип биома для визуализации
};