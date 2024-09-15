#include <ZamEngine.h>
#include <ZamGUI.h>

#include <Core/e_camera.h>

#include <Objects/particleSystem2D.h>
#include <Objects/particleSystem3D.h>

#include <Tools/e_math.h>

Camera2D cam2D;
Camera3D cam3D;

ParticleObject2D particles;
ParticleObject3D part3D;


int main(){

    ZEngineInitSystem(800, 600, "Test");

    ZEngineSetFont("res\\RobotoBlack.ttf");

    Camera2DInit(&cam2D);
    Camera3DInit(&cam3D);

    DrawParam dParam;
    memset(&dParam, 0, sizeof(DrawParam));
   
    dParam.diffuse = "res\\bubble.png";

    Particle2DInit(&particles, &dParam);
    Transform2DSetPosition(&particles, 300, 300);

    Particle3DInit(&part3D, &dParam);
    Transform3DSetPosition(&part3D, 0, 0, -10);

    float ticker = 0;
    while (!ZEngineWindowIsClosed())
    {
        ZEnginePoolEvents();

        //Camera3DMovementUpdate(0.1);

        ticker +=0.1f;

        if(ticker > 1){
            Particle2DAdd(&particles, vec2_f(rand() % 300, rand() % 300), vec2_f(0, 1.0f), 1.0, 1.0, 1);
            Particle3DAdd(&part3D, vec3_f(rand() % 6, rand() % 6, 0), vec3_f(0, 1.0f, 0), 1.0, 0.1, 1);
            ticker =0;
        }
        
        ZEngineDraw(&particles);
        //ZEngineDraw(&part3D);

        ZEngineRender();
    }
        

    GameObjectDestroy(&particles);
    GameObjectDestroy(&part3D);

    EngineDeviceWaitIdle();
    
    ZEngineCleanUp();

    return 0;
}