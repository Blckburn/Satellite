#include "Engine.h"
#include "MapScene.h"
#include "PlanetScene.h"
#include <iostream>
#include <memory>
#include <ctime>

int main(int argc, char* argv[]) {
    // Для работы с консольными приложениями Windows
#ifdef _WIN32
    SDL_SetMainReady();
#endif

    // 1. Парсинг аргументов командной строки
    bool usePlanetScene = true; // Изменено на true по умолчанию
    int sceneType = 0;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--map" || arg == "-m") {
            usePlanetScene = false; // Если указан флаг --map, используем старую сцену
        }
        else if (arg == "--scene" || arg == "-s") {
            if (i + 1 < argc) {
                sceneType = std::stoi(argv[i + 1]);
                i++; // Пропускаем следующий аргумент, так как мы уже его обработали
            }
        }
    }

    // 2. Создание и инициализация движка
    Engine engine("Satellite Engine - Planet Generation Demo", 800, 600);

    if (!engine.initialize()) {
        std::cerr << "Failed to initialize engine. Exiting..." << std::endl;
        return 1;
    }

    // 3. Создание и инициализация сцены в зависимости от аргументов
    std::shared_ptr<Scene> activeScene;

    if (usePlanetScene) {
        // Создаем сцену генерации планет
        auto planetScene = std::make_shared<PlanetScene>("PlanetScene", &engine);
        if (!planetScene->initialize()) {
            std::cerr << "Failed to initialize planet scene. Exiting..." << std::endl;
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
        }

        activeScene = planetScene;
    }
    else {
        // Создаем стандартную MapScene для обратной совместимости
        auto mapScene = std::make_shared<MapScene>("MapScene", &engine);
        if (!mapScene->initialize()) {
            std::cerr << "Failed to initialize map scene. Exiting..." << std::endl;
            return 1;
        }
        activeScene = mapScene;
    }

    // 4. Установка активной сцены
    engine.setActiveScene(activeScene);

    // 5. Вывод инструкций в зависимости от типа сцены
    std::cout << "\n*** Satellite Engine - ";
    if (usePlanetScene) {
        std::cout << "Planet Generation Demo ***\n";
        std::cout << "Controls:\n";
        std::cout << "  WASD or Arrow keys - Move\n";
        std::cout << "  G - Generate random planet\n";
        std::cout << "  1-6 - Generate specific planet types:\n";
        std::cout << "       1: Default, 2: Archipelago, 3: Mountainous,\n";
        std::cout << "       4: Crater, 5: Volcanic, 6: Alien\n";
        std::cout << "  TAB - Toggle display mode (normal, temperature, humidity, etc.)\n";
        std::cout << "  Mouse wheel - Zoom in/out\n";
        std::cout << "  Middle mouse button - Drag camera\n";
        std::cout << "  R - Reset player position\n";
        std::cout << "  ESC - Exit\n\n";
    }
    else {
        std::cout << "Tile System Demo ***\n";
        std::cout << "Controls:\n";
        std::cout << "  WASD or Arrow keys - Move\n";
        std::cout << "  G - Generate new test map\n";
        std::cout << "  Mouse wheel - Zoom in/out\n";
        std::cout << "  Middle mouse button - Drag camera\n";
        std::cout << "  R - Reset player position\n";
        std::cout << "  ESC - Exit\n\n";
    }

    // 6. Запуск движка
    engine.run();

    std::cout << "Application finished successfully." << std::endl;
    return 0;
}