#include "GUI/e_widget_roller.h"
#include "GUI/GUIManager.h"

#include "TigorGUI.h"
#include "TigorEngine.h"

#include "Tools/e_math.h"

vec2 roller_mouse;

int RollerMousePress(EWidget *widget, void *entry, void *args)
{
    EWidgetRoller *roller = (EWidgetRoller *)widget;

    double xpos, ypos;

    TEngineGetCursorPos(&xpos, &ypos);
    roller_mouse.x = xpos;
    roller_mouse.y = ypos;

    roller->widget.color.x = roller->selfColor.x - 0.2f;
    roller->widget.color.y = roller->selfColor.y - 0.2f;
    roller->widget.color.z = roller->selfColor.z - 0.2f;

    return 0;
}

int RollerWidgetRelease(EWidget* widget, void* entry, void* args){

    EWidgetRoller *roller = (EWidgetRoller *)widget;

    roller->widget.color = roller->selfColor;
}

int RollerMouseMove(EWidget *widget, void *entry, void *args)
{
    EWidgetRoller *roller = (EWidgetRoller *)widget;

    double xpos, ypos;
    vec2 te;

    TEngineGetCursorPos(&xpos, &ypos);
    te.x = xpos;
    te.y = ypos;

    te = v2_sub(te, roller_mouse);
    te = v2_divs(te, 2);

    roller->move_val = roller->stable_val + te.y;
    
    WidgetConfirmTrigger((EWidget *)roller, TIGOR_WIDGET_TRIGGER_ROLLER_MOVE, &roller->move_val);

    return 0;
}

void RollerWidgetDraw(EWidgetRoller *roller){

    if(roller->widget.widget_flags & TIGOR_FLAG_WIDGET_VISIBLE){
        
        vec2 pos = v2_add(roller->widget.position, roller->widget.base);

        GUIAddRectFilled(pos, v2_add(pos, roller->widget.scale), roller->widget.color, 5, GUIDrawFlags_RoundCornersAll);
    }    
}

void RollerWidgetInit(EWidgetRoller *roller, vec2 scale, EWidget *parent)
{    
    if(!GUIManagerIsInit())
        return;

    WidgetInit((EWidget *)roller, parent);
    WidgetSetScale((EWidget *)roller, scale.x, scale.y);

    roller->widget.type = TIGOR_WIDGET_TYPE_ROLLER;

    roller->widget.color = roller->selfColor = vec4_f(0.6, 0, 0, 1.0f);
    
    WidgetConnect((EWidget *)roller, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, RollerMousePress, NULL);
    WidgetConnect((EWidget *)roller, TIGOR_WIDGET_TRIGGER_MOUSE_MOVE, RollerMouseMove, NULL);
    WidgetConnect((EWidget *)roller, TIGOR_WIDGET_TRIGGER_MOUSE_RELEASE, RollerWidgetRelease, NULL);
}
