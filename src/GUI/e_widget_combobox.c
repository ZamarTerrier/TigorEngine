#include "GUI/e_widget_combobox.h"
#include "GUI/GUIManager.h"

int ComboboxWidgetPressMain(EWidget* widget, void* entry, void *arg){

    EWidgetCombobox *combo = (EWidgetCombobox *)widget;

    WidgetConfirmTrigger((EWidget *)combo, TIGOR_WIDGET_TRIGGER_COMBOBOX_PRESS, NULL);

    combo->show = !combo->show;

    return 0;
}

int ComboboxWidgetPressSub(EWidget* widget, int id, void *arg){

    EWidgetList *list = (EWidgetList *)widget;

    EWidgetCombobox *parent = (EWidgetCombobox *)widget->parent;

    EWidgetButton *butt =(EWidgetButton *) WidgetFindChild((EWidget *)list, id)->node;

    if(parent == NULL)
        return 1;

    ButtonWidgetSetText((EWidgetButton *)parent, butt->text);

    parent->currId = id;

    WidgetConfirmTrigger((EWidget *)parent, TIGOR_WIDGET_TRIGGER_COMBOBOX_CHANGE_SELLECTED_ITEM, (void *)id);

    return 0;
}

extern void ButtonWidgetDraw(EWidgetButton *button);

void ComboboxWidgetDraw(EWidgetCombobox *combobox){
    
    if(combobox->button.widget.widget_flags & TIGOR_FLAG_WIDGET_VISIBLE){
        ButtonWidgetDraw(&combobox->button);

        WidgetSetPosition((EWidget *)&combobox->list, combobox->button.widget.position.x , combobox->button.widget.position.x + combobox->button.widget.scale.y);
    }

}

void ComboboxWidgetInit(EWidgetCombobox *combobox, vec2 scale, EWidget *parent){

    if(!GUIManagerIsInit())
        return;
        
    ButtonWidgetInit((EWidgetButton *)combobox, scale," ", parent);
    ButtonWidgetSetColor((EWidgetButton *)combobox, 0.4, 0.4, 0.4);

    GameObjectSetDrawFunc((GameObject *)combobox, ComboboxWidgetDraw);

    combobox->button.widget.type = TIGOR_WIDGET_TYPE_COMBOBOX;
    combobox->button.widget.rounding = 0.f;

    combobox->size_x = scale.x;
    combobox->size_y = scale.y;
    combobox->currId = -1;
    combobox->show = false;

    ListWidgetInit(&combobox->list, scale, (EWidget *)combobox);
    WidgetConnect((EWidget *)combobox, TIGOR_WIDGET_TRIGGER_BUTTON_PRESS, ComboboxWidgetPressMain,  NULL);
    WidgetConnect((EWidget *)&combobox->list, TIGOR_WIDGET_TRIGGER_LIST_PRESS_ITEM, (widget_callback)ComboboxWidgetPressSub,  NULL);

    combobox->show = false;

}

void ComboboxWidgetAddItem(EWidgetCombobox *combobox, const char* text){
    EWidgetButton *butt = ListWidgetAddItem(&combobox->list, text);

    butt->widget.widget_flags |= TIGOR_FLAG_WIDGET_ALLOCATED;
    butt->widget.rounding = 0.f;
}
