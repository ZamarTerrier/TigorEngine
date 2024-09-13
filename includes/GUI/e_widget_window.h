#ifndef E_WIDGET_WINDOW_H
#define E_WIDGET_WINDOW_H

#include "Variabels/engine_includes.h"

#include "e_widget.h"
#include "e_widget_button.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    EWidget window;
    EWidget surface;
    EWidgetButton hide;
    EWidgetButton resize;
    EWidgetButton close;
    char name[256];
    bool wasResize;
    bool wasHide;
    bool resizeble;
    vec2 lastSize;
    vec2 lastPos;
    vec2 origScale;
}EWidgetWindow;

void WindowWidgetInit(EWidgetWindow *window, char* name, vec2 siz, vec2 position);
void WindowWidgetShow(EWidgetWindow *window);
void WindowWidgetHide(EWidgetWindow *window);
void WindowWidgetUpdate(EWidgetWindow *window);
void WindowWidgetDraw(EWidgetWindow *window);
void WindowWidgetDestroy(EWidgetWindow *window);
void WindowWidgetAddWidget(EWidgetWindow *window, EWidget *widget);

EWidget *WindowWidgetGetSurface(EWidgetWindow *window);

#ifdef __cplusplus
}
#endif

#endif // E_WIDGET_WINDOW_H
