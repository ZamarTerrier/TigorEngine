#ifndef EWIDGET_ENTRY_H
#define EWIDGET_ENTRY_H

#include "e_widget.h"

#define MAX_ENTERY_LENGTH 2048

typedef struct{
    EWidget widget;
    char text[MAX_ENTERY_LENGTH];
    float fontSize;
    int currPos;
} EWidgetEntry;

void EntryWidgetCharacterCallback(void* arg, uint32_t codepoint);
void EntryWidgetKeyCallback(void* arg,  int key, int scancode, int action, int mods);

void EntryWidgetInit(EWidgetEntry *entry, vec2 scale, EWidget *parent);

char *EntryWidgetGetText(EWidgetEntry *entry);
void EntryWidgetSetText(EWidgetEntry *entry, char *text);
void EntryWidgetCleartext(EWidgetEntry *entry);

#endif //EWIDGET_ENTRY_H