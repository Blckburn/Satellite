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