#include "Objects/gameObject.h"

#include "Core/engine.h"
#include "Core/e_device.h"

extern TEngine engine;

void GameObjectSetInitFunc(GameObject *go, void *func){
    go->InitPoint = func;
}

void GameObjectSetUpdateFunc(GameObject *go, void *func){
    go->UpdatePoint = func;
}

void GameObjectSetDrawFunc(GameObject *go, void *func){
    go->DrawPoint = func;
}

void GameObjectSetCleanFunc(GameObject *go, void *func){
    go->CleanPoint = func;
}

void GameObjectSetRecreateFunc(GameObject *go, void *func){
    go->RecreatePoint = func;
}

void GameObjectSetDestroyFunc(GameObject *go, void *func){
    go->DestroyPoint = func;
}

void GameObjectSetShaderInitFunc(GameObject *go, void *func){
    go->ShaderInitPoint = func;
}

void GameObjectInit(GameObject* go){

    if(go == NULL)
        return;

    if(go->InitPoint != NULL)
        go->InitPoint(go);
}

void GameObjectUpdate(GameObject* go) {

    if(go == NULL)
        return;

    if(go->UpdatePoint == NULL)
        return;

    if(go->UpdatePoint != NULL)
        go->UpdatePoint(go);
}

void GameObjectDraw(GameObject* go) {

    if(go == NULL)
        return;

    if(!(go->flags & TIGOR_GAME_OBJECT_FLAG_INIT))
        return;

    GameObjectUpdate(go);

    if(go->DrawPoint != NULL)
        go->DrawPoint(go);
}

void GameObjectClean(GameObject* go){

    if(go == NULL)
        return;

    if(go->CleanPoint != NULL)
        go->CleanPoint(go);
}

void GameObjectRecreate(GameObject* go){

    if(go == NULL)
        return;

    if(go->RecreatePoint != NULL)
        go->RecreatePoint(go);
}

void GameObjectDestroy(GameObject* go){

    if(go == NULL)
        return;
        
    EngineDeviceWaitIdle();

    void (*destroy)(GameObject* go) = go->DestroyPoint;

    destroy(go);
}

void GameObjectShaderInit(GameObject* go){

    if(go == NULL)
        return;

    if(go->ShaderInitPoint != NULL)
        go->ShaderInitPoint(go);
}
