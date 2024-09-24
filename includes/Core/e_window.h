#ifndef WINENGINE_H
#define WINENGINE_H

#include "wManager/window_manager.h"

#include "Variabels/engine_includes.h"
#include "e_debuger.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    void* instance;
    void* surface;

    wManagerWindow *e_window;

} TWindow;

bool checkValidationLayerSupport();

const char** getRequiredExtensions();

void InitWindow();

static void framebufferResizeCallback(void* window, int width, int height);

void createInstance();

void createSurface();

vec2 getWindowSize();

#ifdef __cplusplus
}
#endif

#endif
