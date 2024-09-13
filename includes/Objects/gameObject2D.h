#ifndef GAMEOBJECT2D_H
#define GAMEOBJECT2D_H

#include "Variabels/engine_includes.h"

#include "Core/e_descriptor.h"
#include "Core/pipeline.h"
#include "Core/e_texture.h"
#include "Core/graphicsObject.h"
#include "Core/e_transform.h"


#include "gameObject.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    GameObject self;
    GraphicsObject graphObj;
    Transform2D transform;
    GameObjectImage *image;
    uint32_t num_images;
    char name[256];
} GameObject2D;

void GameObject2DTransformBufferUpdate(GameObject2D *go, void *data);
void GameObject2DImageBuffer(GameObject2D *go, void *data);

void GameObject2DDefaultUpdate(GameObject2D* go);
void GameObject2DDefaultDraw(GameObject2D* go);

void GameObject2DAddDescriptor(GameObject2D* go, uint32_t shader_indx, uint32_t size, uint32_t stage_bit, UpdateDescriptor Updater, uint32_t layer_indx);
void GameObject2DAddDescriptorTexture(GameObject2D* go, uint32_t shader_indx, uint32_t stage_bit, GameObjectImage *image);

void GameObject2DSetDescriptorUpdate(GameObject2D* go, uint32_t shader_indx, uint32_t bind_index, UpdateDescriptor Updater);
void GameObject2DSetDescriptorTexture(GameObject2D* go, uint32_t shader_indx, uint32_t bind_index, Texture2D *texture);
void GameObject2DSetDescriptorTextureCreate(GameObject2D* go, uint32_t shader_indx, uint32_t bind_index, GameObjectImage *image);

void GameObject2DSetShader(GameObject2D *go, char *vert_path, char *frag_path);
void GameObject2DSetShaderSimple(GameObject2D *go, char *vert_path, char *frag_path);

void GameObject2DInitDraw(GameObject2D *go);
void GameObject2DInit(GameObject2D* go);
void GameObject2DClean(GameObject2D* go);
void GameObject2DRecreate(GameObject2D* go);
void GameObject2DDestroy(GameObject2D* go);

vec2 GameObject2DGetSize(GameObject2D* go);

#ifdef __cplusplus
}
#endif

#endif // GAMEOBJECT2D_H
