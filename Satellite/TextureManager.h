#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <chrono>
#include <vector>
#include "TextureAtlas.h"

/**
 * @brief Класс для централизованного управления текстурами
 */
class TextureManager {
public:
    /**
     * @brief Структура для хранения информации о текстуре
     */
    struct TextureInfo {
        SDL_Texture* texture;                     ///< Указатель на саму текстуру
        std::chrono::steady_clock::time_point lastUsed; ///< Время последнего использования
        int width;                                ///< Ширина текстуры в пикселях
        int height;                               ///< Высота текстуры в пикселях
        size_t memorySize;                        ///< Примерный размер в памяти (в байтах)
        int useCount;                             ///< Счетчик использований
        int priority;                             ///< Приоритет для кэширования (выше = важнее)

        // Информация об атласе (если текстура находится в атласе)
        bool isInAtlas;                           ///< Флаг нахождения в атласе
        std::string atlasId;                      ///< Идентификатор атласа
        SDL_Rect atlasRect;                       ///< Прямоугольник в атласе
        bool rotated;                             ///< Флаг поворота в атласе

        TextureInfo() : texture(nullptr), width(0), height(0),
            memorySize(0), useCount(0), priority(0), isInAtlas(false), rotated(false) {
            lastUsed = std::chrono::steady_clock::now();
            atlasRect = { 0, 0, 0, 0 };
        }
    };

    /**
     * @brief Конструктор
     * @param renderer Указатель на SDL_Renderer
     * @param memoryCacheLimit Ограничение памяти кэша в байтах (0 = без ограничений)
     */
    TextureManager(SDL_Renderer* renderer, size_t memoryCacheLimit = 0);

    /**
     * @brief Деструктор
     */
    ~TextureManager();

    /**
     * @brief Загружает текстуру из файла
     * @param id Идентификатор текстуры
     * @param filePath Путь к файлу текстуры
     * @param priority Приоритет кэширования (выше = важнее)
     * @param useAtlas Использовать атлас текстур (если возможно)
     * @param atlasId Идентификатор атласа (если пусто, используется атлас по умолчанию)
     * @return true в случае успеха, false при ошибке
     */
    bool loadTexture(const std::string& id, const std::string& filePath, int priority = 0,
        bool useAtlas = false, const std::string& atlasId = "");

    /**
     * @brief Получает текстуру по идентификатору
     * @param id Идентификатор текстуры
     * @return Указатель на текстуру или nullptr, если текстура не найдена
     */
    SDL_Texture* getTexture(const std::string& id);

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

    /**
     * @brief Устанавливает максимальный размер кэша текстур
     * @param memoryCacheLimit Максимальный размер в байтах (0 = без ограничений)
     */
    void setMemoryCacheLimit(size_t memoryCacheLimit);

    /**
     * @brief Получение текущего размера кэша текстур
     * @return Текущий размер кэша в байтах
     */
    size_t getCurrentCacheSize() const;

    /**
     * @brief Получение числа текстур в кэше
     * @return Количество текстур
     */
    size_t getTextureCount() const;

    /**
     * @brief Освобождает память, удаляя наименее используемые текстуры
     * @param bytesToFree Сколько байт памяти нужно освободить
     * @param forceRemove Принудительно удалять текстуры даже с высоким приоритетом
     * @return Количество фактически освобожденных байт
     */
    size_t freeCacheMemory(size_t bytesToFree, bool forceRemove = false);

    /**
     * @brief Устанавливает приоритет для текстуры
     * @param id Идентификатор текстуры
     * @param priority Новый приоритет (выше = важнее)
     * @return true в случае успеха, false при ошибке
     */
    bool setTexturePriority(const std::string& id, int priority);

    /**
     * @brief Получает полную информацию о всех текстурах
     * @return Отчет в виде строки
     */
    std::string getTexturesReport() const;

    /**
     * @brief Создает новый атлас текстур
     * @param atlasId Идентификатор атласа
     * @param width Ширина атласа
     * @param height Высота атласа
     * @return true в случае успеха, false при ошибке
     */
    bool createTextureAtlas(const std::string& atlasId, int width, int height);

    /**
     * @brief Добавляет текстуру в атлас
     * @param textureId Идентификатор текстуры
     * @param atlasId Идентификатор атласа
     * @return true в случае успеха, false при ошибке
     */
    bool addTextureToAtlas(const std::string& textureId, const std::string& atlasId);

    /**
     * @brief Загружает текстуру из файла прямо в атлас
     * @param id Идентификатор текстуры
     * @param filePath Путь к файлу текстуры
     * @param atlasId Идентификатор атласа
     * @return true в случае успеха, false при ошибке
     */
    bool loadTextureToAtlas(const std::string& id, const std::string& filePath, const std::string& atlasId);

    /**
     * @brief Проверяет, находится ли текстура в атласе
     * @param id Идентификатор текстуры
     * @return true, если текстура в атласе, false если нет
     */
    bool isTextureInAtlas(const std::string& id) const;

    /**
     * @brief Загружает атлас текстур из файла
     * @param atlasId Идентификатор атласа
     * @param filePath Путь к файлу атласа
     * @param descriptorFilePath Путь к файлу описания атласа
     * @return true в случае успеха, false при ошибке
     */
    bool loadTextureAtlas(const std::string& atlasId, const std::string& filePath, const std::string& descriptorFilePath);

    /**
     * @brief Сохраняет атлас текстур в файл
     * @param atlasId Идентификатор атласа
     * @param filePath Путь к файлу для сохранения атласа
     * @param descriptorFilePath Путь к файлу для сохранения описания атласа
     * @return true в случае успеха, false при ошибке
     */
    bool saveTextureAtlas(const std::string& atlasId, const std::string& filePath, const std::string& descriptorFilePath);

    /**
     * @brief Отрисовывает текстуру
     * @param renderer Указатель на SDL_Renderer
     * @param id Идентификатор текстуры
     * @param dstRect Прямоугольник для отрисовки на экране
     * @param srcRect Прямоугольник исходной текстуры (nullptr = вся текстура)
     * @param angle Угол поворота в градусах (0 = без поворота)
     * @param center Центр вращения (nullptr = центр прямоугольника)
     * @param flip Флаги отражения
     * @return true в случае успеха, false при ошибке
     */
    bool renderTexture(SDL_Renderer* renderer, const std::string& id, const SDL_Rect* dstRect,
        const SDL_Rect* srcRect = nullptr, double angle = 0.0,
        const SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);

private:
    /**
     * @brief Обновляет время последнего использования текстуры
     * @param id Идентификатор текстуры
     */
    void updateTextureUsage(const std::string& id);

    /**
     * @brief Вычисляет примерный размер текстуры в памяти
     * @param texture Указатель на текстуру
     * @param width Ширина текстуры
     * @param height Высота текстуры
     * @return Размер в байтах
     */
    size_t calculateTextureSize(SDL_Texture* texture, int width, int height) const;

    /**
     * @brief Проверяет состояние кэша и выгружает текстуры при необходимости
     */
    void checkCacheState();

    /**
     * @brief Получает указатель на атлас текстур по идентификатору
     * @param atlasId Идентификатор атласа
     * @return Указатель на атлас или nullptr, если атлас не найден
     */
    TextureAtlas* getAtlas(const std::string& atlasId);

    /**
     * @brief Получает идентификатор атласа по умолчанию
     * @return Идентификатор атласа по умолчанию
     */
    std::string getDefaultAtlasId() const;

private:
    SDL_Renderer* m_renderer;                                ///< Указатель на SDL_Renderer
    std::unordered_map<std::string, TextureInfo> m_textureInfos;    ///< Хранилище информации о текстурах
    std::unordered_map<std::string, std::unique_ptr<TextureAtlas>> m_atlases; ///< Хранилище атласов
    size_t m_memoryCacheLimit;                               ///< Максимальный размер кэша в байтах
    size_t m_currentCacheSize;                               ///< Текущий размер кэша в байтах
    std::string m_defaultAtlasId;                            ///< Идентификатор атласа по умолчанию
};