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

extern TEngine engine;

vec3 dir, l_pos;

void Update(float dTime){

    double time = TEngineGetTime();

    l_pos.x = 3 * cos(time);
    l_pos.y = 3 * sin(time);
    l_pos.z = 3 * sin(time);

    dir.x = cos(time);
    dir.y = sin(time);
    dir.z = sin(time);

    engine.lights.lights[0].color = vec3_f(0.1f, 0.1f, 0.1f);
    engine.lights.lights[0].position = l_pos;
    engine.lights.lights[0].direction = Camera3DGetRotation();
    engine.lights.lights[0].direction = v3_muls(engine.lights.lights[0].direction, 1);
    engine.lights.lights[0].type = 1;
    engine.lights.lights[0].intensity = 1;
    engine.lights.lights[0].cutoff = 0.5f;
    engine.lights.lights[0].radius = 1.0f;
    
    engine.lights.lights[1].color = vec3_f(0.1f, 1, 0.1f);
    engine.lights.lights[1].position = v3_muls(l_pos, -1);
    engine.lights.lights[1].direction = v3_muls(dir, -1);
    engine.lights.lights[1].type = 0;
    engine.lights.lights[1].intensity = 1;
    engine.lights.lights[1].cutoff = 0.5;
    engine.lights.lights[1].radius = 1.0f;

    engine.lights.size = 1;

    //Transform3DSetPosition(&light_point, l_pos.x, l_pos.y, l_pos.z);
}

int main(){

    TEngineInitSystem(800, 600, "Test");

    TEngineSetFont("res\\RobotoBlack.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));
   
    //dParam.diffuse = "res\\secretary_tex.png";

    //Load3DglTFModel(&model, "res\\", "Little_Tokyo", 2, &dParam);
    Load3DFBXModel(&model, "res\\Thoughtful Head Shake.fbx", &dParam);

    Transform3DSetScaleT(&model.transform, 0.05, 0.05, -0.05);

    //Load3DObjModel(&model, "res\\rabochiy_i_kolkhoznitsa_lou_poli.obj", &dParam);

    ModelSetLightEnable(&model, true);

    float ticker = 0;
    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Update(0.1);

        Camera3DMovementUpdate(0.1);
        Camera3DRotationUpdate(0.1);

        TEngineDraw(&model);
        
        TEngineRender();

        ModelNextFrame(&model, 0.01, 0);
    }
        
    GameObjectDestroy(&model);

    //EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}