#pragma once

#include "InteractiveObject.h"

/**
 * @brief Класс предмета, который можно подобрать
 */
class PickupItem : public InteractiveObject {
public:
    /**
     * @brief Тип предмета
     */
    enum class ItemType {
        RESOURCE,   ///< Ресурс (материал)
        WEAPON,     ///< Оружие
        ARMOR,      ///< Броня
        CONSUMABLE, ///< Расходуемый предмет
        KEY,        ///< Ключ или предмет квеста
        GENERIC     ///< Обычный предмет
    };

    /**
     * @brief Конструктор
     * @param name Имя предмета
     * @param itemType Тип предмета
     */
    PickupItem(const std::string& name, ItemType itemType = ItemType::GENERIC);

    /**
     * @brief Деструктор
     */
    virtual ~PickupItem();

    /**
     * @brief Инициализация предмета
     * @return true в случае успеха, false при ошибке
     */
    virtual bool initialize() override;

    /**
     * @brief Обновление состояния предмета
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    virtual void update(float deltaTime) override;

    /**
     * @brief Отрисовка предмета
     * @param renderer Указатель на SDL_Renderer
     */
    virtual void render(SDL_Renderer* renderer) override;

    /**
     * @brief Взаимодействие с предметом (подбор)
     * @param player Указатель на игрока
     * @return true, если взаимодействие было успешным
     */
    virtual bool interact(Player* player) override;

    /**
     * @brief Получение типа предмета
     * @return Тип предмета
     */
    ItemType getItemType() const { return m_itemType; }

    /**
     * @brief Установка ценности предмета
     * @param value Ценность предмета
     */
    void setValue(int value) { m_value = value; }

    /**
     * @brief Получение ценности предмета
     * @return Ценность предмета
     */
    int getValue() const { return m_value; }

    /**
     * @brief Установка веса предмета
     * @param weight Вес предмета
     */
    void setWeight(float weight) { m_weight = weight; }

    /**
     * @brief Получение веса предмета
     * @return Вес предмета
     */
    float getWeight() const { return m_weight; }

    /**
     * @brief Установка описания предмета
     * @param description Описание предмета
     */
    void setDescription(const std::string& description) { m_description = description; }

    /**
     * @brief Получение описания предмета
     * @return Описание предмета
     */
    const std::string& getDescription() const { return m_description; }

    /**
     * @brief Установка иконки предмета
     * @param icon Путь к иконке предмета
     */
    void setIcon(const std::string& icon) { m_icon = icon; }

    /**
     * @brief Получение пути к иконке предмета
     * @return Путь к иконке предмета
     */
    const std::string& getIcon() const { return m_icon; }

    /**
     * @brief Пульсация для визуального выделения
     * @param enable Включение/выключение пульсации
     */
    void setPulsating(bool enable) { m_isPulsating = enable; }

private:
    ItemType m_itemType;          ///< Тип предмета
    int m_value;                  ///< Ценность предмета
    float m_weight;               ///< Вес предмета
    std::string m_description;    ///< Описание предмета
    std::string m_icon;           ///< Путь к иконке предмета

    // Визуальные эффекты
    bool m_isPulsating;           ///< Флаг пульсации предмета
    float m_pulsePhase;           ///< Фаза пульсации
    float m_floatHeight;          ///< Высота "парения" предмета
    float m_rotationAngle;        ///< Угол вращения предмета
};