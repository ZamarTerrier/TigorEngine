/// @file Core/engine.h
#ifndef GAMENGINE_H
#define GAMENGINE_H

#include "Variabels/engine_includes.h"

#ifdef __cplusplus
extern "C"
{
#endif

void EngineCreateSilent();
/// @brief Инициализация вулкана
void EngineInitVulkan();
/// @brief Функция ожидания какого-либо события окна
void EngineWaitEvents();
/// @brief Функция ожидания окончания работы устройства
void EngineDeviceWaitIdle();
/// @brief Функция очистки сменяемы буферов
void EngineCleanupSwapChain();
/// @brief Функция пересоздания сменяемых буферов
void EnginereRecreateSwapChain();
/// @brief Функция создания объектов для синхронизации кадров
void EngineCreateSyncobjects();

#ifdef __cplusplus
}
#endif

#endif
