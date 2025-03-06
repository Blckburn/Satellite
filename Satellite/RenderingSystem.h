#pragma once

#include "TileMap.h"
#include "TileRenderer.h"
#include "IsometricRenderer.h"
#include "Player.h"
#include "EntityManager.h"
#include "Camera.h"
#include <SDL.h>
#include <memory>
#include <vector>
#include "Door.h"          // Добавлено
#include "Terminal.h"      // Добавлено
#include "PickupItem.h"    // Добавлено

/**
 * @brief Система для управления рендерингом игрового мира и сущностей
 */
class RenderingSystem {
public:
    /**
     * @brief Конструктор
     * @param tileMap Указатель на карту тайлов
     * @param tileRenderer Указатель на рендерер тайлов
     * @param isoRenderer Указатель на изометрический рендерер
     */
    RenderingSystem(std::shared_ptr<TileMap> tileMap,
        std::shared_ptr<TileRenderer> tileRenderer,
        std::shared_ptr<IsometricRenderer> isoRenderer);

    /**
     * @brief Отрисовка игрового мира и всех сущностей
     * @param renderer SDL рендерер
     * @param camera Указатель на камеру
     * @param player Указатель на игрока
     * @param entityManager Указатель на менеджер сущностей
     * @param biomeType Текущий биом
     */
    void render(SDL_Renderer* renderer,
        std::shared_ptr<Camera> camera,
        std::shared_ptr<Player> player,
        std::shared_ptr<EntityManager> entityManager,
        int biomeType);

    /**
     * @brief Отрисовка индикатора игрока, когда он скрыт стенами
     * @param renderer SDL рендерер
     * @param player Указатель на игрока
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     */
    void renderPlayerIndicator(SDL_Renderer* renderer,
        std::shared_ptr<Player> player,
        int centerX, int centerY);

    /**
 * @brief Получить указатель на TileRenderer
 * @return Указатель на TileRenderer
 */
    TileRenderer* getTileRenderer() const;

private:
    /**
     * @brief Расчет приоритета визуального порядка для изометрической проекции
     * @param x Координата X объекта в мировом пространстве
     * @param y Координата Y объекта в мировом пространстве
     * @param z Высота объекта
     * @param playerFullX Полная X координата игрока
     * @param playerFullY Полная Y координата игрока
     * @param playerDirectionX Направление игрока по X
     * @param playerDirectionY Направление игрока по Y
     * @return Значение приоритета для сортировки
     */
    float calculateZOrderPriority(float x, float y, float z,
        float playerFullX, float playerFullY,
        float playerDirectionX, float playerDirectionY);

    /**
     * @brief Отрисовка сцены с использованием блочной Z-сортировки
     * @param renderer SDL рендерер
     * @param player Указатель на игрока
     * @param entityManager Указатель на менеджер сущностей
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     * @param biomeType Текущий тип биома
     */
    void renderWithBlockSorting(SDL_Renderer* renderer,
        std::shared_ptr<Player> player,
        std::shared_ptr<EntityManager> entityManager,
        int centerX, int centerY,
        int biomeType);

    /**
     * @brief Отрисовка персонажа с гарантией видимости
     * @param renderer SDL рендерер
     * @param player Указатель на игрока
     * @param priority Приоритет отрисовки
     */
    void renderPlayer(SDL_Renderer* renderer, std::shared_ptr<Player> player, float priority);


    /**
 * @brief Проверяет, есть ли на указанной позиции дверь
 * @param x X-координата
 * @param y Y-координата
 * @param interactiveObjects Список интерактивных объектов
 * @return true, если на позиции есть дверь
 */
    bool isDoorAtPosition(int x, int y,
        const std::vector<std::shared_ptr<InteractiveObject>>& interactiveObjects) {
        for (auto& obj : interactiveObjects) {
            if (auto doorObj = std::dynamic_pointer_cast<Door>(obj)) {
                // Получаем позицию двери, округляя до целых
                int doorX = static_cast<int>(doorObj->getPosition().x);
                int doorY = static_cast<int>(doorObj->getPosition().y);

                // Если координаты совпадают, значит на этой позиции есть дверь
                if (doorX == x && doorY == y) {
                    return true;
                }
            }
        }
        return false;
    }

    std::shared_ptr<TileMap> m_tileMap;                ///< Указатель на карту тайлов
    std::shared_ptr<TileRenderer> m_tileRenderer;      ///< Указатель на рендерер тайлов
    std::shared_ptr<IsometricRenderer> m_isoRenderer;  ///< Указатель на изометрический рендерер
};