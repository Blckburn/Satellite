#include "TextureAtlas.h"
#include <algorithm>
#include <fstream>
#include <sstream>

TextureAtlas::TextureAtlas(SDL_Renderer* renderer, int width, int height, const std::string& id)
    : m_renderer(renderer), m_width(width), m_height(height), m_id(id), m_memorySize(0) {
    // Создаем текстуру атласа
    m_atlasTexture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        width,
        height
    );

    if (!m_atlasTexture) {
        std::cerr << "ERROR: Failed to create atlas texture '" << id << "'. SDL Error: "
            << SDL_GetError() << std::endl;
        return;
    }

    // Настраиваем режим смешивания для альфа-канала
    SDL_SetTextureBlendMode(m_atlasTexture, SDL_BLENDMODE_BLEND);

    // Инициализируем размер в памяти
    m_memorySize = calculateAtlasSize();

    // Инициализируем свободное пространство в атласе (весь атлас изначально свободен)
    SDL_Rect initialRect = { 0, 0, width, height };
    m_freeRects.push_back(initialRect);

    std::cout << "Texture atlas '" << id << "' created with size " << width << "x" << height << std::endl;
}

TextureAtlas::~TextureAtlas() {
    if (m_atlasTexture) {
        SDL_DestroyTexture(m_atlasTexture);
        m_atlasTexture = nullptr;
    }
}

bool TextureAtlas::addTexture(const std::string& id, SDL_Texture* texture) {
    if (!texture || !m_atlasTexture) {
        std::cerr << "ERROR: Invalid texture or atlas texture!" << std::endl;
        return false;
    }

    // Проверяем, нет ли уже субтекстуры с таким id
    if (m_subTextures.find(id) != m_subTextures.end()) {
        std::cerr << "ERROR: SubTexture with id '" << id << "' already exists in atlas '" << m_id << "'!" << std::endl;
        return false;
    }

    // Получаем размеры текстуры
    int width, height;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) != 0) {
        std::cerr << "ERROR: Failed to query texture size. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Если текстура больше размеров атласа, сразу выходим
    if (width > m_width || height > m_height) {
        std::cerr << "ERROR: Texture '" << id << "' is too large for atlas '" << m_id
            << "' (" << width << "x" << height << " vs " << m_width << "x" << m_height << ")" << std::endl;
        return false;
    }

    // Находим свободное место в атласе
    SDL_Rect rect;
    bool rotated = false;
    if (!findFreeSpace(width, height, &rect, true)) {
        std::cerr << "ERROR: Not enough space in atlas '" << m_id << "' for texture '" << id
            << "' (" << width << "x" << height << ")" << std::endl;
        return false;
    }

    // Определяем, была ли текстура повернута
    if (rect.w != width || rect.h != height) {
        rotated = true;
    }

    // Сохраняем текущую цель рендеринга
    SDL_Texture* currentTarget = SDL_GetRenderTarget(m_renderer);

    // Устанавливаем атлас как цель рендеринга
    SDL_SetRenderTarget(m_renderer, m_atlasTexture);

    // Копируем текстуру в атлас
    if (!rotated) {
        // Обычное копирование
        SDL_Rect srcRect = { 0, 0, width, height };
        SDL_RenderCopy(m_renderer, texture, &srcRect, &rect);
    }
    else {
        // Для повернутой текстуры нужно использовать SDL_RenderCopyEx с поворотом на 90 градусов
        SDL_Rect srcRect = { 0, 0, width, height };
        SDL_RenderCopyEx(m_renderer, texture, &srcRect, &rect, 90.0, nullptr, SDL_FLIP_NONE);
    }

    // Восстанавливаем предыдущую цель рендеринга
    SDL_SetRenderTarget(m_renderer, currentTarget);

    // Создаем информацию о субтекстуре
    SDL_Rect sourceRect = { 0, 0, width, height };
    SubTexture subTexture(id, rect, sourceRect, rotated);

    // Добавляем в хранилище субтекстур
    m_subTextures[id] = subTexture;

    std::cout << "Texture '" << id << "' added to atlas '" << m_id
        << "' at position (" << rect.x << "," << rect.y << ") with size "
        << rect.w << "x" << rect.h << (rotated ? " (rotated)" : "") << std::endl;

    return true;
}

bool TextureAtlas::addTextureFromFile(const std::string& id, const std::string& filePath) {
    // Проверяем доступность файла
    SDL_RWops* file = SDL_RWFromFile(filePath.c_str(), "rb");
    if (!file) {
        std::cerr << "ERROR: Could not open file '" << filePath << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_RWclose(file);

    // Загружаем изображение
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        std::cerr << "ERROR: Failed to load image '" << filePath << "'. SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // Проверяем и настраиваем формат поверхности для корректного альфа-смешивания
    if (surface->format->Amask == 0) {
        SDL_Surface* optimizedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
        if (optimizedSurface) {
            SDL_FreeSurface(surface);
            surface = optimizedSurface;
        }
    }

    // Создаем текстуру из поверхности
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);  // Освобождаем поверхность

    if (!texture) {
        std::cerr << "ERROR: Failed to create texture from '" << filePath << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Добавляем текстуру в атлас
    bool result = addTexture(id, texture);

    // Освобождаем временную текстуру
    SDL_DestroyTexture(texture);

    return result;
}

bool TextureAtlas::getTextureRect(const std::string& id, SDL_Rect* rect) const {
    auto it = m_subTextures.find(id);
    if (it == m_subTextures.end()) {
        return false;
    }

    if (rect) {
        *rect = it->second.rect;
    }

    return true;
}

bool TextureAtlas::hasTexture(const std::string& id) const {
    return m_subTextures.find(id) != m_subTextures.end();
}

SDL_Texture* TextureAtlas::getAtlasTexture() const {
    return m_atlasTexture;
}

bool TextureAtlas::renderSubTexture(SDL_Renderer* renderer, const std::string& id, const SDL_Rect* dstRect,
    double angle, const SDL_Point* center, SDL_RendererFlip flip) {
    if (!renderer || !m_atlasTexture) {
        return false;
    }

    auto it = m_subTextures.find(id);
    if (it == m_subTextures.end()) {
        std::cerr << "ERROR: SubTexture '" << id << "' not found in atlas '" << m_id << "'!" << std::endl;
        return false;
    }

    const SubTexture& subTexture = it->second;

    // Если субтекстура повернута, нужно учесть это при отрисовке
    if (subTexture.rotated) {
        // Для повернутых текстур применяем дополнительный поворот
        // и настраиваем прямоугольник источника соответствующим образом
        SDL_Rect srcRect = subTexture.rect;
        double finalAngle = angle + 90.0;  // Добавляем 90 градусов к углу поворота

        // При повороте меняем местами ширину и высоту
        if (dstRect) {
            SDL_Rect adjustedDstRect = *dstRect;

            // Если текстура была повернута при добавлении в атлас,
            // нужно учитывать это при отрисовке
            if (subTexture.rotated && (angle == 0.0) && !center) {
                // Если пользователь не задает угол и центр поворота,
                // то мы можем просто поменять местами ширину и высоту
                adjustedDstRect.w = dstRect->h;
                adjustedDstRect.h = dstRect->w;
            }

            return SDL_RenderCopyEx(renderer, m_atlasTexture, &srcRect, &adjustedDstRect,
                finalAngle, center, flip) == 0;
        }
        else {
            return SDL_RenderCopyEx(renderer, m_atlasTexture, &srcRect, nullptr,
                finalAngle, center, flip) == 0;
        }
    }
    else {
        // Для обычных текстур просто копируем соответствующую часть атласа
        return SDL_RenderCopyEx(renderer, m_atlasTexture, &subTexture.rect, dstRect,
            angle, center, flip) == 0;
    }
}

int TextureAtlas::getSubTextureWidth(const std::string& id) const {
    auto it = m_subTextures.find(id);
    if (it == m_subTextures.end()) {
        return -1;
    }

    // Если текстура повернута, возвращаем высоту rect (т.к. ширина и высота меняются местами)
    if (it->second.rotated) {
        return it->second.rect.h;
    }

    return it->second.rect.w;
}

int TextureAtlas::getSubTextureHeight(const std::string& id) const {
    auto it = m_subTextures.find(id);
    if (it == m_subTextures.end()) {
        return -1;
    }

    // Если текстура повернута, возвращаем ширину rect (т.к. ширина и высота меняются местами)
    if (it->second.rotated) {
        return it->second.rect.w;
    }

    return it->second.rect.h;
}

const std::string& TextureAtlas::getId() const {
    return m_id;
}

size_t TextureAtlas::getMemorySize() const {
    return m_memorySize;
}

size_t TextureAtlas::getSubTextureCount() const {
    return m_subTextures.size();
}

bool TextureAtlas::saveToFile(const std::string& filePath) {
    // Проверяем корректность атласа
    if (!m_atlasTexture) {
        std::cerr << "ERROR: Atlas texture is invalid!" << std::endl;
        return false;
    }

    // Создаем поверхность для сохранения
    SDL_Surface* surface = SDL_CreateRGBSurface(0, m_width, m_height, 32,
        0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);

    if (!surface) {
        std::cerr << "ERROR: Failed to create surface for saving atlas. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Сохраняем текущую цель рендеринга
    SDL_Texture* currentTarget = SDL_GetRenderTarget(m_renderer);

    // Создаем временную текстуру для чтения пикселей
    SDL_Texture* tempTexture = SDL_CreateTexture(
        m_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        m_width,
        m_height
    );

    if (!tempTexture) {
        SDL_FreeSurface(surface);
        std::cerr << "ERROR: Failed to create temp texture for reading pixels. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Устанавливаем временную текстуру как цель рендеринга
    SDL_SetRenderTarget(m_renderer, tempTexture);

    // Очищаем текстуру (делаем полностью прозрачной)
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_renderer);

    // Копируем атлас в временную текстуру
    SDL_RenderCopy(m_renderer, m_atlasTexture, nullptr, nullptr);

    // Возвращаем предыдущую цель рендеринга
    SDL_SetRenderTarget(m_renderer, currentTarget);

    // Получаем пиксели из текстуры
    void* pixels = nullptr;
    int pitch = 0;

    if (SDL_LockSurface(surface) != 0) {
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(tempTexture);
        std::cerr << "ERROR: Failed to lock surface. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    pixels = surface->pixels;
    pitch = surface->pitch;

    if (SDL_RenderReadPixels(m_renderer, nullptr, SDL_PIXELFORMAT_RGBA8888, pixels, pitch) != 0) {
        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(tempTexture);
        std::cerr << "ERROR: Failed to read pixels from texture. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_UnlockSurface(surface);

    // Сохраняем поверхность в файл
    if (IMG_SavePNG(surface, filePath.c_str()) != 0) {
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(tempTexture);
        std::cerr << "ERROR: Failed to save atlas to file '" << filePath << "'. SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // Освобождаем ресурсы
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(tempTexture);

    std::cout << "Atlas '" << m_id << "' saved to file '" << filePath << "'" << std::endl;
    return true;
}

bool TextureAtlas::loadFromFile(const std::string& filePath, const std::string& descriptorFilePath) {
    // Очищаем существующие субтекстуры
    m_subTextures.clear();

    // Загружаем текстуру атласа из файла
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if (!surface) {
        std::cerr << "ERROR: Failed to load atlas image '" << filePath << "'. SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // Создаем текстуру атласа
    SDL_Texture* newAtlasTexture = SDL_CreateTextureFromSurface(m_renderer, surface);
    SDL_FreeSurface(surface);

    if (!newAtlasTexture) {
        std::cerr << "ERROR: Failed to create atlas texture from '" << filePath << "'. SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Обновляем размеры атласа
    int width, height;
    SDL_QueryTexture(newAtlasTexture, nullptr, nullptr, &width, &height);

    // Освобождаем старую текстуру атласа
    if (m_atlasTexture) {
        SDL_DestroyTexture(m_atlasTexture);
    }

    // Устанавливаем новую текстуру атласа
    m_atlasTexture = newAtlasTexture;
    m_width = width;
    m_height = height;

    // Загружаем описание атласа из JSON-файла
    // Это простая реализация - в реальном проекте здесь нужна библиотека для работы с JSON

    // TODO: Добавить загрузку описания атласа из JSON-файла

    // Обновляем размер в памяти
    m_memorySize = calculateAtlasSize();

    std::cout << "Atlas '" << m_id << "' loaded from file '" << filePath << "' with size "
        << m_width << "x" << m_height << std::endl;

    return true;
}

bool TextureAtlas::saveDescriptor(const std::string& filePath) {
    // Создаем JSON-описание атласа
    // Это простая реализация - в реальном проекте здесь нужна библиотека для работы с JSON

    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open file '" << filePath << "' for writing!" << std::endl;
        return false;
    }

    file << "{\n";
    file << "  \"atlas\": \"" << m_id << "\",\n";
    file << "  \"width\": " << m_width << ",\n";
    file << "  \"height\": " << m_height << ",\n";
    file << "  \"subtextures\": [\n";

    // Добавляем информацию о субтекстурах
    size_t index = 0;
    for (const auto& pair : m_subTextures) {
        const SubTexture& subTexture = pair.second;

        file << "    {\n";
        file << "      \"id\": \"" << subTexture.id << "\",\n";
        file << "      \"x\": " << subTexture.rect.x << ",\n";
        file << "      \"y\": " << subTexture.rect.y << ",\n";
        file << "      \"width\": " << subTexture.rect.w << ",\n";
        file << "      \"height\": " << subTexture.rect.h << ",\n";
        file << "      \"sourceWidth\": " << subTexture.source.w << ",\n";
        file << "      \"sourceHeight\": " << subTexture.source.h << ",\n";
        file << "      \"rotated\": " << (subTexture.rotated ? "true" : "false") << "\n";
        file << "    }";

        if (index < m_subTextures.size() - 1) {
            file << ",";
        }
        file << "\n";
        index++;
    }

    file << "  ]\n";
    file << "}\n";

    file.close();

    std::cout << "Atlas '" << m_id << "' descriptor saved to file '" << filePath << "'" << std::endl;
    return true;
}

std::string TextureAtlas::getSubTexturesInfo() const {
    std::stringstream ss;
    ss << "Atlas '" << m_id << "' (" << m_width << "x" << m_height << "):\n";
    ss << "Total SubTextures: " << m_subTextures.size() << "\n";
    ss << "Memory size: " << (m_memorySize / 1024) << " KB\n\n";

    for (const auto& pair : m_subTextures) {
        const SubTexture& subTexture = pair.second;
        ss << "- " << subTexture.id << ": ("
            << subTexture.rect.x << "," << subTexture.rect.y << ") "
            << subTexture.rect.w << "x" << subTexture.rect.h;

        if (subTexture.rotated) {
            ss << " (rotated)";
        }

        ss << "\n";
    }

    return ss.str();
}

bool TextureAtlas::findFreeSpace(int width, int height, SDL_Rect* rect, bool allowRotation) {
    if (!rect) {
        return false;
    }

    // Проверяем, можно ли использовать поворот для лучшей упаковки
    bool canRotate = allowRotation && width != height;

    // Перебираем свободные прямоугольники
    for (auto it = m_freeRects.begin(); it != m_freeRects.end(); ++it) {
        const SDL_Rect& freeRect = *it;

        // Пытаемся разместить текстуру без поворота
        if (freeRect.w >= width && freeRect.h >= height) {
            // Нашли подходящее место
            rect->x = freeRect.x;
            rect->y = freeRect.y;
            rect->w = width;
            rect->h = height;

            // Обновляем свободные прямоугольники
            it = m_freeRects.erase(it);

            // Разбиваем оставшееся пространство на новые свободные прямоугольники
            // Разделяем по горизонтали (если после размещения осталось место справа)
            if (freeRect.w > width) {
                SDL_Rect newRect = {
                    freeRect.x + width,  // X - сразу после размещенной текстуры
                    freeRect.y,          // Y - совпадает с размещенной текстурой
                    freeRect.w - width,  // Ширина - оставшееся пространство справа
                    height               // Высота - равна размещенной текстуре
                };
                m_freeRects.push_back(newRect);
            }

            // Разделяем по вертикали (если после размещения осталось место снизу)
            if (freeRect.h > height) {
                SDL_Rect newRect = {
                    freeRect.x,          // X - совпадает с размещенной текстурой
                    freeRect.y + height, // Y - сразу под размещенной текстурой
                    freeRect.w,          // Ширина - вся ширина исходного свободного прямоугольника
                    freeRect.h - height  // Высота - оставшееся пространство снизу
                };
                m_freeRects.push_back(newRect);
            }

            return true;
        }
        // Пытаемся разместить текстуру с поворотом на 90 градусов
        else if (canRotate && freeRect.w >= height && freeRect.h >= width) {
            // Нашли подходящее место (с поворотом)
            rect->x = freeRect.x;
            rect->y = freeRect.y;
            rect->w = height;  // Ширина и высота меняются местами при повороте
            rect->h = width;

            // Обновляем свободные прямоугольники
            it = m_freeRects.erase(it);

            // Разбиваем оставшееся пространство на новые свободные прямоугольники
            // Разделяем по горизонтали (если после размещения осталось место справа)
            if (freeRect.w > height) {
                SDL_Rect newRect = {
                    freeRect.x + height, // X - сразу после размещенной текстуры
                    freeRect.y,          // Y - совпадает с размещенной текстурой
                    freeRect.w - height, // Ширина - оставшееся пространство справа
                    width                // Высота - равна ширине исходной текстуры
                };
                m_freeRects.push_back(newRect);
            }

            // Разделяем по вертикали (если после размещения осталось место снизу)
            if (freeRect.h > width) {
                SDL_Rect newRect = {
                    freeRect.x,         // X - совпадает с размещенной текстурой
                    freeRect.y + width, // Y - сразу под размещенной текстурой
                    freeRect.w,         // Ширина - вся ширина исходного свободного прямоугольника
                    freeRect.h - width  // Высота - оставшееся пространство снизу
                };
                m_freeRects.push_back(newRect);
            }

            return true;
        }
    }

    // Не нашли свободного места
    return false;
}

size_t TextureAtlas::calculateAtlasSize() const {
    // Вычисляем размер текстуры в памяти
    // Формула: ширина * высота * 4 байта на пиксель (RGBA8888) + небольшой overhead
    return static_cast<size_t>(m_width) * m_height * 4 + 512;
}