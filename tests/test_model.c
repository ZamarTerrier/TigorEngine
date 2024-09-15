#include <ZamEngine.h>
#include <ZamGUI.h>

#include <Core/e_camera.h>

#include <Objects/models.h>

#include <Tools/e_math.h>
#include <Tools/objLoader.h>
#include <Tools/glTFLoader.h>

Camera2D cam2D;
Camera3D cam3D;

ModelObject3D model;

int main(){

    ZEngineInitSystem(800, 600, "Test");

    ZEngineSetFont("res\\RobotoBlack.ttf");

    printf("Size uniform is %i\n", sizeof(InvMatrixsBuffer));

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));
   
    dParam.diffuse = "res\\texture.jpg";

    Load3DglTFModel(&model, "res\\", "Little_Tokyo", 2, &dParam);

    Transform3DSetScaleT(&model.transform, 0.1, 0.1, -0.1);

    //Load3DObjModel(&model, "res\\rabochiy_i_kolkhoznitsa_lou_poli.obj", &dParam);

    float ticker = 0;
    while (!ZEngineWindowIsClosed())
    {
        ZEnginePoolEvents();

        Camera3DMovementUpdate(0.1);
        Camera3DUpdateInput(0.1);

        ZEngineDraw(&model);

        ZEngineRender();
    }
        

    GameObjectDestroy(&model);

    EngineDeviceWaitIdle();
    
    ZEngineCleanUp();

    return 0;
}