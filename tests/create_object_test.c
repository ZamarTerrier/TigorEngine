#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/engine.h>
#include <Core/e_camera.h>

#include <Objects/render_texture.h>
#include <Objects/primitiveObject.h>
#include <Objects/shape_object.h>

#include "Tools/e_math.h"

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
    l_pos.z = 3 * sin(time);

    dir.x = cos(time);
    dir.y = sin(time);
    dir.z = sin(time);

    //dir = Camera3DGetRotation();

    engine.lights.lights[0].color = vec3_f(0.01, 0.01, 0.01);
    engine.lights.lights[0].position = l_pos;
    engine.lights.lights[0].direction = dir;
    engine.lights.lights[0].type = 0;
    engine.lights.lights[0].intensity = 1;
    engine.lights.lights[0].cutoff = 0.5f;
    
    engine.lights.lights[1].color = vec3_f(0, 1, 0);
    engine.lights.lights[1].position = v3_muls(l_pos, -1);
    engine.lights.lights[1].direction = v3_muls(dir, -1);
    engine.lights.lights[1].type = 0;
    engine.lights.lights[1].intensity = 1;
    engine.lights.lights[1].cutoff = 0.5;

    
    engine.lights.lights[2].color = vec3_f(0, 0, 1);
    engine.lights.lights[2].position = vec3_f(0,0,0);
    engine.lights.lights[2].direction = Camera3DGetRotation();
    engine.lights.lights[2].direction = v3_muls(engine.lights.lights[2].direction, -1);
    engine.lights.lights[2].type = 2;
    engine.lights.lights[2].intensity = 1;
    engine.lights.lights[2].cutoff = 0.5;

    
    engine.lights.lights[3].color = vec3_f(0, 0, 1);
    engine.lights.lights[3].position = vec3_f(0,0,0);//l_pos;c
    engine.lights.lights[3].direction = v3_muls(dir, -1);
    engine.lights.lights[3].direction.z *= cos(time);
    engine.lights.lights[3].type = 2;
    engine.lights.lights[3].intensity = 1;
    engine.lights.lights[3].cutoff = 0.5f;

    engine.lights.size = 2;

    char buff[125];
    sprintf(buff,"%0.2f, %0.2f, %0.2f", dir.x, dir.y, dir.z);
    GUIAddText(600, 30, vec3_f(0, 0, 0), 7, buff);

    Transform3DSetPosition(&light_point, l_pos.x, l_pos.y, l_pos.z);
}

void LightUpdate(GameObject3D* go, void *data)
{
    LightBuffer lbo = {};
    memset(&lbo, 0, sizeof(LightBuffer));

    memcpy(lbo.lights, engine.lights.lights, sizeof(LightObject) * engine.lights.size );
    lbo.num_lights = engine.lights.size;    

    memcpy(data, (char *)&lbo, sizeof(lbo));
}


int main(){

    TEngineInitSystem(800, 600, "Test");

    //TEngineSetFont("res\\arial.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\texture.jpg";
    dParam.normal = "res\\normal.jpg";

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