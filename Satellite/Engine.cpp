﻿#include "Engine.h"
#include "Scene.h"
#include "ResourceManager.h"
#include <iostream>
#include <SDL_image.h>

Engine::Engine(const std::string& title, int width, int height)
    : m_title(title), m_width(width), m_height(height), m_isRunning(false),
    m_window(nullptr), m_renderer(nullptr), m_deltaTime(0.0f) {
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {
    // 1. Инициализация SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 2. Инициализация SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // 3. Создание окна
    m_window = SDL_CreateWindow(
        m_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_width, m_height,
        SDL_WINDOW_SHOWN
    );

    if (!m_window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 4. Создание рендерера
    m_renderer = SDL_CreateRenderer(
        m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 5. Установка цвета рендеринга по умолчанию (черный)
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

    // 6. Инициализация ResourceManager
    m_resourceManager = std::make_shared<ResourceManager>(m_renderer);
    if (!m_resourceManager) {
        std::cerr << "Failed to create ResourceManager!" << std::endl;
        return false;
    }

    m_isRunning = true;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();

    std::cout << "Engine initialized successfully" << std::endl;
    return true;
}

void Engine::run() {
    if (!m_isRunning) {
        std::cerr << "Engine not initialized or already stopped!" << std::endl;
        return;
    }

    // Основной игровой цикл
    while (m_isRunning) {
        calculateDeltaTime();
        processInput();
        update();
        render();
    }
}

void Engine::shutdown() {
    // 1. Очистка ResourceManager
    if (m_resourceManager) {
        m_resourceManager->clearAll();
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

    // 3. Завершение работы SDL_image и SDL
    IMG_Quit();
    SDL_Quit();

    m_isRunning = false;
    std::cout << "Engine shutdown completed" << std::endl;
}

void Engine::setActiveScene(std::shared_ptr<Scene> scene) {
    m_activeScene = scene;
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
    // 1. Устанавливаем темно-зеленый цвет фона для лучшего визуального соответствия с тайлами травы
    // Этот цвет близок к цвету травы, что сделает черные артефакты менее заметными
    SDL_SetRenderDrawColor(m_renderer, 30, 45, 30, 255);
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