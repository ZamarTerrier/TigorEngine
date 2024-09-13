#include "GUI/e_widget_list.h"

#include "Tools/e_math.h"

int max_size = 2;

int ListWidgetPressItem(EWidget *widget, void *entry, int id){

    EWidgetList *list = (EWidgetList *)widget->parent;

    EWidgetButton *button;

    EWidget *parent = widget->parent;

    ChildStack *child = parent->child;

    while(child != NULL)
    {
        button = child->node;

        ButtonWidgetSetColor(button, parent->color.x, parent->color.y, parent->color.z);

        child = child->next;
    }

    button = (EWidgetButton *)widget;

    ButtonWidgetSetColor(button, parent->color.x + 0.6, parent->color.y, parent->color.z);

    WidgetConfirmTrigger((EWidget *)list, ENGINE_WIDGET_TRIGGER_LIST_PRESS_ITEM, (void *)id);

    return -1;
}

void ListWidgetDraw(EWidgetList *list){

    if(list->widget.widget_flags & ENGINE_FLAG_WIDGET_VISIBLE){

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

    item->widget.widget_flags |= ENGINE_FLAG_WIDGET_ALLOCATED;
    item->widget.rounding = 0.f;

    ButtonWidgetSetColor(item, list->widget.color.x, list->widget.color.y, list->widget.color.z);

    WidgetConnect((EWidget *)item, ENGINE_WIDGET_TRIGGER_BUTTON_PRESS, (widget_callback)ListWidgetPressItem, (void *)list->size);

    list->size ++;
    return item;
}

void ListWidgetRemoveItem(EWidgetList *list, int num){

    if(num + 1 > list->size)
        return;

    ChildStack *child = list->widget.child;

    if(child == NULL)
        return;

    /*if(child->next != NULL && child->before != NULL)
    {
        ChildStack *next = child->next;
        ChildStack *before = child->before;

        WidgetDestroy(child->node);
        child->node = NULL;

        FreeMemory(child);
        next->before = before;
        before->next = next;

    }else if(child->next != NULL){
        WidgetDestroy(child->node);
        child->node = NULL;

        child->next->before = NULL;
        list->widget.child = child->next;
        FreeMemory(child);

    }else if(child->before != NULL){
        WidgetDestroy(child->node);
        child->node = NULL;

        child->before->next = NULL;
        list->widget.last = child->before;

        FreeMemory(child);

    }else{
        if(list->widget.child->node != NULL)
        {
            WidgetDestroy(list->widget.child->node);
            list->widget.child->node = NULL;
        }

        if(list->widget.child != NULL)
        {
            FreeMemory(list->widget.child);
            list->widget.last = NULL;
            list->widget.child = NULL;
        }
    }
*/
    /*list->size--;

    Transform2DSetScale(&list->widget, list->size_x, list->size_y * list->size);

    for(int i=0;i < list->size;i++)
    {
        child = WidgetFindChild(&list->widget, i);

        EWidget* widget = child->node;
        widget->callbacks.stack[2].args = i;
        Transform2DSetPosition(child->node, 0, i * (list->size_y * 2));
        Transform2DSetScale(child->node, list->size_x, list->size_y);
    }*/
}
