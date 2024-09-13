#include <ZamEngine.h>

#include <Core/engine.h>
#include <Core/e_camera.h>

#include <Objects/light_object.h>
#include <Objects/render_texture.h>
#include <Objects/primitiveObject.h>
#include <Objects/shape_object.h>

#include "Tools/e_math.h"

Camera2D cam2D;
Camera3D cam3D;

PrimitiveObject po;

ShapeObject shape;

LightObject light;

bool firstMouse = true;

double lastX, lastY;

double yaw = 90, pitch = 0, sensitivity = 2.0f;

vec3 dir, l_pos;

void CamRotateView(float deltaTime){

    double xpos, ypos;

    ZEngineGetCursorPos(&xpos, &ypos);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity * deltaTime;
    yoffset *= sensitivity * deltaTime;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    vec3 direction = Camera3DGetRotation();
    vec3 next_rotation, result;
    next_rotation.x = cos(yaw * (M_PI / 180)) * cos(pitch * (M_PI / 180));
    next_rotation.y = -sin(pitch * (M_PI / 180));
    next_rotation.z = sin(yaw * (M_PI / 180)) * cos(pitch * (M_PI / 180));

    next_rotation = v3_norm(next_rotation);

    Camera3DSetRotation(next_rotation.x, next_rotation.y, next_rotation.z);
}


void Update(float dTime){
    CamRotateView(dTime);

    double time = ZEngineGetTime();

    dir.x = l_pos.x = 3 * cos(time);
    dir.y = l_pos.y = 3 * sin(time);
    dir.z = l_pos.z = 3 * sin(time);
}

int main(){

    ZEngineInitSystem(800, 600, "Test");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    Camera2DSetActive(&cam2D);
    Camera3DSetActive(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\texture.jpg";

    QuadParams params;
    params.size = 100;
    params.color = vec3_f(1, 1, 1);

    PrimitiveObjectInit(&po, &dParam, ENGINE_PRIMITIVE3D_CUBE, NULL);
    Transform3DSetPosition(&po, 0, 0, -10);
    GameObject3DEnableLight(&po, true);

    QuadParams param;
    param.size = 100;
    param.color = vec3_f(1, 1, 1);

    ShapeObjectInit(&shape, &dParam, ENGINE_SHAPE_OBJECT_QUAD, &param);
    shape.go.transform.img.scale = vec2_f(2, 2);

    while (!ZEngineWindowIsClosed())
    {
        ZEnginePoolEvents();

        Update(0.1);


        ZEngineDraw(&po);
        ZEngineDraw(&shape);

        ZEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&po);
    GameObjectDestroy((GameObject *)&shape);
    
    EngineDeviceWaitIdle();
    
    ZEngineCleanUp();

    return 0;
}