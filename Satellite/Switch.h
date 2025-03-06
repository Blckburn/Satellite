#pragma once

#include "InteractiveObject.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <functional>
#include <map>


class Player;
class TileMap;
class MapScene;
class WorldGenerator;

/**
 * @brief Класс переключателя - интерактивного объекта, вызывающего изменения в окружении
 */
class Switch : public InteractiveObject {
public:
    friend class WorldGenerator;
    /**
     * @brief Типы переключателей, определяющие их внешний вид и функциональность
     */
    enum class SwitchType {
        GRAVITY_ANOMALY,       ///< Природная гравитационная аномалия, влияющая на гравитационное поле
        TELEPORT_GATE,         ///< Древние врата для перемещения между секторами
        RESONANCE_STABILIZER,  ///< Стабилизаторы опасных факторов окружающей среды
        SECURITY_SYSTEM,       ///< Системы безопасности древних руин
        ENERGY_NODE            ///< Энергетические узлы древних сооружений
    };

    /**
     * @brief Конструктор
     * @param name Имя переключателя
     * @param type Тип переключателя
     * @param tileMap Указатель на карту для влияния на окружение
     * @param parentScene Указатель на родительскую сцену
     */
    Switch(const std::string& name, SwitchType type, TileMap* tileMap = nullptr, MapScene* parentScene = nullptr);

    /**
     * @brief Деструктор
     */
    ~Switch() override = default;

    /**
     * @brief Инициализация переключателя
     * @return true в случае успеха, false при ошибке
     */
    bool initialize() override;

    /**
     * @brief Взаимодействие с переключателем
     * @param player Указатель на игрока
     * @return true, если взаимодействие было успешным
     */
    bool interact(Player* player) override;

    /**
     * @brief Обновление состояния переключателя
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void update(float deltaTime) override;

    /**
     * @brief Отображение информации и эффектов переключателя
     * @param renderer SDL рендерер
     * @param font Шрифт для отображения текста
     * @param x X-координата для отображения
     * @param y Y-координата для отображения
     */
    void displayInfo(SDL_Renderer* renderer, TTF_Font* font, int x, int y);

    /**
     * @brief Установка функции обратного вызова при активации переключателя
     * @param callback Функция, вызываемая после успешной активации
     */
    void setActivationCallback(std::function<void(Player*, Switch*)> callback);

    /**
     * @brief Получение типа переключателя
     * @return Тип переключателя
     */
    SwitchType getSwitchType() const { return m_switchType; }

    /**
     * @brief Проверка, был ли переключатель уже активирован
     * @return true, если переключатель был активирован
     */
    bool isActivated() const { return m_activated; }

    /**
     * @brief Установка состояния активации
     * @param activated Состояние активации
     */
    void setActivated(bool activated);

    /**
     * @brief Получение символа-индикатора для переключателя
     * @return Символ, отображаемый над переключателем
     */
    std::string getIndicatorSymbol() const;

    /**
     * @brief Начать процесс активации переключателя (с каст-временем)
     * @return true, если процесс активации был начат успешно
     */
    bool startActivation();

    /**
     * @brief Отменить процесс активации переключателя
     */
    void cancelActivation();

    /**
     * @brief Завершить процесс активации переключателя
     */
    void completeActivation();

    /**
     * @brief Проверить, идет ли процесс активации переключателя
     * @return true, если идет процесс активации
     */
    bool isActivating() const { return m_isActivating; }

    /**
     * @brief Получить текущий прогресс активации
     * @return Значение от 0.0 до 1.0
     */
    float getActivationProgress() const { return m_activationProgress; }

    /**
     * @brief Установка радиуса действия эффекта
     * @param radius Радиус действия эффекта
     */
    void setEffectRadius(float radius) { m_effectRadius = radius; }

    /**
     * @brief Получение радиуса действия эффекта
     * @return Радиус действия эффекта
     */
    float getEffectRadius() const { return m_effectRadius; }

    /**
     * @brief Установка длительности эффекта (0 для постоянного)
     * @param duration Длительность эффекта в секундах
     */
    void setEffectDuration(float duration) { m_effectDuration = duration; }

    /**
     * @brief Получение длительности эффекта
     * @return Длительность эффекта в секундах
     */
    float getEffectDuration() const { return m_effectDuration; }

    /**
     * @brief Проверка, активен ли эффект в данный момент
     * @return true, если эффект активен
     */
    bool isEffectActive() const { return m_effectActive; }

    /**
     * @brief Обновление подсказки при взаимодействии в зависимости от прогресса
     */
    void updateActivationHint();

    /**
     * @brief Проверка, нужно ли отображать символ-индикатор
     * @return true, если символ должен отображаться
     */
    bool shouldShowIndicator() const { return !m_activated || m_effectActive; }

    /**
     * @brief Отметка о последнем взаимодействии с переключателем
     */
    void markAsRead() { /* Для переключателей это не требуется */ }


    /**
 * @brief Получение X-координаты назначения телепортации
 * @return X-координата телепортации
 */
    int getTeleportDestX() const { return m_teleportDestX; }

    /**
     * @brief Получение Y-координаты назначения телепортации
     * @return Y-координата телепортации
     */
    int getTeleportDestY() const { return m_teleportDestY; }

    /**
  * @brief Проверка, установлены ли координаты телепортации
  * @return true, если координаты установлены
  */
    bool hasTeleportDestination() const { return m_teleportDestX >= 0 && m_teleportDestY >= 0; }


private:
    /**
     * @brief Применить эффект переключателя к окружению
     */
    void applyEffect();

    /**
     * @brief Деактивировать эффект переключателя
     */
    void deactivateEffect();

    /**
     * @brief Обновление визуальных эффектов в зависимости от состояния
     * @param deltaTime Время, прошедшее с предыдущего кадра
     */
    void updateVisualEffects(float deltaTime);

    /**
     * @brief Генерация описания переключателя в зависимости от типа
     */
    void generateDescription();

    SwitchType m_switchType;                              ///< Тип переключателя
    bool m_activated;                                     ///< Был ли переключатель активирован
    float m_activationTime;                               ///< Время, прошедшее с момента активации
    bool m_displayingInfo;                                ///< Отображается ли сейчас информация

    // Поля для каст-времени активации
    bool m_isActivating;                                  ///< Идет ли процесс активации
    float m_activationTimer;                              ///< Таймер активации
    float m_activationRequiredTime;                       ///< Требуемое время активации
    float m_activationProgress;                           ///< Прогресс активации

    // Поля для эффекта
    float m_effectRadius;                                 ///< Радиус действия эффекта
    float m_effectDuration;                               ///< Длительность эффекта (0 для постоянного)
    float m_effectTimer;                                  ///< Таймер действия эффекта
    bool m_effectActive;                                  ///< Активен ли эффект в данный момент

    // Поля специфичные для телепортации
    int m_teleportDestX = -1;                             ///< X-координата точки назначения телепортации
    int m_teleportDestY = -1;                             ///< Y-координата точки назначения телепортации
    SDL_Color m_originalTileColor = { 255, 255, 255, 255 }; ///< Оригинальный цвет тайла для восстановления

    // Хранилище для измененных гравитационной аномалией тайлов
    std::map<std::pair<int, int>, bool> m_affectedTiles;  ///< Карта тайлов, на которые повлияла аномалия (координаты -> исходная проходимость)

    // Внешние ссылки
    TileMap* m_tileMap;                                   ///< Указатель на карту для влияния на окружение
    MapScene* m_parentScene;                              ///< Указатель на родительскую сцену

    // Визуальные эффекты
    float m_pulsePhase;                                   ///< Фаза пульсации для визуальных эффектов
    SDL_Color m_activeColor;                              ///< Цвет активированного переключателя
    SDL_Color m_inactiveColor;                            ///< Цвет неактивированного переключателя

    std::string m_description;                            ///< Описание переключателя
    std::function<void(Player*, Switch*)> m_activationCallback; ///< Функция обратного вызова при активации


};