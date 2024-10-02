#include "GUI/e_widget_window.h"

#include "TigorGUI.h"
#include "TigorEngine.h"

#include <vulkan/vulkan.h>

#include "GUI/GUIManager.h"

#include "Tools/e_math.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_shapes.h"

extern TEngine engine;

vec2 e_var_mouse, e_var_temp, e_var_tscale ;

uint32_t button_wind_offset = 8;

int WindowWidgetSetSize(EWidgetWindow* window, float x, float y)
{

    WidgetSetScale((EWidget *)window, x, y);    

    vec2 botSize = window->window.scale;
    botSize.y -= 40;
    botSize.x -= 10;

    WidgetSetScale(&window->surface, botSize.x, botSize.y);

    return 0;
}

int WindowWidgetPress(EWidget* widget, void* entry, void* args)
{

    double xpos, ypos;

    TEngineGetCursorPos(&xpos, &ypos);
    e_var_mouse.x = xpos * 2;
    e_var_mouse.y = ypos * 2;

    e_var_temp = widget->position;
    e_var_tscale = widget->scale;

    return 0;
}

int WindowWidgetMove(EWidget* widget, void* entry, void* args)
{

    EWidgetWindow *window = (EWidgetWindow *)args;

    if(window->wasResize)
        return 1;

    vec2 te;
    double xpos, ypos;

    TEngineGetCursorPos(&xpos, &ypos);
    te.x = xpos * 2;
    te.y = ypos * 2;


    if(e_var_mouse.y > e_var_temp.y + 50 && window->resizeble)
    {
        te = v2_sub(te, e_var_mouse);
        vec2 scale = v2_add(e_var_tscale, te);

        scale.x = scale.x <= 0 ? 100 : scale.x;
        scale.y = scale.y <= 0 ? 20 : scale.y;
        WindowWidgetSetSize(window, scale.x, scale.y);

        window->wasHide = false;
        window->wasResize = false;
    }
    else
    {
        te = v2_sub(te, e_var_mouse);
        te = v2_add(e_var_temp, te);
        WidgetSetPosition(widget, te.x, te.y);
        
        window->origScale = te;
    }

    return 0;
}

int WindowWidgetCloseButton(EWidget* widget, void* entry, void *arg){

    EWidgetWindow *window = (EWidgetWindow *)arg;

    window->window.widget_flags &= ~(TIGOR_FLAG_WIDGET_VISIBLE);

    WidgetConfirmTrigger((EWidget *)window, TIGOR_WIDGET_TRIGGER_WINDOW_CLOSE, NULL);
    
    EWidget *child_widget = NULL;
    ChildStack *child = window->surface.child;

    while(child != NULL)
    {
        child_widget = child->node;

        if(child_widget != NULL)
            child_widget->widget_flags &= ~(TIGOR_FLAG_WIDGET_VISIBLE);
            
        child = child->next;
    }

    child = window->window.child;

    while(child != NULL)
    {
        child_widget = child->node;

        if(child_widget != NULL)
            child_widget->widget_flags &= ~(TIGOR_FLAG_WIDGET_VISIBLE);
            
        child = child->next;
    }

    return 0;
}

int WindowWidgetResizeButton(EWidget* widget, void* entry, void *arg){

    EWidgetWindow *window = (EWidgetWindow *)arg;

    if(!window->resizeble)
        return 1;

    if(!window->wasResize && !window->wasHide)
    {
        window->lastPos = window->window.position;
        window->lastSize = window->window.scale;
    }else if(window->wasResize){
        WidgetSetPosition(&window->window, window->lastPos.x, window->lastPos.y);
        WindowWidgetSetSize(window, window->lastSize.x, window->lastSize.y);
        window->wasHide = false;
        window->wasResize = false;
        return 1;
    }

    window->wasResize = true;

    vec2 size = {engine.width * 2, engine.height * 2};

    WindowWidgetSetSize(window, size.x, size.y);
    WidgetSetPosition(&window->window, 0, 0);


    return 0;
}

int WindowWidgetHideButton(EWidget* widget, void* entry, void *arg){

    EWidgetWindow *window = (EWidgetWindow *)arg;

    if(!window->wasHide && !window->wasResize)
    {
        window->lastPos = window->window.position;
        window->lastSize = window->window.scale;
        
        
        EWidget *widget = NULL;
        ChildStack *child = window->surface.child;

        while(child != NULL)
        {
            widget = child->node;

            if(widget != NULL)
                widget->widget_flags &= ~(TIGOR_FLAG_WIDGET_VISIBLE);
            
            child = child->next;
        }

    }else if(window->wasHide){
        WidgetSetPosition(&window->window, window->lastPos.x, window->lastPos.y);
        WindowWidgetSetSize(window, window->lastSize.x, window->lastSize.y);
        window->wasHide = false;
        window->wasResize = false;

        EWidget *widget = NULL;
        ChildStack *child = window->surface.child;

        while(child != NULL)
        {
            widget = child->node;

            if(widget != NULL)
                widget->widget_flags |= TIGOR_FLAG_WIDGET_VISIBLE;
            
            child = child->next;
        }

        return 1;
    }

    window->wasHide = true;

    WindowWidgetSetSize(window, 200, 40);
    WidgetSetPosition((EWidget *)&widget->parent->go, 60, (engine.height * 2) - 60);

    return 0;
}

extern void WidgetDraw(EWidget* ew);

void WindowWidgetDraw(EWidgetWindow *window){

    if(window->window.widget_flags & TIGOR_FLAG_WIDGET_VISIBLE){

        vec2 pos = v2_add(window->window.position, window->window.base);

        WidgetDraw(&window->window);

        GUIAddText(pos.x + 5, pos.y + 15, vec3_f(0, 0, 0), 9, window->name);

        WidgetSetPosition(&window->surface, pos.x + 5, pos.y + 30);


        vec2 b_pos = v2_add(pos, vec2_f(window->window.scale.x , button_wind_offset));

        WidgetSetPosition((EWidget *)&window->close, b_pos.x - 30, b_pos.y);
        WidgetSetPosition((EWidget *)&window->resize, b_pos.x - 60, b_pos.y);
        WidgetSetPosition((EWidget *)&window->hide, b_pos.x - 90, b_pos.y);

        ChildStack *child = window->surface.child;

        while(child != NULL)
        {
            WidgetSetBase(child->node, window->surface.position.x, window->surface.position.y);
            
            child = child->next;
        }
    }

}

void WindowWidgetDestroy(EWidgetWindow *window){
    ChildStack *child = window->window.child;
    ChildStack *next;

    if(!(window->window.go.flags & TIGOR_GAME_OBJECT_FLAG_INIT))
        return;
    
    while(child != NULL)
    {
        next = child->next;   

        GameObjectDestroy(child->node);

        FreeMemory(child);
    
        child = next;
    }
    
    FreeMemory(window->window.callbacks.stack);
    
    window->window.go.flags &= ~(TIGOR_GAME_OBJECT_FLAG_INIT);

    WidgetRemoveStack(window);  
}


void WindowWidgetInitWindow(EWidget* widget, vec2 scale, vec2 position){

    WidgetInit(widget, NULL);
    GameObjectSetDrawFunc((GameObject *)widget, (void *)WindowWidgetDraw);
    GameObjectSetDestroyFunc((GameObject *)widget, (void *)WindowWidgetDestroy);

    widget->type = TIGOR_WIDGET_TYPE_WINDOW;

    WidgetSetColor(widget, vec3_f(1, 1, 1));
    WidgetSetScale(widget, scale.x, scale.y);
    WidgetSetPosition(widget, position.x, position.y);
}

void InitName(EWidgetWindow *window, char* name, EWidget *parent)
{
    int len = strlen(name);

    memset(window->name, 0, 256);
    memcpy(window->name, name, len);
}

void InitSurface(EWidget* widget, vec2 scale, EWidget *parent){

    WidgetInit(widget, parent);

    WidgetSetColor(widget, vec3_f(0.5f, 0.5f, 0.5f));

    vec2 botSize = scale;
    botSize.y -= 40;
    botSize.x -= 10;

    WidgetSetScale(widget, botSize.x, botSize.y);
}

void InitClose(EWidget* widget, vec2 size, EWidget *parent){

    ButtonWidgetInit((EWidgetButton *)widget, vec2_f(20, 20), NULL, parent);
    
    widget->rounding = 5.0f;

    ButtonWidgetSetColor((EWidgetButton *)widget, 1.0f, 0.0f, 0.0);
}

void InitResize(EWidget* widget, vec2 size, EWidget *parent){
    
    ButtonWidgetInit((EWidgetButton *)widget, vec2_f(20, 20), NULL, parent);
    
    widget->rounding = 5.0f;
    
    ButtonWidgetSetColor((EWidgetButton *)widget, 0.0f, 1.0f, 0.0f);
}

void InitHide(EWidget* widget, vec2 size, EWidget *parent){

    ButtonWidgetInit((EWidgetButton *)widget, vec2_f(20, 20), NULL, parent);

    widget->rounding = 5.0f;

    ButtonWidgetSetColor((EWidgetButton *)widget, 0.0f, 0.0f, 1.0f);
}

void WindowWidgetInit(EWidgetWindow *window, char* name, vec2 size, vec2 position)
{
    if(!GUIManagerIsInit())
        return;

    WindowWidgetInitWindow(&window->window, size, position);
    InitSurface(&window->surface, size, &window->window);
    InitName(window, name, &window->window);

    InitClose((EWidget *)&window->close, size, &window->window);
    InitResize((EWidget *)&window->resize, size, &window->window);
    InitHide((EWidget *)&window->hide, size, &window->window);

    window->origScale = size;

    WidgetConnect(&window->window, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, WindowWidgetPress, NULL);
    WidgetConnect(&window->window, TIGOR_WIDGET_TRIGGER_MOUSE_MOVE, WindowWidgetMove, window);

    WidgetConnect((EWidget *)&window->close, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, WindowWidgetCloseButton, window);
    WidgetConnect((EWidget *)&window->resize, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, WindowWidgetResizeButton, window);
    WidgetConnect((EWidget *)&window->hide, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, WindowWidgetHideButton, window);

    window->window.type = TIGOR_WIDGET_TYPE_WINDOW;
    window->wasHide = false;
    window->wasResize = false;
    window->resizeble = true;
}

void WindowWidgetAddWidget(EWidgetWindow *window, EWidget *widget){

    WidgetSetParent((EWidget *)widget, &window->surface);
}

void WindowWidgetShow(EWidgetWindow *window){
    window->window.widget_flags |= TIGOR_FLAG_WIDGET_VISIBLE;

    WidgetConfirmTrigger((EWidget *)window, TIGOR_WIDGET_TRIGGER_WINDOW_OPEN, NULL);

    EWidget *widget = NULL;
    ChildStack *child = window->surface.child;

    while(child != NULL)
    {
        widget = child->node;

        if(widget != NULL)
            widget->widget_flags |= TIGOR_FLAG_WIDGET_VISIBLE;
            
        child = child->next;
    }
    
    child = window->window.child;

    while(child != NULL)
    {
        widget = child->node;

        if(widget != NULL)
            widget->widget_flags |= TIGOR_FLAG_WIDGET_VISIBLE;
            
        child = child->next;
    }

}

void WindowWidgetHide(EWidgetWindow *window){
    window->window.widget_flags &= ~(TIGOR_FLAG_WIDGET_VISIBLE);

    WidgetConfirmTrigger((EWidget *)window, TIGOR_WIDGET_TRIGGER_WINDOW_CLOSE, NULL);

    EWidget *widget = NULL;
    ChildStack *child = window->surface.child;

    while(child != NULL)
    {
        widget = child->node;

        if(widget != NULL)
            widget->widget_flags &= ~(TIGOR_FLAG_WIDGET_VISIBLE);
            
        child = child->next;
    }
    
    child = window->window.child;

    while(child != NULL)
    {
        widget = child->node;

        if(widget != NULL)
            widget->widget_flags &= ~(TIGOR_FLAG_WIDGET_VISIBLE);
            
        child = child->next;
    }
}

EWidget *WindowWidgetGetSurface(EWidgetWindow *window){
    return &window->surface;
}