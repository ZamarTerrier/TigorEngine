#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Variabels/engine_includes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
    TIGOR_GAME_OBJECT_FLAG_LIGHT = 0x1,
    TIGOR_GAME_OBJECT_FLAG_WIRED = 0x2,
    TIGOR_GAME_OBJECT_FLAG_SELF_CAMERA = 0x4,
    TIGOR_GAME_OBJECT_FLAG_INIT = 0x8,
    TIGOR_GAME_OBJECT_FLAG_SHADED = 0x10,
    TIGOR_GAME_OBJECT_FLAG_LIGHT_SHADER = 0x20
} EngineGmaeObjectFlag;

typedef enum{
    TIGOR_GAME_OBJECT_TYPE_2D,
    TIGOR_GAME_OBJECT_TYPE_3D,
    TIGOR_GAME_OBJECT_TYPE_MODEL,
    TIGOR_GAME_OBJECT_TYPE_PARTICLE_2D,
    TIGOR_GAME_OBJECT_TYPE_PARTICLE_3D,
} GameObjectType;

typedef struct GameObject{
    void (*InitPoint)(struct GameObject* go);
    void (*UpdatePoint)(struct GameObject* go);
    void (*DrawPoint)(struct GameObject* go);
    void (*CleanPoint)(struct GameObject* go);
    void (*RecreatePoint)(struct GameObject* go);
    void (*DestroyPoint)(struct GameObject* go);
    
    void (*ShaderInitPoint)(struct GameObject* go);

    uint32_t obj_type;
    uint32_t flags;
    void *vert;
    void *frag;
} GameObject;

void GameObjectSetInitFunc(GameObject *go, void *func);
void GameObjectSetUpdateFunc(GameObject *go, void *func);
void GameObjectSetDrawFunc(GameObject *go, void *func);
void GameObjectSetCleanFunc(GameObject *go, void *func);
void GameObjectSetRecreateFunc(GameObject *go, void *func);
void GameObjectSetDestroyFunc(GameObject *go, void *func);

void GameObjectSetShaderInitFunc(GameObject *go, void *func);

void GameObjectInit(GameObject* go);
void GameObjectUpdate(GameObject* go) ;
void GameObjectDraw(GameObject* go);
void GameObjectClean(GameObject* go);
void GameObjectRecreate(GameObject* go);
void GameObjectDestroy(GameObject* go);

void GameObjectShaderInit(GameObject* go);

#ifdef __cplusplus
}
#endif

#endif // GAMEOBJECT_H
