#include "Engine.h"
#include "MapScene.h"
#include "PlanetScene.h"
#include "Logger.h"
#include <iostream>
#include <memory>
#include <ctime>

int main(int argc, char* argv[]) {
    // Для работы с консольными приложениями Windows
#ifdef _WIN32
    SDL_SetMainReady();
#endif

    // Инициализация системы логирования
    Logger::getInstance().initialize(true, "satellite.log", LogLevel::INFO, LogLevel::DEBUG);
    LOG_INFO("Satellite Engine starting...");

    // 1. Парсинг аргументов командной строки
    bool usePlanetScene = true; // Изменено на true по умолчанию
    int sceneType = 0;
    bool debugMode = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--map" || arg == "-m") {
            usePlanetScene = false; // Если указан флаг --map, используем старую сцену
            LOG_INFO("Using Map Scene mode");
        }
        else if (arg == "--scene" || arg == "-s") {
            if (i + 1 < argc) {
                sceneType = std::stoi(argv[i + 1]);
                LOG_INFO("Using scene type: " + std::to_string(sceneType));
                i++; // Пропускаем следующий аргумент, так как мы уже его обработали
            }
        }
        else if (arg == "--debug" || arg == "-d") {
            debugMode = true;
            Logger::getInstance().setConsoleLogLevel(LogLevel::DEBUG);
            LOG_INFO("Debug mode enabled");
        }
        else if (arg == "--quiet" || arg == "-q") {
            Logger::getInstance().setConsoleLogLevel(LogLevel::WARNING);
            LOG_INFO("Quiet mode enabled (only warnings and errors)");
        }
    }

    // В релизной сборке отключаем большую часть логирования, кроме явного включения
#ifdef NDEBUG
    if (!debugMode) {
        DisableLoggingForRelease();
        LOG_INFO("Release mode - most logging disabled");
    }
#endif

    // 2. Создание и инициализация движка
    Engine engine("Satellite Engine - Planet Generation Demo", 800, 600);

    if (!engine.initialize()) {
        LOG_ERROR("Failed to initialize engine. Exiting...");
        return 1;
    }

    // 3. Создание и инициализация сцены в зависимости от аргументов
    std::shared_ptr<Scene> activeScene;

    if (usePlanetScene) {
        // Создаем сцену генерации планет
        auto planetScene = std::make_shared<PlanetScene>("PlanetScene", &engine);
        if (!planetScene->initialize()) {
            LOG_ERROR("Failed to initialize planet scene. Exiting...");
            return 1;
        }

        // Если указан тип сцены (тип планеты), генерируем соответствующую планету
        if (sceneType >= 1 && sceneType <= 6) {
            MapGenerator::GenerationType terrainType;
            float temperature = 20.0f;
            float waterCoverage = 0.5f;

            switch (sceneType) {
            case 1: // DEFAULT
                terrainType = MapGenerator::GenerationType::DEFAULT;
                break;
            case 2: // ARCHIPELAGO
                terrainType = MapGenerator::GenerationType::ARCHIPELAGO;
                temperature = 25.0f;
                waterCoverage = 0.7f;
                break;
            case 3: // MOUNTAINOUS
                terrainType = MapGenerator::GenerationType::MOUNTAINOUS;
                temperature = 10.0f;
                waterCoverage = 0.3f;
                break;
            case 4: // CRATER
                terrainType = MapGenerator::GenerationType::CRATER;
                temperature = 5.0f;
                waterCoverage = 0.2f;
                break;
            case 5: // VOLCANIC
                terrainType = MapGenerator::GenerationType::VOLCANIC;
                temperature = 60.0f;
                waterCoverage = 0.3f;
                break;
            case 6: // ALIEN
                terrainType = MapGenerator::GenerationType::ALIEN;
                temperature = 30.0f;
                waterCoverage = 0.4f;
                break;
            default:
                terrainType = MapGenerator::GenerationType::DEFAULT;
            }

            planetScene->generateCustomPlanet(temperature, waterCoverage, terrainType);
            LOG_INFO("Generated custom planet with type: " + std::to_string(sceneType));
        }

        activeScene = planetScene;
    }
    else {
        // Создаем стандартную MapScene для обратной совместимости
        auto mapScene = std::make_shared<MapScene>("MapScene", &engine);
        if (!mapScene->initialize()) {
            LOG_ERROR("Failed to initialize map scene. Exiting...");
            return 1;
        }
        activeScene = mapScene;
    }

    // 4. Установка активной сцены
    engine.setActiveScene(activeScene);

    // 5. Вывод инструкций в зависимости от типа сцены
    if (debugMode) {
        LOG_INFO("*** Satellite Engine - " + std::string(usePlanetScene ? "Planet Generation Demo" : "Tile System Demo") + " ***");
        LOG_INFO("Controls:");

        if (usePlanetScene) {
            LOG_INFO("  WASD or Arrow keys - Move");
            LOG_INFO("  G - Generate random planet");
            LOG_INFO("  1-6 - Generate specific planet types");
            LOG_INFO("  TAB - Toggle display mode");
            LOG_INFO("  F1 - Toggle debug information");
            LOG_INFO("  Mouse wheel - Zoom in/out");
            LOG_INFO("  Middle mouse button - Drag camera");
            LOG_INFO("  R - Reset player position");
            LOG_INFO("  ESC - Exit");
        }
        else {
            LOG_INFO("  WASD or Arrow keys - Move");
            LOG_INFO("  G - Generate new test map");
            LOG_INFO("  Mouse wheel - Zoom in/out");
            LOG_INFO("  Middle mouse button - Drag camera");
            LOG_INFO("  R - Reset player position");
            LOG_INFO("  ESC - Exit");
        }
    }

    // 6. Запуск движка
    engine.run();

    LOG_INFO("Application finished successfully.");
    return 0;
}