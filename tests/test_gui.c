#include <ZamEngine.h>
#include <ZamGUI.h>

#include <Core/engine.h>
#include <Core/e_camera.h>

#include <GUI/e_widget_list.h>
#include <GUI/e_widget_button.h>
#include <GUI/e_widget_combobox.h>
#include <GUI/e_widget_roller.h>
#include <GUI/e_widget_range.h>
#include <GUI/e_widget_window.h>
#include <GUI/e_widget_image.h>

#include <Tools/e_math.h>

Camera2D cam2D;
Camera3D cam3D;

EWidgetRoller roller;

EWidgetRange range;

EWidgetWindow window;

EWidgetButton button;

EWidgetImage image;

EWidgetList list;

float source;

void GetValue(EWidget *widget, float *value, void *arg){
    source = *value;
}

int main(){

    ZEngineInitSystem(800, 600, "Test");

    ZEngineSetFont("res\\RobotoBlack.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    Camera2DSetActive(&cam2D);
    Camera3DSetActive(&cam3D);

    WindowWidgetInit(&window, "test", vec2_f(300, 200), vec2_f(200, 200));
    

    float rot = 0;
        
    char buffer[256];

    uint32_t curr = 0;
    while (!ZEngineWindowIsClosed())
    {
        ZEnginePoolEvents();
        
        sprintf(buffer, "Value is %f", source);

        GUIAddText(300, 300, vec3_f(0,0,0), 9, buffer);
        
        GUIAddText(300, 400, vec3_f(0,0,0), 9, " У попа была собака");

        rot +=0.001f;

        ZEngineRender();
    }
        
    EngineDeviceWaitIdle();
    
    ZEngineCleanUp();

    return 0;
}