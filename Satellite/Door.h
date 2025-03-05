#pragma once

#include "InteractiveObject.h"
#include "TileMap.h"
#include <memory>

// Forward declarations
class MapScene;
class IsometricRenderer;
class InteractionSystem; // Добавляем forward declaration для InteractionSystem

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
     * @brief Установка системы взаимодействия
     * @param system Указатель на систему взаимодействия
     */
    void setInteractionSystem(InteractionSystem* system) { m_interactionSystem = system; }

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

    /**
     * @brief Начать процесс взаимодействия с дверью (открытие/закрытие с кастом)
     * @return true, если взаимодействие было начато успешно
     */
    bool startInteraction();

    /**
     * @brief Отменить процесс взаимодействия с дверью
     */
    void cancelInteraction();

    /**
     * @brief Проверить, идет ли процесс взаимодействия с дверью
     * @return true, если идет процесс взаимодействия
     */
    bool isInteracting() const { return m_isInteracting; }

    /**
     * @brief Получить текущий прогресс взаимодействия
     * @return Значение от 0.0 до 1.0
     */
    float getInteractionProgress() const { return m_interactionProgress; }

    /**
     * @brief Обновить отображение прогресса взаимодействия
     * @param progress Новое значение прогресса (0.0-1.0)
     */
    void updateInteractionProgress(float progress);

    /**
     * @brief Отрисовка индикатора взаимодействия над дверью
     * @param renderer SDL рендерер
     * @param isoRenderer Изометрический рендерер для преобразования координат
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void render(SDL_Renderer* renderer, IsometricRenderer* isoRenderer, int centerX, int centerY);


    /**
     * @brief Сбрасывает флаг требования отпускания клавиши
     */
    void resetKeyReleaseRequirement();

    /**
     * @brief Проверяет, требуется ли отпустить клавишу перед новым взаимодействием
     * @return true, если требуется отпустить клавишу
     */
    bool isRequiringKeyRelease() const;


private:
    /**
     * @brief Обновление состояния проходимости тайла
     */
    void updateTileWalkability();

    /**
     * @brief Обновляет подсказку для взаимодействия в зависимости от биома и состояния
     */
    void updateInteractionHint();

    /**
     * @brief Обновляет подсказку во время процесса взаимодействия
     */
    void updateInteractionHintDuringCast();

    /**
     * @brief Завершает процесс взаимодействия с дверью
     */
    void completeInteraction();

    bool m_isOpen;          ///< Флаг состояния двери (открыта/закрыта)
    TileMap* m_tileMap;     ///< Указатель на карту для обновления проходимости
    int m_tileX;            ///< X координата тайла, на котором находится дверь
    int m_tileY;            ///< Y координата тайла, на котором находится дверь
    SDL_Color m_openColor;  ///< Цвет двери в открытом состоянии
    SDL_Color m_closedColor; ///< Цвет двери в закрытом состоянии
    MapScene* m_parentScene; ///< Указатель на родительскую сцену (для обратной совместимости)
    InteractionSystem* m_interactionSystem; ///< Указатель на систему взаимодействия
    bool m_isVertical;         ///< Ориентация двери (вертикальная/горизонтальная)
    int m_biomeType;           ///< Тип биома для визуализации

    // Свойства для системы "каст-времени"
    bool m_isInteracting;               ///< Флаг, показывающий, что идет процесс взаимодействия
    float m_interactionTimer;           ///< Таймер для отслеживания времени взаимодействия
    float m_interactionRequiredTime;    ///< Требуемое время для завершения взаимодействия (в секундах)
    float m_interactionProgress;        ///< Прогресс взаимодействия (0.0 - 1.0)

    bool m_actionJustCompleted;     ///< Флаг, показывающий, что действие только что завершилось (для предотвращения автоповтора)
    float m_cooldownTimer;          ///< Таймер кулдауна после завершения действия
    bool m_requireKeyRelease;     ///< Флаг, показывающий, что требуется отпустить клавишу E перед новым взаимодействием
};