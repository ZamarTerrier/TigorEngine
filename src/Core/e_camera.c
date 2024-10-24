#include "TigorEngine.h"

#include "Core/e_camera.h"

#include "Tools/e_math.h"

#include "Data/e_resource_data.h"

extern TEngine engine;

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

    cam->yaw = 90;
    cam->pitch = 0;
    cam->sensitivity = 2.0f;

    cam->moveSpeed = 1.0f;
    cam->cameraSpeed = 1.5f;

    cam->firstMouse = true;
    cam->walk = false;
    
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

void Camera2DSetRotation(float angle){
    Camera2D* cam = (Camera2D*)engine.cam2D;

    cam->angle = angle;
}

void Camera3DSetRotation(float x, float y, float z){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    cam->rotation.x = x;
    cam->rotation.y = y;
    cam->rotation.z = z;
}

float Camra2DGetRotation(){
    Camera2D* cam = (Camera2D*)engine.cam2D;

    return cam->angle;
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

void Camera3DMovementUpdate(float deltaTime){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    vec3 up = {0.0f,1.0f,0.0f};

    vec3 pos = Camera3DGetPosition();
    vec3 viewRot = Camera3DGetRotation();
    vec3 some_pos;

    if (TEngineGetKeyPress(TIGOR_KEY_W)){
        if(cam->walk)
            viewRot.y = 0;
        some_pos = v3_sub(pos, v3_muls( viewRot, cam->cameraSpeed * deltaTime));

        Camera3DSetPosition(some_pos.x, some_pos.y, some_pos.z);
    }else if (TEngineGetKeyPress(TIGOR_KEY_S)){
        if(cam->walk)
            viewRot.y = 0;
        some_pos = v3_add(pos, v3_muls( viewRot, cam->cameraSpeed * deltaTime));
        Camera3DSetPosition(some_pos.x, some_pos.y, some_pos.z);
    }


    if (TEngineGetKeyPress(TIGOR_KEY_A)){
        some_pos = v3_sub(pos, v3_muls(v3_norm(v3_cross(viewRot, up)), cam->cameraSpeed * deltaTime));
        Camera3DSetPosition(some_pos.x, some_pos.y, some_pos.z);
    }else if (TEngineGetKeyPress(TIGOR_KEY_D)){
        some_pos = v3_add(pos, v3_muls(v3_norm(v3_cross(viewRot, up)), cam->cameraSpeed * deltaTime));
        Camera3DSetPosition(some_pos.x, some_pos.y, some_pos.z);
    }

    if (TEngineGetKeyPress(TIGOR_KEY_LEFT_SHIFT)){
        cam->cameraSpeed = cam->moveSpeed * 10;
    }else{
        cam->cameraSpeed = cam->moveSpeed * 3;
    }

    /*if (TEngineGetKeyPress(TIGOR_KEY_1)){
        walk = true;
    }else if (TEngineGetKeyPress(TIGOR_KEY_2)){
        walk = false;
    }*/

    /*if (TEngineGetKeyPress(TIGOR_KEY_SPACE) && force <= 0 && grounded){
        jump = true;
        force = 5;
    }*/
}

void Camera3DRotationUpdate(float deltaTime){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    double xpos, ypos;

    TEngineGetCursorPos(&xpos, &ypos);

    if (cam->firstMouse)
    {
        cam->lastX = xpos;
        cam->lastY = ypos;
        cam->firstMouse = false;
    }

    double xoffset, yoffset;

    if(cam->lockMouse){
        xoffset = engine.width / 2 - xpos;
        yoffset = engine.height / 2 - ypos;
    }else{
        xoffset = xpos - cam->lastX;
        yoffset = cam->lastY - ypos; 
    }
    
    cam->lastX = xpos;
    cam->lastY = ypos;

    xoffset *= cam->sensitivity * deltaTime;
    yoffset *= cam->sensitivity * deltaTime;

    cam->yaw   += xoffset;
    cam->pitch += yoffset;

    if(cam->pitch > 89.0f)
        cam->pitch = 89.0f;
    if(cam->pitch < -89.0f)
        cam->pitch = -89.0f;

    vec3 direction = Camera3DGetRotation();
    vec3 next_rotation, result;
    next_rotation.x = cos(cam->yaw * (M_PI / 180)) * cos(cam->pitch * (M_PI / 180));
    next_rotation.y = -sin(cam->pitch * (M_PI / 180));
    next_rotation.z = -sin(cam->yaw * (M_PI / 180)) * cos(cam->pitch * (M_PI / 180));

    if(cam->lockMouse)
        next_rotation.z *= -1;

    next_rotation = v3_norm(next_rotation);

    Camera3DSetRotation(next_rotation.x, next_rotation.y, next_rotation.z);

    if(cam->lockMouse)
        TEngineFixedCursorCenter();
}

void Camera3DSetLockCursor(){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    cam->lockMouse = !cam->lockMouse;

    if(cam->lockMouse)
        TEngineHideCursor(1);
    else
        TEngineHideCursor(2);
}

bool Camera3DGetLockCursor(){
    Camera3D* cam = (Camera3D*)engine.cam3D;

    return cam->lockMouse;
}