#ifndef E_WIDGET_H
#define E_WIDGET_H

#include "Variabels/engine_includes.h"

#include "Objects/gameObject2D.h"

#define MAX_WIDGET_MASKS 64
#define MAX_WIDGET_DRAW 256

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
    TIGOR_WIDGET_TRIGGER_MOUSE_PRESS,
    TIGOR_WIDGET_TRIGGER_MOUSE_RELEASE,
    TIGOR_WIDGET_TRIGGER_MOUSE_MOVE,
    TIGOR_WIDGET_TRIGGER_MOUSE_IN,
    TIGOR_WIDGET_TRIGGER_MOUSE_OUT,
    TIGOR_WIDGET_TRIGGER_MOUSE_STAY,
    TIGOR_WIDGET_TRIGGER_WIDGET_FOCUS,
    TIGOR_WIDGET_TRIGGER_WIDGET_UNFOCUS,
    TIGOR_WIDGET_TRIGGER_BUTTON_PRESS,
    TIGOR_WIDGET_TRIGGER_COMBOBOX_PRESS,
    TIGOR_WIDGET_TRIGGER_COMBOBOX_CHANGE_SELLECTED_ITEM,
    TIGOR_WIDGET_TRIGGER_LIST_PRESS_ITEM,
    TIGOR_WIDGET_TRIGGER_MENU_PRESS_ITEM,
    TIGOR_WIDGET_TRIGGER_ENTRY_CHAR_INPUT,
    TIGOR_WIDGET_TRIGGER_ENTRY_UPDATE,
    TIGOR_WIDGET_TRIGGER_ENTRY_KEY_PRESS_INPUT,
    TIGOR_WIDGET_TRIGGER_ENTRY_KEY_REPEAT_INPUT,
    TIGOR_WIDGET_TRIGGER_ENTRY_KEY_RELEASE_INPUT,
    TIGOR_WIDGET_TRIGGER_WINDOW_OPEN,
    TIGOR_WIDGET_TRIGGER_WINDOW_CLOSE,
    TIGOR_WIDGET_TRIGGER_RANGE_CHANGE,
    TIGOR_WIDGET_TRIGGER_SCROLL_CHANGE,
    TIGOR_WIDGET_TRIGGER_ROLLER_MOVE,
} EngineWidgetTriggersEnum;

typedef enum{
    TIGOR_WIDGET_TYPE_WIDGET,
    TIGOR_WIDGET_TYPE_TEXT,
    TIGOR_WIDGET_TYPE_BUTTON,
    TIGOR_WIDGET_TYPE_LIST,
    TIGOR_WIDGET_TYPE_ROLLER,
    TIGOR_WIDGET_TYPE_SCROLL,
    TIGOR_WIDGET_TYPE_MENU,
    TIGOR_WIDGET_TYPE_COMBOBOX,
    TIGOR_WIDGET_TYPE_ENTRY,
    TIGOR_WIDGET_TYPE_ENTRY_AREA,
    TIGOR_WIDGET_TYPE_RANGE,
    TIGOR_WIDGET_TYPE_IMAGE,
    TIGOR_WIDGET_TYPE_WINDOW
} EngineWidgetTypeEnum;

typedef enum{
    TIGOR_FLAG_WIDGET_IN = 0x1,
    TIGOR_FLAG_WIDGET_WAS_IN = 0x2,
    TIGOR_FLAG_WIDGET_OUT = 0x4,
    TIGOR_FLAG_WIDGET_WAS_OUT = 0x8,
    TIGOR_FLAG_WIDGET_ACTIVE = 0x10,
    TIGOR_FLAG_WIDGET_VISIBLE = 0x20,
    TIGOR_FLAG_WIDGET_SELF_VISIBLE = 0x40,
    TIGOR_FLAG_WIDGET_ALLOCATED = 0x80,
} EngineWidgetFlag;

typedef struct{
    void* func;
    void *args;
    int trigger;
} CallbackStruct;

typedef struct{
    CallbackStruct* stack;
    int size;
} CallbackStack;

typedef struct EWidget{
    GameObject go;
    vec2 offset;
    vec4 color;
    vec2 base;
    vec2 position;
    vec2 scale;
    float rounding;
    float transparent;
    struct EWidget* parent;
    struct ChildStack* child;
    struct ChildStack* first;
    struct ChildStack* last;
    EngineWidgetFlag widget_flags;
    EngineWidgetTypeEnum type;
    CallbackStack callbacks;
} EWidget;

typedef struct{
    vec2 position;
    vec2 size;
}ObjectParams;

typedef struct{
    vec2 offset;
    vec2 position;
    vec2 size;
    vec4 color __attribute__ ((aligned (16)));
    float transparent __attribute__ ((aligned (16)));
} GUIBuffer;

typedef struct{
    ObjectParams objs[MAX_WIDGET_MASKS];
    int size;
} MaskObjectBuffer;


typedef int(*widget_callback)(EWidget *widget, void *, void*);

void WidgetRemoveStack(EWidget *ew);

int WidgetFindIdChild(EWidget* widget);
ChildStack * WidgetFindChild(EWidget* widget, int num);
void WidgetUpdateScissor(EWidget *widget);
void WidgetUpdateScissorFromParent(EWidget *parent, GameObject2D *child);
void WidgetSetParent(EWidget* ew, EWidget* parent);
void WidgetSetColor(EWidget* ew, vec4 color);
void WidgetSetScale(EWidget* ew, float xscale, float yscale);
void WidgetSetPosition(EWidget* ew, float xpos, float ypos);
void WidgetSetBase(EWidget* ew, float xpos, float ypos);
void WidgetInit(EWidget *ew, EWidget *parent);
void WidgetAddDefault(EWidget *widget, void *render);
void WidgetConfirmTrigger(EWidget* widget, int trigger, void *entry);
void WidgetConnect(EWidget *widget, int trigger, widget_callback callback, void *args);
void WidgetRecreate(EWidget *widget);
void WidgetEventsPipe(ChildStack* child);
void WidgetDraw(EWidget *widget);
void WidgetDestroy(EWidget * widget);

#ifdef __cplusplus
}
#endif

#endif // E_WIDGET_H
