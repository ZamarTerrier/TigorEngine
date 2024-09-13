#ifndef E_WIDGET_ROLLER_H
#define E_WIDGET_ROLLER_H

#include "e_widget.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    EWidget widget;
    bool vertical;
    float move_val;
    float stable_val;
    vec3 selfColor;
} EWidgetRoller;

void RollerWidgetInit(EWidgetRoller *roller, vec2 scale, EWidget *parent);

#ifdef __cplusplus
}
#endif

#endif // E_WIDGET_ROLLER_H
