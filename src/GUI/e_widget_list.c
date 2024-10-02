#include "GUI/e_widget_list.h"

#include "Tools/e_math.h"

int max_size = 2;

int ListWidgetPressItem(EWidget *widget, void *entry, void *arg){

    EWidgetList *list = (EWidgetList *)widget->parent;

    EWidgetButton *button;

    EWidget *parent = widget->parent;

    ChildStack *child = parent->child;

    uint32_t indx = 0;
    while(child != NULL)
    {
        button = child->node;

        ButtonWidgetSetColor(button, parent->color.x, parent->color.y, parent->color.z);

        child = child->next;
    }

    child = parent->child;
    
    while(child != NULL)
    {
        if(child->node == widget)
            break;

        child = child->next;
        indx++;
    }

    button = (EWidgetButton *)widget;

    ButtonWidgetSetColor(button, parent->color.x + 0.6, parent->color.y, parent->color.z);

    WidgetConfirmTrigger((EWidget *)list, TIGOR_WIDGET_TRIGGER_LIST_PRESS_ITEM, (void *)indx);

    return -1;
}

void ListWidgetDraw(EWidgetList *list){

    if(list->widget.widget_flags & TIGOR_FLAG_WIDGET_VISIBLE){

        ChildStack *child = list->widget.child;

        float ypos = 0;
        vec2 pos = v2_add(list->widget.position, list->widget.base);
        while(child != NULL){

            WidgetSetBase(child->node, pos.x , pos.y + ypos);

            ypos += list->widget.scale.y;

            child = child->next;
        }
    }
}

void ListWidgetInit(EWidgetList *list, vec2 scale, EWidget *parent){

    if(!GUIManagerIsInit())
        return;
        
    WidgetInit((EWidget *)list, parent);
    WidgetSetScale((EWidget *)list, scale.x, scale.y);

    GameObjectSetDrawFunc((GameObject *)list, ListWidgetDraw);

    WidgetSetColor((EWidget *)list, vec3_f(0.4, 0.4, 0.4));
}

void ListWidgetSetColor(EWidgetList *list, vec3 color){

    WidgetSetColor((EWidget *)list, color);

    ChildStack *child = list->widget.child;
    EWidgetButton *button;

    while(child != NULL)
    {
        button = child->node;
        ButtonWidgetSetColor(button, list->widget.color.x, list->widget.color.y, list->widget.color.z);

        child = child->next;
    }
}


EWidgetButton *ListWidgetAddItem(EWidgetList *list, const char *text){

    EWidgetButton *item = (EWidgetButton *) AllocateMemory(1, sizeof(EWidgetButton));

    ButtonWidgetInit(item, list->widget.scale, text, (EWidget *)list);

    item->widget.widget_flags |= TIGOR_FLAG_WIDGET_ALLOCATED;
    item->widget.rounding = 0.f;

    ButtonWidgetSetColor(item, list->widget.color.x, list->widget.color.y, list->widget.color.z);

    WidgetConnect((EWidget *)item, TIGOR_WIDGET_TRIGGER_BUTTON_PRESS, (widget_callback)ListWidgetPressItem, NULL);

    list->size ++;
    return item;
}

void ListWidgetRemoveItem(EWidgetList *list, int num){

    if(num + 1 > list->size)
        return;

    ChildStack *child = list->widget.child, *before = NULL;

    if(child == NULL)
        return;

    uint32_t counter = 0;
    while(child != NULL){

        if(counter == num)
            break;

        before = child;
        child = child->next;

        counter++;
    }

    if(child == NULL)
        return;

    if(child->next != NULL){
        if(before != NULL){
            child->next->before = before;
            before->next = child->next;
        }
        else{
            child->next->before = NULL;
            list->widget.child = child->next;
        }
    }else{
        if(before != NULL){
            before->next = NULL;
        }
    }

    GameObjectDestroy(child->node);
    child->node = NULL;
    FreeMemory(child);
    child = NULL;

    list->size --;

}

void ListWidgetClear(EWidgetList *list){
    ChildStack *child = list->widget.child, *next;

    while(child != NULL){
        next = child->next;
        
        if(child->node != NULL)
            GameObjectDestroy(child->node);

        child->node = NULL;
        FreeMemory(child);
        child = next;
    }

    list->widget.child = NULL;
}
