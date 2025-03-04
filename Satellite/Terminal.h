#pragma once

#include "InteractiveObject.h"
#include <SDL.h>  // Включаем SDL.h
#include <SDL_ttf.h>  // Включаем SDL_ttf.h напрямую вместо forward declaration
#include <vector>
#include <string>
#include <functional>

/**
 * @brief Класс терминала - интерактивного объекта, предоставляющего информацию
 */
class Terminal : public InteractiveObject {
public:
    /**
     * @brief Типы терминалов, определяющие их внешний вид и функциональность
     */
    enum class TerminalType {
        RESEARCH_SENSOR,   ///< Датчик, оставленный предыдущими экспедициями
        ANCIENT_CONSOLE,   ///< Древняя консоль неизвестной цивилизации
        EMERGENCY_BEACON,  ///< Аварийный маяк с записями о катастрофе
        SCIENCE_STATION    ///< Научная станция для проведения исследований
    };

    /**
     * @brief Конструктор
     * @param name Имя терминала
     * @param type Тип терминала
     */
    Terminal(const std::string& name, TerminalType type = TerminalType::RESEARCH_SENSOR);

    /**
     * @brief Деструктор
     */
    ~Terminal() override = default;

    /**
     * @brief Инициализация терминала
     * @return true в случае успеха, false при ошибке
     */
    bool initialize() override;

    /**
     * @brief Взаимодействие с терминалом
     * @param player Указатель на игрока
     * @return true, если взаимодействие было успешным
     */
    bool interact(Player* player) override;

    /**
     * @brief Обновление состояния терминала
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void update(float deltaTime) override;

    /**
     * @brief Отображение информации терминала
     * @param renderer SDL рендерер
     * @param font Шрифт для отображения текста
     * @param x X-координата для отображения
     * @param y Y-координата для отображения
     */
    void displayInfo(SDL_Renderer* renderer, TTF_Font* font, int x, int y);

    /**
     * @brief Добавление информационной записи
     * @param title Заголовок записи
     * @param content Содержание записи
     */
    void addEntry(const std::string& title, const std::string& content);

    /**
     * @brief Установка функции обратного вызова при активации терминала
     * @param callback Функция, вызываемая после взаимодействия
     */
    void setActivationCallback(std::function<void(Player*, Terminal*)> callback);

    /**
     * @brief Получение типа терминала
     * @return Тип терминала
     */
    TerminalType getTerminalType() const { return m_terminalType; }

    /**
     * @brief Проверка, был ли терминал уже активирован
     * @return true, если терминал был активирован
     */
    bool isActivated() const { return m_activated; }

    /**
     * @brief Установка состояния активации
     * @param activated Состояние активации
     */
    void setActivated(bool activated) { m_activated = activated; }

    /**
     * @brief Получение списка записей
     * @return Вектор пар заголовок-содержимое
     */
    const std::vector<std::pair<std::string, std::string>>& getEntries() const { return m_entries; }

    /**
 * @brief Получение символа-индикатора для терминала
 * @return Символ, отображаемый над терминалом
 */
    std::string getIndicatorSymbol() const;

    /**
     * @brief Проверка, нужно ли отображать символ-индикатор
     * @return true, если символ должен отображаться
     */
    bool shouldShowIndicator() const { return !m_wasEverRead; }

    /**
     * @brief Отметка о прочтении терминала (скроет индикатор)
     */
    void markAsRead() { m_wasEverRead = true; }

    /**
 * @brief Получение индекса выбранной записи
 * @return Индекс записи для отображения
 */
    int getSelectedEntryIndex() const { return m_selectedEntryIndex; }

    /**
     * @brief Выбор случайной записи (вызывается при инициализации)
     */
    void selectRandomEntry();

    /**
 * @brief Установка индекса выбранной записи
 * @param index Индекс записи для отображения
 */
    void setSelectedEntryIndex(int index) { m_selectedEntryIndex = index; }


private:
    TerminalType m_terminalType;   ///< Тип терминала
    bool m_activated;              ///< Был ли терминал активирован
    float m_activationTime;        ///< Время, прошедшее с момента активации
    bool m_displayingInfo;         ///< Отображается ли сейчас информация
    bool m_wasEverRead = false;  ///< Флаг, был ли терминал когда-либо прочитан
    int m_selectedEntryIndex = -1;  ///< Индекс случайно выбранной записи для отображения

    std::vector<std::pair<std::string, std::string>> m_entries;  ///< Записи терминала (заголовок, содержимое)
    std::function<void(Player*, Terminal*)> m_activationCallback; ///< Функция обратного вызова при активации
};