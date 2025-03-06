#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>

/**
 * @brief Структура для хранения информации о субтекстуре в атласе
 */
struct SubTexture {
    std::string id;       ///< Идентификатор субтекстуры
    SDL_Rect rect;        ///< Прямоугольник, определяющий расположение в атласе
    SDL_Rect source;      ///< Исходный прямоугольник для текстуры (до упаковки)
    bool rotated;         ///< Флаг, указывающий, что текстура повернута на 90 градусов

    SubTexture() : rotated(false) {
        rect = { 0, 0, 0, 0 };
        source = { 0, 0, 0, 0 };
    }

    SubTexture(const std::string& _id, const SDL_Rect& _rect, const SDL_Rect& _source, bool _rotated = false)
        : id(_id), rect(_rect), source(_source), rotated(_rotated) {
    }
};

/**
 * @brief Класс для управления атласом текстур
 */
class TextureAtlas {
public:
    /**
     * @brief Конструктор
     * @param renderer Указатель на SDL_Renderer
     * @param width Ширина атласа
     * @param height Высота атласа
     * @param id Идентификатор атласа
     */
    TextureAtlas(SDL_Renderer* renderer, int width, int height, const std::string& id);

    /**
     * @brief Деструктор
     */
    ~TextureAtlas();

    /**
     * @brief Добавляет текстуру в атлас
     * @param id Идентификатор текстуры
     * @param texture Указатель на текстуру для добавления
     * @return true в случае успеха, false если не удалось найти место в атласе
     */
    bool addTexture(const std::string& id, SDL_Texture* texture);

    /**
     * @brief Добавляет текстуру из файла в атлас
     * @param id Идентификатор текстуры
     * @param filePath Путь к файлу текстуры
     * @return true в случае успеха, false при ошибке
     */
    bool addTextureFromFile(const std::string& id, const std::string& filePath);

    /**
     * @brief Получает координаты субтекстуры в атласе
     * @param id Идентификатор субтекстуры
     * @param rect Указатель на прямоугольник для заполнения координатами
     * @return true в случае успеха, false если субтекстура не найдена
     */
    bool getTextureRect(const std::string& id, SDL_Rect* rect) const;

    /**
     * @brief Проверяет наличие субтекстуры
     * @param id Идентификатор субтекстуры
     * @return true, если субтекстура существует, false в противном случае
     */
    bool hasTexture(const std::string& id) const;

    /**
     * @brief Получает указатель на атлас-текстуру
     * @return Указатель на SDL_Texture атласа
     */
    SDL_Texture* getAtlasTexture() const;

    /**
     * @brief Отрисовывает субтекстуру
     * @param renderer Указатель на SDL_Renderer
     * @param id Идентификатор субтекстуры
     * @param dstRect Прямоугольник для отрисовки
     * @param angle Угол поворота (в градусах)
     * @param center Центр вращения (может быть nullptr)
     * @param flip Флаги отражения
     * @return true в случае успеха, false при ошибке
     */
    bool renderSubTexture(SDL_Renderer* renderer, const std::string& id, const SDL_Rect* dstRect,
        double angle = 0.0, const SDL_Point* center = nullptr,
        SDL_RendererFlip flip = SDL_FLIP_NONE);

    /**
     * @brief Получает ширину субтекстуры
     * @param id Идентификатор субтекстуры
     * @return Ширина субтекстуры или -1, если субтекстура не найдена
     */
    int getSubTextureWidth(const std::string& id) const;

    /**
     * @brief Получает высоту субтекстуры
     * @param id Идентификатор субтекстуры
     * @return Высота субтекстуры или -1, если субтекстура не найдена
     */
    int getSubTextureHeight(const std::string& id) const;

    /**
     * @brief Получает идентификатор атласа
     * @return Идентификатор атласа
     */
    const std::string& getId() const;

    /**
     * @brief Получает размер атласа в байтах
     * @return Размер атласа в памяти
     */
    size_t getMemorySize() const;

    /**
     * @brief Получает количество субтекстур в атласе
     * @return Количество субтекстур
     */
    size_t getSubTextureCount() const;

    /**
     * @brief Сохраняет атлас текстур в файл
     * @param filePath Путь к файлу для сохранения
     * @return true в случае успеха, false при ошибке
     */
    bool saveToFile(const std::string& filePath);

    /**
     * @brief Загружает атлас текстур из файла
     * @param filePath Путь к файлу атласа
     * @param descriptorFilePath Путь к файлу описания атласа (формат JSON)
     * @return true в случае успеха, false при ошибке
     */
    bool loadFromFile(const std::string& filePath, const std::string& descriptorFilePath);

    /**
     * @brief Создает описание атласа в формате JSON
     * @param filePath Путь к файлу для сохранения описания
     * @return true в случае успеха, false при ошибке
     */
    bool saveDescriptor(const std::string& filePath);

    /**
     * @brief Получает информацию о всех субтекстурах в виде строки
     * @return Строка с информацией о субтекстурах
     */
    std::string getSubTexturesInfo() const;

private:
    /**
     * @brief Находит свободное место в атласе для размещения текстуры
     * @param width Ширина текстуры
     * @param height Высота текстуры
     * @param rect Указатель на прямоугольник для заполнения найденными координатами
     * @param allowRotation Разрешить поворот текстуры для лучшей упаковки
     * @return true, если место найдено, false если атлас заполнен
     */
    bool findFreeSpace(int width, int height, SDL_Rect* rect, bool allowRotation = true);

    /**
     * @brief Вычисляет примерный размер атласа в памяти
     * @return Размер в байтах
     */
    size_t calculateAtlasSize() const;

private:
    SDL_Renderer* m_renderer;                       ///< Указатель на SDL_Renderer
    SDL_Texture* m_atlasTexture;                    ///< Текстура атласа
    std::string m_id;                               ///< Идентификатор атласа
    int m_width;                                    ///< Ширина атласа
    int m_height;                                   ///< Высота атласа
    std::unordered_map<std::string, SubTexture> m_subTextures; ///< Хранилище информации о субтекстурах
    std::vector<SDL_Rect> m_freeRects;              ///< Список свободных прямоугольников для размещения текстур
    size_t m_memorySize;                            ///< Размер атласа в памяти
};