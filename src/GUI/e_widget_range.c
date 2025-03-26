#include "GUI/e_widget_range.h"
#include "GUI/GUIManager.h"

#include "TigorGUI.h"
#include "TigorEngine.h"


#include "Tools/e_math.h"

vec2 range_mouse, range_temp;

int RangeWidgetPress(EWidget* widget, void* entry, void* args){

    EWidgetRange *range = args;

    double xpos, ypos;

    TEngineGetCursorPos(&xpos, &ypos);
    range_mouse.x = xpos * 2;
    range_mouse.y = ypos * 2;

    range_temp = range->rangePos;

    WidgetSetColor(&range->range, vec4_f(range->selfColor.x- 0.2f, range->selfColor.y - 0.2f, range->selfColor.z - 0.2f, range->selfColor.w));

    return 0;
}

int RangeWidgetRelease(EWidget* widget, void* entry, void* args){

    EWidgetRange *range = args;

    WidgetSetColor(&range->range, range->selfColor);
}

int RangeWidgetMove(EWidget* widget, void* entry, void* args){

    vec2 te;
    double xpos, ypos;

    EWidgetRange *range = (EWidgetRange *)args;

    TEngineGetCursorPos(&xpos, &ypos);
    te.x = xpos * 2;
    te.y = ypos * 2;

    te = v2_sub(te, range_mouse);

    int len = range->max - range->min;

    vec2 size = range->widget.scale;

    te.x += range_temp.x;

    if(te.x < -range->range.scale.x / 2)
        te.x = -range->range.scale.x / 2;

    if(te.x > size.x - range->range.scale.x / 2)
        te.x = size.x - range->range.scale.x / 2;

    float diff =  len / size.x;

    float val = (te.x + range->range.scale.x / 2) * diff + range->min;

    if(range->dest != NULL)
        *range->dest = val;

    range->rangePos.x = te.x;

    WidgetConfirmTrigger((EWidget *)range, TIGOR_WIDGET_TRIGGER_RANGE_CHANGE, &val);

    return 0;
}

void RangeWidgetDraw(EWidgetRange *range){

    if(range->widget.widget_flags & TIGOR_FLAG_WIDGET_VISIBLE){
        
        vec2 pos = v2_add(range->widget.position, range->widget.base);

        GUIAddLine(vec2_f(pos.x, pos.y + range->widget.scale.y / 2),vec2_f(pos.x + range->widget.scale.x, pos.y + range->widget.scale.y / 2), vec4_f(1, 1, 1, 1), 1.0f);

        WidgetSetPosition(&range->range, pos.x + range->rangePos.x, pos.y + range->rangePos.y);

        GUIAddRectFilled(range->range.position, v2_add(range->range.position, range->range.scale), range->range.color, range->range.rounding, GUIDrawFlags_RoundCornersAll);
    }
}

void RangeWidgetInit(EWidgetRange *range, vec2 scale, float min, float max, EWidget *parent){

    if(!GUIManagerIsInit())
        return;
        
    WidgetInit((EWidget *)&range->widget, parent);
    WidgetSetScale((EWidget *)range, scale.x, scale.y);

    GameObjectSetDrawFunc((GameObject *)range, RangeWidgetDraw);

    range->widget.type = TIGOR_WIDGET_TYPE_RANGE;

    WidgetInit((EWidget *)&range->range, &range->widget);
    WidgetSetScale((EWidget *)&range->range, 30, scale.y);
    range->range.rounding = 10.0f;

    range->min = min;
    range->max = max;
    range->dest = NULL;

    range->selfColor = vec4_f(0.6, 0.3, 0.1, 1.0f);
    range->rangePos = vec2_f(0, 0);

    WidgetSetColor((EWidget *)&range->range, range->selfColor);

    WidgetConnect((EWidget *)&range->range, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, RangeWidgetPress, range);
    WidgetConnect((EWidget *)&range->range, TIGOR_WIDGET_TRIGGER_MOUSE_MOVE, RangeWidgetMove, range);
    WidgetConnect((EWidget *)&range->range, TIGOR_WIDGET_TRIGGER_MOUSE_RELEASE, RangeWidgetRelease, range);
}

void RangeWidgetSetValueDestin(EWidgetRange *range, float *val_dest)
{
    range->dest = val_dest;
}

void RangeWidgetSetValue(EWidgetRange *range, float val)
{

    vec2 size = v2_muls(Transform2DGetScale((struct GameObject2D_T *)&range->widget), 2);
    size.x -= 20;

    vec2 te = Transform2DGetPosition((struct GameObject2D_T *)&range->range);

    int len = range->max - range->min;
    float diff =  len / size.x;

    te.x = (val - range->min) / diff;
    Transform2DSetPosition((struct GameObject2D_T *)&range->range, te.x, te.y);
}
