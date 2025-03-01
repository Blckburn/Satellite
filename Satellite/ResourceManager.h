#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

/**
 * @brief Класс для управления ресурсами (текстурами, звуками и т.д.)
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

private:
    SDL_Renderer* m_renderer;                                  ///< Указатель на SDL рендерер
    std::unordered_map<std::string, SDL_Texture*> m_textures;  ///< Хранилище текстур
};