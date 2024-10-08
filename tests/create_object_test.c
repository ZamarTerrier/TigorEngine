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

ShapeObject shape;

vec3 dir, l_pos;

void Update(float dTime){

    double time = TEngineGetTime();

    dir.x = l_pos.x = 3 * cos(time);
    dir.y = l_pos.y = 3 * sin(time);
    dir.z = l_pos.z = 3 * sin(time);
}

int main(){

    TEngineInitSystem(800, 600, "Test");

    //TEngineSetFont("res\\arial.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\texture.jpg";

    QuadParams params;
    params.size = 100;
    params.color = vec3_f(1, 1, 1);

    PrimitiveObjectInit(&po, &dParam, TIGOR_PRIMITIVE3D_CUBE, NULL);
    Transform3DSetPosition(&po, 0, 0, 10);

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
        TEngineDraw(&shape);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&po);
    GameObjectDestroy((GameObject *)&shape);
    
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}