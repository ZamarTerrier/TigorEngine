#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Variabels/engine_includes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
    ENGINE_GAME_OBJECT_FLAG_LIGHT = 0x1,
    ENGINE_GAME_OBJECT_FLAG_WIRED = 0x2,
    ENGINE_GAME_OBJECT_FLAG_SELF_CAMERA = 0x4,
    ENGINE_GAME_OBJECT_FLAG_INIT = 0x8,
    ENGINE_GAME_OBJECT_FLAG_SHADED = 0x10
} EngineGmaeObjectFlag;

typedef enum{
    ENGINE_GAME_OBJECT_TYPE_2D,
    ENGINE_GAME_OBJECT_TYPE_3D
} GameObjectType;

typedef struct GameObject{
    void (*InitPoint)(struct GameObject* go);
    void (*UpdatePoint)(struct GameObject* go);
    void (*DrawPoint)(struct GameObject* go);
    void (*CleanPoint)(struct GameObject* go);
    void (*RecreatePoint)(struct GameObject* go);
    void (*DestroyPoint)(struct GameObject* go);
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

void GameObjectInit(GameObject* go);
void GameObjectUpdate(GameObject* go) ;
void GameObjectDraw(GameObject* go);
void GameObjectClean(GameObject* go);
void GameObjectRecreate(GameObject* go);
void GameObjectDestroy(GameObject* go);

#ifdef __cplusplus
}
#endif

#endif // GAMEOBJECT_H
