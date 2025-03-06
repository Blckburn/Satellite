#include "ResourceManager.h"


ResourceManager::ResourceManager(SDL_Renderer* renderer)
    : m_renderer(renderer) {
    m_textureManager = std::make_unique<TextureManager>(renderer);
}

ResourceManager::~ResourceManager() {
    clearAll();
}

void ResourceManager::clearAll() {
    // Очищаем TextureManager
    if (m_textureManager) {
        m_textureManager->clearAll();
    }

    // Уничтожаем все шрифты
    for (auto& pair : m_fonts) {
        TTF_CloseFont(pair.second);
    }
    m_fonts.clear();

    std::cout << "All resources cleared." << std::endl;
}

bool ResourceManager::loadFont(const std::string& id, const std::string& filePath, int fontSize) {
    // Проверка существования шрифта с таким id
    if (m_fonts.find(id) != m_fonts.end()) {
        std::cout << "Font with id '" << id << "' already exists. Removing old font." << std::endl;
        removeFont(id);
    }

    // Проверка доступности файла
    SDL_RWops* file = SDL_RWFromFile(filePath.c_str(), "rb");
    if (!file) {
        std::cerr << "ERROR: Could not open font file '" << filePath << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_RWclose(file);

    // Загрузка шрифта
    TTF_Font* font = TTF_OpenFont(filePath.c_str(), fontSize);
    if (!font) {
        std::cerr << "ERROR: Failed to load font '" << filePath << "'. SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Сохранение шрифта в хранилище
    m_fonts[id] = font;

    std::cout << "SUCCESS: Font '" << id << "' loaded successfully from '" << filePath << "' with size " << fontSize << std::endl;
    return true;
}

TTF_Font* ResourceManager::getFont(const std::string& id) const {
    auto it = m_fonts.find(id);
    if (it != m_fonts.end()) {
        return it->second;
    }

    std::cerr << "Font with id '" << id << "' not found!" << std::endl;
    return nullptr;
}

bool ResourceManager::hasFont(const std::string& id) const {
    return m_fonts.find(id) != m_fonts.end();
}

void ResourceManager::removeFont(const std::string& id) {
    auto it = m_fonts.find(id);
    if (it != m_fonts.end()) {
        TTF_CloseFont(it->second);
        m_fonts.erase(it);
        std::cout << "Font '" << id << "' removed." << std::endl;
    }
}

SDL_Texture* ResourceManager::createTextTexture(const std::string& text, const std::string& fontId, SDL_Color color) {
    // Получаем шрифт
    TTF_Font* font = getFont(fontId);
    if (!font) {
        std::cerr << "ERROR: Could not create text texture. Font '" << fontId << "' not found." << std::endl;
        return nullptr;
    }

    // Создаем поверхность с текстом
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "ERROR: Could not create text surface. SDL_ttf Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    // Создаем текстуру из поверхности
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);  // Освобождаем поверхность

    if (!texture) {
        std::cerr << "ERROR: Could not create texture from text surface. SDL Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    return texture;
}

void ResourceManager::renderText(SDL_Renderer* renderer, const std::string& text, const std::string& fontId,
    int x, int y, SDL_Color color) {
    if (text.empty()) {
        return;  // Нечего отображать
    }

    // Получаем размеры окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Создаем текстуру с текстом
    SDL_Texture* texture = createTextTexture(text, fontId, color);
    if (!texture) {
        return;  // Не удалось создать текстуру
    }

    // Получаем размеры текстуры
    int textureWidth, textureHeight;
    SDL_QueryTexture(texture, nullptr, nullptr, &textureWidth, &textureHeight);

    // Проверяем, не выходит ли текст за пределы экрана
    int maxDisplayWidth = windowWidth - 60; // Отступы по 30 пикселей с каждой стороны

    if (textureWidth > maxDisplayWidth) {
        // Если текст слишком широкий, масштабируем его по ширине
        textureHeight = static_cast<int>((float)textureHeight * ((float)maxDisplayWidth / textureWidth));
        textureWidth = maxDisplayWidth;
    }

    // Настройка целевого прямоугольника для отображения
    // Центрируем текст по X и Y координатам
    SDL_Rect dstRect = {
        x - textureWidth / 2,
        y - textureHeight / 2,
        textureWidth,
        textureHeight
    };

    // Отрисовка текстуры с текстом
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);

    // Освобождение созданной текстуры
    SDL_DestroyTexture(texture);
}

SDL_Renderer* ResourceManager::getRenderer() const {
    return m_renderer;
}

TextureManager* ResourceManager::getTextureManager() const {
    return m_textureManager.get();
}

bool ResourceManager::loadTileTextures() {
    if (!m_textureManager) {
        std::cerr << "ERROR: TextureManager is not available for loading tile textures!" << std::endl;
        return false;
    }

    // Выводим информацию о загрузке
    std::cout << "Loading tile textures from 'assets/textures/' directory..." << std::endl;

    // Будем отслеживать, какие текстуры загрузились успешно
    bool allSuccessful = true;
    std::vector<std::pair<std::string, std::string>> texturesToLoad = {
        {"tile_floor", "assets/textures/tile_floor.png"},
        {"tile_grass", "assets/textures/tile_grass.png"},
        {"tile_ice", "assets/textures/tile_ice.png"},
        {"tile_wall", "assets/textures/tile_wall.png"},
        {"tile_water", "assets/textures/tile_water.png"}
    };

    for (const auto& texPair : texturesToLoad) {
        const std::string& texId = texPair.first;
        const std::string& texPath = texPair.second;

        // Проверяем файл
        SDL_RWops* file = SDL_RWFromFile(texPath.c_str(), "rb");
        if (!file) {
            std::cerr << "ERROR: Could not open texture file '" << texPath
                << "'. SDL Error: " << SDL_GetError() << std::endl;
            allSuccessful = false;
            continue;
        }
        SDL_RWclose(file);

        // Пытаемся загрузить текстуру
        try {
            bool success = m_textureManager->loadTexture(texId, texPath, 2);
            if (success) {
                std::cout << "Successfully loaded texture '" << texId
                    << "' from file '" << texPath << "'" << std::endl;
            }
            else {
                std::cerr << "ERROR: Failed to load texture '" << texId
                    << "' from file '" << texPath << "'" << std::endl;
                allSuccessful = false;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "EXCEPTION loading texture '" << texId
                << "': " << e.what() << std::endl;
            allSuccessful = false;
        }
    }

    // Дополнительно выводим информацию о результате
    if (allSuccessful) {
        std::cout << "All tile textures loaded successfully!" << std::endl;
    }
    else {
        std::cerr << "WARNING: Some tile textures failed to load!" << std::endl;
    }

    return allSuccessful;
}

SDL_Texture* ResourceManager::getTileTexture(TileType type, int biomeType) const {
    if (!m_textureManager) {
        return nullptr;
    }

    // Определяем имя текстуры в зависимости от типа тайла и биома
    std::string textureName;

    switch (type) {
    case TileType::FLOOR:
        switch (biomeType) {
        case 1: // FOREST
            textureName = "tile_grass";
            break;
        case 3: // TUNDRA
            textureName = "tile_ice";
            break;
        default:
            textureName = "tile_floor";
            break;
        }
        break;
    case TileType::GRASS:
        textureName = "tile_grass";
        break;
    case TileType::WALL:
    case TileType::OBSTACLE:
    case TileType::ROCK_FORMATION:
        textureName = "tile_wall";
        break;
    case TileType::WATER:
        textureName = "tile_water";
        break;
    case TileType::ICE:
    case TileType::SNOW:
        textureName = "tile_ice";
        break;
        // Для других типов тайлов можно добавить соответствующие текстуры
    default:
        // Для неизвестных типов возвращаем nullptr
        return nullptr;
    }

    // Проверяем наличие текстуры перед возвратом
    if (!m_textureManager->hasTexture(textureName)) {
        return nullptr;
    }

    // Получаем текстуру по имени
    return m_textureManager->getTexture(textureName);
}
