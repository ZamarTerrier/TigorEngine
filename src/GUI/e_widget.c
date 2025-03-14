#include "GUI/e_widget.h"
#include <vulkan/vulkan.h>

#include "wManager/window_manager.h"
#include "wManager/manager_includes.h"

#include "TigorGUI.h"

#include "GUI/GUIManager.h"

#include "Tools/e_math.h"
#include "Tools/e_shaders.h"

#include "Core/engine.h"
#include "Core/e_window.h"
#include "Core/pipeline.h"
#include "Core/e_buffer.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_shapes.h"

extern TEngine engine;
extern GUIManager gui;

bool e_var_wasReleased = true, e_var_leftMouse = false;

EWidget* e_var_sellected = NULL;
EWidget* e_var_last_sellected = NULL;

void WidgetAcceptStack(EWidget* ew){
    
    if(gui.first_widget->node == NULL){
        gui.first_widget->next = AllocateMemory(1, sizeof(ChildStack));
        gui.first_widget->next->before = gui.first_widget;
        gui.first_widget->node = ew; 
        gui.last_widget = gui.first_widget;
    }
    else{

        ChildStack *child = gui.first_widget;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = AllocateMemory(1, sizeof(ChildStack));
        child->next->before = child;
        child->node = ew; 

        gui.last_widget = child;
    }
}

void WidgetRemoveStack(EWidget* ew){

    ChildStack *child = gui.first_widget;
    ChildStack *before = NULL;

    while(child != NULL){

        if(child->node != NULL)
            if(child->node == ew)
                break;
        
        before = child;
        child = child->next;
    }

    if(child == NULL)
        return;

    if(child->next != NULL){
        if(before != NULL){
            child->next->before = before;
            before->next = child->next;

            if(child->next->node == NULL)
                gui.last_widget = before;

        }
        else{
            child->next->before = NULL;
            gui.first_widget = child->next;
        }
    }else{
        if(before != NULL){
            before->next = NULL;
            gui.last_widget = before;
        }
    }

    if((ew->widget_flags & TIGOR_FLAG_WIDGET_ALLOCATED))
        FreeMemory(ew);
        
    child->node = NULL;
    FreeMemory(child);
}

void WidgetSetParent(EWidget* ew, EWidget* parent){

    ew->parent = parent;
    
    if(parent != NULL)
    {
        if(parent->child != NULL)
        {
            ChildStack *child = parent->child;

            while(child->next != NULL)
            {
                child = child->next;
            }

            child->next = (ChildStack *)AllocateMemory(1, sizeof(ChildStack));
            child->next->before = child;
            child->next->node = ew;

            parent->last = child->next;

        }else{
            parent->child = (ChildStack *)AllocateMemory(1, sizeof(ChildStack));
            parent->child->next = NULL;
            parent->child->before = NULL;
            parent->child->node = ew;
            parent->last = parent->child;
        }
    }
}

int WidgetFindIdChild(EWidget* widget)
{
    EWidget *parent = widget->parent;

    ChildStack *child = parent->child;
    int counter = 0;

    while(child != NULL && child->node != widget)
    {
        counter ++;
        child = child->next;
    }

    if(child == NULL && counter == 0)
        return -1;

    return counter;
}

ChildStack *WidgetFindChild(EWidget* widget, int num)
{
    ChildStack *child = widget->child;
    int counter = 0;

    if(child != NULL)
    {
        while(child->next != NULL && counter < num)
        {
            counter ++;
            child = child->next;
        }

        if(counter != num)
            return NULL;
    }

    return child;
}

void WidgetSetColor(EWidget* ew, vec3 color){
    
    ew->color = color;
}

void WidgetSetScale(EWidget* ew, float xscale, float yscale){
    
    ew->scale.x = xscale;
    ew->scale.y = yscale;
}

void WidgetSetPosition(EWidget* ew, float xpos, float ypos){
    
    ew->position.x = xpos;
    ew->position.y = ypos;
}

void WidgetSetBase(EWidget* ew, float xpos, float ypos){
    
    ew->base.x = xpos;
    ew->base.y = ypos;
}

void WidgetUpdate(EWidget* ew){
    
}

void WidgetDraw(EWidget* ew){

    if(ew->widget_flags & TIGOR_FLAG_WIDGET_VISIBLE){
        GUIAddRectFilled(v2_add(ew->base, ew->position), v2_add(v2_add(ew->position, ew->scale), ew->base), ew->color, 5, GUIDrawFlags_RoundCornersAll);
    }    
}

void WidgetInit(EWidget* ew, EWidget* parent){
    
    if(!GUIManagerIsInit())
        return;

    memset(ew, 0, sizeof(EWidget));

    ew->type = TIGOR_WIDGET_TYPE_WIDGET;

    GameObjectSetUpdateFunc((GameObject *)ew, (void *)WidgetUpdate);
    GameObjectSetDrawFunc((GameObject *)ew, (void *)WidgetDraw);
    GameObjectSetDestroyFunc((GameObject *)ew, (void *)WidgetDestroy);

    WidgetSetColor(ew, vec3_f(0.2, 0.2, 0.2));

    ew->offset.x = 0;
    ew->offset.y = 0;
    ew->transparent = 1.0f;
    ew->rounding = 0.f;
    ew->child = NULL;

    WidgetSetParent(ew, parent);

    WidgetAcceptStack(ew);

    ew->widget_flags = TIGOR_FLAG_WIDGET_ACTIVE | TIGOR_FLAG_WIDGET_VISIBLE | TIGOR_FLAG_WIDGET_SELF_VISIBLE;

    ew->callbacks.stack = (CallbackStruct *) AllocateMemory(MAX_GUI_CALLBACKS, sizeof(CallbackStruct));
    ew->callbacks.size = 0;
    
    
    ew->go.flags |= TIGOR_GAME_OBJECT_FLAG_INIT;
}

void WidgetConnect(EWidget* widget, int trigger, widget_callback callback, void* args){

    if(widget->callbacks.size + 1 >= MAX_GUI_CALLBACKS)
    {
        printf("Слишком много калбэков!\n");
        return;
    }

    widget->callbacks.stack[widget->callbacks.size].args = args;
    widget->callbacks.stack[widget->callbacks.size].func = callback;
    widget->callbacks.stack[widget->callbacks.size].trigger = trigger;

    widget->callbacks.size ++;
}

void WidgetConfirmTrigger(EWidget* widget, int trigger, void *entry){
    int res = 0;

    for(int i=0;i < widget->callbacks.size;i++)
    {
        if(widget->callbacks.stack[i].trigger == trigger)
        {
            widget_callback temp = widget->callbacks.stack[i].func;
            res = temp(widget, entry,  widget->callbacks.stack[i].args);

            if(res < 0)
                return;
        }
    }
}

int WidgetCheck(EWidget *widget){
    
    TWindow *window = (TWindow *)engine.window;

    if(widget == NULL)
        return 0;
        
    if(!(widget->widget_flags & TIGOR_FLAG_WIDGET_ACTIVE) || !(widget->widget_flags & TIGOR_FLAG_WIDGET_VISIBLE))
        return 0;

    double xpos, ypos;

    wManagerGetCursorPos(window->e_window, &xpos, &ypos);

    xpos *= 2;
    ypos *= 2;

    if(xpos > (widget->position.x + widget->base.x) && xpos < (widget->position.x + widget->base.x + widget->scale.x) &&
            ypos > (widget->position.y + widget->base.y) && ypos < (widget->position.y + widget->base.y + widget->scale.y)){
                return true;
            }

    widget->widget_flags |= TIGOR_FLAG_WIDGET_OUT;

    return false;
}

EWidget* WidgetCheckMouseInner(ChildStack* child){

    ChildStack* next = child->before;    

    EWidget *widget = child->node;
    EWidget *lastfind = NULL;

    if(widget == NULL)
        return NULL;
        
    while((!(widget->widget_flags & TIGOR_FLAG_WIDGET_ACTIVE) || !(widget->widget_flags & TIGOR_FLAG_WIDGET_VISIBLE)) && next != NULL){

        widget = next->node;

        next = next->before;
    }
    
    if(widget == NULL)
        return NULL;

    if(WidgetCheck(widget))
        return widget;

    while(next != NULL)
    {
        widget = next->node;

        if(widget != NULL)
        {
            if(WidgetCheck(widget)){
                widget->widget_flags |= TIGOR_FLAG_WIDGET_IN;
                return widget;
            }
        }

        next = next->before;
    }


    return NULL;
}


void WidgetEventsPipe(ChildStack *child)
{
    if(child == NULL)
        return;

    TWindow *window = (TWindow *)engine.window;

    EWidget *widget = child->node;

    if(e_var_wasReleased && e_var_sellected == NULL){
        e_var_sellected = WidgetCheckMouseInner(child);
    }

    if(e_var_sellected != NULL)
    {      
        widget->widget_flags |= TIGOR_FLAG_WIDGET_IN;

        if((e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_WAS_OUT) && !(e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_WAS_IN))
            WidgetConfirmTrigger(e_var_sellected, TIGOR_WIDGET_TRIGGER_MOUSE_IN, NULL);
        else if(!(e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_WAS_OUT) && (e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_WAS_IN))
            WidgetConfirmTrigger(e_var_sellected, TIGOR_WIDGET_TRIGGER_MOUSE_OUT, NULL);
        else if(!(e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_WAS_OUT)&& (e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_WAS_IN))
            WidgetConfirmTrigger(e_var_sellected, TIGOR_WIDGET_TRIGGER_MOUSE_STAY, NULL);

        if(e_var_leftMouse && e_var_wasReleased)
        {
            if(engine.e_var_current_entry != e_var_sellected)
                engine.e_var_current_entry = NULL;

            WidgetConfirmTrigger(e_var_sellected, TIGOR_WIDGET_TRIGGER_MOUSE_PRESS, NULL);
            e_var_wasReleased = false;

            if(e_var_last_sellected != e_var_sellected){

                if(e_var_last_sellected != NULL)
                    WidgetConfirmTrigger(e_var_last_sellected, TIGOR_WIDGET_TRIGGER_WIDGET_UNFOCUS, NULL);

                WidgetConfirmTrigger(e_var_sellected, TIGOR_WIDGET_TRIGGER_WIDGET_FOCUS, NULL);
            }

            e_var_last_sellected = e_var_sellected;
        }
        else if(e_var_leftMouse)
            WidgetConfirmTrigger(e_var_sellected, TIGOR_WIDGET_TRIGGER_MOUSE_MOVE, NULL);
        else if(!e_var_leftMouse && !e_var_wasReleased)
        {
            WidgetConfirmTrigger(e_var_sellected, TIGOR_WIDGET_TRIGGER_MOUSE_RELEASE, NULL);
            e_var_wasReleased = true;
        }else
        {
            e_var_sellected->widget_flags |= (((e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_WAS_OUT) | (e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_OUT)) | \
                                            ((e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_WAS_IN) | (e_var_sellected->widget_flags & TIGOR_FLAG_WIDGET_IN)));
        }

    }

    int state = 0;

    state = wManagerGetMouseButton(window->e_window, TIGOR_MOUSE_BUTTON_LEFT);

    if(state == TIGOR_PRESS)
        e_var_leftMouse = true;
    else
        e_var_leftMouse = false;

    if(e_var_wasReleased){
        e_var_sellected = NULL;
    }

}

void WidgetDestroy(EWidget *widget){

    if(!(widget->go.flags & TIGOR_GAME_OBJECT_FLAG_INIT))
        return;

    ChildStack *child = widget->child;
    ChildStack *next;

    while(child != NULL)
    {      
        next = child->next;

        if(child->node != NULL)
            GameObjectDestroy(child->node);   
            
        FreeMemory(child);          

        child = next;
    }

    FreeMemory(widget->callbacks.stack);

    widget->go.flags &= ~(TIGOR_GAME_OBJECT_FLAG_INIT);

    WidgetRemoveStack(widget);  
}
