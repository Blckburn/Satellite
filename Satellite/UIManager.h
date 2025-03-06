#pragma once

#include "TileMap.h"
#include "IsometricRenderer.h"
#include "Player.h"
#include "Engine.h"
#include "Terminal.h"
#include "InteractionSystem.h"
#include "Door.h"
#include <SDL.h>
#include <memory>
#include <string>

/**
 * @brief Система для управления пользовательским интерфейсом
 */
class UIManager {
public:
    /**
     * @brief Конструктор
     * @param engine Указатель на движок (для доступа к ResourceManager)
     */
    UIManager(Engine* engine);

    /**
     * @brief Отрисовка интерфейса
     * @param renderer SDL рендерер
     * @param isoRenderer Изометрический рендерер
     * @param tileMap Карта тайлов
     * @param player Указатель на игрока (может быть nullptr)
     * @param interactionSystem Указатель на систему взаимодействия
     * @param showDebug Флаг для отображения отладочной информации
     */
    void render(SDL_Renderer* renderer,
        std::shared_ptr<IsometricRenderer> isoRenderer,
        std::shared_ptr<TileMap> tileMap,
        std::shared_ptr<Player> player,
        std::shared_ptr<InteractionSystem> interactionSystem,
        bool showDebug);

    /**
     * @brief Отрисовка подсказки взаимодействия
     * @param renderer SDL рендерер
     * @param prompt Текст подсказки
     */
    void renderInteractionPrompt(SDL_Renderer* renderer, const std::string& prompt);

    /**
     * @brief Отрисовка информации терминала
     * @param renderer SDL рендерер
     * @param terminal Указатель на терминал
     */
    void renderTerminalInfo(SDL_Renderer* renderer, std::shared_ptr<Terminal> terminal);

    /**
     * @brief Отрисовка отладочной информации
     * @param renderer SDL рендерер
     * @param isoRenderer Указатель на изометрический рендерер
     * @param tileMap Указатель на карту тайлов
     * @param player Указатель на игрока
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void renderDebug(SDL_Renderer* renderer,
        std::shared_ptr<IsometricRenderer> isoRenderer,
        std::shared_ptr<TileMap> tileMap,
        std::shared_ptr<Player> player,
        int centerX, int centerY);

    /**
     * @brief Сокращает длинный текст, если он превышает максимальную длину
     * @param text Исходный текст
     * @param maxLength Максимальная длина текста
     * @return Сокращенный текст или исходный текст, если он короче maxLength
     */
    static std::string truncateText(const std::string& text, size_t maxLength);

    /**
 * @brief Отрисовка информации переключателя
 * @param renderer SDL рендерер
 * @param switchObj Указатель на переключатель
 */
    void renderSwitchInfo(SDL_Renderer* renderer, std::shared_ptr<Switch> switchObj);

private:
    Engine* m_engine;  ///< Указатель на движок
};