#ifndef E_WIDGET_RANGE_H
#define E_WIDGET_RANGE_H

#include "e_widget.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    EWidget widget;
    EWidget range;
    float min;
    float max;
    float *dest;
    vec2 rangePos;
    vec4 selfColor;
} EWidgetRange;

void RangeWidgetInit(EWidgetRange *range, vec2 scale, float min, float max, EWidget *parent);
void RangeWidgetSetValueDestin(EWidgetRange *range, float *val_dest);
void RangeWidgetSetValue(EWidgetRange *range, float val);

#ifdef __cplusplus
}
#endif

#endif // E_WIDGET_RANGE_H
