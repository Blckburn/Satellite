#include "Terminal.h"
#include "Player.h"
#include "Logger.h"

Terminal::Terminal(const std::string& name, TerminalType type)
    : InteractiveObject(name, InteractiveType::TERMINAL),
    m_terminalType(type),
    m_activated(false),
    m_activationTime(0.0f),
    m_displayingInfo(false)
{
    // Увеличиваем высоту для всех терминалов
    setPosition(getPosition().x, getPosition().y, 1.0f); // Высота в 1 тайл вместо 0.3f

    // Настройка параметров в зависимости от типа терминала
    switch (type) {
    case TerminalType::RESEARCH_SENSOR:
        // Легко заметный, яркий цвет для исследовательских датчиков
        setColor({ 50, 220, 220, 255 }); // Бирюзовый цвет
        setInteractionRadius(2.0f);
        setInteractionHint("Press E to access research data");
        break;

    case TerminalType::ANCIENT_CONSOLE:
        // Древние консоли - мистический фиолетовый оттенок
        setColor({ 180, 100, 220, 255 }); // Фиолетовый цвет
        setInteractionRadius(1.8f);
        setInteractionHint("Press E to decode ancient console");
        break;

    case TerminalType::EMERGENCY_BEACON:
        // Аварийные маяки - яркий оранжевый, сигнальный цвет
        setColor({ 255, 120, 30, 255 }); // Оранжевый цвет
        setInteractionRadius(2.2f);
        setInteractionHint("Press E to analyze emergency beacon");
        break;

    case TerminalType::SCIENCE_STATION:
        // Научные станции - насыщенный синий, цвет высоких технологий
        setColor({ 40, 120, 255, 255 }); // Синий цвет
        setInteractionRadius(1.7f);
        setInteractionHint("Press E to operate science station");
        break;
    }
}

bool Terminal::initialize() {
    // Базовая инициализация интерактивного объекта
    if (!InteractiveObject::initialize()) {
        return false;
    }

    // Добавляем стандартную запись в зависимости от типа терминала
    switch (m_terminalType) {
    case TerminalType::RESEARCH_SENSOR:
        addEntry("Research Sensor Active", "Environmental analysis in progress. Accessing stored data...");
        break;

    case TerminalType::ANCIENT_CONSOLE:
        addEntry("Unknown Technology", "Attempting to decode alien interface. Translation matrix incomplete.");
        break;

    case TerminalType::EMERGENCY_BEACON:
        addEntry("Emergency Signal", "WARNING: Critical situation detected. Retrieving last recorded message...");
        break;

    case TerminalType::SCIENCE_STATION:
        addEntry("Science Station", "Multi-purpose research terminal. Ready for experimental procedures.");
        break;
    }

    selectRandomEntry();

    LOG_INFO("Terminal initialized: " + getName() + " (Type: " + std::to_string(static_cast<int>(m_terminalType)) + ")");
    return true;
}

bool Terminal::interact(Player* player) {
    if (!player || !isInteractable()) {
        return false;
    }

    LOG_INFO("Terminal interaction: " + getName());

    // Первичная активация (если ещё не активирован)
    if (!m_activated) {
        m_activated = true;
        m_activationTime = 0.0f;
        m_displayingInfo = true;

        // Изменение подсказки после активации
        setInteractionHint("Press E to view terminal data");

        // Добавляем звуковой эффект при первой активации (в будущем)
        // if (sound system implemented) playSound("terminal_activate");

        LOG_INFO("Terminal " + getName() + " activated for the first time");
    }
    else {
        // Если уже активирован, просто показываем информацию
        m_displayingInfo = true;
        LOG_INFO("Terminal " + getName() + " accessed again");
    }

    // Отмечаем терминал как прочитанный (скрываем индикатор)
    markAsRead();

    // Вызываем функцию обратного вызова, если она задана
    if (m_activationCallback) {
        m_activationCallback(player, this);
    }

    return InteractiveObject::interact(player);
}

void Terminal::update(float deltaTime) {
    // Обновляем базовый интерактивный объект
    InteractiveObject::update(deltaTime);

    // Если терминал активирован, увеличиваем время с момента активации
    if (m_activated) {
        m_activationTime += deltaTime;
    }

    // Если терминал отображает информацию, проверяем, не пора ли скрыть её
    if (m_displayingInfo) {
        // Информация отображается в течение 5 секунд
        if (m_activationTime > 5.0f) {
            m_displayingInfo = false;
        }
    }

    // Если терминал активирован, можно добавить визуальные эффекты
    // Например, пульсацию цвета или мигание (в будущем)
}

void Terminal::displayInfo(SDL_Renderer* renderer, TTF_Font* font, int x, int y) {
    if (!m_displayingInfo || !renderer || !font) {
        return;
    }

    // Здесь будет код для отображения информации терминала
    // В полной реализации он будет использовать систему UI
    // и отображать записи m_entries

    // Временная заглушка
    LOG_INFO("Terminal '" + getName() + "' displaying info");
}

void Terminal::addEntry(const std::string& title, const std::string& content) {
    m_entries.push_back({ title, content });
    LOG_INFO("Entry added to terminal '" + getName() + "': " + title);
}

void Terminal::setActivationCallback(std::function<void(Player*, Terminal*)> callback) {
    m_activationCallback = callback;
}

std::string Terminal::getIndicatorSymbol() const {
    // Более крупные символы, чтобы они были заметнее
    switch (m_terminalType) {
    case TerminalType::RESEARCH_SENSOR:
        return "?";  // Знак вопроса для исследовательских данных
    case TerminalType::ANCIENT_CONSOLE:
        return "!";  // Восклицательный знак для древней технологии
    case TerminalType::EMERGENCY_BEACON:
        return "*";  // Звездочка для экстренных сообщений
    case TerminalType::SCIENCE_STATION:
        return "+";  // Плюс для научной станции
    default:
        return "?";  // По умолчанию знак вопроса
    }
}

void Terminal::selectRandomEntry() {
    // Если записей нет, нечего выбирать
    if (m_entries.empty()) {
        m_selectedEntryIndex = -1;
        return;
    }

    // Определяем стартовый индекс (пропускаем первую запись, если она имеет то же имя, что и терминал)
    int startIndex = 0;
    if (!m_entries.empty() && m_entries[0].first == getName()) {
        startIndex = 1;
    }

    // Если есть доступные записи для выбора
    if (startIndex < m_entries.size()) {
        // Выбираем случайную запись из доступных (исключая последнюю предупреждающую)
        int numRegularEntries = m_entries.size() - 1;
        if (numRegularEntries > startIndex) {
            m_selectedEntryIndex = startIndex + (rand() % (numRegularEntries - startIndex));
        }
        else {
            m_selectedEntryIndex = startIndex;
        }
    }
    else {
        m_selectedEntryIndex = -1;
    }
}
