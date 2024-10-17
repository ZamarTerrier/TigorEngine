#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/engine.h>
#include <Core/e_camera.h>

#include <Objects/render_texture.h>
#include <Objects/primitiveObject.h>
#include <Objects/light_object.h>
#include <Objects/shape_object.h>

#include "Tools/e_math.h"

LightObject *light1, *light2, *light3, *light4;

Camera2D cam2D;
Camera3D cam3D;

PrimitiveObject po;

PrimitiveObject light_point;

ShapeObject shape;

vec3 dir, l_pos;

extern TEngine engine;

void Update(float dTime){

    double time = TEngineGetTime();

    l_pos.x = 3 * cos(time);
    l_pos.y = 3 * sin(time);
    l_pos.z = 0;//3 * sin(time);

    dir.x = cos(time * M_PI / 2 / 10);
    dir.y = sin(time * M_PI / 2 / 10);
    dir.z = sin(time * M_PI / 2 / 10);

    //dir = Camera3DGetRotation();

    LightObjectSetDirection(light1, dir.x, dir.y, dir.z);

    LightObjectSetPosition(light2, l_pos.x, l_pos.y, l_pos.z);
    LightObjectSetPosition(light3, l_pos.x * -1, l_pos.y * -1, l_pos.z * -1);

    vec3 s_pos = Camera3DGetPosition();
    LightObjectSetPosition(light4, s_pos.x, s_pos.y, s_pos.z);
    vec3 s_dir = v3_muls(Camera3DGetRotation(), -1);
    LightObjectSetDirection(light4, s_dir.x, s_dir.y, s_dir.z);

    char buff[125];
    sprintf(buff,"%0.2f, %0.2f, %0.2f", dir.x, dir.y, dir.z);
    GUIAddText(600, 30, vec3_f(0, 0, 0), 7, buff);

    Transform3DSetPosition(&light_point, l_pos.x, l_pos.y, l_pos.z);
}


int main(){

    TEngineInitSystem(800, 600, "Test");

    //TEngineSetFont("res\\arial.ttf");

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

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\texture.jpg";
    //dParam.normal = "res\\normal.jpg";

    QuadParams params;
    params.size = 100;
    params.color = vec3_f(1, 1, 1);

    PrimitiveObjectInit(&po, &dParam, TIGOR_PRIMITIVE3D_CUBE, NULL);
    Transform3DSetPosition(&po, 0, 0, 7);
    Transform3DSetScale(&po, 5, 5, 5);
    GameObject3DEnableLight(&po, true);

    SphereParam sParam;
    sParam.radius = 1;
    sParam.sectorCount = 10;
    sParam.stackCount = 10;

    PrimitiveObjectInit(&light_point, &dParam, TIGOR_PRIMITIVE3D_SPHERE, &sParam);

    QuadParams param;
    param.size = 100;
    param.color = vec3_f(1, 1, 1);

    ShapeObjectInit(&shape, &dParam, TIGOR_SHAPE_OBJECT_QUAD, &param);
    Transform2DSetScale(&shape, 100, 100);
    Transform2DSetPosition(&shape, 400, 400);
    shape.go.transform.img.scale = vec2_f(2, 2);

    float rot = 0;
    
    vec2 c_pos = {-100, -100};

    Camera2DSetPosition(c_pos.x, c_pos.y);
    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Camera3DMovementUpdate(0.1);
        Camera3DRotationUpdate(0.1);

        Update(0.1);

        rot += 1.0f;
        c_pos.x -= 1.1f;

        Camera2DSetRotation(rot);
        Camera2DSetPosition(c_pos.x, c_pos.y);
        GUIAddText(30, 30, vec3_f(1, 0, 0), 9, "У попа была собака");
        GUIAddText(30, 70, vec3_f(1, 0, 0), 9, "Entscheidungsschwierigkeiten");

        TEngineDraw(&po);
        TEngineDraw(&light_point);
        //TEngineDraw(&shape);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&po);
    GameObjectDestroy((GameObject *)&light_point);
    GameObjectDestroy((GameObject *)&shape);
    
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}