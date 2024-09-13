#ifndef GAMEOBJECT3D_H
#define GAMEOBJECT3D_H

#include "gameObject.h"

#include "Core/graphicsObject.h"
#include "Core/e_transform.h"

#define MAX_INSTANCES UINT16_MAX

#ifdef __cplusplus
 extern "C" {
#endif

typedef struct GameObject3D{
    GameObject self;
    GraphicsObject graphObj;
    Transform3D transform;
    GameObjectImage *images;
    uint32_t num_images;
    VertexInstance3D instances[MAX_INSTANCES];
    BufferObject buffer;
    uint32_t num_instances;
    char name[256];
} GameObject3D;

void GameObject3DDescriptorModelUpdate      (GameObject3D* go, void *data);
void GameObject3DDirLightModelUpdate        (GameObject3D* go, void *data);
void GameObject3DSpotLightModelUpdate       (GameObject3D* go, void *data);
void GameObject3DOmniLightModelUpdate       (GameObject3D* go, void *data);
void GameObject3DDescriptorLghtMatrixUpdate (GameObject3D* go, void *data);
void GameObject3DDescriptorDirLightsUpdate  (GameObject3D* go, void *data);
void GameObject3DDescriptorPointLightsUpdate(GameObject3D* go, void *data);
void GameObject3DDescriptorSpotLightsUpdate (GameObject3D* go, void *data);
void GameObject3DLigtStatusBufferUpdate     (GameObject3D* go, void *data);
void GameObject3DSDFBufferUpdate            (GameObject3D* go, void *data);

void GameObject3DDefaultUpdate(GameObject3D* go);
void GameObject3DDefaultDraw(GameObject3D* go);

void GameObject3DAddDescriptor(GameObject3D* go, uint32_t shader_indx, uint32_t size, uint32_t stage_bit, UpdateDescriptor Updater, uint32_t layer_indx);
void GameObject3DAddDescriptorTexture(GameObject3D* go, uint32_t shader_indx, uint32_t stage_bit, GameObjectImage *image);

void GameObject3DSetDescriptorUpdate(GameObject3D* go, uint32_t shader_indx, uint32_t bind_index, UpdateDescriptor Updater);
void GameObject3DSetDescriptorTexture(GameObject3D* go, uint32_t shader_indx, uint32_t bind_index, Texture2D *texture);
void GameObject3DSetDescriptorTextureCreate(GameObject3D* go, uint32_t shader_indx, uint32_t bind_index, GameObjectImage *image);

void GameObject3DSetShader(GameObject3D *go, char *vert_path, char *frag_path);
void GameObject3DSetShaderSimple(GameObject3D *go, char *vert_path, char *frag_path);

void GameObject3DInitDraw(GameObject3D *go);
void GameObject3DClean(GameObject3D* go);
void GameObject3DRecreate(GameObject3D* go);
void GameObject3DDestroy(GameObject3D* go);

void GameObject3DAddInstance(GameObject3D *go, VertexInstance3D vertex);
void GameObject3DSetInstance(GameObject3D *go, uint32_t indx, VertexInstance3D vertex);
void GameObject3DRemoveInstance(GameObject3D *go, uint32_t indx);

int GameObject3DInitTextures(GameObject3D *go, DrawParam *dParam);

void GameObject3DInit(GameObject3D *go);
void GameObject3DInitInstances(GameObject3D *go);
void GameObject3DUpdateInstances(GameObject3D *go);


void GameObject3DAddShadowDescriptor(GameObject3D *go, uint32_t type, void *render, uint32_t layer_indx);
void GameObject3DAddOmiShadow(GameObject3D *go, void *render, uint32_t layer_indx);

void GameObject3DEnableLight(GameObject3D *go, bool enable);

#ifdef __cplusplus
}
#endif

#endif // GAMEOBJECT3D_H
