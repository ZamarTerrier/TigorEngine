#ifndef MODELS_H
#define MODELS_H

#include "Variabels/engine_includes.h"
#include "Variabels/model_include.h"

#include "Objects/gameObject.h"
#include "Objects/gameObject3D.h"

#include "Core/graphicsObject.h"
#include "Core/e_transform.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
    TIGOR_MODEL_NONE,
    TIGOR_MODEL_TYPE_GLTF,
    TIGOR_MODEL_TYPE_FBX,
    TIGOR_MODEL_TYPE_OBJ
} ModelType;

typedef struct{
    GameObject3D *models;
    uint32_t num_models;
    uint32_t id_node;
    mat4 model;
} ModelNode;

typedef struct{
    GameObject self;
    Transform3D transform;

    ModelNode *nodes;
    uint32_t num_draw_nodes;

    mat4 join_mats[MAX_BONES];

    void *obj;
    uint32_t obj_size;

    ModelType type;
} ModelObject3D;

void ModelModelBufferUpdate(ModelObject3D* mo, uint32_t indx_node, void *data);

void ModelNextFrame(ModelObject3D *mo, double time, int num_animation);

void ModelDefaultInit(ModelObject3D *mo, GameObjectType type);
void ModelDefaultUpdate(ModelObject3D* mo);
void ModelDefaultDraw(ModelObject3D* mo, void *command);
void ModelClean(ModelObject3D* mo);
void ModelRecreate(ModelObject3D* mo);
void ModelDestroy(ModelObject3D* mo);

void ModelSetLightEnable(void *obj, bool enable);
void ModelSetSelCameraEnable(void *obj, bool enable);
void ModelSetSomeViewport(ModelObject3D* mo, float x, float y, float height, float width);

#ifdef __cplusplus
}
#endif

#endif // MODELS_H
