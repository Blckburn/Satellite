#include "UIManager.h"
#include "Logger.h"
#include "ResourceManager.h"
#include <cmath>

UIManager::UIManager(Engine* engine)
    : m_engine(engine) {
    LOG_INFO("UIManager initialized");
}

void UIManager::render(SDL_Renderer* renderer,
    std::shared_ptr<IsometricRenderer> isoRenderer,
    std::shared_ptr<TileMap> tileMap,
    std::shared_ptr<Player> player,
    std::shared_ptr<InteractionSystem> interactionSystem,
    bool showDebug) {
    // Получаем размеры окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    // 1. Отрисовка отладочной информации, если включен режим отладки
    if (showDebug && player && tileMap && isoRenderer) {
        renderDebug(renderer, isoRenderer, tileMap, player, centerX, centerY);
    }

    // 2. Отрисовка подсказки для взаимодействия
    if (interactionSystem && interactionSystem->shouldShowInteractionPrompt()) {
        renderInteractionPrompt(renderer, interactionSystem->getInteractionPrompt());
    }

    // 3. Отрисовка информации терминала
    if (interactionSystem && interactionSystem->isDisplayingTerminalInfo() &&
        interactionSystem->getCurrentTerminal()) {
        renderTerminalInfo(renderer, interactionSystem->getCurrentTerminal());
    }
}

void UIManager::renderInteractionPrompt(SDL_Renderer* renderer, const std::string& prompt) {
    // Проверяем, не пустая ли подсказка
    if (prompt.empty()) {
        return;
    }

    // Получаем размеры окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Проверяем, доступен ли ResourceManager и есть ли шрифт
    if (m_engine && m_engine->getResourceManager() &&
        m_engine->getResourceManager()->hasFont("default")) {

        // Цвет текста (ярко-белый)
        SDL_Color textColor = { 255, 255, 255, 255 };

        // Создаем временную текстуру с текстом, чтобы определить её размеры
        TTF_Font* font = m_engine->getResourceManager()->getFont("default");
        if (!font) return;

        // Получаем размеры текста
        int textWidth, textHeight;
        TTF_SizeText(font, prompt.c_str(), &textWidth, &textHeight);

        // Добавляем отступы
        int padding = 20;
        int promptWidth = textWidth + padding * 2;
        int promptHeight = textHeight + padding;

        // Минимальная ширина подложки для коротких сообщений
        int minWidth = 300;
        if (promptWidth < minWidth) {
            promptWidth = minWidth;
        }

        // Максимальная ширина подложки, не выходящая за пределы экрана
        int maxWidth = windowWidth - 60; // Оставляем отступ по 30 пикселей с каждой стороны
        if (promptWidth > maxWidth) {
            promptWidth = maxWidth;
        }

        // Отрисовываем полупрозрачный прямоугольник внизу экрана
        SDL_Rect promptRect = {
            windowWidth / 2 - promptWidth / 2,
            windowHeight - 60,
            promptWidth,
            promptHeight
        };

        // Устанавливаем цвет прямоугольника (полупрозрачный черный)
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);  // Немного больше непрозрачности
        SDL_RenderFillRect(renderer, &promptRect);

        // Рисуем рамку с лучшим визуальным эффектом
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);  // Серая рамка
        SDL_RenderDrawRect(renderer, &promptRect);

        // Добавляем внутреннюю рамку для эффекта углубления
        SDL_Rect innerRect = {
            promptRect.x + 2,
            promptRect.y + 2,
            promptRect.w - 4,
            promptRect.h - 4
        };
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);  // Темно-серая внутренняя рамка
        SDL_RenderDrawRect(renderer, &innerRect);

        // Отрисовываем текст с использованием ResourceManager
        m_engine->getResourceManager()->renderText(
            renderer,
            prompt,
            "default",
            windowWidth / 2,  // X-координата (центр экрана)
            windowHeight - 60 + promptHeight / 2, // Y-координата (центр подложки)
            textColor
        );
    }
    else {
        // Логируем подсказку только при первом взаимодействии, если нет доступных шрифтов
        static std::string lastPrompt;
        if (lastPrompt != prompt) {
            lastPrompt = prompt;
            LOG_INFO("Interaction prompt: " + prompt);
        }

        // Если идет взаимодействие с дверью, рисуем полоску прогресса
        if (prompt.find("Door") != std::string::npos || prompt.find("door") != std::string::npos) {
            // Получаем размеры окна
            int windowWidth, windowHeight;
            SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

            // Создаем полоску прогресса под основной подсказкой
            int progressWidth = 300;
            int progressHeight = 8;

            SDL_Rect progressBg = {
                windowWidth / 2 - progressWidth / 2,
                windowHeight - 40, // Расположение под текстом подсказки
                progressWidth,
                progressHeight
            };

            // Фон полоски (темно-серый, полупрозрачный)
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 180);
            SDL_RenderFillRect(renderer, &progressBg);

            // Заполненная часть полоски (зеленая)
            SDL_Rect progressFill = progressBg;
            progressFill.w = static_cast<int>(progressFill.w * 0.5f); // Пример заполнения 50%

            // Цвет прогресс-бара - зеленый
            SDL_SetRenderDrawColor(renderer, 50, 220, 50, 220);
            SDL_RenderFillRect(renderer, &progressFill);

            // Рамка для полоски
            SDL_SetRenderDrawColor(renderer, 180, 180, 180, 200);
            SDL_RenderDrawRect(renderer, &progressBg);
        }
    }
}

void UIManager::renderTerminalInfo(SDL_Renderer* renderer, std::shared_ptr<Terminal> terminal) {
    if (!terminal || !renderer) {
        return;
    }

    // Получаем размеры окна
    int windowWidth, windowHeight;
    SDL_GetRendererOutputSize(renderer, &windowWidth, &windowHeight);

    // Проверяем, доступен ли ResourceManager и есть ли шрифт
    if (m_engine && m_engine->getResourceManager() &&
        m_engine->getResourceManager()->hasFont("default")) {

        // Получаем шрифт для отображения текста
        TTF_Font* font = m_engine->getResourceManager()->getFont("default");
        if (!font) return;

        // Эффект "компрометации системы" - кратковременное появление предупреждения
        // Каждые 3 секунды на 0.8 секунд появляется предупреждение
        Uint32 currentTime = SDL_GetTicks();
        bool showCompromisedMessage = (currentTime % 3800) < 800; // 0.8 секунды каждые 3.8 секунды

        // Получаем записи терминала
        const auto& entries = terminal->getEntries();

        // Проверяем наличие записей
        if (entries.size() < 2) return;

        // Определяем, какую запись показывать
        int selectedIndex = terminal->getSelectedEntryIndex();

        // Определяем текущий контент и заголовок
        std::string headerText;
        std::string contentText;
        SDL_Color textColor;
        SDL_Color bgColor;

        if (showCompromisedMessage && entries.size() > 0) {
            // Показываем предупреждение о компрометации (используем последнюю запись)
            size_t warningIndex = entries.size() - 1;
            headerText = entries[warningIndex].first;
            contentText = entries[warningIndex].second;

            // Яркий красный цвет для предупреждения
            textColor = { 255, 70, 70, 255 };

            // Темный фон с красным оттенком
            bgColor = { 40, 0, 0, 220 };
        }
        else if (selectedIndex >= 0 && selectedIndex < entries.size()) {
            // Показываем выбранную запись в обычном цвете
            headerText = entries[selectedIndex].first;
            contentText = entries[selectedIndex].second;

            // Определяем цвет в зависимости от типа терминала
            switch (terminal->getTerminalType()) {
            case Terminal::TerminalType::RESEARCH_SENSOR:
                textColor = { 220, 255, 255, 255 }; // Светло-бирюзовый
                bgColor = { 0, 45, 45, 220 }; // Темно-бирюзовый фон
                break;
            case Terminal::TerminalType::ANCIENT_CONSOLE:
                textColor = { 230, 200, 255, 255 }; // Светло-фиолетовый
                bgColor = { 40, 0, 60, 220 }; // Темно-фиолетовый фон
                break;
            case Terminal::TerminalType::EMERGENCY_BEACON:
                textColor = { 255, 220, 180, 255 }; // Светло-оранжевый
                bgColor = { 60, 20, 0, 220 }; // Темно-оранжевый фон
                break;
            case Terminal::TerminalType::SCIENCE_STATION:
                textColor = { 180, 220, 255, 255 }; // Светло-синий
                bgColor = { 0, 30, 60, 220 }; // Темно-синий фон
                break;
            default:
                textColor = { 255, 255, 255, 255 }; // Белый
                bgColor = { 0, 0, 0, 220 }; // Черный фон
            }
        }
        else {
            // Если нет подходящих записей, показываем стандартный текст
            headerText = terminal->getName();
            contentText = "No data available.";
            textColor = { 255, 255, 255, 255 }; // Белый
            bgColor = { 0, 0, 0, 220 }; // Черный фон
        }

        // Ширина и высота информационного окна
        int infoWidth = windowWidth / 2 + 100;
        int infoHeight = windowHeight / 2 + 50;

        // Отрисовываем полупрозрачный прямоугольник для фона
        SDL_Rect infoRect = {
            windowWidth / 2 - infoWidth / 2,
            windowHeight / 2 - infoHeight / 2,
            infoWidth,
            infoHeight
        };

        // Устанавливаем цвет фона
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_RenderFillRect(renderer, &infoRect);

        // Рисуем рамку для окна терминала
        // Для предупреждения рисуем красную рамку
        if (showCompromisedMessage) {
            SDL_SetRenderDrawColor(renderer, 255, 70, 70, 200);
        }
        else {
            SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, 180);
        }
        SDL_RenderDrawRect(renderer, &infoRect);

        // Отображаем название терминала вверху (всегда)
        std::string terminalTitle = terminal->getName();

        // Рисуем заголовок
        SDL_Surface* titleSurface = TTF_RenderText_Blended(font, terminalTitle.c_str(), textColor);
        if (titleSurface) {
            SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
            if (titleTexture) {
                SDL_Rect titleRect;
                titleRect.w = titleSurface->w;
                titleRect.h = titleSurface->h;
                titleRect.x = windowWidth / 2 - titleRect.w / 2;
                titleRect.y = infoRect.y + 20;

                SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
                SDL_DestroyTexture(titleTexture);
            }
            SDL_FreeSurface(titleSurface);
        }

        // Отрисовываем разделительную линию под заголовком
        SDL_Rect dividerRect = {
            infoRect.x + 40,
            infoRect.y + 55,
            infoRect.w - 80,
            1
        };
        SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, 150);
        SDL_RenderFillRect(renderer, &dividerRect);

        // Максимальная ширина текста для размещения внутри окна
        int maxTextWidth = infoWidth - 100; // Оставляем отступы по бокам

        // Вертикальное смещение
        int yOffset = 80;

        // Отображаем заголовок записи
        SDL_Surface* headerSurface = TTF_RenderText_Blended(font, headerText.c_str(), textColor);
        if (headerSurface) {
            SDL_Texture* headerTexture = SDL_CreateTextureFromSurface(renderer, headerSurface);
            if (headerTexture) {
                SDL_Rect headerRect;
                headerRect.w = headerSurface->w;
                headerRect.h = headerSurface->h;
                headerRect.x = infoRect.x + 40;
                headerRect.y = infoRect.y + yOffset;

                SDL_RenderCopy(renderer, headerTexture, NULL, &headerRect);
                SDL_DestroyTexture(headerTexture);
            }
            SDL_FreeSurface(headerSurface);
        }

        // Создаем цвет для содержимого (немного прозрачнее)
        SDL_Color contentColor = { textColor.r, textColor.g, textColor.b, 200 };

        // Разбиваем текст на строки, чтобы поместить их в окно
        std::vector<std::string> lines;

        // Разделяем текст на строки максимум по 40-45 символов
        int maxLineLength = 40;

        int startPos = 0;
        while (startPos < contentText.length()) {
            int endPos = startPos + maxLineLength;
            if (endPos >= contentText.length()) {
                // Если это конец текста, добавляем оставшуюся часть
                lines.push_back(contentText.substr(startPos));
                break;
            }

            // Находим последний пробел перед endPos
            int lastSpace = contentText.rfind(' ', endPos);
            if (lastSpace > startPos) {
                // Если есть пробел, разбиваем по нему
                lines.push_back(contentText.substr(startPos, lastSpace - startPos));
                startPos = lastSpace + 1;
            }
            else {
                // Если пробела нет, просто разбиваем по maxLineLength
                lines.push_back(contentText.substr(startPos, maxLineLength));
                startPos += maxLineLength;
            }
        }

        // Отображаем каждую строку содержимого
        int lineOffset = 30; // Начальное смещение от заголовка
        for (const auto& line : lines) {
            SDL_Surface* lineSurface = TTF_RenderText_Blended(font, line.c_str(), contentColor);
            if (lineSurface) {
                SDL_Texture* lineTexture = SDL_CreateTextureFromSurface(renderer, lineSurface);
                if (lineTexture) {
                    SDL_Rect lineRect;
                    lineRect.w = lineSurface->w;
                    lineRect.h = lineSurface->h;
                    lineRect.x = infoRect.x + 45; // Небольшой отступ от края
                    lineRect.y = infoRect.y + yOffset + lineOffset;

                    SDL_RenderCopy(renderer, lineTexture, NULL, &lineRect);
                    SDL_DestroyTexture(lineTexture);
                }
                SDL_FreeSurface(lineSurface);
            }

            lineOffset += 25; // Переходим к следующей строке
        }

        // Добавляем подсказку для закрытия внизу
        SDL_Surface* promptSurface = TTF_RenderText_Blended(font, "Press E to close",
            { textColor.r, textColor.g, textColor.b, 180 });
        if (promptSurface) {
            SDL_Texture* promptTexture = SDL_CreateTextureFromSurface(renderer, promptSurface);
            if (promptTexture) {
                SDL_Rect promptRect;
                promptRect.w = promptSurface->w;
                promptRect.h = promptSurface->h;
                promptRect.x = windowWidth / 2 - promptRect.w / 2;
                promptRect.y = infoRect.y + infoHeight - 25;

                SDL_RenderCopy(renderer, promptTexture, NULL, &promptRect);
                SDL_DestroyTexture(promptTexture);
            }
            SDL_FreeSurface(promptSurface);
        }
    }
    else {
        // Если шрифты недоступны, просто выводим в лог
        LOG_INFO("Terminal info display: " + terminal->getName());
    }
}

void UIManager::renderDebug(SDL_Renderer* renderer,
    std::shared_ptr<IsometricRenderer> isoRenderer,
    std::shared_ptr<TileMap> tileMap,
    std::shared_ptr<Player> player,
    int centerX, int centerY) {
    if (!player || !tileMap || !isoRenderer) return;

    // Получаем координаты персонажа
    float playerFullX = player->getFullX();
    float playerFullY = player->getFullY();
    float collisionSize = player->getCollisionSize();

    // 1. Отображение коллизионного прямоугольника персонажа
    int screenX[4], screenY[4];

    // Получаем экранные координаты для каждого угла коллизионной области
    // Верхний левый угол
    isoRenderer->worldToDisplay(
        playerFullX - collisionSize,
        playerFullY - collisionSize,
        0.0f, centerX, centerY, screenX[0], screenY[0]
    );

    // Верхний правый угол
    isoRenderer->worldToDisplay(
        playerFullX + collisionSize,
        playerFullY - collisionSize,
        0.0f, centerX, centerY, screenX[1], screenY[1]
    );

    // Нижний правый угол
    isoRenderer->worldToDisplay(
        playerFullX + collisionSize,
        playerFullY + collisionSize,
        0.0f, centerX, centerY, screenX[2], screenY[2]
    );

    // Нижний левый угол
    isoRenderer->worldToDisplay(
        playerFullX - collisionSize,
        playerFullY + collisionSize,
        0.0f, centerX, centerY, screenX[3], screenY[3]
    );

    // Рисуем коллизионную область
    SDL_Point collisionPoints[5] = {
        {screenX[0], screenY[0]},
        {screenX[1], screenY[1]},
        {screenX[2], screenY[2]},
        {screenX[3], screenY[3]},
        {screenX[0], screenY[0]} // Замыкаем контур
    };

    // Рисуем жёлтую коллизионную рамку
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderDrawLines(renderer, collisionPoints, 5);

    // 2. Отображение границ текущего тайла
    int currentTileX = static_cast<int>(playerFullX);
    int currentTileY = static_cast<int>(playerFullY);

    int tileScreenX[4], tileScreenY[4];

    // Верхний левый угол тайла
    isoRenderer->worldToDisplay(
        currentTileX, currentTileY,
        0.0f, centerX, centerY, tileScreenX[0], tileScreenY[0]
    );

    // Верхний правый угол тайла
    isoRenderer->worldToDisplay(
        currentTileX + 1.0f, currentTileY,
        0.0f, centerX, centerY, tileScreenX[1], tileScreenY[1]
    );

    // Нижний правый угол тайла
    isoRenderer->worldToDisplay(
        currentTileX + 1.0f, currentTileY + 1.0f,
        0.0f, centerX, centerY, tileScreenX[2], tileScreenY[2]
    );

    // Нижний левый угол тайла
    isoRenderer->worldToDisplay(
        currentTileX, currentTileY + 1.0f,
        0.0f, centerX, centerY, tileScreenX[3], tileScreenY[3]
    );

    // Рисуем границы текущего тайла
    SDL_Point tilePoints[5] = {
        {tileScreenX[0], tileScreenY[0]},
        {tileScreenX[1], tileScreenY[1]},
        {tileScreenX[2], tileScreenY[2]},
        {tileScreenX[3], tileScreenY[3]},
        {tileScreenX[0], tileScreenY[0]} // Замыкаем контур
    };

    // Используем бирюзовый цвет для текущего тайла
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderDrawLines(renderer, tilePoints, 5);

    // 3. Отображение окрестных тайлов и информации о проходимости
    int neighborOffsets[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},  // Верхний ряд тайлов
        {-1, 0},            {1, 0},   // Средний ряд (без центра)
        {-1, 1},  {0, 1},  {1, 1}     // Нижний ряд тайлов
    };

    for (int i = 0; i < 8; i++) {
        int neighborX = currentTileX + neighborOffsets[i][0];
        int neighborY = currentTileY + neighborOffsets[i][1];

        if (tileMap->isValidCoordinate(neighborX, neighborY)) {
            bool isWalkable = tileMap->isTileWalkable(neighborX, neighborY);

            // Получаем экранные координаты углов соседнего тайла
            int neighborScreenX[4], neighborScreenY[4];

            // Верхний левый угол
            isoRenderer->worldToDisplay(
                neighborX, neighborY,
                0.0f, centerX, centerY, neighborScreenX[0], neighborScreenY[0]
            );

            // Верхний правый угол
            isoRenderer->worldToDisplay(
                neighborX + 1.0f, neighborY,
                0.0f, centerX, centerY, neighborScreenX[1], neighborScreenY[1]
            );

            // Нижний правый угол
            isoRenderer->worldToDisplay(
                neighborX + 1.0f, neighborY + 1.0f,
                0.0f, centerX, centerY, neighborScreenX[2], neighborScreenY[2]
            );

            // Нижний левый угол
            isoRenderer->worldToDisplay(
                neighborX, neighborY + 1.0f,
                0.0f, centerX, centerY, neighborScreenX[3], neighborScreenY[3]
            );

            // Создаем массив точек для соседнего тайла
            SDL_Point neighborPoints[5] = {
                {neighborScreenX[0], neighborScreenY[0]},
                {neighborScreenX[1], neighborScreenY[1]},
                {neighborScreenX[2], neighborScreenY[2]},
                {neighborScreenX[3], neighborScreenY[3]},
                {neighborScreenX[0], neighborScreenY[0]}
            };

            // Цвет зависит от проходимости тайла
            if (isWalkable) {
                // Зеленый для проходимых тайлов
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
            }
            else {
                // Красный для непроходимых тайлов
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
            }

            // Рисуем границы соседнего тайла
            SDL_RenderDrawLines(renderer, neighborPoints, 5);
        }
    }
}

std::string UIManager::truncateText(const std::string& text, size_t maxLength) {
    if (text.length() <= maxLength) {
        return text;
    }

    // Отрезаем часть текста и добавляем многоточие
    return text.substr(0, maxLength - 3) + "...";
}