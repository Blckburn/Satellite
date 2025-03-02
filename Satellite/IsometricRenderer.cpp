#include "IsometricRenderer.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <iostream>

IsometricRenderer::IsometricRenderer(int tileWidth, int tileHeight)
    : m_tileWidth(tileWidth), m_tileHeight(tileHeight),
    m_cameraX(0.0f), m_cameraY(0.0f), m_cameraZoom(1.0f) {
}

IsometricRenderer::~IsometricRenderer() {
}

void IsometricRenderer::worldToScreen(float worldX, float worldY, int& screenX, int& screenY) const {
    // Проверка на некорректные входные данные
    if (std::isnan(worldX) || std::isnan(worldY)) {
        screenX = 0;
        screenY = 0;
        return;
    }

    // Применяем смещение камеры
    float offsetX = worldX - m_cameraX;
    float offsetY = worldY - m_cameraY;

    // Основное изометрическое преобразование с учетом масштаба
    float fScreenX = (offsetX - offsetY) * (m_tileWidth / 2.0f) * m_cameraZoom;
    float fScreenY = (offsetX + offsetY) * (m_tileHeight / 2.0f) * m_cameraZoom;

    // Проверка на переполнение или некорректные результаты
    if (std::isnan(fScreenX) || std::isnan(fScreenY) ||
        std::isinf(fScreenX) || std::isinf(fScreenY)) {
        screenX = 0;
        screenY = 0;
        return;
    }

    // Преобразуем в целые числа для отрисовки с округлением
    screenX = static_cast<int>(round(fScreenX));
    screenY = static_cast<int>(round(fScreenY));
}

void IsometricRenderer::renderTile(SDL_Renderer* renderer, float worldX, float worldY, float height,
    SDL_Color color, int centerX, int centerY) const {
    // Используем новую систему координат
    int baseX, baseY;
    worldToScreen(worldX, worldY, baseX, baseY);

    // Смещение центра экрана
    baseX += centerX;
    baseY += centerY;

    // Вычисляем смещение по высоте
    int heightOffset = getHeightInPixels(height);

    // Вычисляем размеры с учетом масштаба
    int scaledTileWidth = static_cast<int>(m_tileWidth * m_cameraZoom);
    int scaledTileHeight = static_cast<int>(m_tileHeight * m_cameraZoom);

    // Вершины ромба
    SDL_Point points[4];
    points[0] = { baseX, baseY - heightOffset };                              // Верхняя вершина
    points[1] = { baseX + scaledTileWidth / 2, baseY + scaledTileHeight / 2 - heightOffset }; // Правая вершина
    points[2] = { baseX, baseY + scaledTileHeight - heightOffset };          // Нижняя вершина
    points[3] = { baseX - scaledTileWidth / 2, baseY + scaledTileHeight / 2 - heightOffset }; // Левая вершина

    // Устанавливаем цвет
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Заполняем ромб
    fillPolygon(renderer, points, 4);

    // Рисуем контур для четкости
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer,
            points[i].x, points[i].y,
            points[(i + 1) % 4].x, points[(i + 1) % 4].y);
    }
}

void IsometricRenderer::renderVolumetricTile(SDL_Renderer* renderer, float worldX, float worldY, float height,
    SDL_Color topColor, SDL_Color leftColor, SDL_Color rightColor, int centerX, int centerY) const {
    if (height <= 0.0f) {
        // Если высота нулевая или отрицательная, рисуем обычный тайл
        renderTile(renderer, worldX, worldY, 0.0f, topColor, centerX, centerY);
        return;
    }

    // Используем новую систему координат
    int baseX, baseY;    // Координаты базовой точки (без учета высоты)
    worldToScreen(worldX, worldY, baseX, baseY);

    // Смещение центра экрана
    baseX += centerX;
    baseY += centerY;

    // Вычисляем смещение по высоте
    int heightOffset = getHeightInPixels(height);

    // Вычисляем размеры с учетом масштаба
    int scaledTileWidth = static_cast<int>(m_tileWidth * m_cameraZoom);
    int scaledTileHeight = static_cast<int>(m_tileHeight * m_cameraZoom);

    // Вершины для верхней грани (ромб)
    SDL_Point topFace[4];
    topFace[0] = { baseX, baseY - heightOffset };                              // Верхняя вершина
    topFace[1] = { baseX + scaledTileWidth / 2, baseY + scaledTileHeight / 2 - heightOffset }; // Правая вершина
    topFace[2] = { baseX, baseY + scaledTileHeight - heightOffset };          // Нижняя вершина
    topFace[3] = { baseX - scaledTileWidth / 2, baseY + scaledTileHeight / 2 - heightOffset }; // Левая вершина

    // Левая грань (четырехугольник)
    SDL_Point leftFace[4];
    leftFace[0] = { baseX - scaledTileWidth / 2, baseY + scaledTileHeight / 2 - heightOffset }; // Верхняя левая
    leftFace[1] = { baseX, baseY + scaledTileHeight - heightOffset }; // Верхняя правая
    leftFace[2] = { baseX, baseY + scaledTileHeight }; // Нижняя правая
    leftFace[3] = { baseX - scaledTileWidth / 2, baseY + scaledTileHeight / 2 }; // Нижняя левая

    // Правая грань (четырехугольник)
    SDL_Point rightFace[4];
    rightFace[0] = { baseX, baseY + scaledTileHeight - heightOffset }; // Верхняя левая
    rightFace[1] = { baseX + scaledTileWidth / 2, baseY + scaledTileHeight / 2 - heightOffset }; // Верхняя правая
    rightFace[2] = { baseX + scaledTileWidth / 2, baseY + scaledTileHeight / 2 }; // Нижняя правая
    rightFace[3] = { baseX, baseY + scaledTileHeight }; // Нижняя левая

    // Сначала рисуем левую и правую грани, затем верхнюю для правильного перекрытия
    SDL_SetRenderDrawColor(renderer, leftColor.r, leftColor.g, leftColor.b, leftColor.a);
    fillPolygon(renderer, leftFace, 4);

    SDL_SetRenderDrawColor(renderer, rightColor.r, rightColor.g, rightColor.b, rightColor.a);
    fillPolygon(renderer, rightFace, 4);

    SDL_SetRenderDrawColor(renderer, topColor.r, topColor.g, topColor.b, topColor.a);
    fillPolygon(renderer, topFace, 4);

    // Рисуем контуры для четкости
    SDL_SetRenderDrawColor(renderer, topColor.r * 0.8, topColor.g * 0.8, topColor.b * 0.8, topColor.a);
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer, topFace[i].x, topFace[i].y, topFace[(i + 1) % 4].x, topFace[(i + 1) % 4].y);
    }

    SDL_SetRenderDrawColor(renderer, leftColor.r * 0.8, leftColor.g * 0.8, leftColor.b * 0.8, leftColor.a);
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer, leftFace[i].x, leftFace[i].y, leftFace[(i + 1) % 4].x, leftFace[(i + 1) % 4].y);
    }

    SDL_SetRenderDrawColor(renderer, rightColor.r * 0.8, rightColor.g * 0.8, rightColor.b * 0.8, rightColor.a);
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer, rightFace[i].x, rightFace[i].y, rightFace[(i + 1) % 4].x, rightFace[(i + 1) % 4].y);
    }
}

void IsometricRenderer::screenToWorld(int screenX, int screenY, float& worldX, float& worldY) const {
    // Обратное изометрическое преобразование с учетом масштаба
    float scaledScreenX = screenX / m_cameraZoom;
    float scaledScreenY = screenY / m_cameraZoom;

    worldX = (scaledScreenX / (m_tileWidth / 2.0f) + scaledScreenY / (m_tileHeight / 2.0f)) / 2.0f;
    worldY = (scaledScreenY / (m_tileHeight / 2.0f) - scaledScreenX / (m_tileWidth / 2.0f)) / 2.0f;

    // Применяем смещение камеры
    worldX += m_cameraX;
    worldY += m_cameraY;
}

void IsometricRenderer::worldToDisplay(float worldX, float worldY, float worldZ,
    int centerX, int centerY,
    int& displayX, int& displayY) const {
    // Шаг 1: Преобразование из мировых в экранные координаты (без учета высоты)
    int screenX, screenY;
    worldToScreen(worldX, worldY, screenX, screenY);

    // Шаг 2: Учет высоты (Z-координаты)
    int heightOffset = getHeightInPixels(worldZ);
    screenY -= heightOffset;

    // Шаг 3: Преобразование в абсолютные экранные координаты с учетом центра экрана
    displayX = screenX + centerX;
    displayY = screenY + centerY;
}

int IsometricRenderer::getHeightInPixels(float worldHeight) const {
    // Используем константу HEIGHT_SCALE вместо локального значения
    return static_cast<int>(worldHeight * HEIGHT_SCALE * m_cameraZoom);
}

int IsometricRenderer::getScaledSize(int size) const {
    return static_cast<int>(size * m_cameraZoom);
}

void IsometricRenderer::renderGrid(SDL_Renderer* renderer, int centerX, int centerY, int gridSize, SDL_Color color) const {
    // Установка цвета для сетки
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Переберем только нужный диапазон для gridSize
    for (int y = -gridSize; y <= gridSize; ++y) {
        for (int x = -gridSize; x <= gridSize; ++x) {
            // Получаем экранные координаты для текущей ячейки сетки
            int screenX, screenY;
            worldToScreen(x, y, screenX, screenY);

            // Добавляем смещение относительно центра экрана
            screenX += centerX;
            screenY += centerY;

            // Вычисляем размеры с учетом масштаба
            int scaledTileWidth = static_cast<int>(m_tileWidth * m_cameraZoom);
            int scaledTileHeight = static_cast<int>(m_tileHeight * m_cameraZoom);

            // Создаем точки для ромба
            SDL_Point points[5];
            points[0] = { screenX, screenY }; // Верхний угол
            points[1] = { screenX + scaledTileWidth / 2, screenY + scaledTileHeight / 2 }; // Правый угол
            points[2] = { screenX, screenY + scaledTileHeight }; // Нижний угол
            points[3] = { screenX - scaledTileWidth / 2, screenY + scaledTileHeight / 2 }; // Левый угол
            points[4] = { screenX, screenY }; // Замыкаем контур

            // Рисуем контур ромба
            SDL_RenderDrawLines(renderer, points, 5);
        }
    }
}

void IsometricRenderer::renderDebugPoint(SDL_Renderer* renderer, float worldX, float worldY,
    float worldZ, SDL_Color color, int centerX, int centerY) const {
    // Преобразуем мировые координаты в координаты отображения
    int displayX, displayY;
    worldToDisplay(worldX, worldY, worldZ, centerX, centerY, displayX, displayY);

    // Размер точки с учетом масштаба
    int size = getScaledSize(5);

    // Рисуем заполненный квадрат
    SDL_Rect rect = { displayX - size / 2, displayY - size / 2, size, size };
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);

    // Рисуем черную рамку для большей видимости
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

void IsometricRenderer::setCameraPosition(float x, float y) {
    m_cameraX = x;
    m_cameraY = y;
}

void IsometricRenderer::setCameraZoom(float scale) {
    // Ограничиваем масштаб
    if (scale < 0.1f) scale = 0.1f;
    if (scale > 5.0f) scale = 5.0f;

    m_cameraZoom = scale;
}

void IsometricRenderer::fillPolygon(SDL_Renderer* renderer, const SDL_Point* points, int count) const {
    // Если меньше 3 точек, рисуем просто линии
    if (count < 3) {
        for (int i = 0; i < count - 1; ++i) {
            SDL_RenderDrawLine(renderer, points[i].x, points[i].y, points[i + 1].x, points[i + 1].y);
        }
        return;
    }

    // Находим минимальную и максимальную Y-координаты
    int minY = points[0].y;
    int maxY = points[0].y;

    for (int i = 1; i < count; ++i) {
        if (points[i].y < minY) minY = points[i].y;
        if (points[i].y > maxY) maxY = points[i].y;
    }

    // Создаем массив для хранения пересечений
    std::vector<int> nodeX;
    nodeX.reserve(count);

    // Обрабатываем каждую строку сканирования
    for (int y = minY; y <= maxY; ++y) {
        // Очищаем массив пересечений для новой строки
        nodeX.clear();

        // Находим все пересечения с ребрами полигона
        for (int i = 0; i < count; ++i) {
            int j = (i + 1) % count;

            // Проверяем, пересекает ли строка сканирования ребро
            if ((points[i].y <= y && points[j].y > y) ||
                (points[i].y > y && points[j].y <= y)) {
                // Вычисляем X-координату пересечения
                int x = points[i].x + (y - points[i].y) *
                    (points[j].x - points[i].x) /
                    (points[j].y - points[i].y);

                nodeX.push_back(x);
            }
        }

        // Сортируем X-координаты пересечений
        std::sort(nodeX.begin(), nodeX.end());

        // Закрашиваем участки между парами пересечений
        for (size_t i = 0; i < nodeX.size(); i += 2) {
            if (i + 1 < nodeX.size()) {
                SDL_RenderDrawLine(renderer, nodeX[i], y, nodeX[i + 1], y);
            }
        }
    }
}

void IsometricRenderer::renderTileWithTexture(SDL_Renderer* renderer, SDL_Texture* texture,
    float worldX, float worldY, float height,
    int centerX, int centerY) const {

    // 1. Получаем базовые экранные координаты
    int screenX, screenY;
    worldToScreen(worldX, worldY, screenX, screenY);

    // 2. Смещение центра экрана
    screenX += centerX;
    screenY += centerY;

    // 3. Вычисляем смещение по высоте
    int heightOffset = getHeightInPixels(height);
    screenY -= heightOffset;

    // 4. Вычисляем размеры с учетом масштаба
    int scaledTileWidth = static_cast<int>(m_tileWidth * m_cameraZoom);
    int scaledTileHeight = static_cast<int>(m_tileHeight * m_cameraZoom);

    // 5. Определяем вершины ромба для изометрической проекции
    SDL_Point points[4];
    points[0] = { screenX, screenY };                                // Верхняя вершина
    points[1] = { screenX + scaledTileWidth / 2, screenY + scaledTileHeight / 2 }; // Правая вершина
    points[2] = { screenX, screenY + scaledTileHeight };             // Нижняя вершина
    points[3] = { screenX - scaledTileWidth / 2, screenY + scaledTileHeight / 2 }; // Левая вершина

    // 6. Выбираем цвет в зависимости от положения (шахматный порядок)
    SDL_Color tileColor;
    if ((int)(worldX + worldY) % 2 == 0) {
        // Для травы - зеленый
        tileColor = { 30, 150, 30, 255 };
    }
    else {
        // Для камня - светло-серый
        tileColor = { 180, 180, 180, 255 };
    }

    // 7. Заполняем ромб выбранным цветом
    SDL_SetRenderDrawColor(renderer, tileColor.r, tileColor.g, tileColor.b, tileColor.a);
    fillPolygon(renderer, points, 4);

    // 8. Добавляем тонкую рамку
    SDL_SetRenderDrawColor(renderer, 20, 35, 20, 255);
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer,
            points[i].x, points[i].y,
            points[(i + 1) % 4].x, points[(i + 1) % 4].y);
    }
}

void IsometricRenderer::renderVolumetricTileWithTextures(SDL_Renderer* renderer,
    SDL_Texture* topTexture,
    SDL_Texture* leftTexture,
    SDL_Texture* rightTexture,
    float worldX, float worldY, float height,
    int centerX, int centerY) const {

    // 1. Проверка наличия высоты и текстур
    if (height <= 0.0f) {
        // Если высота нулевая или отрицательная, рисуем обычный тайл
        if (topTexture) {
            renderTileWithTexture(renderer, topTexture, worldX, worldY, 0.0f, centerX, centerY);
        }
        else {
            SDL_Color grayColor = { 150, 150, 150, 255 };
            renderTile(renderer, worldX, worldY, 0.0f, grayColor, centerX, centerY);
        }
        return;
    }

    // 2. Получение базовых экранных координат
    int screenX, screenY;
    worldToScreen(worldX, worldY, screenX, screenY);

    // 3. Применение смещения центра экрана
    screenX += centerX;
    screenY += centerY;

    // 4. Вычисление высоты с фиксированным коэффициентом 1.5f для лучшего 3D эффекта
    float heightFactor = 1.5f;
    int heightOffset = static_cast<int>(getHeightInPixels(height) * heightFactor);

    // 5. Расчет размеров с учетом масштаба
    int scaledTileWidth = static_cast<int>(m_tileWidth * m_cameraZoom);
    int scaledTileHeight = static_cast<int>(m_tileHeight * m_cameraZoom);

    // 6. Определение координат для верхней грани (ромб)
    SDL_Point topFace[4];
    topFace[0] = { screenX, screenY - heightOffset };
    topFace[1] = { screenX + scaledTileWidth / 2, screenY + scaledTileHeight / 2 - heightOffset };
    topFace[2] = { screenX, screenY + scaledTileHeight - heightOffset };
    topFace[3] = { screenX - scaledTileWidth / 2, screenY + scaledTileHeight / 2 - heightOffset };

    // 7. Определение координат для левой грани (четырехугольник)
    SDL_Point leftFace[4];
    leftFace[0] = { screenX - scaledTileWidth / 2, screenY + scaledTileHeight / 2 - heightOffset };
    leftFace[1] = { screenX, screenY + scaledTileHeight - heightOffset };
    leftFace[2] = { screenX, screenY + scaledTileHeight };
    leftFace[3] = { screenX - scaledTileWidth / 2, screenY + scaledTileHeight / 2 };

    // 8. Определение координат для правой грани (четырехугольник)
    SDL_Point rightFace[4];
    rightFace[0] = { screenX, screenY + scaledTileHeight - heightOffset };
    rightFace[1] = { screenX + scaledTileWidth / 2, screenY + scaledTileHeight / 2 - heightOffset };
    rightFace[2] = { screenX + scaledTileWidth / 2, screenY + scaledTileHeight / 2 };
    rightFace[3] = { screenX, screenY + scaledTileHeight };

    // 9. Отрисовка левой грани с улучшенными параметрами для текстуры
    if (leftTexture) {
        // Создаем целевую область для левой грани с точными координатами
        SDL_Rect leftRect = {
            std::min(leftFace[0].x, leftFace[3].x),
            std::min(leftFace[0].y, leftFace[1].y),
            abs(leftFace[1].x - leftFace[0].x),
            abs(leftFace[3].y - leftFace[0].y)
        };

        // Применяем текстуру с корректным форматом
        SDL_SetTextureBlendMode(leftTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(leftTexture, 255);
        SDL_RenderCopy(renderer, leftTexture, nullptr, &leftRect);
    }
    else {
        // Улучшенное цветовое затенение с увеличенным контрастом
        SDL_Color leftColor = { 120, 120, 120, 255 };
        SDL_SetRenderDrawColor(renderer, leftColor.r, leftColor.g, leftColor.b, leftColor.a);
        fillPolygon(renderer, leftFace, 4);
    }

    // 10. Отрисовка правой грани с улучшенными параметрами для текстуры
    if (rightTexture) {
        // Создаем целевую область для правой грани с точными координатами
        SDL_Rect rightRect = {
            std::min(rightFace[0].x, rightFace[3].x),
            std::min(rightFace[0].y, rightFace[1].y),
            abs(rightFace[1].x - rightFace[0].x),
            abs(rightFace[3].y - rightFace[0].y)
        };

        // Применяем текстуру с корректным форматом
        SDL_SetTextureBlendMode(rightTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(rightTexture, 255);
        SDL_RenderCopy(renderer, rightTexture, nullptr, &rightRect);
    }
    else {
        // Улучшенное цветовое затенение с увеличенным контрастом
        SDL_Color rightColor = { 80, 80, 80, 255 };
        SDL_SetRenderDrawColor(renderer, rightColor.r, rightColor.g, rightColor.b, rightColor.a);
        fillPolygon(renderer, rightFace, 4);
    }

    // 11. Отрисовка верхней грани (ромба) поверх остальных граней
    if (topTexture) {
        // Создаем область для отрисовки верхней грани с точными координатами
        SDL_Rect topRect = {
            topFace[3].x,
            topFace[0].y,
            topFace[1].x - topFace[3].x,
            topFace[2].y - topFace[0].y
        };

        // Применяем текстуру с корректным форматом
        SDL_SetTextureBlendMode(topTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(topTexture, 255);
        SDL_RenderCopy(renderer, topTexture, nullptr, &topRect);
    }
    else {
        // Стандартное заполнение цветом
        SDL_Color topColor = { 150, 150, 150, 255 };
        SDL_SetRenderDrawColor(renderer, topColor.r, topColor.g, topColor.b, topColor.a);
        fillPolygon(renderer, topFace, 4);
    }

    // 12. Рисуем контуры граней для лучшей видимости
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Контур верхней грани
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer, topFace[i].x, topFace[i].y, topFace[(i + 1) % 4].x, topFace[(i + 1) % 4].y);
    }

    // Контур левой грани
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer, leftFace[i].x, leftFace[i].y, leftFace[(i + 1) % 4].x, leftFace[(i + 1) % 4].y);
    }

    // Контур правой грани
    for (int i = 0; i < 4; ++i) {
        SDL_RenderDrawLine(renderer, rightFace[i].x, rightFace[i].y, rightFace[(i + 1) % 4].x, rightFace[(i + 1) % 4].y);
    }

    // 13. Добавляем вертикальные ребра для усиления 3D эффекта
    SDL_RenderDrawLine(renderer, topFace[3].x, topFace[3].y, leftFace[3].x, leftFace[3].y);
    SDL_RenderDrawLine(renderer, topFace[1].x, topFace[1].y, rightFace[2].x, rightFace[2].y);
    SDL_RenderDrawLine(renderer, topFace[2].x, topFace[2].y, leftFace[2].x, leftFace[2].y);
}