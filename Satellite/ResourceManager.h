#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>  // Добавлен включение SDL_ttf
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

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
     * @brief Загружает текстуру из файла
     * @param id Идентификатор ресурса
     * @param filePath Путь к файлу текстуры
     * @return true в случае успеха, false при ошибке
     */
    bool loadTexture(const std::string& id, const std::string& filePath);

    /**
     * @brief Получает текстуру по идентификатору
     * @param id Идентификатор ресурса
     * @return Указатель на текстуру или nullptr, если текстура не найдена
     */
    SDL_Texture* getTexture(const std::string& id) const;

    /**
     * @brief Проверяет, загружена ли текстура с указанным идентификатором
     * @param id Идентификатор ресурса
     * @return true, если текстура загружена, false если нет
     */
    bool hasTexture(const std::string& id) const;

    /**
     * @brief Удаляет текстуру из менеджера ресурсов
     * @param id Идентификатор ресурса
     */
    void removeTexture(const std::string& id);

    /**
     * @brief Освобождает все ресурсы
     */
    void clearAll();

    /**
     * @brief Получает размеры текстуры
     * @param id Идентификатор текстуры
     * @param width Ширина текстуры (выходной параметр)
     * @param height Высота текстуры (выходной параметр)
     * @return true в случае успеха, false если текстура не найдена
     */
    bool getTextureSize(const std::string& id, int& width, int& height) const;

    /**
     * @brief Создает изометрическую текстуру из обычной для использования на тайле
     * @param id Идентификатор исходной текстуры
     * @param newId Идентификатор новой изометрической текстуры
     * @param tileWidth Ширина изометрического тайла
     * @param tileHeight Высота изометрического тайла
     * @return true в случае успеха, false при ошибке
     */
    bool createIsometricTexture(const std::string& id, const std::string& newId, int tileWidth, int tileHeight);

    /**
     * @brief Создает изометрическую текстуру для грани объемного тайла
     * @param id Идентификатор исходной текстуры
     * @param newId Идентификатор новой изометрической текстуры
     * @param faceType Тип грани: 0 - верхняя, 1 - левая, 2 - правая
     * @param tileWidth Ширина изометрического тайла
     * @param tileHeight Высота изометрического тайла
     * @return true в случае успеха, false при ошибке
     */
    bool createIsometricFaceTexture(const std::string& id, const std::string& newId, int faceType, int tileWidth, int tileHeight);

    /**
     * @brief Debug function to output texture information
     * @param id Texture identifier
     * @return true if texture exists and is valid, false otherwise
     */
    bool debugTextureInfo(const std::string& id);

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

private:
    SDL_Renderer* m_renderer;                                  ///< Указатель на SDL рендерер
    std::unordered_map<std::string, SDL_Texture*> m_textures;  ///< Хранилище текстур
    std::unordered_map<std::string, TTF_Font*> m_fonts;        ///< Хранилище шрифтов
};