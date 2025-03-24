#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/engine.h>
#include <Core/e_camera.h>

#include <GUI/e_widget_range.h>
#include <GUI/e_widget_entry.h>

#include <Objects/sprite_object.h>

#include "Tools/e_math.h"

Camera2D cam2D;
Camera3D cam3D;

SpriteObject sprite;

EWidgetRange range;
EWidgetEntry entry;

vec3 dir, l_pos;

extern TEngine engine;

void Update(float dTime){

}


int main(){

    TEngineInitSystem(800, 600, "Test");

    //TEngineSetFont("res\\arial.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\hero.png";
    //dParam.normal = "res\\normal.jpg";

    SpriteObjectInit(&sprite, &dParam);
    Transform2DSetPosition(&sprite, 100, 100);
    Transform2DSetScale(&sprite, 25, 27);

    uint32_t x_offset = 0;
    uint32_t y_offset = 520;

    float timer = 1.0f, degreaser = 10;

    RangeWidgetInit(&range, vec2_f(100, 30), 10, 100, NULL);
    WidgetSetPosition(&range, 400, 300);
    RangeWidgetSetValueDestin(&range, &degreaser);

    EntryWidgetInit(&entry, vec2_f(130, 30), NULL);
    WidgetSetPosition(&entry, 100, 100);

    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Camera3DMovementUpdate(0.1);
        Camera3DRotationUpdate(0.1);

        Update(0.1);

        timer -= degreaser / 100;

        if(timer < 0)
        {
            timer = 1.0f;

            x_offset += 120;
        }

        if(x_offset > 1080)
            x_offset = 0;
        
        SpriteObjectSetOffsetRect(&sprite, x_offset, y_offset, 120, 130);

        vec4 cliper = vec4_f(0, 0, 200, 75);
        GUIAddTextClippedU8(30, 30, vec3_f(1, 0, 0), 7, "У попа была собака", &cliper);
        GUIAddTextClippedU8(30, 70, vec3_f(1, 0, 0), 7, "Entscheidungsschwierigkeiten", &cliper);

        TEngineDraw(&sprite);
        //TEngineDraw(&shape);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&sprite);
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}