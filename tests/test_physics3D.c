#include <TigorEngine.h>
#include <TigorGUI.h>

#include <Core/engine.h>
#include <Core/e_camera.h>
#include <Core/e_physics.h>

#include <Objects/render_texture.h>
#include <Objects/primitiveObject.h>
#include <Objects/light_object.h>
#include <Objects/shape_object.h>

#include "Tools/e_math.h"

Camera2D cam2D;
Camera3D cam3D;

PrimitiveObject po;
PrimitiveObject po2;
RigidBody *rb_box;
RigidBody *rb_box2;

PrimitiveObject plane;
RigidBody *rb_plane;

extern TEngine engine;

extern float RAD2DEG(float x);

void Update(float dTime){

    double time = TEngineGetTime();

    PhysicsUpdate(dTime);

    Transform3DSetPosition(&po, rb_box->params.position.x, rb_box->params.position.y, rb_box->params.position.z);
    Transform3DSetRotate(&po, RAD2DEG(rb_box->params.rotating.x), RAD2DEG(rb_box->params.rotating.y), RAD2DEG(rb_box->params.rotating.z));
    Transform3DSetPosition(&po2, rb_box2->params.position.x, rb_box2->params.position.y, rb_box2->params.position.z);
    Transform3DSetRotate(&po2, RAD2DEG(rb_box2->params.rotating.x), RAD2DEG(rb_box2->params.rotating.y), RAD2DEG(rb_box2->params.rotating.z));
}


int main(){

    TEngineInitSystem(800, 600, "Test");

    //TEngineSetFont("res\\arial.ttf");

    rb_box = PhysicsInitObject(TIGOR_RIGIDBODY_TYPE_BOX, true);
    rb_box2 = PhysicsInitObject(TIGOR_RIGIDBODY_TYPE_BOX, true);
    rb_plane = PhysicsInitObject(TIGOR_RIGIDBODY_TYPE_BOX, true);

    rb_box->params.rotating = vec3_f(0, 0, 0.4);
    rb_box2->params.rotating = vec3_f(0, 0, 0);

    rb_plane->mass = 0.0f;
    rb_plane->params.size = vec3_f(50, 1, 50);

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);
        
    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));

    dParam.diffuse = "res\\texture.jpg";
    //dParam.normal = "res\\normal.jpg";

    PrimitiveObjectInit(&po, &dParam, TIGOR_PRIMITIVE3D_CUBE, NULL);
    Transform3DSetPosition(&po, 0, 10, 7);    
    PrimitiveObjectInit(&po2, &dParam, TIGOR_PRIMITIVE3D_CUBE, NULL);
    Transform3DSetPosition(&po2, 0, 0, 7);
    
    PrimitiveObjectInit(&plane, &dParam, TIGOR_PRIMITIVE3D_CUBE, NULL);
    Transform3DSetPosition(&plane, 0, -5, 7);
    Transform3DSetScale(&plane, 50, 1, 50);

    rb_box->params.position = Transform3DGetPosition(&po);
    rb_box2->params.position = Transform3DGetPosition(&po2);
    rb_plane->params.position = Transform3DGetPosition(&plane);

    RigidBodySynchCollisionVolumes(rb_box);
    RigidBodySynchCollisionVolumes(rb_box2);
    RigidBodySynchCollisionVolumes(rb_plane);

    while (!TEngineWindowIsClosed())
    {
        TEnginePoolEvents();

        Camera3DMovementUpdate(0.1);
        Camera3DRotationUpdate(0.1);

        Update(0.01);

        GUIAddText(30, 30, vec3_f(1, 0, 0), 9, "У попа была собака");
        GUIAddText(30, 70, vec3_f(1, 0, 0), 9, "Entscheidungsschwierigkeiten");

        TEngineDraw(&po);
        TEngineDraw(&po2);
        TEngineDraw(&plane);

        TEngineRender();
    }
    
    GameObjectDestroy((GameObject *)&po);
    GameObjectDestroy((GameObject *)&po2);
    GameObjectDestroy((GameObject *)&plane);
    
    EngineDeviceWaitIdle();
    
    TEngineCleanUp();

    return 0;
}