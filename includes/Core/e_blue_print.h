#ifndef E_BLUE_PRINT_H
#define E_BLUE_PRINT_H

#include "Core/e_buffer.h"
#include "Core/e_texture.h"

#include "Variabels/engine_includes.h"
#include "Variabels/e_pipeline_variables.h"

typedef enum{
    TIGOR_BLUE_PRINT_FLAG_SINGLE_IMAGE = 0x1,
    TIGOR_BLUE_PRINT_FLAG_LINKED_UNIFORM = 0x2,
    TIGOR_BLUE_PRINT_FLAG_LINKED_TEXTURE = 0x4,
} EngineBluePrintFlag;

typedef struct{
    void *update;
    Texture2D *textures;
    BufferContainer uniform;
    uint32_t descrType;
    uint32_t desr_set_indx;
    uint32_t binding;
    uint32_t stageflag;
    uint32_t count;
    uint32_t buffsize;
    uint32_t flags;
    uint32_t indx_layer;
    GameObjectImage *image;
} BluePrintDescriptor;

typedef struct{
    uint32_t stageFlags;
    uint32_t offset;
    uint32_t size;
} BluePrintPushConstant;

typedef struct{
    PipelineSetting setting;
    BluePrintDescriptor descriptors[MAX_UNIFORMS];
    uint32_t num_descriptors;
    BluePrintPushConstant push_constants[MAX_UNIFORMS];
    uint32_t num_push_constants;
    uint32_t curr_bind;
    void *render_point;
} BluePrintPack;

typedef struct{
    BluePrintPack blue_print_packs[MAX_BLUE_PRINTS];
    uint32_t num_blue_print_packs;
    uint32_t isShadow;
} Blueprints;

typedef void (*UpdateDescriptor)(void *go, void *data);

uint32_t BluePrintInit(Blueprints *blueprints);

void BluePrintSetSettingsValue(Blueprints *blueprints, uint32_t indx_pack, uint32_t type, uint32_t value);
uint32_t BluePrintGetSettingsValue(Blueprints *blueprints, uint32_t indx_pack, uint32_t type);

void BluePrintAddSomeUpdater(Blueprints *blueprints, uint32_t indx_pack, uint32_t bind_indx, UpdateDescriptor Updater);

void BluePrintAddPushConstant(Blueprints *blueprints, uint32_t indx_pack, uint64_t size, uint32_t stage, uint32_t offset);

BluePrintDescriptor *BluePrintAddExistUniformStorage(Blueprints *blueprints, uint32_t indx_pack, uint32_t flags, BufferContainer uniform, void *update_func, uint32_t layer_indx);
BluePrintDescriptor *BluePrintAddUniformStorage(Blueprints *blueprints, uint32_t indx_pack, uint64_t size, uint32_t flags, void *update_func, uint32_t layer_indx);
void BluePrintAddUniformObjectC(Blueprints *blueprints, uint32_t indx_pack, uint64_t size, uint32_t flags, uint32_t binding);
void BluePrintAddUniformObject(Blueprints *blueprints, uint32_t indx_pack, uint64_t size, uint32_t flags, void *update_func, uint32_t layer_indx);

void BluePrintRecreateUniform(BluePrintPack *pack);

BluePrintDescriptor *BluePrintAddTextureC(Blueprints *blueprints, uint32_t indx_pack, uint32_t stage_bit, uint32_t binding);
void BluePrintSetTextureImage(Blueprints *blueprints, uint32_t indx_pack, Texture2D *texture, uint32_t bind_indx);
void BluePrintSetTextureImageCreate(Blueprints *blueprints, uint32_t indx_pack, GameObjectImage *image, uint32_t bind_indx);

void BluePrintAddRenderImageCube(Blueprints *blueprints, uint32_t indx_pack, uint32_t indx_cube, void *obj);
void BluePrintAddRenderImageVector(Blueprints *blueprints, uint32_t indx_pack, void *obj, uint32_t size);
void BluePrintAddRenderImageArray(Blueprints *blueprints, uint32_t indx_pack, void *obj, uint32_t size);
void BluePrintAddRenderImage(Blueprints *blueprints, uint32_t indx_pack, void *render);
void BluePrintAddTextureImageArrayCreate(Blueprints *blueprints, uint32_t indx_pack, GameObjectImage *images, uint32_t size, uint32_t bind_indx);

BluePrintDescriptor *BluePrintAddTextureImage(Blueprints *blueprints, uint32_t indx_pack, GameObjectImage *image, uint32_t stage_bit);

void BluePrintClearAll(Blueprints *blueprints);

#endif // E_BLUE_PRINT_H
