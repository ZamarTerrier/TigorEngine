#include "Core/e_camera.h"

#include "Data/e_resource_data.h"

extern ZEngine engine;

void Camera2DInit(Camera2D *cam){

    memset(cam, 0, sizeof(Camera2D));

    cam->scale.x = 1;
    cam->scale.y = 1;
}

void Camera3DInit(Camera3D *cam){

    memset(cam, 0, sizeof(Camera3D));

    cam->scale.x = 1;
    cam->scale.y = 1;
    cam->scale.z = 1;
    cam->view_distance = 1000;
    cam->view_angle = 75.0f;
    cam->view_near = 0.01f;
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
