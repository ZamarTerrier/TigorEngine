#include "Core/e_camera.h"

#include "Tools/e_math.h"

#include "Data/e_resource_data.h"

extern ZEngine engine;

bool firstMouse = true;

double lastX, lastY;

double yaw = 90, pitch = 0, sensitivity = 2.0f;

float moveSpeed = 2.0f;
float cameraSpeed = 3.5f;

bool walk = false;

void Camera2DInit(Camera2D *cam){

    memset(cam, 0, sizeof(Camera2D));

    cam->scale.x = 1;
    cam->scale.y = 1;

    if(engine.cam2D == NULL)
        Camera2DSetActive(cam);
}

void Camera3DInit(Camera3D *cam){

    memset(cam, 0, sizeof(Camera3D));

    cam->scale.x = 1;
    cam->scale.y = 1;
    cam->scale.z = 1;
    cam->view_distance = 1000;
    cam->view_angle = 75.0f;
    cam->view_near = 0.01f;
    
    if(engine.cam3D == NULL)
        Camera3DSetActive(cam);
    
    vec3 next_rotation;

    next_rotation.x = cos(90 * (M_PI / 180)) * cos(0 * (M_PI / 180));
    next_rotation.y = -sin(0 * (M_PI / 180));
    next_rotation.z = sin(90 * (M_PI / 180)) * cos(0 * (M_PI / 180));
    
    next_rotation = v3_norm(next_rotation);

    Camera3DSetRotation(next_rotation.x, next_rotation.y, next_rotation.z);
}

void Camera3DSetViewDistance(Camera3D *cam, double distance)
{
    cam->view_distance = distance;
}

void Camera3DSetViewAngle(Camera3D *cam, double angle)
{
    cam->view_angle = angle;
}

void Camera3DSetViewNear(Camera3D *cam, double v_near)
{
    cam->view_near = v_near;
}

void Camera3DSetActive(Camera3D *cam){
    engine.cam3D = cam;
}

void Camera2DSetActive(Camera2D *cam){
    engine.cam2D = cam;
}

void Camera3DSetRotation(float x, float y, float z){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    cam->rotation.x = x;
    cam->rotation.y = y;
    cam->rotation.z = z;
}

vec3 Camera3DGetRotation(){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    return cam->rotation;
}

void Camera2DSetPosition(float x, float y){
    Camera2D* cam = (Camera2D*)engine.cam2D;

    cam->position.x = x;
    cam->position.y = y;
}

void Camera3DSetPosition(float x, float y, float z){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    cam->position.x = x;
    cam->position.y = y;
    cam->position.z = z;
}

vec2 Camera2DGetPosition(){
    Camera2D* cam = (Camera2D*)engine.cam2D;

    return cam->position;
}

vec3 Camera3DGetPosition(){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    return cam->position;
}

void Camera2DSetScale(float x, float y){
    Camera2D* cam = (Camera2D*)engine.cam2D;

    cam->scale.x = x;
    cam->scale.y = y;
}

void Camera3DSetScale(float x, float y, float z){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    cam->scale.x = x;
    cam->scale.y = y;
    cam->scale.z = z;
}

void Camera3DUpdateInput(float deltaTime){
    vec3 up = {0.0f,1.0f,0.0f};

    vec3 pos = Camera3DGetPosition();
    vec3 viewRot = Camera3DGetRotation();
    vec3 some_pos;

    if (EngineGetKeyPress(ENGINE_KEY_W)){
        if(walk)
            viewRot.y = 0;
        some_pos = v3_sub(pos, v3_muls( viewRot, cameraSpeed * deltaTime));

        Camera3DSetPosition(some_pos.x, some_pos.y, some_pos.z);
    }else if (EngineGetKeyPress(ENGINE_KEY_S)){
        if(walk)
            viewRot.y = 0;
        some_pos = v3_add(pos, v3_muls( viewRot, cameraSpeed * deltaTime));
        Camera3DSetPosition(some_pos.x, some_pos.y, some_pos.z);
    }


    if (EngineGetKeyPress(ENGINE_KEY_A)){
        some_pos = v3_sub(pos, v3_muls(v3_norm(v3_cross(viewRot, up)), cameraSpeed * deltaTime));
        Camera3DSetPosition(some_pos.x, some_pos.y, some_pos.z);
    }else if (EngineGetKeyPress(ENGINE_KEY_D)){
        some_pos = v3_add(pos, v3_muls(v3_norm(v3_cross(viewRot, up)), cameraSpeed * deltaTime));
        Camera3DSetPosition(some_pos.x, some_pos.y, some_pos.z);
    }

    if (EngineGetKeyPress(ENGINE_KEY_LEFT_SHIFT)){
        cameraSpeed = moveSpeed * 10;
    }else{
        cameraSpeed = moveSpeed * 3;
    }

    /*if (EngineGetKeyPress(ENGINE_KEY_1)){
        walk = true;
    }else if (EngineGetKeyPress(ENGINE_KEY_2)){
        walk = false;
    }*/

    /*if (EngineGetKeyPress(ENGINE_KEY_SPACE) && force <= 0 && grounded){
        jump = true;
        force = 5;
    }*/
}

void Camera3DMovementUpdate(float deltaTime){

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
    next_rotation.z = -sin(yaw * (M_PI / 180)) * cos(pitch * (M_PI / 180));

    next_rotation = v3_norm(next_rotation);

    Camera3DSetRotation(next_rotation.x, next_rotation.y, next_rotation.z);
}
