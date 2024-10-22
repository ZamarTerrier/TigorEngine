#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/e_camera.h>

#include <Objects/terrain_object.h>

#include "Tools/e_math.h"

Camera2D cam2D;
Camera3D cam3D;

TerrainObject terrain;

extern TEngine engine;

void Update(float dTime){

}


int main(){

    TEngineInitSystem(800, 600, "Test");

    //TEngineSetFont("res\\arial.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    printf("Tesselation buffer size is : %i\n", sizeof(TesselationBuffer));
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\texture.jpg";
    //dParam.normal = "res\\normal.jpg";

    TerrainParam tParam;

    TerrainObjectMakeDefaultParams(&tParam, 512, 512, 512);

    TerrainObjectInit(&terrain, &dParam, &tParam);

    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Camera3DMovementUpdate(0.1);
        Camera3DRotationUpdate(0.1);

        Update(0.1);

        TEngineDraw(&terrain);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&terrain);
    
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}