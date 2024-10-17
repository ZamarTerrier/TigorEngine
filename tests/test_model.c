#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/e_camera.h>

#include <Objects/models.h>
#include <Objects/light_object.h>

#include <Tools/e_math.h>
#include <Tools/objLoader.h>
#include <Tools/glTFLoader.h>
#include <Tools/fbxLoader.h>

LightObject *light1, *light2, *light3, *light4;

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

    LightObjectSetDirection(light1, dir.x, dir.y, dir.z);

    LightObjectSetPosition(light2, l_pos.x, l_pos.y, l_pos.z);
    LightObjectSetPosition(light3, l_pos.x * -1, l_pos.y * -1, l_pos.z * -1);

    vec3 s_pos = Camera3DGetPosition();
    LightObjectSetPosition(light4, s_pos.x, s_pos.y, s_pos.z);
    vec3 s_dir = v3_muls(Camera3DGetRotation(), -1);
    LightObjectSetDirection(light4, s_dir.x, s_dir.y, s_dir.z);

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

    Load3DglTFModel(&model, "res\\", "Little_Tokyo", 2, &dParam);
    //Load3DFBXModel(&model, "res\\Thoughtful Head Shake.fbx", &dParam);

    Transform3DSetScaleT(&model.transform, 0.1, 0.1, -0.1);

    //Load3DObjModel(&model, "res\\rabochiy_i_kolkhoznitsa_lou_poli.obj", &dParam);

    ModelSetLightEnable(&model, true);

    
    light1 = LightObjectAdd(TIGOR_LIGHT_OBJECT_TYPE_DIRECTIONAL_LIGHT);
    light2 = LightObjectAdd(TIGOR_LIGHT_OBJECT_TYPE_POINT_LIGHT);
    LightObjectSetColor(light2, 0, 1, 0);
    LightObjectSetRadius(light2, 10.0f);
    light3 = LightObjectAdd(TIGOR_LIGHT_OBJECT_TYPE_POINT_LIGHT);
    LightObjectSetColor(light3, 0, 0, 1);
    LightObjectSetRadius(light3, 10.0f);
    light4 = LightObjectAdd(TIGOR_LIGHT_OBJECT_TYPE_SPOT_LIGHT);
    LightObjectSetColor(light4, 1, 0, 0);
    LightObjectSetRadius(light4, 200.0f);

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