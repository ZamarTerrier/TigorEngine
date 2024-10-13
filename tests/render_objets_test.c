#include <TigorEngine.h>

#include <Core/engine.h>
#include <Core/e_camera.h>

#include <Objects/render_texture.h>
#include <Objects/primitiveObject.h>
#include <wManager/window_manager.h>

PrimitiveObject po;

Camera2D cam2D;
Camera3D cam3D;

int main(){

    TEngineInitSystem(800, 600, "Test");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    Camera2DSetActive(&cam2D);
    Camera3DSetActive(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    ToolsAddStrings(dParam.diffuse, 256, "D:\\Projects\\Test\\res\\", "texture.jpg");

    PrimitiveObjectInit(&po, &dParam, TIGOR_PRIMITIVE3D_CUBE, NULL);

    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        TEngineDraw(&po);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&po);
    
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}