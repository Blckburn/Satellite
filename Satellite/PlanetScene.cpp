#include "PlanetScene.h"
#include "Engine.h"
#include <iostream>
#include <sstream>
#include <iomanip>

PlanetScene::PlanetScene(const std::string& name, Engine* engine)
    : Scene(name), m_engine(engine), m_playerX(0.0f), m_playerY(0.0f),
    m_displayMode(DisplayMode::NORMAL) {
}

PlanetScene::~PlanetScene() {
}

bool PlanetScene::initialize() {
    // Инициализация изометрического рендерера
    m_isoRenderer = std::make_shared<IsometricRenderer>(64, 32);

    // Инициализация камеры
    m_camera = std::make_shared<Camera>(800, 600);
    m_camera->setTarget(&m_playerX, &m_playerY);

    // Создание карты размером 100x100 тайлов
    m_tileMap = std::make_shared<TileMap>(100, 100);
    if (!m_tileMap->initialize()) {
        std::cerr << "Failed to initialize tile map" << std::endl;
        return false;
    }

    // Инициализация рендерера тайлов
    m_tileRenderer = std::make_shared<TileRenderer>(m_isoRenderer.get());

    // Инициализация генератора миров
    m_worldGenerator = std::make_shared<WorldGenerator>();

    // Генерация первой планеты
    generateRandomPlanet();

    std::cout << "PlanetScene initialized successfully" << std::endl;
    return true;
}

void PlanetScene::handleEvent(const SDL_Event& event) {
    // Обработка событий камеры
    m_camera->handleEvent(event);

    // Обработка клавиатуры для перемещения персонажа
    if (event.type == SDL_KEYDOWN) {
        int curX = static_cast<int>(m_playerX);
        int curY = static_cast<int>(m_playerY);
        int newX = curX;
        int newY = curY;

        switch (event.key.keysym.sym) {
        case SDLK_w: // Северо-запад (NW)
            newX -= 1;
            newY -= 1;
            break;
        case SDLK_s: // Юго-восток (SE)
            newX += 1;
            newY += 1;
            break;
        case SDLK_a: // Юго-запад (SW)
            newX -= 1;
            newY += 1;
            break;
        case SDLK_d: // Северо-восток (NE)
            newX += 1;
            newY -= 1;
            break;
        case SDLK_UP: // Север (N)
            newY -= 1;
            break;
        case SDLK_DOWN: // Юг (S)
            newY += 1;
            break;
        case SDLK_LEFT: // Запад (W)
            newX -= 1;
            break;
        case SDLK_RIGHT: // Восток (E)
            newX += 1;
            break;
        case SDLK_r: // Сброс позиции
            m_playerX = m_tileMap->getWidth() / 2.0f;
            m_playerY = m_tileMap->getHeight() / 2.0f;
            break;
        case SDLK_g: // Генерация новой планеты
            generateRandomPlanet();
            break;
        case SDLK_1: // Генерация типа DEFAULT
            generateCustomPlanet(20.0f, 0.5f, MapGenerator::GenerationType::DEFAULT);
            break;
        case SDLK_2: // Генерация типа ARCHIPELAGO
            generateCustomPlanet(25.0f, 0.7f, MapGenerator::GenerationType::ARCHIPELAGO);
            break;
        case SDLK_3: // Генерация типа MOUNTAINOUS
            generateCustomPlanet(10.0f, 0.3f, MapGenerator::GenerationType::MOUNTAINOUS);
            break;
        case SDLK_4: // Генерация типа CRATER
            generateCustomPlanet(5.0f, 0.2f, MapGenerator::GenerationType::CRATER);
            break;
        case SDLK_5: // Генерация типа VOLCANIC
            generateCustomPlanet(60.0f, 0.3f, MapGenerator::GenerationType::VOLCANIC);
            break;
        case SDLK_6: // Генерация типа ALIEN
            generateCustomPlanet(30.0f, 0.4f, MapGenerator::GenerationType::ALIEN);
            break;
        case SDLK_TAB: // Переключение режима отображения
            toggleDisplayMode();
            break;
        }

        // Проверяем возможность перемещения
        if (newX != curX || newY != curY) { // Если позиция изменилась
            if (m_tileMap->isValidCoordinate(newX, newY) && m_tileMap->isTileWalkable(newX, newY)) {
                m_playerX = static_cast<float>(newX);
                m_playerY = static_cast<float>(newY);
            }
        }
    }

    // Передаем события в базовый класс
    Scene::handleEvent(event);
}

void PlanetScene::update(float deltaTime) {
    // Обновление камеры
    m_camera->update(deltaTime);

    // Передаем события в базовый класс
    Scene::update(deltaTime);
}

void PlanetScene::render(SDL_Renderer* renderer) {
    // Очищаем экран
    SDL_SetRenderDrawColor(renderer, 20, 35, 20, 255);
    SDL_RenderClear(renderer);

    // Получаем размер окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    // Настраиваем рендерер с учетом камеры
    m_isoRenderer->setCameraPosition(m_camera->getX(), m_camera->getY());
    m_isoRenderer->setCameraZoom(m_camera->getZoom());

    // Рендерим тайлы с учетом режима отображения
    renderTiles(renderer, centerX, centerY);

    // Добавляем игрока (красный куб)
    SDL_Color playerColor = { 255, 0, 0, 255 };
    SDL_Color playerLeftColor = { 200, 0, 0, 255 };
    SDL_Color playerRightColor = { 150, 0, 0, 255 };

    m_tileRenderer->addVolumetricTile(
        m_playerX, m_playerY, 0.5f,
        nullptr, nullptr, nullptr,
        playerColor, playerLeftColor, playerRightColor,
        100 // Наивысший приоритет для игрока
    );

    // Рендерим все тайлы
    m_tileRenderer->render(renderer, centerX, centerY);

    // Отрисовываем индикатор над игроком
    int indicatorX, indicatorY;
    m_isoRenderer->worldToDisplay(m_playerX, m_playerY, 0.7f,
        centerX, centerY,
        indicatorX, indicatorY);

    // Размер индикатора с учетом масштаба
    int indicatorSize = m_isoRenderer->getScaledSize(8);

    // Рисуем белый квадрат с черной обводкой
    SDL_Rect indicator = {
        indicatorX - indicatorSize,
        indicatorY - indicatorSize,
        indicatorSize * 2,
        indicatorSize * 2
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &indicator);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &indicator);

    // Отрисовываем информацию о планете
    renderPlanetInfo(renderer);

    // Отрисовываем легенду для текущего режима отображения
    renderDisplayLegend(renderer);

    // Отрисовываем дополнительные сущности
    Scene::render(renderer);
}

void PlanetScene::renderTiles(SDL_Renderer* renderer, int centerX, int centerY) {
    // Очищаем и заполняем TileRenderer тайлами
    m_tileRenderer->clear();

    int width = m_tileMap->getWidth();
    int height = m_tileMap->getHeight();

    // Определяем видимую область карты
    float cameraX = m_camera->getX();
    float cameraY = m_camera->getY();
    float zoom = m_camera->getZoom();
    int viewportRadius = static_cast<int>(30.0f / zoom) + 1; // Радиус видимой области

    // Преобразуем мировые координаты камеры в координаты тайла
    int camTileX = static_cast<int>(cameraX);
    int camTileY = static_cast<int>(cameraY);

    // Определяем границы видимой области (с запасом)
    int startX = std::max(0, camTileX - viewportRadius);
    int startY = std::max(0, camTileY - viewportRadius);
    int endX = std::min(width - 1, camTileX + viewportRadius);
    int endY = std::min(height - 1, camTileY + viewportRadius);

    // Добавляем тайлы в видимой области
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            const MapTile* tile = m_tileMap->getTile(x, y);
            if (tile) {
                // Получаем базовые свойства тайла
                TileType type = tile->getType();
                float height = tile->getHeight();
                SDL_Color color;

                // Определяем цвет в зависимости от режима отображения
                switch (m_displayMode) {
                case DisplayMode::NORMAL:
                    // Обычный режим - используем цвет тайла
                    color = tile->getColor();
                    break;

                case DisplayMode::TEMPERATURE:
                    // Температура: от синего (холодно) к красному (жарко)
                {
                    float temp = tile->getTemperature();
                    if (temp < 0.0f) {
                        // Холодные цвета (синий -> голубой)
                        float factor = std::min(1.0f, -temp / 50.0f);
                        color = {
                            static_cast<Uint8>(100 * (1.0f - factor)),
                            static_cast<Uint8>(100 * (1.0f - factor) + 155 * factor),
                            255,
                            255
                        };
                    }
                    else {
                        // Теплые цвета (зеленый -> желтый -> красный)
                        float factor = std::min(1.0f, temp / 100.0f);
                        if (factor < 0.5f) {
                            // Зеленый -> желтый
                            float localFactor = factor * 2.0f;
                            color = {
                                static_cast<Uint8>(255 * localFactor),
                                255,
                                static_cast<Uint8>(50 * (1.0f - localFactor)),
                                255
                            };
                        }
                        else {
                            // Желтый -> красный
                            float localFactor = (factor - 0.5f) * 2.0f;
                            color = {
                                255,
                                static_cast<Uint8>(255 * (1.0f - localFactor)),
                                0,
                                255
                            };
                        }
                    }
                }
                break;

                case DisplayMode::HUMIDITY:
                    // Влажность: от желтого (сухо) к синему (влажно)
                {
                    float humidity = tile->getHumidity();
                    if (humidity < 0.5f) {
                        // Сухо: желтый -> зеленый
                        float factor = humidity * 2.0f;
                        color = {
                            static_cast<Uint8>(255 * (1.0f - factor)),
                            static_cast<Uint8>(255 - 100 * factor),
                            static_cast<Uint8>(50 + 100 * factor),
                            255
                        };
                    }
                    else {
                        // Влажно: зеленый -> синий
                        float factor = (humidity - 0.5f) * 2.0f;
                        color = {
                            0,
                            static_cast<Uint8>(155 * (1.0f - factor)),
                            static_cast<Uint8>(150 + 105 * factor),
                            255
                        };
                    }
                }
                break;

                case DisplayMode::ELEVATION:
                    // Высота: от темно-зеленого (низко) к белому (высоко)
                {
                    float elevation = tile->getElevation();
                    if (elevation < 0.3f) {
                        // Низкая высота: темно-зеленый
                        float factor = elevation / 0.3f;
                        color = {
                            static_cast<Uint8>(20 + 60 * factor),
                            static_cast<Uint8>(80 + 60 * factor),
                            static_cast<Uint8>(20 + 40 * factor),
                            255
                        };
                    }
                    else if (elevation < 0.7f) {
                        // Средняя высота: светло-зеленый -> коричневый
                        float factor = (elevation - 0.3f) / 0.4f;
                        color = {
                            static_cast<Uint8>(80 + 100 * factor),
                            static_cast<Uint8>(140 - 40 * factor),
                            static_cast<Uint8>(60 - 40 * factor),
                            255
                        };
                    }
                    else {
                        // Высокая высота: коричневый -> белый
                        float factor = (elevation - 0.7f) / 0.3f;
                        color = {
                            static_cast<Uint8>(180 + 75 * factor),
                            static_cast<Uint8>(100 + 155 * factor),
                            static_cast<Uint8>(20 + 235 * factor),
                            255
                        };
                    }
                }
                break;

                case DisplayMode::RADIATION:
                    // Радиация: от зеленого (безопасно) к красному (опасно)
                {
                    float radiation = tile->getRadiationLevel();
                    if (radiation < 0.3f) {
                        // Безопасно: зеленый
                        color = { 50, 200, 50, 255 };
                    }
                    else if (radiation < 0.7f) {
                        // Умеренная радиация: желтый
                        color = { 230, 230, 0, 255 };
                    }
                    else {
                        // Высокая радиация: красный
                        color = { 255, 50, 50, 255 };
                    }
                }
                break;

                case DisplayMode::RESOURCES:
                    // Ресурсы: от серого (мало) к фиолетовому (много)
                {
                    float resources = tile->getResourceDensity();
                    if (resources < 0.3f) {
                        // Мало ресурсов: серый
                        color = { 150, 150, 150, 255 };
                    }
                    else if (resources < 0.7f) {
                        // Средний уровень: синий
                        color = { 50, 50, 200, 255 };
                    }
                    else {
                        // Много ресурсов: фиолетовый
                        color = { 200, 50, 200, 255 };
                    }
                }
                break;

                case DisplayMode::BIOMES:
                    // Биомы: разные цвета для разных биомов
                {
                    // Используем ID биома для создания уникального цвета
                    int biomeId = tile->getBiomeId();
                    int hue = (biomeId * 40) % 360; // Шаг в 40 градусов для разных биомов

                    // Преобразуем HSV в RGB
                    float h = static_cast<float>(hue) / 60.0f;
                    int hi = static_cast<int>(h);
                    float f = h - hi;

                    float s = 0.8f; // Насыщенность
                    float v = 0.9f; // Яркость

                    float p = v * (1.0f - s);
                    float q = v * (1.0f - s * f);
                    float t = v * (1.0f - s * (1.0f - f));

                    switch (hi) {
                    case 0: case 6:
                        color = { static_cast<Uint8>(v * 255), static_cast<Uint8>(t * 255), static_cast<Uint8>(p * 255), 255 };
                        break;
                    case 1:
                        color = { static_cast<Uint8>(q * 255), static_cast<Uint8>(v * 255), static_cast<Uint8>(p * 255), 255 };
                        break;
                    case 2:
                        color = { static_cast<Uint8>(p * 255), static_cast<Uint8>(v * 255), static_cast<Uint8>(t * 255), 255 };
                        break;
                    case 3:
                        color = { static_cast<Uint8>(p * 255), static_cast<Uint8>(q * 255), static_cast<Uint8>(v * 255), 255 };
                        break;
                    case 4:
                        color = { static_cast<Uint8>(t * 255), static_cast<Uint8>(p * 255), static_cast<Uint8>(v * 255), 255 };
                        break;
                    case 5:
                        color = { static_cast<Uint8>(v * 255), static_cast<Uint8>(p * 255), static_cast<Uint8>(q * 255), 255 };
                        break;
                    default:
                        color = { 150, 150, 150, 255 };
                    }
                }
                break;

                default:
                    color = tile->getColor();
                }

                if (type == TileType::EMPTY) {
                    continue; // Не рендерим пустые тайлы
                }

                if (height > 0.0f) {
                    // Объемный тайл
                    SDL_Color topColor = color;
                    SDL_Color leftColor = {
                        static_cast<Uint8>(color.r * 0.7f),
                        static_cast<Uint8>(color.g * 0.7f),
                        static_cast<Uint8>(color.b * 0.7f),
                        color.a
                    };
                    SDL_Color rightColor = {
                        static_cast<Uint8>(color.r * 0.5f),
                        static_cast<Uint8>(color.g * 0.5f),
                        static_cast<Uint8>(color.b * 0.5f),
                        color.a
                    };

                    m_tileRenderer->addVolumetricTile(
                        static_cast<float>(x), static_cast<float>(y), height,
                        nullptr, nullptr, nullptr,
                        topColor, leftColor, rightColor,
                        0 // Приоритет
                    );
                }
                else {
                    // Плоский тайл
                    m_tileRenderer->addFlatTile(
                        static_cast<float>(x), static_cast<float>(y),
                        nullptr, // Без текстуры
                        color,
                        0 // Приоритет
                    );
                }
            }
        }
    }
}

void PlanetScene::renderPlanetInfo(SDL_Renderer* renderer) {
    // Получаем размер окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Создаем информационную панель в верхней части экрана
    SDL_Rect infoPanel = { 10, 10, windowWidth - 20, 100 };

    // Рисуем полупрозрачный фон
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &infoPanel);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &infoPanel);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Здесь должен быть код для отображения текста
    // В рамках этого примера просто выводим в консоль
    std::cout << "Planet Info: " << getPlanetInfo() << std::endl;

    // Можно добавить визуализацию через SDL_ttf или аналогичные библиотеки
}

void PlanetScene::renderDisplayLegend(SDL_Renderer* renderer) {
    // Получаем размер окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Создаем панель легенды в правом нижнем углу
    int legendWidth = 200;
    int legendHeight = 150;
    SDL_Rect legendPanel = { windowWidth - legendWidth - 10, windowHeight - legendHeight - 10, legendWidth, legendHeight };

    // Рисуем полупрозрачный фон
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &legendPanel);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &legendPanel);

    // Отображаем градиент в зависимости от режима
    int gradientHeight = 20;
    int gradientTopMargin = 30;
    SDL_Rect gradientRect = { legendPanel.x + 10, legendPanel.y + gradientTopMargin, legendPanel.w - 20, gradientHeight };

    // Рисуем градиент
    int steps = 50;
    int stepWidth = gradientRect.w / steps;

    for (int i = 0; i < steps; ++i) {
        SDL_Rect stepRect = { gradientRect.x + i * stepWidth, gradientRect.y, stepWidth, gradientRect.h };

        // Определяем цвет в зависимости от режима отображения
        SDL_Color color;
        float factor = static_cast<float>(i) / (steps - 1);

        switch (m_displayMode) {
        case DisplayMode::TEMPERATURE:
            if (factor < 0.5f) {
                // Холодные цвета (синий -> зеленый)
                float localFactor = factor * 2.0f;
                color = {
                    static_cast<Uint8>(0 + 255 * localFactor),
                    static_cast<Uint8>(0 + 255 * localFactor),
                    static_cast<Uint8>(255 * (1.0f - localFactor)),
                    255
                };
            }
            else {
                // Теплые цвета (зеленый -> красный)
                float localFactor = (factor - 0.5f) * 2.0f;
                color = {
                    255,
                    static_cast<Uint8>(255 * (1.0f - localFactor)),
                    0,
                    255
                };
            }
            break;

        case DisplayMode::HUMIDITY:
            // Влажность: от желтого (сухо) к синему (влажно)
            if (factor < 0.5f) {
                // Сухо: желтый -> зеленый
                float localFactor = factor * 2.0f;
                color = {
                    static_cast<Uint8>(255 * (1.0f - localFactor)),
                    static_cast<Uint8>(255 - 100 * localFactor),
                    static_cast<Uint8>(50 + 100 * localFactor),
                    255
                };
            }
            else {
                // Влажно: зеленый -> синий
                float localFactor = (factor - 0.5f) * 2.0f;
                color = {
                    0,
                    static_cast<Uint8>(155 * (1.0f - localFactor)),
                    static_cast<Uint8>(150 + 105 * localFactor),
                    255
                };
            }
            break;

        case DisplayMode::ELEVATION:
            // Высота: от темно-зеленого (низко) к белому (высоко)
            if (factor < 0.3f) {
                // Низкая высота: темно-зеленый
                float localFactor = factor / 0.3f;
                color = {
                    static_cast<Uint8>(20 + 60 * localFactor),
                    static_cast<Uint8>(80 + 60 * localFactor),
                    static_cast<Uint8>(20 + 40 * localFactor),
                    255
                };
            }
            else if (factor < 0.7f) {
                // Средняя высота: светло-зеленый -> коричневый
                float localFactor = (factor - 0.3f) / 0.4f;
                color = {
                    static_cast<Uint8>(80 + 100 * localFactor),
                    static_cast<Uint8>(140 - 40 * localFactor),
                    static_cast<Uint8>(60 - 40 * localFactor),
                    255
                };
            }
            else {
                // Высокая высота: коричневый -> белый
                float localFactor = (factor - 0.7f) / 0.3f;
                color = {
                    static_cast<Uint8>(180 + 75 * localFactor),
                    static_cast<Uint8>(100 + 155 * localFactor),
                    static_cast<Uint8>(20 + 235 * localFactor),
                    255
                };
            }
            break;

        case DisplayMode::RADIATION:
            // Радиация: от зеленого (безопасно) к красному (опасно)
            if (factor < 0.3f) {
                // Безопасно: зеленый
                color = { 50, 200, 50, 255 };
            }
            else if (factor < 0.7f) {
                // Умеренная радиация: желтый
                float localFactor = (factor - 0.3f) / 0.4f;
                color = {
                    static_cast<Uint8>(50 + 180 * localFactor),
                    static_cast<Uint8>(200 + 30 * localFactor),
                    static_cast<Uint8>(50 - 50 * localFactor),
                    255
                };
            }
            else {
                // Высокая радиация: красный
                float localFactor = (factor - 0.7f) / 0.3f;
                color = {
                    static_cast<Uint8>(230 + 25 * localFactor),
                    static_cast<Uint8>(230 - 180 * localFactor),
                    static_cast<Uint8>(0 + 50 * localFactor),
                    255
                };
            }
            break;

        case DisplayMode::RESOURCES:
            // Ресурсы: от серого (мало) к фиолетовому (много)
            if (factor < 0.3f) {
                // Мало ресурсов: серый
                color = { 150, 150, 150, 255 };
            }
            else if (factor < 0.7f) {
                // Средний уровень: синий
                float localFactor = (factor - 0.3f) / 0.4f;
                color = {
                    static_cast<Uint8>(150 - 100 * localFactor),
                    static_cast<Uint8>(150 - 100 * localFactor),
                    static_cast<Uint8>(150 + 50 * localFactor),
                    255
                };
            }
            else {
                // Много ресурсов: фиолетовый
                float localFactor = (factor - 0.7f) / 0.3f;
                color = {
                    static_cast<Uint8>(50 + 150 * localFactor),
                    static_cast<Uint8>(50),
                    static_cast<Uint8>(200),
                    255
                };
            }
            break;

        case DisplayMode::BIOMES:
            // Для биомов просто делаем радужный градиент
        {
            int hue = static_cast<int>(factor * 360) % 360;

            // Преобразуем HSV в RGB
            float h = static_cast<float>(hue) / 60.0f;
            int hi = static_cast<int>(h);
            float f = h - hi;

            float s = 0.8f; // Насыщенность
            float v = 0.9f; // Яркость

            float p = v * (1.0f - s);
            float q = v * (1.0f - s * f);
            float t = v * (1.0f - s * (1.0f - f));

            switch (hi) {
            case 0: case 6:
                color = { static_cast<Uint8>(v * 255), static_cast<Uint8>(t * 255), static_cast<Uint8>(p * 255), 255 };
                break;
            case 1:
                color = { static_cast<Uint8>(q * 255), static_cast<Uint8>(v * 255), static_cast<Uint8>(p * 255), 255 };
                break;
            case 2:
                color = { static_cast<Uint8>(p * 255), static_cast<Uint8>(v * 255), static_cast<Uint8>(t * 255), 255 };
                break;
            case 3:
                color = { static_cast<Uint8>(p * 255), static_cast<Uint8>(q * 255), static_cast<Uint8>(v * 255), 255 };
                break;
            case 4:
                color = { static_cast<Uint8>(t * 255), static_cast<Uint8>(p * 255), static_cast<Uint8>(v * 255), 255 };
                break;
            case 5:
                color = { static_cast<Uint8>(v * 255), static_cast<Uint8>(p * 255), static_cast<Uint8>(q * 255), 255 };
                break;
            default:
                color = { 150, 150, 150, 255 };
            }
        }
        break;

        default:
            // Для обычного режима просто градиент серого
            color = {
                static_cast<Uint8>(factor * 255),
                static_cast<Uint8>(factor * 255),
                static_cast<Uint8>(factor * 255),
                255
            };
        }

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &stepRect);
    }

    // Отображаем подписи с минимальным и максимальным значениями
    // Здесь должен быть код для отображения текста
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void PlanetScene::generateRandomPlanet() {
    if (!m_worldGenerator || !m_tileMap) return;

    // Генерируем случайную планету
    m_planetData = m_worldGenerator->generateRandomPlanet(m_tileMap.get());

    // Устанавливаем позицию игрока в центре карты
    m_playerX = m_tileMap->getWidth() / 2.0f;
    m_playerY = m_tileMap->getHeight() / 2.0f;

    std::cout << "Generated new random planet: " << m_planetData.name << std::endl;
    std::cout << m_planetData.description << std::endl;
}

void PlanetScene::generateCustomPlanet(float temperature, float waterCoverage, MapGenerator::GenerationType terrainType) {
    if (!m_worldGenerator || !m_tileMap) return;

    // Генерируем планету с заданными параметрами
    m_planetData = m_worldGenerator->generateCustomPlanet(m_tileMap.get(), temperature, waterCoverage, terrainType);

    // Устанавливаем позицию игрока в центре карты
    m_playerX = m_tileMap->getWidth() / 2.0f;
    m_playerY = m_tileMap->getHeight() / 2.0f;

    std::cout << "Generated custom planet: " << m_planetData.name << std::endl;
    std::cout << m_planetData.description << std::endl;
}

void PlanetScene::toggleDisplayMode() {
    // Переключаем на следующий режим отображения
    switch (m_displayMode) {
    case DisplayMode::NORMAL:
        m_displayMode = DisplayMode::TEMPERATURE;
        std::cout << "Display mode: Temperature" << std::endl;
        break;
    case DisplayMode::TEMPERATURE:
        m_displayMode = DisplayMode::HUMIDITY;
        std::cout << "Display mode: Humidity" << std::endl;
        break;
    case DisplayMode::HUMIDITY:
        m_displayMode = DisplayMode::ELEVATION;
        std::cout << "Display mode: Elevation" << std::endl;
        break;
    case DisplayMode::ELEVATION:
        m_displayMode = DisplayMode::RADIATION;
        std::cout << "Display mode: Radiation" << std::endl;
        break;
    case DisplayMode::RADIATION:
        m_displayMode = DisplayMode::RESOURCES;
        std::cout << "Display mode: Resources" << std::endl;
        break;
    case DisplayMode::RESOURCES:
        m_displayMode = DisplayMode::BIOMES;
        std::cout << "Display mode: Biomes" << std::endl;
        break;
    case DisplayMode::BIOMES:
        m_displayMode = DisplayMode::NORMAL;
        std::cout << "Display mode: Normal" << std::endl;
        break;
    default:
        m_displayMode = DisplayMode::NORMAL;
        std::cout << "Display mode: Normal" << std::endl;
    }
}

std::string PlanetScene::getPlanetInfo() const {
    std::ostringstream info;

    info << "Planet: " << m_planetData.name << "\n";
    info << "Temperature: " << std::fixed << std::setprecision(1) << m_planetData.averageTemperature << "°C, ";
    info << "Water coverage: " << std::fixed << std::setprecision(1) << (m_planetData.waterCoverage * 100.0f) << "%, ";
    info << "Gravity: " << std::fixed << std::setprecision(2) << m_planetData.gravityMultiplier << "g, ";
    info << "Atmosphere: " << std::fixed << std::setprecision(2) << m_planetData.atmosphereDensity;

    if (m_planetData.hasLife) {
        info << " | Has life";
    }

    return info.str();
}

std::string PlanetScene::getTileInfo(int x, int y) const {
    if (!m_tileMap || !m_isoRenderer) return "No tile info available";

    // Преобразуем экранные координаты в координаты мира
    float worldX, worldY;
    m_isoRenderer->screenToWorld(x, y, worldX, worldY);

    // Округляем до целых координат тайла
    int tileX = static_cast<int>(worldX);
    int tileY = static_cast<int>(worldY);

    // Проверяем, что координаты в пределах карты
    if (!m_tileMap->isValidCoordinate(tileX, tileY)) {
        return "Outside map bounds";
    }

    // Получаем тайл
    const MapTile* tile = m_tileMap->getTile(tileX, tileY);
    if (!tile) {
        return "Invalid tile";
    }

    // Формируем информацию о тайле
    std::ostringstream info;

    info << "Pos: (" << tileX << ", " << tileY << ")\n";
    info << "Type: " << TileTypeToString(tile->getType()) << "\n";
    info << "Temp: " << std::fixed << std::setprecision(1) << tile->getTemperature() << "°C, ";
    info << "Humidity: " << std::fixed << std::setprecision(2) << tile->getHumidity() << "\n";
    info << "Elevation: " << std::fixed << std::setprecision(2) << tile->getElevation() << ", ";
    info << "Height: " << std::fixed << std::setprecision(2) << tile->getHeight() << "\n";
    info << "Resources: " << std::fixed << std::setprecision(2) << tile->getResourceDensity() << ", ";
    info << "Radiation: " << std::fixed << std::setprecision(2) << tile->getRadiationLevel() << "\n";

    // Добавляем информацию о проходимости и прозрачности
    info << "Walkable: " << (tile->isWalkable() ? "Yes" : "No") << ", ";
    info << "Transparent: " << (tile->isTransparent() ? "Yes" : "No") << "\n";

    // Информация о биоме
    info << "Biome ID: " << tile->getBiomeId();

    // Если есть декорации, отображаем их
    const auto& decorations = tile->getDecorations();
    if (!decorations.empty()) {
        info << "\nDecorations:";
        for (const auto& decor : decorations) {
            info << "\n  - " << decor.name << " (ID: " << decor.id << ", Scale: "
                << std::fixed << std::setprecision(2) << decor.scale
                << (decor.animated ? ", Animated" : "") << ")";
        }
    }

    // Информация об опасности
    if (tile->isHazardous()) {
        info << "\nHAZARD WARNING: This tile is dangerous!";
    }

    return info.str();
}