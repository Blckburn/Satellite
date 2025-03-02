#pragma once

#include "Biome.h"
#include "TileRenderer.h"
#include "IsometricRenderer.h"
#include <vector>
#include <memory>
#include <SDL.h>

/**
 * @brief Класс для визуализации доступных биомов в отдельном окне
 */
class BiomeVisualizer {
public:
    /**
     * @brief Конструктор
     * @param biomes Вектор биомов для визуализации
     * @param isoRenderer Указатель на изометрический рендерер
     */
    BiomeVisualizer(const std::vector<std::shared_ptr<Biome>>& biomes, IsometricRenderer* isoRenderer);

    /**
     * @brief Деструктор
     */
    ~BiomeVisualizer();

    /**
     * @brief Инициализация визуализатора
     * @return true в случае успеха, false при ошибке
     */
    bool initialize();

    /**
     * @brief Отрисовка панели биомов
     * @param renderer Указатель на SDL_Renderer
     * @param centerX X координата центра экрана
     * @param centerY Y координата центра экрана
     * @param panelX X координата панели
     * @param panelY Y координата панели
     * @param panelWidth Ширина панели
     * @param panelHeight Высота панели
     */
    void render(SDL_Renderer* renderer, int centerX, int centerY,
        int panelX, int panelY, int panelWidth, int panelHeight);

    /**
     * @brief Обработка нажатия на панель биомов
     * @param x X координата нажатия
     * @param y Y координата нажатия
     * @param panelX X координата панели
     * @param panelY Y координата панели
     * @param panelWidth Ширина панели
     * @param panelHeight Высота панели
     * @return ID выбранного биома или -1, если биом не выбран
     */
    int handleClick(int x, int y, int panelX, int panelY, int panelWidth, int panelHeight);

    /**
     * @brief Получение информации о биоме
     * @param biomeId ID биома
     * @return Строка с информацией о биоме
     */
    std::string getBiomeInfo(int biomeId) const;

    /**
     * @brief Установка активного биома
     * @param biomeId ID биома
     */
    void setActiveBiome(int biomeId);

    /**
     * @brief Получение ID активного биома
     * @return ID активного биома или -1, если нет активного биома
     */
    int getActiveBiome() const { return m_activeBiomeId; }

private:
    /**
     * @brief Создание примера тайла для заданного биома
     * @param biome Указатель на биом
     * @param tileRenderer Указатель на рендерер тайлов
     * @param x X координата тайла
     * @param y Y координата тайла
     */
    void createBiomeSample(const std::shared_ptr<Biome>& biome, TileRenderer* tileRenderer, float x, float y);

private:
    std::vector<std::shared_ptr<Biome>> m_biomes;  ///< Вектор биомов
    IsometricRenderer* m_isoRenderer;              ///< Указатель на изометрический рендерер
    int m_activeBiomeId;                           ///< ID активного биома
    int m_biomeColumns;                            ///< Количество столбцов в сетке биомов
    int m_sampleSize;                              ///< Размер примера тайла
};