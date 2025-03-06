#include "TextureManager.h"
#include <algorithm>
#include <sstream>
#include <map>

TextureManager::TextureManager(SDL_Renderer* renderer, size_t memoryCacheLimit)
    : m_renderer(renderer), m_memoryCacheLimit(memoryCacheLimit), m_currentCacheSize(0), m_defaultAtlasId("default") {
    if (!m_renderer) {
        std::cerr << "ERROR: TextureManager initialized with null renderer!" << std::endl;
    }

    // Создаем атлас по умолчанию, если включена поддержка атласов
    if (memoryCacheLimit > 0) {
        createTextureAtlas(m_defaultAtlasId, 1024, 1024);
    }
}

TextureManager::~TextureManager() {
    clearAll();
}


bool TextureManager::loadTexture(const std::string& id, const std::string& filePath,
    int priority, bool useAtlas, const std::string& atlasId) {
    // Проверка существования текстуры с таким id
    if (m_textureInfos.find(id) != m_textureInfos.end()) {
        std::cout << "Texture with id '" << id << "' already exists. Removing old texture." << std::endl;
        removeTexture(id);
    }

    // Если указано использование атласа, загружаем текстуру через атлас
    if (useAtlas) {
        std::string actualAtlasId = atlasId.empty() ? m_defaultAtlasId : atlasId;

        // Проверяем наличие атласа
        TextureAtlas* atlas = getAtlas(actualAtlasId);
        if (!atlas) {
            // Если атлас не найден, но указан не атлас по умолчанию, создаем его
            if (actualAtlasId != m_defaultAtlasId) {
                if (!createTextureAtlas(actualAtlasId, 1024, 1024)) {
                    std::cerr << "ERROR: Could not create atlas '" << actualAtlasId << "'" << std::endl;
                    return false;
                }
                atlas = getAtlas(actualAtlasId);
                if (!atlas) {
                    std::cerr << "ERROR: Atlas '" << actualAtlasId << "' not found after creation." << std::endl;
                    return false;
                }
            }
            else {
                std::cerr << "ERROR: Default atlas not found. Creating it." << std::endl;
                if (!createTextureAtlas(m_defaultAtlasId, 1024, 1024)) {
                    std::cerr << "ERROR: Could not create default atlas." << std::endl;
                    return false;
                }
                atlas = getAtlas(m_defaultAtlasId);
            }
        }

        // Загружаем текстуру непосредственно в атлас
        return loadTextureToAtlas(id, filePath, actualAtlasId);
    }

    // Иначе загружаем текстуру обычным способом (без атласа)

    // 1. Проверка доступности файла
    SDL_RWops* file = SDL_RWFromFile(filePath.c_str(), "rb");
    if (!file) {
        std::cerr << "ERROR: Could not open file '" << filePath << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_RWclose(file);

    // 2. Загрузка изображения
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        std::cerr << "ERROR: Failed to load image '" << filePath << "'. SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // 3. Проверка и настройка формата поверхности для корректного альфа-смешивания
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

    // 4. Вывод диагностической информации
    std::cout << "INFO: Surface dimensions: " << surface->w << "x" << surface->h
        << ", BPP: " << static_cast<int>(surface->format->BitsPerPixel)
        << ", Format: " << (surface->format->Amask ? "with alpha" : "no alpha")
        << std::endl;

    // 5. Создание текстуры из поверхности
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);

    // Запоминаем размеры текстуры для последующего использования
    int width = surface->w;
    int height = surface->h;

    SDL_FreeSurface(surface);  // Освобождаем поверхность

    if (!texture) {
        std::cerr << "ERROR: Failed to create texture from '" << filePath << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // 6. Настройка корректного режима смешивания для текстуры
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // 7. Создаем структуру TextureInfo
    TextureInfo info;
    info.texture = texture;
    info.width = width;
    info.height = height;
    info.priority = priority;
    info.lastUsed = std::chrono::steady_clock::now();
    info.useCount = 1;
    info.memorySize = calculateTextureSize(texture, width, height);
    info.isInAtlas = false;

    // 8. Сохраняем текстуру в хранилище
    m_textureInfos[id] = info;

    // 9. Обновляем текущий размер кэша
    m_currentCacheSize += info.memorySize;

    // 10. Проверяем состояние кэша и выгружаем текстуры при необходимости
    checkCacheState();

    std::cout << "SUCCESS: Texture '" << id << "' loaded successfully from '" << filePath
        << "' (" << width << "x" << height << ", " << info.memorySize / 1024 << " KB)" << std::endl;
    return true;
}

SDL_Texture* TextureManager::getTexture(const std::string& id) {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        std::cerr << "Texture with id '" << id << "' not found!" << std::endl;
        return nullptr;
    }

    // Обновляем время последнего использования
    updateTextureUsage(id);

    // Если текстура находится в атласе, возвращаем текстуру атласа
    if (it->second.isInAtlas) {
        TextureAtlas* atlas = getAtlas(it->second.atlasId);
        if (!atlas) {
            std::cerr << "ERROR: Texture atlas '" << it->second.atlasId << "' not found." << std::endl;
            return nullptr;
        }
        return atlas->getAtlasTexture();
    }

    return it->second.texture;
}

bool TextureManager::hasTexture(const std::string& id) const {
    return m_textureInfos.find(id) != m_textureInfos.end();
}

void TextureManager::removeTexture(const std::string& id) {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        return;  // Текстура уже удалена или не существует
    }

    // Если текстура находится в атласе, просто удаляем информацию о ней
    if (it->second.isInAtlas) {
        // Пока не удаляем субтекстуры из атласа, так как это может быть сложно
        // В будущем можно добавить поддержку уплотнения атласа
        m_textureInfos.erase(it);
        return;
    }

    // Освобождаем ресурсы текстуры
    if (it->second.texture) {
        // Обновляем размер кэша
        m_currentCacheSize -= it->second.memorySize;

        // Удаляем текстуру
        SDL_DestroyTexture(it->second.texture);
        m_textureInfos.erase(it);

        std::cout << "Texture '" << id << "' removed." << std::endl;
    }
}

void TextureManager::clearAll() {
    // Освобождаем все текстуры
    for (auto& pair : m_textureInfos) {
        if (!pair.second.isInAtlas && pair.second.texture) {
            SDL_DestroyTexture(pair.second.texture);
        }
    }
    m_textureInfos.clear();

    // Освобождаем все атласы
    m_atlases.clear();

    // Сбрасываем счетчик размера кэша
    m_currentCacheSize = 0;

    std::cout << "All textures and atlases cleared." << std::endl;
}

bool TextureManager::getTextureSize(const std::string& id, int& width, int& height) const {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        return false;
    }

    const TextureInfo& info = it->second;

    // Если текстура находится в атласе, берем размеры из информации о субтекстуре
    if (info.isInAtlas) {
        if (info.rotated) {
            width = info.height;
            height = info.width;
        }
        else {
            width = info.width;
            height = info.height;
        }
    }
    else {
        // Иначе запрашиваем размеры у текстуры
        if (SDL_QueryTexture(info.texture, nullptr, nullptr, &width, &height) != 0) {
            std::cerr << "ERROR: Failed to query texture size. SDL Error: " << SDL_GetError() << std::endl;
            return false;
        }
    }

    return true;
}

bool TextureManager::setTextureColorMod(const std::string& id, Uint8 r, Uint8 g, Uint8 b) {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        std::cerr << "ERROR: Texture '" << id << "' not found!" << std::endl;
        return false;
    }

    if (SDL_SetTextureColorMod(it->second.texture, r, g, b) != 0) {
        std::cerr << "ERROR: Failed to set color modulation for texture '" << id << "'. SDL Error: "
            << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool TextureManager::setTextureAlphaMod(const std::string& id, Uint8 alpha) {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        std::cerr << "ERROR: Texture '" << id << "' not found!" << std::endl;
        return false;
    }

    if (SDL_SetTextureAlphaMod(it->second.texture, alpha) != 0) {
        std::cerr << "ERROR: Failed to set alpha modulation for texture '" << id << "'. SDL Error: "
            << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool TextureManager::setTextureBlendMode(const std::string& id, SDL_BlendMode blendMode) {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        std::cerr << "ERROR: Texture '" << id << "' not found!" << std::endl;
        return false;
    }

    if (SDL_SetTextureBlendMode(it->second.texture, blendMode) != 0) {
        std::cerr << "ERROR: Failed to set blend mode for texture '" << id << "'. SDL Error: "
            << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

bool TextureManager::debugTextureInfo(const std::string& id) const {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        std::cerr << "DEBUG: Texture '" << id << "' not found!" << std::endl;
        return false;
    }

    const TextureInfo& info = it->second;
    SDL_Texture* texture = info.texture;

    // Получаем детальную информацию о текстуре
    Uint32 format;
    int access, width, height;
    if (SDL_QueryTexture(texture, &format, &access, &width, &height) != 0) {
        std::cerr << "DEBUG: Error getting texture info for '" << id << "': " << SDL_GetError() << std::endl;
        return false;
    }

    // Выводим информацию
    std::cout << "DEBUG: Texture '" << id << "':" << std::endl;
    std::cout << "  - Size: " << width << "x" << height << std::endl;
    std::cout << "  - Memory Size: " << (info.memorySize / 1024) << " KB" << std::endl;
    std::cout << "  - Use Count: " << info.useCount << std::endl;
    std::cout << "  - Priority: " << info.priority << std::endl;

    // Получаем имя формата
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

    // Получаем имя типа доступа
    std::string accessName;
    switch (access) {
    case SDL_TEXTUREACCESS_STATIC: accessName = "STATIC"; break;
    case SDL_TEXTUREACCESS_STREAMING: accessName = "STREAMING"; break;
    case SDL_TEXTUREACCESS_TARGET: accessName = "TARGET"; break;
    default: accessName = "UNKNOWN";
    }
    std::cout << "  - Access type: " << accessName << std::endl;

    // Проверяем режим смешивания
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

    // Проверяем альфа-модуляцию
    Uint8 alpha;
    if (SDL_GetTextureAlphaMod(texture, &alpha) != 0) {
        std::cerr << "DEBUG: Error getting alpha modulation for texture '" << id << "': " << SDL_GetError() << std::endl;
    }
    else {
        std::cout << "  - Alpha modulation: " << static_cast<int>(alpha) << std::endl;
    }

    // Проверяем цветовую модуляцию
    Uint8 r, g, b;
    if (SDL_GetTextureColorMod(texture, &r, &g, &b) != 0) {
        std::cerr << "DEBUG: Error getting color modulation for texture '" << id << "': " << SDL_GetError() << std::endl;
    }
    else {
        std::cout << "  - Color modulation: RGB(" << static_cast<int>(r) << ","
            << static_cast<int>(g) << "," << static_cast<int>(b) << ")" << std::endl;
    }

    return true;
}

void TextureManager::setMemoryCacheLimit(size_t memoryCacheLimit) {
    m_memoryCacheLimit = memoryCacheLimit;

    // Если новый лимит меньше текущего размера кэша, выгружаем текстуры
    checkCacheState();
}

size_t TextureManager::getCurrentCacheSize() const {
    return m_currentCacheSize;
}

size_t TextureManager::getTextureCount() const {
    return m_textureInfos.size();
}

size_t TextureManager::freeCacheMemory(size_t bytesToFree, bool forceRemove) {
    // Если нет необходимости освобождать память, выходим
    if (bytesToFree == 0) {
        return 0;
    }

    // Создаем список текстур, отсортированный по времени последнего использования
    std::vector<std::pair<std::string, TextureInfo*>> textures;
    for (auto& pair : m_textureInfos) {
        // Пропускаем текстуры в атласах, так как их размер учитывается в размере атласа
        if (pair.second.isInAtlas) {
            continue;
        }
        textures.push_back(std::make_pair(pair.first, &pair.second));
    }

    // Сортируем текстуры по времени последнего использования (самые давние - в начале)
    std::sort(textures.begin(), textures.end(),
        [](const std::pair<std::string, TextureInfo*>& a, const std::pair<std::string, TextureInfo*>& b) {
            // Сортировка с учетом приоритета
            if (a.second->priority != b.second->priority) {
                return a.second->priority < b.second->priority;  // Низкий приоритет выгружается первым
            }
            return a.second->lastUsed < b.second->lastUsed;  // Затем по времени последнего использования
        });

    size_t freedBytes = 0;

    // Удаляем текстуры, начиная с самых давно использованных
    for (const auto& pair : textures) {
        // Если текстура имеет высокий приоритет и не требуется принудительное удаление, пропускаем ее
        if (!forceRemove && pair.second->priority > 2) {
            continue;
        }

        // Удаляем текстуру
        std::string textureId = pair.first;
        size_t textureSize = pair.second->memorySize;

        std::cout << "INFO: Freeing texture '" << textureId << "' ("
            << textureSize / 1024 << " KB) due to cache limit." << std::endl;

        // Удаляем текстуру из памяти SDL
        if (pair.second->texture) {
            SDL_DestroyTexture(pair.second->texture);
        }

        // Удаляем текстуру из хранилища
        m_textureInfos.erase(textureId);

        // Обновляем счетчик освобожденной памяти
        freedBytes += textureSize;

        // Если освободили достаточно памяти, выходим
        if (freedBytes >= bytesToFree) {
            break;
        }
    }

    // Обновляем текущий размер кэша
    m_currentCacheSize -= freedBytes;

    return freedBytes;
}

bool TextureManager::setTexturePriority(const std::string& id, int priority) {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        std::cerr << "Texture with id '" << id << "' not found!" << std::endl;
        return false;
    }

    it->second.priority = priority;
    return true;
}

std::string TextureManager::getTexturesReport() const {
    std::stringstream ss;
    ss << "Texture Manager Report\n";
    ss << "---------------------\n";
    ss << "Total textures: " << m_textureInfos.size() << "\n";
    ss << "Current cache size: " << (m_currentCacheSize / 1024) << " KB\n";
    ss << "Cache limit: " << (m_memoryCacheLimit ? std::to_string(m_memoryCacheLimit / 1024) + " KB" : "unlimited") << "\n\n";

    ss << "Atlases:\n";
    if (m_atlases.empty()) {
        ss << "  No texture atlases\n";
    }
    else {
        for (const auto& pair : m_atlases) {
            const TextureAtlas* atlas = pair.second.get();
            ss << "  " << pair.first << ": " << atlas->getSubTextureCount() << " textures, "
                << (atlas->getMemorySize() / 1024) << " KB\n";
        }
    }

    ss << "\nTextures:\n";
    for (const auto& pair : m_textureInfos) {
        const TextureInfo& info = pair.second;
        ss << "  " << pair.first << ": " << info.width << "x" << info.height;

        if (info.isInAtlas) {
            ss << ", in atlas '" << info.atlasId << "'";
            if (info.rotated) {
                ss << " (rotated)";
            }
        }
        else {
            ss << ", " << (info.memorySize / 1024) << " KB";
        }

        ss << ", priority=" << info.priority << ", used=" << info.useCount << " times\n";
    }

    return ss.str();
}

void TextureManager::updateTextureUsage(const std::string& id) {
    auto it = m_textureInfos.find(id);
    if (it != m_textureInfos.end()) {
        it->second.lastUsed = std::chrono::steady_clock::now();
        it->second.useCount++;
    }
}

size_t TextureManager::calculateTextureSize(SDL_Texture* texture, int width, int height) const {
    if (!texture) {
        return 0;
    }

    // Вычисляем размер текстуры в памяти
    Uint32 format;
    int access;
    if (SDL_QueryTexture(texture, &format, &access, nullptr, nullptr) != 0) {
        // В случае ошибки, используем приблизительный расчет
        return static_cast<size_t>(width) * height * 4;  // RGBA: 4 байта на пиксель
    }

    // Определяем количество бит на пиксель для формата текстуры
    int bitsPerPixel;
    switch (format) {
    case SDL_PIXELFORMAT_RGB332:
        bitsPerPixel = 8;
        break;
    case SDL_PIXELFORMAT_RGB444:
    case SDL_PIXELFORMAT_RGB555:
    case SDL_PIXELFORMAT_BGR555:
    case SDL_PIXELFORMAT_ARGB4444:
    case SDL_PIXELFORMAT_RGBA4444:
    case SDL_PIXELFORMAT_ABGR4444:
    case SDL_PIXELFORMAT_BGRA4444:
    case SDL_PIXELFORMAT_ARGB1555:
    case SDL_PIXELFORMAT_RGBA5551:
    case SDL_PIXELFORMAT_ABGR1555:
    case SDL_PIXELFORMAT_BGRA5551:
    case SDL_PIXELFORMAT_RGB565:
    case SDL_PIXELFORMAT_BGR565:
        bitsPerPixel = 16;
        break;
    case SDL_PIXELFORMAT_RGB24:
    case SDL_PIXELFORMAT_BGR24:
        bitsPerPixel = 24;
        break;
    case SDL_PIXELFORMAT_RGB888:
    case SDL_PIXELFORMAT_RGBX8888:
    case SDL_PIXELFORMAT_BGR888:
    case SDL_PIXELFORMAT_BGRX8888:
    case SDL_PIXELFORMAT_ARGB8888:
    case SDL_PIXELFORMAT_RGBA8888:
    case SDL_PIXELFORMAT_ABGR8888:
    case SDL_PIXELFORMAT_BGRA8888:
        bitsPerPixel = 32;
        break;
    default:
        bitsPerPixel = 32;  // По умолчанию предполагаем 32 бита на пиксель
    }

    // Размер текстуры в байтах + небольшой overhead на служебные структуры
    return static_cast<size_t>(width) * height * bitsPerPixel / 8 + 512;
}

void TextureManager::checkCacheState() {
    // Если нет ограничения на размер кэша, выходим
    if (m_memoryCacheLimit == 0) {
        return;
    }

    // Если текущий размер кэша превышает лимит, выгружаем текстуры
    if (m_currentCacheSize > m_memoryCacheLimit) {
        size_t bytesToFree = m_currentCacheSize - m_memoryCacheLimit + m_memoryCacheLimit / 10;  // +10% запас
        freeCacheMemory(bytesToFree);
    }
}

bool TextureManager::createIsometricTexture(const std::string& id, const std::string& newId, int tileWidth, int tileHeight) {
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

    // 11. Создаем информацию о текстуре
    TextureInfo info;
    info.texture = resultTexture;
    info.lastUsed = std::chrono::steady_clock::now();
    info.width = tileWidth;
    info.height = tileHeight;
    info.useCount = 1;
    info.priority = 3; // Средний приоритет для производных текстур
    info.memorySize = calculateTextureSize(resultTexture, tileWidth, tileHeight);

    // 12. Сохраняем и обновляем размер кэша
    m_textureInfos[newId] = info;
    m_currentCacheSize += info.memorySize;

    // 13. Проверяем состояние кэша
    checkCacheState();

    std::cout << "Isometric texture '" << newId << "' created successfully, size: "
        << (info.memorySize / 1024) << " KB" << std::endl;
    return true;
}

bool TextureManager::createIsometricFaceTexture(const std::string& id, const std::string& newId, int faceType, int tileWidth, int tileHeight) {
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

    // 11. Создаем информацию о текстуре
    TextureInfo info;
    info.texture = targetTexture;
    info.lastUsed = std::chrono::steady_clock::now();
    info.width = targetWidth;
    info.height = targetHeight;
    info.useCount = 1;
    info.priority = 3; // Средний приоритет для производных текстур
    info.memorySize = calculateTextureSize(targetTexture, targetWidth, targetHeight);

    // 12. Сохраняем и обновляем размер кэша
    m_textureInfos[newId] = info;
    m_currentCacheSize += info.memorySize;

    // 13. Проверяем состояние кэша
    checkCacheState();

    std::cout << "Isometric face texture '" << newId << "' (type " << faceType << ") created successfully, size: "
        << (info.memorySize / 1024) << " KB" << std::endl;
    return true;
}

bool TextureManager::createTextureAtlas(const std::string& atlasId, int width, int height) {
    // Проверяем, не существует ли уже атлас с таким id
    if (m_atlases.find(atlasId) != m_atlases.end()) {
        std::cerr << "ERROR: Texture atlas with id '" << atlasId << "' already exists." << std::endl;
        return false;
    }

    // Создаем новый атлас
    try {
        auto atlas = std::make_unique<TextureAtlas>(m_renderer, width, height, atlasId);

        // Проверяем, успешно ли создан атлас (через проверку наличия текстуры)
        if (!atlas->getAtlasTexture()) {
            std::cerr << "ERROR: Failed to create texture atlas '" << atlasId << "'." << std::endl;
            return false;
        }

        // Добавляем атлас в хранилище
        m_atlases[atlasId] = std::move(atlas);

        // Обновляем размер кэша
        m_currentCacheSize += m_atlases[atlasId]->getMemorySize();

        std::cout << "Texture atlas '" << atlasId << "' created with size "
            << width << "x" << height << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: Exception creating texture atlas '" << atlasId
            << "': " << e.what() << std::endl;
        return false;
    }
}

TextureAtlas* TextureManager::getAtlas(const std::string& atlasId) {
    auto it = m_atlases.find(atlasId);
    if (it != m_atlases.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::string TextureManager::getDefaultAtlasId() const {
    return m_defaultAtlasId;
}

bool TextureManager::addTextureToAtlas(const std::string& textureId, const std::string& atlasId) {
    // Получаем атлас
    TextureAtlas* atlas = getAtlas(atlasId);
    if (!atlas) {
        std::cerr << "ERROR: Texture atlas '" << atlasId << "' not found." << std::endl;
        return false;
    }

    // Проверяем наличие текстуры
    auto it = m_textureInfos.find(textureId);
    if (it == m_textureInfos.end() || !it->second.texture) {
        std::cerr << "ERROR: Texture '" << textureId << "' not found." << std::endl;
        return false;
    }

    // Проверяем, не находится ли текстура уже в атласе
    if (it->second.isInAtlas) {
        std::cerr << "ERROR: Texture '" << textureId << "' is already in atlas '"
            << it->second.atlasId << "'." << std::endl;
        return false;
    }

    // Добавляем текстуру в атлас
    if (!atlas->addTexture(textureId, it->second.texture)) {
        return false;
    }

    // Получаем прямоугольник текстуры в атласе
    SDL_Rect rect;
    if (!atlas->getTextureRect(textureId, &rect)) {
        std::cerr << "ERROR: Failed to get texture rect for '" << textureId
            << "' in atlas '" << atlasId << "'." << std::endl;
        return false;
    }

    // Обновляем информацию о текстуре
    it->second.isInAtlas = true;
    it->second.atlasId = atlasId;
    it->second.atlasRect = rect;

    // Определяем, была ли текстура повернута в атласе
    // Для этого сравниваем размеры оригинальной текстуры и прямоугольника в атласе
    if ((it->second.width != rect.w) || (it->second.height != rect.h)) {
        it->second.rotated = true;
    }
    else {
        it->second.rotated = false;
    }

    std::cout << "Texture '" << textureId << "' added to atlas '" << atlasId << "'." << std::endl;
    return true;
}

bool TextureManager::loadTextureToAtlas(const std::string& id, const std::string& filePath, const std::string& atlasId) {
    // Получаем атлас
    TextureAtlas* atlas = getAtlas(atlasId);
    if (!atlas) {
        std::cerr << "ERROR: Texture atlas '" << atlasId << "' not found." << std::endl;
        return false;
    }

    // Загружаем текстуру непосредственно в атлас
    if (!atlas->addTextureFromFile(id, filePath)) {
        std::cerr << "ERROR: Failed to add texture '" << id << "' to atlas '" << atlasId << "'." << std::endl;
        return false;
    }

    // Получаем прямоугольник текстуры в атласе
    SDL_Rect rect;
    if (!atlas->getTextureRect(id, &rect)) {
        std::cerr << "ERROR: Failed to get texture rect for '" << id << "' in atlas '" << atlasId << "'." << std::endl;
        return false;
    }

    // Получаем размеры текстуры
    int width = atlas->getSubTextureWidth(id);
    int height = atlas->getSubTextureHeight(id);

    if (width <= 0 || height <= 0) {
        std::cerr << "ERROR: Invalid texture dimensions for '" << id << "' in atlas '" << atlasId << "'." << std::endl;
        return false;
    }

    // Создаем структуру TextureInfo
    TextureInfo info;
    info.texture = nullptr;  // Текстура находится в атласе
    info.width = width;
    info.height = height;
    info.priority = 0;
    info.lastUsed = std::chrono::steady_clock::now();
    info.useCount = 1;
    info.memorySize = 0;  // Размер учитывается в атласе
    info.isInAtlas = true;
    info.atlasId = atlasId;
    info.atlasRect = rect;

    // Определяем, была ли текстура повернута в атласе
    // Для текстур, загруженных напрямую в атлас, нужно специальная проверка
    if ((width != rect.w) || (height != rect.h)) {
        info.rotated = true;
    }

    // Сохраняем информацию о текстуре
    m_textureInfos[id] = info;

    std::cout << "SUCCESS: Texture '" << id << "' loaded into atlas '" << atlasId
        << "' (" << width << "x" << height << ")" << std::endl;
    return true;
}

bool TextureManager::isTextureInAtlas(const std::string& id) const {
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        return false;
    }
    return it->second.isInAtlas;
}

bool TextureManager::loadTextureAtlas(const std::string& atlasId, const std::string& filePath, const std::string& descriptorFilePath) {
    // Проверяем, существует ли уже атлас с таким id
    auto it = m_atlases.find(atlasId);

    if (it != m_atlases.end()) {
        // Если атлас существует, сначала удаляем все связанные с ним текстуры
        std::vector<std::string> texturesToRemove;

        for (const auto& pair : m_textureInfos) {
            if (pair.second.isInAtlas && pair.second.atlasId == atlasId) {
                texturesToRemove.push_back(pair.first);
            }
        }

        for (const auto& id : texturesToRemove) {
            removeTexture(id);
        }

        // Обновляем размер кэша
        m_currentCacheSize -= it->second->getMemorySize();

        // Удаляем сам атлас
        m_atlases.erase(it);
    }

    // Создаем новый атлас
    auto atlas = std::make_unique<TextureAtlas>(m_renderer, 1, 1, atlasId);  // Временные размеры

    // Загружаем атлас из файла
    if (!atlas->loadFromFile(filePath, descriptorFilePath)) {
        std::cerr << "ERROR: Failed to load texture atlas '" << atlasId
            << "' from file '" << filePath << "'." << std::endl;
        return false;
    }

    // Добавляем атлас в хранилище
    m_atlases[atlasId] = std::move(atlas);

    // Обновляем размер кэша
    m_currentCacheSize += m_atlases[atlasId]->getMemorySize();

    // TODO: Загрузка информации о субтекстурах из дескриптора

    std::cout << "Texture atlas '" << atlasId << "' loaded from file '" << filePath << "'." << std::endl;
    return true;
}

bool TextureManager::saveTextureAtlas(const std::string& atlasId, const std::string& filePath, const std::string& descriptorFilePath) {
    // Получаем атлас
    TextureAtlas* atlas = getAtlas(atlasId);
    if (!atlas) {
        std::cerr << "ERROR: Texture atlas '" << atlasId << "' not found." << std::endl;
        return false;
    }

    // Сохраняем атлас в файл
    if (!atlas->saveToFile(filePath)) {
        std::cerr << "ERROR: Failed to save texture atlas '" << atlasId
            << "' to file '" << filePath << "'." << std::endl;
        return false;
    }

    // Сохраняем дескриптор атласа
    if (!atlas->saveDescriptor(descriptorFilePath)) {
        std::cerr << "ERROR: Failed to save texture atlas descriptor '" << atlasId
            << "' to file '" << descriptorFilePath << "'." << std::endl;
        return false;
    }

    std::cout << "Texture atlas '" << atlasId << "' saved to file '" << filePath
        << "' with descriptor '" << descriptorFilePath << "'." << std::endl;
    return true;
}

bool TextureManager::renderTexture(SDL_Renderer* renderer, const std::string& id, const SDL_Rect* dstRect,
    const SDL_Rect* srcRect, double angle, const SDL_Point* center,
    SDL_RendererFlip flip) {
    if (!renderer) {
        std::cerr << "ERROR: Invalid renderer for texture rendering." << std::endl;
        return false;
    }

    // Обновляем счетчик использования текстуры
    updateTextureUsage(id);

    // Проверяем, находится ли текстура в атласе
    auto it = m_textureInfos.find(id);
    if (it == m_textureInfos.end()) {
        std::cerr << "ERROR: Texture '" << id << "' not found." << std::endl;
        return false;
    }

    const TextureInfo& info = it->second;

    if (info.isInAtlas) {
        // Получаем атлас
        TextureAtlas* atlas = getAtlas(info.atlasId);
        if (!atlas) {
            std::cerr << "ERROR: Texture atlas '" << info.atlasId << "' not found." << std::endl;
            return false;
        }

        // Если указан исходный прямоугольник, нужно скорректировать его
        // относительно положения в атласе
        if (srcRect) {
            // Создаем скорректированный прямоугольник
            SDL_Rect adjustedSrcRect = info.atlasRect;

            // Коррекция для повернутых текстур
            if (info.rotated) {
                // Для повернутых текстур нужно по-особому пересчитывать координаты
                adjustedSrcRect.x = info.atlasRect.x + srcRect->y;
                adjustedSrcRect.y = info.atlasRect.y + srcRect->x;
                adjustedSrcRect.w = srcRect->h;
                adjustedSrcRect.h = srcRect->w;
            }
            else {
                // Для обычных текстур просто смещаем координаты
                adjustedSrcRect.x = info.atlasRect.x + srcRect->x;
                adjustedSrcRect.y = info.atlasRect.y + srcRect->y;
                adjustedSrcRect.w = srcRect->w;
                adjustedSrcRect.h = srcRect->h;
            }

            // Проверяем, не выходит ли прямоугольник за границы атласа
            if (adjustedSrcRect.x + adjustedSrcRect.w > info.atlasRect.x + info.atlasRect.w ||
                adjustedSrcRect.y + adjustedSrcRect.h > info.atlasRect.y + info.atlasRect.h) {
                std::cerr << "ERROR: Source rectangle out of bounds for texture '" << id << "'." << std::endl;
                return false;
            }

            // Отрисовываем текстуру из атласа с использованием скорректированного прямоугольника
            double finalAngle = angle;
            if (info.rotated) {
                finalAngle += 90.0;  // Дополнительный поворот для повернутых текстур
            }

            return SDL_RenderCopyEx(renderer, atlas->getAtlasTexture(), &adjustedSrcRect, dstRect,
                finalAngle, center, flip) == 0;
        }
        else {
            // Если исходный прямоугольник не указан, просто отрисовываем всю текстуру из атласа
            return atlas->renderSubTexture(renderer, id, dstRect, angle, center, flip);
        }
    }
    else {
        // Обычная отрисовка текстуры (не из атласа)
        if (!info.texture) {
            std::cerr << "ERROR: Invalid texture for '" << id << "'." << std::endl;
            return false;
        }

        return SDL_RenderCopyEx(renderer, info.texture, srcRect, dstRect, angle, center, flip) == 0;
    }
}



















