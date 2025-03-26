#ifndef E_WIDGET_BUTTON_H
#define E_WIDGET_BUTTON_H

#include "Variabels/engine_includes.h"

#include "e_widget.h"
#include "e_widget_image.h"

#include <wchar.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    EWidget widget;   
    char text[256]; 
    vec4 selfColor;
} EWidgetButton;

void ButtonWidgetInit(EWidgetButton *button, vec2 scale, const char *text, EWidget *parent);
void ButtonWidgetSetText(EWidgetButton *button, const char *text);
void ButtonWidgetSetColor(EWidgetButton *button, float r, float g, float b);

#ifdef __cplusplus
}
#endif

#endif // E_WIDGET_BUTTON_H
