#include "ResourceManager.h"

ResourceManager::ResourceManager(SDL_Renderer* renderer)
    : m_renderer(renderer) {
}

ResourceManager::~ResourceManager() {
    clearAll();
}

bool ResourceManager::loadTexture(const std::string& id, const std::string& filePath) {
    // 1. Проверка существования текстуры с таким id
    if (m_textures.find(id) != m_textures.end()) {
        std::cout << "Texture with id '" << id << "' already exists. Removing old texture." << std::endl;
        removeTexture(id);
    }

    // 2. Проверка доступности файла
    SDL_RWops* file = SDL_RWFromFile(filePath.c_str(), "rb");
    if (!file) {
        std::cerr << "ERROR: Could not open file '" << filePath << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_RWclose(file);

    // 3. Загрузка изображения с дополнительной диагностикой
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        std::cerr << "ERROR: Failed to load image '" << filePath << "'. SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // 4. Проверка и настройка формата поверхности для корректного альфа-смешивания
    if (surface->format->Amask == 0) {
        std::cout << "INFO: Image '" << filePath << "' doesn't have alpha channel, converting..." << std::endl;
        SDL_Surface* optimizedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
        if (optimizedSurface) {
            SDL_FreeSurface(surface);
            surface = optimizedSurface;
        }
        else {
            std::cerr << "WARNING: Failed to convert surface format. SDL Error: " << SDL_GetError() << std::endl;
        }
    }

    // 5. Создание текстуры из поверхности с проверкой результата
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);

    // 6. Вывод диагностической информации
    std::cout << "INFO: Surface dimensions: " << surface->w << "x" << surface->h
        << ", BPP: " << static_cast<int>(surface->format->BitsPerPixel)
        << ", Format: " << (surface->format->Amask ? "with alpha" : "no alpha")
        << std::endl;

    SDL_FreeSurface(surface);  // Освобождаем поверхность, она больше не нужна

    if (!texture) {
        std::cerr << "ERROR: Failed to create texture from '" << filePath << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 7. Настройка корректного режима смешивания для текстуры
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // 8. Сохранение текстуры в хранилище
    m_textures[id] = texture;

    std::cout << "SUCCESS: Texture '" << id << "' loaded successfully from '" << filePath << "'" << std::endl;
    return true;
}

SDL_Texture* ResourceManager::getTexture(const std::string& id) const {
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        return it->second;
    }

    std::cerr << "Texture with id '" << id << "' not found!" << std::endl;
    return nullptr;
}

bool ResourceManager::hasTexture(const std::string& id) const {
    return m_textures.find(id) != m_textures.end();
}

void ResourceManager::removeTexture(const std::string& id) {
    auto it = m_textures.find(id);
    if (it != m_textures.end()) {
        SDL_DestroyTexture(it->second);
        m_textures.erase(it);
        std::cout << "Texture '" << id << "' removed." << std::endl;
    }
}

void ResourceManager::clearAll() {
    // Уничтожаем все текстуры
    for (auto& pair : m_textures) {
        SDL_DestroyTexture(pair.second);
    }
    m_textures.clear();

    // Уничтожаем все шрифты
    for (auto& pair : m_fonts) {
        TTF_CloseFont(pair.second);
    }
    m_fonts.clear();

    std::cout << "All resources cleared." << std::endl;
}

bool ResourceManager::getTextureSize(const std::string& id, int& width, int& height) const {
    SDL_Texture* texture = getTexture(id);
    if (!texture) {
        return false;
    }

    // Получаем размеры текстуры
    SDL_QueryTexture(texture, nullptr, nullptr, &width, &height);
    return true;
}

bool ResourceManager::createIsometricTexture(const std::string& id, const std::string& newId, int tileWidth, int tileHeight) {
    // 1. Получаем исходную текстуру
    SDL_Texture* sourceTexture = getTexture(id);
    if (!sourceTexture) {
        std::cerr << "Source texture '" << id << "' not found!" << std::endl;
        return false;
    }

    // 2. Создаем новую текстуру для изометрического тайла
    SDL_Texture* resultTexture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        tileWidth,
        tileHeight
    );

    if (!resultTexture) {
        std::cerr << "Failed to create result texture: " << SDL_GetError() << std::endl;
        return false;
    }

    // 3. Сохраняем текущую цель рендеринга
    SDL_Texture* currentTarget = SDL_GetRenderTarget(m_renderer);
    SDL_SetRenderTarget(m_renderer, resultTexture);

    // 4. Заполняем фоновым цветом (как фон сцены)
    SDL_SetRenderDrawColor(m_renderer, 20, 35, 20, 255);
    SDL_RenderClear(m_renderer);

    // 5. Получаем размеры исходной текстуры
    int sourceWidth, sourceHeight;
    SDL_QueryTexture(sourceTexture, nullptr, nullptr, &sourceWidth, &sourceHeight);

    // 6. Размещаем текстуру внутри ромба
    // Вычисляем размер для сохранения пропорций
    float aspectRatio = static_cast<float>(sourceWidth) / sourceHeight;

    // Максимальный размер, который поместится в ромб
    // Учитываем, что ромб занимает примерно 70% ширины и высоты прямоугольника
    int maxWidth = static_cast<int>(tileWidth * 0.7f);
    int maxHeight = static_cast<int>(tileHeight * 0.7f);

    // Вычисляем размер с сохранением пропорций
    int destWidth, destHeight;
    if (maxWidth / aspectRatio <= maxHeight) {
        destWidth = maxWidth;
        destHeight = static_cast<int>(maxWidth / aspectRatio);
    }
    else {
        destHeight = maxHeight;
        destWidth = static_cast<int>(maxHeight * aspectRatio);
    }

    // Центрируем в тайле
    SDL_Rect destRect = {
        (tileWidth - destWidth) / 2,
        (tileHeight - destHeight) / 2,
        destWidth,
        destHeight
    };

    // 7. Копируем исходную текстуру
    SDL_RenderCopy(m_renderer, sourceTexture, NULL, &destRect);

    // 8. Рисуем ромб вокруг текстуры для обозначения границ
    SDL_Point points[5];
    points[0] = { tileWidth / 2, 0 };                    // Верхняя вершина
    points[1] = { tileWidth, tileHeight / 2 };           // Правая вершина
    points[2] = { tileWidth / 2, tileHeight };           // Нижняя вершина
    points[3] = { 0, tileHeight / 2 };                   // Левая вершина
    points[4] = { tileWidth / 2, 0 };                    // Замыкаем контур

    // Рисуем тонкий контур ромба
    SDL_SetRenderDrawColor(m_renderer, 20, 40, 20, 100);
    SDL_RenderDrawLines(m_renderer, points, 5);

    // 9. Восстанавливаем исходную цель рендеринга
    SDL_SetRenderTarget(m_renderer, currentTarget);

    // 10. Настраиваем результирующую текстуру
    SDL_SetTextureBlendMode(resultTexture, SDL_BLENDMODE_NONE);
    m_textures[newId] = resultTexture;

    std::cout << "Isometric texture '" << newId << "' created successfully" << std::endl;
    return true;
}


bool ResourceManager::createIsometricFaceTexture(const std::string& id, const std::string& newId, int faceType, int tileWidth, int tileHeight) {
    // 1. Получаем исходную текстуру
    SDL_Texture* sourceTexture = getTexture(id);
    if (!sourceTexture) {
        std::cerr << "Source texture '" << id << "' not found!" << std::endl;
        return false;
    }

    // 2. Определяем размеры для грани в зависимости от типа
    int targetWidth, targetHeight;

    switch (faceType) {
    case 0: // Верхняя грань (ромб)
        targetWidth = tileWidth;
        targetHeight = tileHeight;
        break;
    case 1: // Левая грань (четырехугольник)
    case 2: // Правая грань (четырехугольник)
        targetWidth = tileWidth / 2;
        targetHeight = tileHeight;
        break;
    default:
        std::cerr << "Invalid face type: " << faceType << std::endl;
        return false;
    }

    // 3. Проверяем возможности рендерера для создания текстуры-цели
    SDL_RendererInfo rendererInfo;
    if (SDL_GetRendererInfo(m_renderer, &rendererInfo) != 0) {
        std::cerr << "Failed to get renderer info: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(rendererInfo.flags & SDL_RENDERER_TARGETTEXTURE)) {
        std::cerr << "Renderer does not support render-to-texture! Cannot create isometric textures." << std::endl;
        return false;
    }

    // 4. Получаем размеры исходной текстуры
    int sourceWidth, sourceHeight;
    SDL_QueryTexture(sourceTexture, nullptr, nullptr, &sourceWidth, &sourceHeight);

    // 5. Создаем новую текстуру для грани с поддержкой альфа-канала
    SDL_Texture* targetTexture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        targetWidth,
        targetHeight
    );

    if (!targetTexture) {
        std::cerr << "Failed to create target texture '" << newId << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 6. Настраиваем альфа-смешивание и сохраняем текущую цель рендеринга
    SDL_SetTextureBlendMode(targetTexture, SDL_BLENDMODE_BLEND);
    SDL_Texture* currentTarget = SDL_GetRenderTarget(m_renderer);
    SDL_SetRenderTarget(m_renderer, targetTexture);

    // 7. Очищаем текстуру и делаем ее полностью прозрачной
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_renderer);

    // 8. Рисуем исходную текстуру на целевую
    SDL_Rect destRect = { 0, 0, targetWidth, targetHeight };
    SDL_RenderCopy(m_renderer, sourceTexture, nullptr, &destRect);

    // 9. Для боковых граней добавляем затемнение
    if (faceType > 0) {
        // Более контрастное затемнение для боковых граней
        int alpha = (faceType == 1) ? 100 : 140;  // Левая грань светлее, правая темнее

        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, alpha);
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderFillRect(m_renderer, &destRect);

        // Добавляем градиент для дополнительного эффекта глубины
        for (int y = 0; y < targetHeight; ++y) {
            int additionalAlpha = static_cast<int>(y * 35 / targetHeight);
            SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, additionalAlpha);
            SDL_RenderDrawLine(m_renderer, 0, y, targetWidth, y);
        }
    }

    // 10. Восстанавливаем исходную цель рендеринга
    SDL_SetRenderTarget(m_renderer, currentTarget);

    // 11. Сохраняем новую текстуру в менеджере ресурсов
    m_textures[newId] = targetTexture;

    std::cout << "Isometric face texture '" << newId << "' (type " << faceType << ") created successfully" << std::endl;
    return true;
}

bool ResourceManager::debugTextureInfo(const std::string& id) {
    SDL_Texture* texture = getTexture(id);
    if (!texture) {
        std::cerr << "DEBUG: Texture '" << id << "' not found!" << std::endl;
        return false;
    }

    // Get detailed information about the texture
    Uint32 format;
    int access, width, height;
    if (SDL_QueryTexture(texture, &format, &access, &width, &height) != 0) {
        std::cerr << "DEBUG: Error getting texture info for '" << id << "': " << SDL_GetError() << std::endl;
        return false;
    }

    // Output information
    std::cout << "DEBUG: Texture '" << id << "':" << std::endl;
    std::cout << "  - Size: " << width << "x" << height << std::endl;

    // Get format name
    std::string formatName;
    switch (format) {
    case SDL_PIXELFORMAT_UNKNOWN: formatName = "UNKNOWN"; break;
    case SDL_PIXELFORMAT_INDEX1LSB: formatName = "INDEX1LSB"; break;
    case SDL_PIXELFORMAT_INDEX1MSB: formatName = "INDEX1MSB"; break;
    case SDL_PIXELFORMAT_INDEX4LSB: formatName = "INDEX4LSB"; break;
    case SDL_PIXELFORMAT_INDEX4MSB: formatName = "INDEX4MSB"; break;
    case SDL_PIXELFORMAT_INDEX8: formatName = "INDEX8"; break;
    case SDL_PIXELFORMAT_RGB332: formatName = "RGB332"; break;
    case SDL_PIXELFORMAT_RGB444: formatName = "RGB444"; break;
    case SDL_PIXELFORMAT_RGB555: formatName = "RGB555"; break;
    case SDL_PIXELFORMAT_BGR555: formatName = "BGR555"; break;
    case SDL_PIXELFORMAT_ARGB4444: formatName = "ARGB4444"; break;
    case SDL_PIXELFORMAT_RGBA4444: formatName = "RGBA4444"; break;
    case SDL_PIXELFORMAT_ABGR4444: formatName = "ABGR4444"; break;
    case SDL_PIXELFORMAT_BGRA4444: formatName = "BGRA4444"; break;
    case SDL_PIXELFORMAT_ARGB1555: formatName = "ARGB1555"; break;
    case SDL_PIXELFORMAT_RGBA5551: formatName = "RGBA5551"; break;
    case SDL_PIXELFORMAT_ABGR1555: formatName = "ABGR1555"; break;
    case SDL_PIXELFORMAT_BGRA5551: formatName = "BGRA5551"; break;
    case SDL_PIXELFORMAT_RGB565: formatName = "RGB565"; break;
    case SDL_PIXELFORMAT_BGR565: formatName = "BGR565"; break;
    case SDL_PIXELFORMAT_RGB24: formatName = "RGB24"; break;
    case SDL_PIXELFORMAT_BGR24: formatName = "BGR24"; break;
    case SDL_PIXELFORMAT_RGB888: formatName = "RGB888"; break;
    case SDL_PIXELFORMAT_RGBX8888: formatName = "RGBX8888"; break;
    case SDL_PIXELFORMAT_BGR888: formatName = "BGR888"; break;
    case SDL_PIXELFORMAT_BGRX8888: formatName = "BGRX8888"; break;
    case SDL_PIXELFORMAT_ARGB8888: formatName = "ARGB8888"; break;
    case SDL_PIXELFORMAT_RGBA8888: formatName = "RGBA8888"; break;
    case SDL_PIXELFORMAT_ABGR8888: formatName = "ABGR8888"; break;
    case SDL_PIXELFORMAT_BGRA8888: formatName = "BGRA8888"; break;
    default: formatName = "OTHER (value: " + std::to_string(format) + ")";
    }
    std::cout << "  - Format: " << formatName << std::endl;

    // Get access type name
    std::string accessName;
    switch (access) {
    case SDL_TEXTUREACCESS_STATIC: accessName = "STATIC"; break;
    case SDL_TEXTUREACCESS_STREAMING: accessName = "STREAMING"; break;
    case SDL_TEXTUREACCESS_TARGET: accessName = "TARGET"; break;
    default: accessName = "UNKNOWN";
    }
    std::cout << "  - Access type: " << accessName << std::endl;

    // Check blend mode
    SDL_BlendMode blendMode;
    if (SDL_GetTextureBlendMode(texture, &blendMode) != 0) {
        std::cerr << "DEBUG: Error getting blend mode for texture '" << id << "': " << SDL_GetError() << std::endl;
    }
    else {
        std::string blendName;
        switch (blendMode) {
        case SDL_BLENDMODE_NONE: blendName = "NONE"; break;
        case SDL_BLENDMODE_BLEND: blendName = "BLEND"; break;
        case SDL_BLENDMODE_ADD: blendName = "ADD"; break;
        case SDL_BLENDMODE_MOD: blendName = "MOD"; break;
        default: blendName = "UNKNOWN";
        }
        std::cout << "  - Blend mode: " << blendName << std::endl;
    }

    // Check alpha modulation
    Uint8 alpha;
    if (SDL_GetTextureAlphaMod(texture, &alpha) != 0) {
        std::cerr << "DEBUG: Error getting alpha modulation for texture '" << id << "': " << SDL_GetError() << std::endl;
    }
    else {
        std::cout << "  - Alpha modulation: " << static_cast<int>(alpha) << std::endl;
    }

    // Check color modulation
    Uint8 r, g, b;
    if (SDL_GetTextureColorMod(texture, &r, &g, &b) != 0) {
        std::cerr << "DEBUG: Error getting color modulation for texture '" << id << "': " << SDL_GetError() << std::endl;
    }
    else {
        std::cout << "  - Color modulation: RGB(" << static_cast<int>(r) << "," << static_cast<int>(g) << "," << static_cast<int>(b) << ")" << std::endl;
    }



    return true;
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