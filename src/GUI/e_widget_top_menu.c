#include "GUI/e_widget_top_menu.h"
#include "GUI/e_widget_button.h"

#include "Data/e_resource_data.h"

extern ZEngine engine;

int TopMenuWidgetFocus(EWidget *widget, void *entry, void *args)
{
    EWidget *menu= widget;

    if(menu->type != ENGINE_WIDGET_TYPE_MENU)
    {
        menu = menu->parent;

        while(menu->type != ENGINE_WIDGET_TYPE_MENU)
        {
            menu = menu->parent;

            if(menu == NULL)
                return 1;
        }
    }

    int iter = 0;
    ChildStack *child = WidgetFindChild(menu, iter);
    EWidget *temp_w;

    while (child != NULL) {

        temp_w = child->node;

        if(temp_w->type == ENGINE_WIDGET_TYPE_LIST)
            temp_w->widget_flags &= ~(ENGINE_FLAG_WIDGET_VISIBLE);

        child = WidgetFindChild(menu, iter);

        iter ++;
    }

    return 0;
}

int ToggleMenu(EWidget *widget, void *entry, EWidgetList *list)
{
    EWidget *menu = widget->parent;

    while(menu->type != ENGINE_WIDGET_TYPE_MENU)
    {
        menu = menu->parent;

        if(menu == NULL)
            return 1;
    }

    int iter = 0;
    ChildStack *child = WidgetFindChild(menu, iter);
    EWidget *temp_w;

    while (child != NULL) {

        temp_w = child->node;

        if(temp_w->type == ENGINE_WIDGET_TYPE_LIST && temp_w != list)
            temp_w->widget_flags &= ~(ENGINE_FLAG_WIDGET_VISIBLE);

        child = WidgetFindChild(menu, iter);

        iter ++;
    }

    /*if(!(list->widget.widget_flags & ENGINE_FLAG_WIDGET_VISIBLE))
        list->widget.widget_flags |= ENGINE_FLAG_WIDGET_VISIBLE;
    else
        list->widget.widget_flags &= ~(ENGINE_FLAG_WIDGET_VISIBLE);*/

    return 0;
}

int MenuPressItem(EWidget *widget, int id, void *arg)
{
    EWidget *menu = widget->parent;

    while(menu->type != ENGINE_WIDGET_TYPE_MENU)
    {
        menu = menu->parent;

        if(menu == NULL)
            return 1;
    }

    int iter = 0;
    ChildStack *child = WidgetFindChild(menu, iter);
    EWidget *temp_w;

    while (child != NULL) {

        temp_w = child->node;

        if(temp_w->type == ENGINE_WIDGET_TYPE_LIST)
            temp_w->widget_flags &= ~(ENGINE_FLAG_WIDGET_VISIBLE);

        child = WidgetFindChild(menu, iter);

        iter ++;
    }

    MenuData data;
    data.num_menu = WidgetFindIdChild(widget) - 1;
    data.elem_id = id;

    WidgetConfirmTrigger(menu, ENGINE_WIDGET_TRIGGER_MENU_PRESS_ITEM, &data);

    return 0;
}

void TopMenuWidgetResize(EWidgetTopMenu *top_menu)
{
    if(top_menu->window != NULL)
    {
        vec2 size = Transform2DGetScale((struct GameObject2D_T *)&top_menu->window->surface);

        Transform2DSetScale((struct GameObject2D_T *)&top_menu->widget, size.x, size.y);
        Transform2DSetScale((struct GameObject2D_T *)&top_menu->top, size.x, 20);
        Transform2DSetPosition((struct GameObject2D_T *)&top_menu->top, 0, 0);
        Transform2DSetPosition((struct GameObject2D_T *)&top_menu->widget, 0, 20);
    }else{
        Transform2DSetScale((struct GameObject2D_T *)&top_menu->widget, engine.width, engine.height);
        Transform2DSetScale((struct GameObject2D_T *)&top_menu->top, engine.width, 20);
        Transform2DSetPosition((struct GameObject2D_T *)&top_menu->top, 0, 0);
        Transform2DSetPosition((struct GameObject2D_T *)&top_menu->widget, 0, 0);
    }
}

void TopMenuWidgetInit(EWidgetTopMenu *top_menu, DrawParam *dParam,EWidgetWindow *window)
{
    
    if(!GUIManagerIsInit())
        return;
        
    WidgetInit(&top_menu->widget, (EWidget *)window);

    top_menu->widget.type = ENGINE_WIDGET_TYPE_MENU;

    WidgetInit(&top_menu->top, &top_menu->widget);
    top_menu->widget.transparent = 0.0f;
    top_menu->window = window;

    memset(top_menu->list, 0, 256 * sizeof(LinkedButtonList));
    top_menu->num_elems = 0;

    TopMenuWidgetResize(top_menu);

    WidgetConnect(&top_menu->widget, ENGINE_WIDGET_TRIGGER_WIDGET_FOCUS, TopMenuWidgetFocus, NULL);

    top_menu->render = dParam->render;
}

int TopMenuWidgetAddMenu(EWidgetTopMenu *top_menu, char *name, DrawParam *dParam)
{
    top_menu->num_elems ++;

    EWidgetButton *b_menu = AllocateMemory(1, sizeof(EWidgetButton));
    //ButtonWidgetInit(b_menu, name, &top_menu->top);

    Transform2DSetPosition((struct GameObject2D_T *)b_menu, (top_menu->num_elems - 1) * 240, 0);
    Transform2DSetScale((struct GameObject2D_T *)b_menu, 120, 20);

    top_menu->list[top_menu->num_elems - 1].button = b_menu;

    return top_menu->num_elems - 1;
}

EWidget *TopMenuWidgetAddItem(EWidgetTopMenu *top_menu, int num_menu, char *name, DrawParam *dParam)
{
    EWidgetButton *button;

    if(top_menu->list[num_menu].list != NULL)
    {
        //button = ListWidgetAddItem(top_menu->list[num_menu].list, name, dParam);

        return (EWidget *)top_menu->list[num_menu].list;
    }

    EWidgetList *l_menu = AllocateMemory(1, sizeof(EWidgetList));

    EWidget *point;

    if(top_menu->window == NULL)
        point = &top_menu->widget;
    else
        point = (EWidget *)top_menu->window;

    //ListWidgetInit(l_menu, 110, 20, dParam, point);
    ListWidgetSetColor(l_menu, (vec3){ 0.6, 0.6, 0.6});

    vec2 pos = Transform2DGetPosition((struct GameObject2D_T *)top_menu->list[top_menu->num_elems - 1].button);
    Transform2DSetPosition((struct GameObject2D_T *)l_menu, pos.x, 40);
    //l_menu->widget.widget_flags &= ~(ENGINE_FLAG_WIDGET_VISIBLE);
    top_menu->list[num_menu].list = l_menu;

    //button = ListWidgetAddItem(top_menu->list[num_menu].list, name, dParam);

    WidgetConnect((EWidget *)top_menu->list[top_menu->num_elems - 1].list, ENGINE_WIDGET_TRIGGER_LIST_PRESS_ITEM, (widget_callback)MenuPressItem, NULL);
    WidgetConnect((EWidget *)top_menu->list[top_menu->num_elems - 1].button, ENGINE_WIDGET_TRIGGER_BUTTON_PRESS, (widget_callback)ToggleMenu, (void *)top_menu->list[top_menu->num_elems - 1].list);

    return (EWidget *)top_menu->list[num_menu].list;
}
