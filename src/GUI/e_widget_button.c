#include "GUI/e_widget_button.h"
#include "GUI/GUIManager.h"

#include "TigorGUI.h"

#include "Tools/e_math.h"

#include "TigorEngine.h"

extern TEngine engine;

int ButtonWidgetPress(EWidget *widget, void* entry, void *arg){
    EWidgetButton *button = (EWidgetButton *)widget;
    
    WidgetSetColor(&button->widget, vec4_f(button->selfColor.x- 0.2f, button->selfColor.y - 0.2f, button->selfColor.z - 0.2f, button->selfColor.w));

    return 0;
}

int ButtonWidgetRelease(EWidget *widget, void* entry, void *arg){

    EWidgetButton *button = (EWidgetButton *)widget;
    
    WidgetSetColor(&button->widget, button->selfColor);

    WidgetConfirmTrigger(widget, TIGOR_WIDGET_TRIGGER_BUTTON_PRESS, NULL);

    return 0;
}

void ButtonWidgetDraw(EWidgetButton *button){
            
    if(button->widget.widget_flags & TIGOR_FLAG_WIDGET_VISIBLE){   

        vec2 pos = v2_add(button->widget.position, button->widget.base);

        GUIAddRectFilled(pos, v2_add(pos, button->widget.scale), button->widget.color, button->widget.rounding, GUIDrawFlags_RoundCornersAll);
        GUIAddText(pos.x, pos.y + button->widget.scale.y / 2, vec4_f(0,0,0, 1.0f), 7, button->text);
    }            
}


void ButtonWidgetInit(EWidgetButton *button, vec2 scale, const char *text, EWidget *parent){

    if(!GUIManagerIsInit())
        return;
        
    memset(button, 0, sizeof(EWidgetButton));

    WidgetInit((EWidget *)button, parent);

    GameObjectSetDrawFunc((GameObject *)button, (void *)ButtonWidgetDraw);

    button->widget.type = TIGOR_WIDGET_TYPE_BUTTON;
    button->widget.rounding = 10.0f;    

    button->selfColor = (vec4){ 1, 1, 1, 1};

    if(text != NULL)
        ButtonWidgetSetText(button, text);

    WidgetSetColor((EWidget *)&button->widget, button->selfColor);
    WidgetSetScale((EWidget *)button, scale.x, scale.y);

    WidgetConnect(&button->widget, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, ButtonWidgetPress, NULL);
    WidgetConnect(&button->widget, TIGOR_WIDGET_TRIGGER_MOUSE_RELEASE, ButtonWidgetRelease, NULL);

}

void ButtonWidgetSetText(EWidgetButton *button, const char *text){
    uint32_t len = strlen(text);

    memset(button->text, 0, 256);
    memcpy(button->text, text, len);
}

void ButtonWidgetSetColor(EWidgetButton *button, float r, float g, float b){
    button->selfColor.x = r;
    button->selfColor.y = g;
    button->selfColor.z = b;
    
    WidgetSetColor(&button->widget, button->selfColor);
}
