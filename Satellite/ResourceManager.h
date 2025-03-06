#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "TextureManager.h"
#include "TileType.h"


/**
 * @brief Класс для управления ресурсами (текстурами, звуками, шрифтами и т.д.)
 */
class ResourceManager {
public:
    /**
     * @brief Конструктор
     * @param renderer Указатель на SDL рендерер
     */
    ResourceManager(SDL_Renderer* renderer);

    /**
     * @brief Деструктор - освобождает все ресурсы
     */
    ~ResourceManager();

    /**
     * @brief Освобождает все ресурсы
     */
    void clearAll();

    /**
     * @brief Загружает шрифт из файла
     * @param id Идентификатор шрифта
     * @param filePath Путь к файлу шрифта
     * @param fontSize Размер шрифта
     * @return true в случае успеха, false при ошибке
     */
    bool loadFont(const std::string& id, const std::string& filePath, int fontSize);

    /**
     * @brief Получает шрифт по идентификатору
     * @param id Идентификатор шрифта
     * @return Указатель на шрифт или nullptr, если шрифт не найден
     */
    TTF_Font* getFont(const std::string& id) const;

    /**
     * @brief Проверяет, загружен ли шрифт с указанным идентификатором
     * @param id Идентификатор шрифта
     * @return true, если шрифт загружен, false если нет
     */
    bool hasFont(const std::string& id) const;

    /**
     * @brief Удаляет шрифт из менеджера ресурсов
     * @param id Идентификатор шрифта
     */
    void removeFont(const std::string& id);

    /**
     * @brief Создает текстуру с текстом
     * @param text Текст для отображения
     * @param fontId Идентификатор шрифта
     * @param color Цвет текста
     * @return Указатель на созданную текстуру или nullptr при ошибке
     */
    SDL_Texture* createTextTexture(const std::string& text, const std::string& fontId, SDL_Color color);

    /**
     * @brief Отрисовывает текст на экране
     * @param renderer Указатель на SDL_Renderer
     * @param text Текст для отображения
     * @param fontId Идентификатор шрифта
     * @param x X-координата
     * @param y Y-координата
     * @param color Цвет текста
     */
    void renderText(SDL_Renderer* renderer, const std::string& text, const std::string& fontId,
        int x, int y, SDL_Color color);

    /**
     * @brief Получает указатель на SDL_Renderer
     * @return Указатель на SDL_Renderer
     */
    SDL_Renderer* getRenderer() const;

    /**
     * @brief Получает указатель на TextureManager
     * @return Указатель на TextureManager
     */
    TextureManager* getTextureManager() const;

    /**
    * @brief Загружает стандартные текстуры тайлов
    * @return true в случае успеха, false при ошибке
    */
    bool loadTileTextures();

    /**
     * @brief Получает текстуру тайла по типу и биому
     * @param type Тип тайла
     * @param biomeType Тип биома (1-Forest, 2-Desert, 3-Tundra, 4-Volcanic)
     * @return Указатель на текстуру или nullptr, если текстура не найдена
     */
    SDL_Texture* getTileTexture(TileType type, int biomeType = 0) const;

private:
    SDL_Renderer* m_renderer;                              ///< Указатель на SDL рендерер
    std::unique_ptr<TextureManager> m_textureManager;      ///< Указатель на TextureManager
    std::unordered_map<std::string, TTF_Font*> m_fonts;    ///< Хранилище шрифтов
};