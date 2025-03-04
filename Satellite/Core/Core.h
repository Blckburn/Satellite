/**
 * @file Core.h
 * @brief Единый заголовочный файл для ядра движка
 */

#pragma once

// Базовые компоненты
#include "Core/Base/Types.h"
#include "Core/Base/Entity.h"
#include "Core/Base/Scene.h"

// Системные компоненты
#include "Core/System/Engine.h"
#include "Core/System/Logger.h"

// Математические компоненты
#include "Core/Math/Vector2.h"

// Утилиты
#include "Core/Utils/MathUtils.h"
#include "Core/Utils/StringUtils.h"

/**
 * @namespace Satellite
 * @brief Основное пространство имен для движка Satellite
 */
namespace Satellite {

/**
 * @brief Инициализация основных компонентов ядра
 * @return true в случае успеха, false при ошибке
 */
inline bool initializeCore() {
    // Инициализация логгера
    Logger::getInstance().initialize(true, "satellite.log");
    LogInfo("Core system initialized");
    return true;
}

/**
 * @brief Завершение работы ядра
 */
inline void shutdownCore() {
    Logger::getInstance().shutdown();
}

} // namespace Satellite