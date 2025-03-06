#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

/**
 * @brief Класс для централизованного управления текстурами
 */
class TextureManager {
public:
    /**
     * @brief Конструктор
     * @param renderer Указатель на SDL_Renderer
     */
    TextureManager(SDL_Renderer* renderer);

    /**
     * @brief Деструктор
     */
    ~TextureManager();

    /**
     * @brief Загружает текстуру из файла
     * @param id Идентификатор текстуры
     * @param filePath Путь к файлу текстуры
     * @return true в случае успеха, false при ошибке
     */
    bool loadTexture(const std::string& id, const std::string& filePath);

    /**
     * @brief Получает текстуру по идентификатору
     * @param id Идентификатор текстуры
     * @return Указатель на текстуру или nullptr, если текстура не найдена
     */
    SDL_Texture* getTexture(const std::string& id) const;

    /**
     * @brief Проверяет наличие текстуры
     * @param id Идентификатор текстуры
     * @return true, если текстура существует, false в противном случае
     */
    bool hasTexture(const std::string& id) const;

    /**
     * @brief Удаляет текстуру
     * @param id Идентификатор текстуры
     */
    void removeTexture(const std::string& id);

    /**
     * @brief Освобождает все текстуры
     */
    void clearAll();

    /**
     * @brief Получает размеры текстуры
     * @param id Идентификатор текстуры
     * @param width Ширина текстуры (выходной параметр)
     * @param height Высота текстуры (выходной параметр)
     * @return true в случае успеха, false при ошибке
     */
    bool getTextureSize(const std::string& id, int& width, int& height) const;

    /**
     * @brief Устанавливает цветовую модуляцию для текстуры
     * @param id Идентификатор текстуры
     * @param r Компонент красного цвета (0-255)
     * @param g Компонент зеленого цвета (0-255)
     * @param b Компонент синего цвета (0-255)
     * @return true в случае успеха, false при ошибке
     */
    bool setTextureColorMod(const std::string& id, Uint8 r, Uint8 g, Uint8 b);

    /**
     * @brief Устанавливает альфа-модуляцию для текстуры
     * @param id Идентификатор текстуры
     * @param alpha Значение альфа-канала (0-255)
     * @return true в случае успеха, false при ошибке
     */
    bool setTextureAlphaMod(const std::string& id, Uint8 alpha);

    /**
     * @brief Устанавливает режим смешивания для текстуры
     * @param id Идентификатор текстуры
     * @param blendMode Режим смешивания
     * @return true в случае успеха, false при ошибке
     */
    bool setTextureBlendMode(const std::string& id, SDL_BlendMode blendMode);

    /**
     * @brief Отображает информацию о текстуре в консоли
     * @param id Идентификатор текстуры
     * @return true в случае успеха, false при ошибке
     */
    bool debugTextureInfo(const std::string& id) const;

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

private:
    SDL_Renderer* m_renderer;                              ///< Указатель на SDL_Renderer
    std::unordered_map<std::string, SDL_Texture*> m_textures;  ///< Хранилище текстур
};