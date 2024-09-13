#include "GUI/e_widget_entry_area.h"

#include "Core/engine.h"
#include "Core/e_window.h"

#include "wManager/window_manager.h"
#include "wManager/manager_includes.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern ZEngine engine;

extern void EntryWidgetPress(EWidget *widget, void *entry, void *arg);
extern void EntryWidgetUnfocus(EWidget *widget, void *entry, void *arg);
extern void EntryWidgetCharInput(EWidget* widget, uint32_t codepoint, void *arg);

bool e_var_backspace = false;

void *EntryAreaWidgetMakeAnotherText(EWidgetEntryArea * area)
{

}
void EntryAreaWidgetMakeAnotherLine(EWidgetEntryArea * area)
{

}

void EntryAreaWidgetMakeDelete(EWidgetEntryArea *area)
{
    
}

void EntryAreaWidgetInsertText(EWidgetEntryArea *area, const char *src)
{    
    
}

int EntryAreaWidgetKeyPressInput(EWidget* widget, int key, void *arg){


    return 0;
}

int EntryAreaWidgetKeyRepeatInput(EWidget* widget, int key, void *arg)
{
    if(key == ENGINE_KEY_BACKSPACE)
        EntryAreaWidgetMakeDelete((EWidgetEntryArea *)widget);

    return 0;
}


void EntryAreaWidgetInit(EWidgetEntryArea *entry, int fontSize, DrawParam *dParam, EWidget* parent){

}
