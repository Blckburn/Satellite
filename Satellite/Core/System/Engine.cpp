/**
 * @file Engine.cpp
 * @brief Реализация основного класса движка
 */

#include "Core/System/Engine.h"
#include "Core/Base/Scene.h"
#include "Core/System/Logger.h"

// Временно включаем прямые заголовки SDL, 
// в будущем их нужно будет изолировать в отдельных адаптерах
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

namespace Satellite {

Engine::Engine(const std::string& title, int width, int height)
    : m_title(title), m_width(width), m_height(height), m_isRunning(false),
    m_window(nullptr), m_renderer(nullptr), m_deltaTime(0.0f) {
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {
    // Инициализация логгера
    Logger::getInstance().initialize(true, "satellite.log");
    LogInfo("Initializing engine: " + m_title);

    // 1. Инициализация SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LogError("SDL could not initialize! SDL Error: " + std::string(SDL_GetError()));
        return false;
    }

    // 2. Инициализация SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        LogError("SDL_image could not initialize! SDL_image Error: " + std::string(IMG_GetError()));
        return false;
    }

    // 3. Инициализация SDL_ttf
    if (TTF_Init() < 0) {
        LogError("SDL_ttf could not initialize! SDL_ttf Error: " + std::string(TTF_GetError()));
        return false;
    }

    // 4. Создание окна
    m_window = SDL_CreateWindow(
        m_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_width, m_height,
        SDL_WINDOW_SHOWN
    );

    if (!m_window) {
        LogError("Window could not be created! SDL Error: " + std::string(SDL_GetError()));
        return false;
    }

    // 5. Создание рендерера
    m_renderer = SDL_CreateRenderer(
        m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        LogError("Renderer could not be created! SDL Error: " + std::string(SDL_GetError()));
        return false;
    }

    // 6. Установка цвета рендеринга по умолчанию (черный)
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

    // 7. Инициализация ResourceManager
    // Примечание: временно закомментировано, так как ResourceManager будет перенесен в Systems
    // m_resourceManager = std::make_shared<ResourceManager>(m_renderer);
    // if (!m_resourceManager) {
    //     LogError("Failed to create ResourceManager!");
    //     return false;
    // }

    m_isRunning = true;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();

    LogInfo("Engine initialized successfully");
    return true;
}

void Engine::run() {
    if (!m_isRunning) {
        LogError("Engine not initialized or already stopped!");
        return;
    }

    LogInfo("Starting main game loop");

    // Основной игровой цикл
    while (m_isRunning) {
        calculateDeltaTime();
        processInput();
        update();
        render();
    }

    LogInfo("Main game loop ended");
}

void Engine::shutdown() {
    LogInfo("Engine shutting down");

    // 1. Очистка ResourceManager
    if (m_resourceManager) {
        // m_resourceManager->clearAll();
        m_resourceManager.reset();
    }

    // 2. Освобождение ресурсов SDL
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    // 3. Завершение работы SDL_ttf, SDL_image и SDL
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    m_isRunning = false;
    LogInfo("Engine shutdown completed");
}

void Engine::setActiveScene(std::shared_ptr<Scene> scene) {
    m_activeScene = scene;
    LogInfo("Active scene set to: " + scene->getName());
}

void Engine::processInput() {
    SDL_Event event;

    // Обработка всех ожидающих событий
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_isRunning = false;
        }

        // Выход по нажатию Escape
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            m_isRunning = false;
        }

        // Передаем события в активную сцену, если она существует
        if (m_activeScene) {
            m_activeScene->handleEvent(event);
        }
    }
}

void Engine::update() {
    // Обновляем активную сцену, если она существует
    if (m_activeScene) {
        m_activeScene->update(m_deltaTime);
    }
}

void Engine::render() {
    // 1. Выбираем цвет фона в зависимости от текущего биома
    switch (m_currentBiome) {
    case 1: // FOREST
        SDL_SetRenderDrawColor(m_renderer, 10, 20, 10, 255);
        break;
    case 2: // DESERT
        SDL_SetRenderDrawColor(m_renderer, 20, 15, 10, 255);
        break;
    case 3: // TUNDRA
        SDL_SetRenderDrawColor(m_renderer, 10, 15, 20, 255);
        break;
    case 4: // VOLCANIC
        SDL_SetRenderDrawColor(m_renderer, 20, 10, 10, 255);
        break;
    default:
        SDL_SetRenderDrawColor(m_renderer, 30, 45, 30, 255);
        break;
    }

    SDL_RenderClear(m_renderer);

    // 2. Отрисовка активной сцены, если она существует
    if (m_activeScene) {
        m_activeScene->render(m_renderer);
    }

    // 3. Вывод отрисованного кадра на экран
    SDL_RenderPresent(m_renderer);
}

void Engine::calculateDeltaTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    m_deltaTime = std::chrono::duration<float>(currentTime - m_lastFrameTime).count();
    m_lastFrameTime = currentTime;
}

} // namespace Satellite