#ifndef TERRAIN_OBJECT_H
#define TERRIAN_OBJECT_H

#include "Objects/gameObject3D.h"

#include "Variabels/engine_includes.h"

typedef enum{
    ENGINE_TERRIAN_FLAGS_CUSTOM_TEXTURE = 0x1,
    ENGINE_TERRIAN_FLAGS_MIXED_TEXTURE = 0x2,
    ENGINE_TERRIAN_FLAGS_REPEATE_TEXTURE = 0x4,
    ENGINE_TERRIAN_FLAGS_GENERATE_TEXTURE = 0x8,
    ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS = 0x10,
    ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS_OLD = 0x20,
    ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS_PERLIN = 0x40,
} TerrainFlags;

typedef struct{
    uint32_t size_factor;
    float displaisment_factor;
    float tesselation_factor;
    uint32_t amplitude;
    uint32_t frequency;
    uint32_t octaves;
} TerrainGeneratorParam;

typedef struct{
    uint32_t height_map_scale;
    uint32_t texture_scale;
    uint32_t num_textures;
} TerrainTextureParam;

typedef struct{
    uint32_t size_patch;
    uint32_t flags;
    float vertex_step;
    TerrainGeneratorParam t_g_param;
    TerrainTextureParam t_t_param;
    char *texture_map;
    char **textures;
} TerrainParam;

typedef struct{
    GameObject3D go;
    uint32_t flags;
    void *height_map;
    uint32_t width;
    uint32_t height;
    float t_shift;
    TerrainTextureParam t_t_param;
    TerrainGeneratorParam t_g_param;
    void *tesc_shader;
    void *tese_shader;
} TerrainObject;


void TerrainObjectInit(TerrainObject *to, DrawParam *dParam, TerrainParam *tParam);

float TerrainObjectGetHeight(TerrainObject *to, uint32_t x, uint32_t y);

void TerrainObjectMakeDefaultParams(TerrainParam *tParam, uint32_t height_map_size);

#endif //TERRIAN_OBJECT_H