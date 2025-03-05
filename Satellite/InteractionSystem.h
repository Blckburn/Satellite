#pragma once

#include "Player.h"
#include "EntityManager.h"
#include "TileMap.h"
#include "Door.h"
#include "Terminal.h"
#include <memory>
#include <vector>
#include <string>
#include "PickupItem.h"  

/**
 * @brief Класс для управления взаимодействием между игроком и объектами мира
 */
class InteractionSystem {
public:
    /**
     * @brief Конструктор
     * @param player Указатель на игрока
     * @param entityManager Указатель на менеджер сущностей
     * @param tileMap Указатель на карту тайлов
     */
    InteractionSystem(std::shared_ptr<Player> player,
        std::shared_ptr<EntityManager> entityManager,
        std::shared_ptr<TileMap> tileMap);

    /**
     * @brief Деструктор
     */
    ~InteractionSystem() = default;

    /**
     * @brief Обработка взаимодействия с объектами
     */
    void handleInteraction();

    /**
     * @brief Обновление состояния системы взаимодействия
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void update(float deltaTime);

    /**
     * @brief Запоминает позицию открытой двери
     * @param x X-координата двери
     * @param y Y-координата двери
     * @param name Имя двери
     */
    void rememberDoorPosition(int x, int y, const std::string& name);

    /**
     * @brief Проверяет, является ли тайл открытой дверью
     * @param x X-координата
     * @param y Y-координата
     * @return true, если это открытая дверь
     */
    bool isOpenDoorTile(int x, int y) const;

    /**
     * @brief Закрывает дверь на указанной позиции
     * @param x X-координата
     * @param y Y-координата
     */
    void closeDoorAtPosition(int x, int y);

    /**
     * @brief Удаляет информацию о двери из списка открытых дверей
     * @param x X-координата двери
     * @param y Y-координата двери
     */
    void forgetDoorPosition(int x, int y);

    /**
     * @brief Получение текущего интерактивного сообщения
     * @return Текст сообщения
     */
    const std::string& getInteractionPrompt() const { return m_interactionPrompt; }

    /**
     * @brief Проверка, нужно ли отображать подсказку
     * @return true, если подсказка должна отображаться
     */
    bool shouldShowInteractionPrompt() const { return m_showInteractionPrompt; }

    /**
     * @brief Очистка интерактивного сообщения
     */
    void clearInteractionPrompt() { m_showInteractionPrompt = false; }

    /**
     * @brief Устанавливает функцию для создания новой двери
     * @param createDoorCallback Функция создания двери
     */
    void setCreateDoorCallback(std::function<void(float, float, const std::string&)> createDoorCallback) {
        m_createDoorCallback = createDoorCallback;
    }

    /**
     * @brief Проверяет, идет ли сейчас взаимодействие с дверью
     * @return true, если идет взаимодействие с дверью
     */
    bool isInteractingWithDoor() const { return m_isInteractingWithDoor; }

    /**
     * @brief Проверяет, отображается ли информация терминала
     * @return true, если отображается информация терминала
     */
    bool isDisplayingTerminalInfo() const { return m_isDisplayingTerminalInfo; }

    /**
     * @brief Получает текущий терминал, с которым идет взаимодействие
     * @return Указатель на терминал или nullptr
     */
    std::shared_ptr<Terminal> getCurrentTerminal() const { return m_currentInteractingTerminal; }

    /**
     * @brief Закрывает окно терминала
     */
    void closeTerminalInfo() {
        m_isDisplayingTerminalInfo = false;
        m_currentInteractingTerminal = nullptr;
    }

    /**
     * @brief Сокращает длинный текст, если он превышает максимальную длину
     * @param text Исходный текст
     * @param maxLength Максимальная длина текста
     * @return Сокращенный текст или исходный текст, если он короче maxLength
     */
    static std::string truncateText(const std::string& text, size_t maxLength);

private:
    std::shared_ptr<Player> m_player;                  ///< Указатель на игрока
    std::shared_ptr<EntityManager> m_entityManager;    ///< Указатель на менеджер сущностей
    std::shared_ptr<TileMap> m_tileMap;                ///< Указатель на карту тайлов

    float m_interactionPromptTimer;                    ///< Таймер для отображения подсказки
    std::string m_interactionPrompt;                   ///< Текст подсказки для взаимодействия
    bool m_showInteractionPrompt;                      ///< Флаг отображения подсказки

    // Структура для хранения информации об открытых дверях
    struct OpenDoorInfo {
        int tileX;          // X-координата двери
        int tileY;          // Y-координата двери
        std::string name;   // Имя двери
    };

    std::vector<OpenDoorInfo> m_openDoors;             ///< Список открытых дверей

    /**
    * @brief Указатель на текущую дверь, с которой идет взаимодействие
    */
    std::shared_ptr<Door> m_currentInteractingDoor;

    /**
     * @brief Флаг, показывающий, идет ли в данный момент взаимодействие с дверью
     */
    bool m_isInteractingWithDoor;

    /**
     * @brief Указатель на текущий терминал, с которым идет взаимодействие
     */
    std::shared_ptr<Terminal> m_currentInteractingTerminal;

    /**
     * @brief Флаг, показывающий, отображается ли сейчас информация терминала
     */
    bool m_isDisplayingTerminalInfo;

    /**
     * @brief Функция обратного вызова для создания новой двери
     */
    std::function<void(float, float, const std::string&)> m_createDoorCallback;
};