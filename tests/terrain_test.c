#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/e_camera.h>

#include <Objects/terrain_object.h>

#include "Tools/e_math.h"

Camera2D cam2D;
Camera3D cam3D;

TerrainObject terrain;

extern TEngine engine;

bool pressed = false;

void KeyCallback(void* window, int key, int scancode, int action, int mods){
    
    if ( (key == TIGOR_KEY_ESCAPE || key == -1)  && action == TIGOR_PRESS && !pressed){

        Camera3DSetLockCursor();

        pressed = true;

    }else if(key == TIGOR_KEY_ESCAPE && action == TIGOR_RELEASE)
        pressed = false;
}

void Update(float dTime){


}


int main(){

    TEngineInitSystem(800, 600, "Test");

    TEngineSetKeyCallback((void *)KeyCallback);

    //TEngineSetFont("res\\arial.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    printf("Tesselation buffer size is : %i\n", sizeof(TesselationBuffer));
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\grass_01_color_1k.png";
    //dParam.normal = "res\\normal.jpg";

    TerrainParam tParam;

    TerrainObjectMakeDefaultParams(&tParam, 2048);

    TerrainObjectInit(&terrain, &dParam, &tParam);

    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Camera3DMovementUpdate(0.1);
        Camera3DRotationUpdate(0.1);

        vec3 pos = Camera3DGetPosition();

        float some_val = TerrainObjectGetHeight(&terrain, pos.x, pos.z);

        pos.y = lerp(pos.y, some_val + 50, 0.1f);

        Camera3DSetPosition(pos.x, pos.y, pos.z);

        Update(0.1);

        TEngineDraw(&terrain);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&terrain);
    
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}