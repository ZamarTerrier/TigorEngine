#include "GUI/e_widget_scroll.h"
#include "GUI/e_widget_list.h"

#include "TigorEngine.h"

#include "Tools/e_math.h"

vec2 scroll_mouse, scroll_temp;

int ScrollWidgetPress(EWidgetScroll *widget, void* entry, void* args){

    double xpos, ypos;

    TEngineGetCursorPos(&xpos, &ypos);
    scroll_mouse.x = xpos;
    scroll_mouse.y = ypos;

    scroll_temp = Transform2DGetPosition((struct GameObject2D_T *)widget);

    return 0;
}

int ScrollWidgetMove(EWidget* widget, void* entry, EWidgetScroll* scroll){

    vec2 te, te2;
    double xpos, ypos;

    TEngineGetCursorPos(&xpos, &ypos);
    te.x = xpos;
    te.y = ypos;

    te = v2_sub(te, scroll_mouse);
    te = v2_add(scroll_temp, v2_muls(te, 2));

    te2 = Transform2DGetPosition((struct GameObject2D_T *)widget);
    te.x = te2.x;

    vec2 size = v2_muls(Transform2DGetScale((struct GameObject2D_T *)scroll), 2);

    size.y -= scroll->scroll_size * 2;

    if(te.y  < 0)
        te.y = 0;

    if(te.y > size.y)
        te.y = size.y;

    float diff = te.y / size.y;

    Transform2DSetPosition((struct GameObject2D_T *)widget, te.x, te.y);

    WidgetConfirmTrigger((EWidget *)scroll, TIGOR_WIDGET_TRIGGER_SCROLL_CHANGE, &diff);

    return 0;
}

void ScrollWidgetInit(EWidgetScroll *scroll, uint32_t width, uint32_t height, EWidget *parent)
{
    if(!GUIManagerIsInit())
        return;
        
    WidgetInit((EWidget *)scroll, parent);

    scroll->widget.type = TIGOR_WIDGET_TYPE_SCROLL;

    scroll->widget.color = (vec3){1.0f, 1.0f, 1.0f};
    Transform2DSetScale((struct GameObject2D_T *)scroll, width, height);

    scroll->widget.transparent = 0.0f;

    WidgetInit(&scroll->scroll, scroll);
    scroll->scroll.color = (vec3){1.0f, 0.0f, 0.0f};

    Transform2DSetScale((struct GameObject2D_T *)&scroll->scroll, 20, 20);
    Transform2DSetPosition((struct GameObject2D_T *)&scroll->scroll, width * 2 - 40, 0);

    WidgetConnect((EWidget *)&scroll->scroll, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, (widget_callback)ScrollWidgetPress, scroll);
    WidgetConnect((EWidget *)&scroll->scroll, TIGOR_WIDGET_TRIGGER_MOUSE_MOVE, (widget_callback)ScrollWidgetMove, scroll);
}

void ScrollWidgetUpdate(EWidgetScroll *scroll, void *list)
{
    EWidgetList *some_list = list;

    vec2 mySize = Transform2DGetScale((struct GameObject2D_T *)scroll);

    float razn = mySize.y / (some_list->size_y * some_list->size);

    if(razn < 1){
        scroll->scroll_size = mySize.y * razn;
        scroll->scroll.widget_flags |= (TIGOR_FLAG_WIDGET_ACTIVE | TIGOR_FLAG_WIDGET_VISIBLE);
    }else{
        scroll->scroll.widget_flags &= ~(TIGOR_FLAG_WIDGET_ACTIVE | TIGOR_FLAG_WIDGET_VISIBLE);
        scroll->scroll_size = mySize.y;
    }

    Transform2DSetScale((struct GameObject2D_T *)&scroll->scroll, 20, scroll->scroll_size);
}

void ScrollWidgetSetScrollSize(EWidgetScroll *scroll, float percent)
{
    vec2 scale = Transform2DGetScale((struct GameObject2D_T *)scroll);

    Transform2DSetScale((struct GameObject2D_T *)&scroll->scroll, 20, 20);

}
