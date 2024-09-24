#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/e_camera.h>

#include <Objects/models.h>

#include <Tools/e_math.h>
#include <Tools/objLoader.h>
#include <Tools/glTFLoader.h>
#include <Tools/fbxLoader.h>

Camera2D cam2D;
Camera3D cam3D;

ModelObject3D model;

int main(){

    TEngineInitSystem(800, 600, "Test");

    TEngineSetFont("res\\RobotoBlack.ttf");

    printf("Size uniform is %i\n", sizeof(InvMatrixsBuffer));

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));
   
    dParam.diffuse = "res\\secretary_tex.png";

    //Load3DglTFModel(&model, "res\\", "Little_Tokyo", 2, &dParam);
    Load3DFBXModel(&model, "res\\Dismissing Gesture.fbx", &dParam);

    Transform3DSetScaleT(&model.transform, 0.1, 0.1, -0.1);

    //Load3DObjModel(&model, "res\\rabochiy_i_kolkhoznitsa_lou_poli.obj", &dParam);

    float ticker = 0;
    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Camera3DMovementUpdate(0.1);
        Camera3DUpdateInput(0.1);

        TEngineDraw(&model);

        TEngineRender();

        ModelNextFrame(&model, 0.02, 1);
    }
        

    GameObjectDestroy(&model);

    //EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}