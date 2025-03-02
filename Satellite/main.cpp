#include "Engine.h"
#include "MapScene.h"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    // Для работы с консольными приложениями Windows
#ifdef _WIN32
    SDL_SetMainReady();
#endif

    // 1. Создание и инициализация движка
    Engine engine("Satellite Engine - Tile System Demo", 800, 600);

    if (!engine.initialize()) {
        std::cerr << "Failed to initialize engine. Exiting..." << std::endl;
        return 1;
    }

    // 2. Создание и инициализация MapScene с передачей указателя на движок
    auto mapScene = std::make_shared<MapScene>("MapScene", &engine);
    if (!mapScene->initialize()) {
        std::cerr << "Failed to initialize map scene. Exiting..." << std::endl;
        return 1;
    }

    // 3. Установка активной сцены
    engine.setActiveScene(mapScene);

    // 4. Вывод инструкций
    std::cout << "\n*** Satellite Engine - Tile System Demo ***\n";
    std::cout << "Controls:\n";
    std::cout << "Movement:\n";
    std::cout << "  W or Up Arrow    - Move north\n";
    std::cout << "  S or Down Arrow  - Move south\n";
    std::cout << "  D or Right Arrow - Move east\n";
    std::cout << "  A or Left Arrow  - Move west\n";
    std::cout << "  W+D              - Move northeast\n";
    std::cout << "  W+A              - Move northwest\n";
    std::cout << "  S+D              - Move southeast\n";
    std::cout << "  S+A              - Move southwest\n";
    std::cout << "\nOther controls:\n";
    std::cout << "  G - Generate new test map\n";
    std::cout << "  Mouse wheel - Zoom in/out\n";
    std::cout << "  Middle mouse button - Drag camera\n";
    std::cout << "  R - Reset player position\n";
    std::cout << "  ESC - Exit\n\n";

    // 5. Запуск движка
    engine.run();

    std::cout << "Application finished successfully." << std::endl;
    return 0;
}