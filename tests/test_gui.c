#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/engine.h>
#include <Core/e_camera.h>

#include <GUI/e_widget_list.h>
#include <GUI/e_widget_button.h>
#include <GUI/e_widget_combobox.h>
#include <GUI/e_widget_roller.h>
#include <GUI/e_widget_range.h>
#include <GUI/e_widget_window.h>
#include <GUI/e_widget_image.h>
#include <GUI/e_widget_entry.h>

#include <Tools/e_math.h>

Camera2D cam2D;
Camera3D cam3D;

EWidgetRoller roller;

EWidgetRange range;

EWidgetWindow window;

EWidgetButton button;

EWidgetImage image;

EWidgetList list;

EWidgetEntry entry;

float source;

void GetValue(EWidget *widget, float *value, void *arg){
    source = *value;
}

int main(){

    TEngineInitSystem(800, 600, "Test");

    TEngineSetFont("res\\RobotoBlack.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    WindowWidgetInit(&window, "test", vec2_f(800, 400), vec2_f(200, 200));

    ListWidgetInit(&list, vec2_f(70, 30), WindowWidgetGetSurface(&window));
    ListWidgetAddItem(&list, "awdawdawd");
    ListWidgetAddItem(&list, "awdawdawd");
    ListWidgetAddItem(&list, "awdawdawd");
    ListWidgetAddItem(&list, "awdawdawd");

    ListWidgetRemoveItem(&list, 0);

    EntryWidgetInit(&entry, vec2_f(400, 30), WindowWidgetGetSurface(&window));
    Transform2DSetPosition(&entry, 200, 200);

    RangeWidgetInit(&range, vec2_f(100, 50), 0, 100, NULL);
    
    float rot = 0;
        
    char buffer[256];

    uint32_t curr = 0;
    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();
        
        sprintf(buffer, "Value is %f", source);

        GUIAddText(300, 300, vec3_f(0,0,0), 9, buffer);
        
        GUIAddText(300, 400, vec3_f(0,0,0), 9, " У попа была собака");

        rot +=0.001f;

        TEngineRender();
    }
        
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}