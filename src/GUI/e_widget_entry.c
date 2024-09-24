#include "GUI/e_widget_entry.h"

#include "Core/e_window.h"

#include "Tools/e_tools.h"

#include "wManager/manager_includes.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern TEngine engine;

bool e_ctrl_press = false, e_c_press = false, e_v_press = false, e_pasted = false, e_copied = false;

int EntryWidgetDestroyDefault(EWidgetEntry *entry){

}

void EntryWidgetInit(EWidgetEntry *entry, int fontSize, DrawParam *dParam, EWidget* parent){

}

void EntryWidgetUpdate(){

}

void EntryWidgetCharacterCallback(void* window, uint32_t codepoint){

}

void EntryWidgetKeyCallback(void* window,  int key, int scancode, int action, int mods){

}

char *EntryWidgetGetText(EWidgetEntry *entry){

}

void EntryWidgetSetText(EWidgetEntry *entry, char *text){

}
