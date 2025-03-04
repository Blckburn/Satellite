#pragma once

#include <SDL.h>
#include <string>
#include <memory>
#include <chrono>

class Scene;
class ResourceManager;

/**
 * @brief Основной класс движка, управляющий игровым циклом
 */
class Engine {
public:
    /**
* @brief Установка текущего биома для визуализации
* @param biomeType Тип биома
*/
    void setCurrentBiome(int biomeType) { m_currentBiome = biomeType; }

    /**
     * @brief Конструктор движка
     * @param title Заголовок окна
     * @param width Ширина окна
     * @param height Высота окна
     */
    Engine(const std::string& title, int width, int height);

    /**
     * @brief Деструктор
     */
    ~Engine();

    /**
     * @brief Инициализация движка
     * @return true в случае успеха, false при ошибке
     */
    bool initialize();

    /**
     * @brief Запускает игровой цикл
     */
    void run();

    /**
     * @brief Завершает работу движка
     */
    void shutdown();

    /**
     * @brief Устанавливает активную сцену
     * @param scene Указатель на сцену
     */
    void setActiveScene(std::shared_ptr<Scene> scene);

    /**
     * @brief Получает указатель на SDL_Renderer
     * @return Указатель на SDL_Renderer
     */
    SDL_Renderer* getRenderer() const { return m_renderer; }

    /**
     * @brief Получает указатель на ResourceManager
     * @return Указатель на ResourceManager
     */
    std::shared_ptr<ResourceManager> getResourceManager() const { return m_resourceManager; }

    /**
     * @brief Получает время, прошедшее с последнего кадра
     * @return Время в секундах
     */
    float getDeltaTime() const { return m_deltaTime; }

    /**
     * @brief Проверяет, работает ли движок
     * @return true, если движок работает, false в противном случае
     */
    bool isRunning() const { return m_isRunning; }

private:
    /**
     * @brief Обрабатывает ввод пользователя
     */
    void processInput();

    /**
     * @brief Обновляет логику игры
     */
    void update();

    /**
     * @brief Отрисовывает текущий кадр
     */
    void render();

    /**
     * @brief Вычисляет время, прошедшее с последнего кадра
     */
    void calculateDeltaTime();


private:
    std::string m_title;           ///< Заголовок окна
    int m_width;                   ///< Ширина окна
    int m_height;                  ///< Высота окна
    bool m_isRunning;              ///< Флаг работы движка

    SDL_Window* m_window;          ///< Указатель на окно SDL
    SDL_Renderer* m_renderer;      ///< Указатель на рендерер SDL

    std::shared_ptr<ResourceManager> m_resourceManager;  ///< Менеджер ресурсов
    std::shared_ptr<Scene> m_activeScene;  ///< Активная сцена

    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrameTime;  ///< Время последнего кадра
    float m_deltaTime;             ///< Время между кадрами
    int m_currentBiome = 0; ///< Текущий биом для визуализации

};