#ifndef E_VERTEX_H
#define E_VERTEX_H

#include "e_math_variables.h"

typedef enum{
    TIGOR_VERTEX_TYPE_NONE,
    TIGOR_VERTEX_TYPE_2D_OBJECT,
    TIGOR_VERTEX_TYPE_3D_OBJECT,
    TIGOR_VERTEX_TYPE_3D_INSTANCE,
    TIGOR_VERTEX_TYPE_TREE_INSTANCE,
    TIGOR_VERTEX_TYPE_MODEL_OBJECT,
    TIGOR_VERTEX_TYPE_2D_PARTICLE,
    TIGOR_VERTEX_TYPE_3D_PARTICLE,
    TIGOR_VERTEX_TYPE_TERRAIN,
} VertexType;

typedef struct {
    vec2 position;
    vec4 color;
    vec2 texCoord;
} Vertex2D;

typedef struct {
    vec3 position;
    vec3 rotation;
    float scale;
} VertexInstance3D;

typedef struct {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texCoord;
} Vertex3D;

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
    vec3 color;
} TreeVertex3D;

typedef struct {
    vec2 position;
    float size;
    vec3 color;
} ParticleVertex2D;

typedef struct {
    vec3 position;
    float size;
    vec3 color;
} ParticleVertex3D;

typedef struct{
    vec2 position;
    vec2 texture_uv;
} SkyVertex;

typedef struct {
    vec3 position;
    vec3 normal;
    vec3 color;
    vec2 texCoord;
    uint32_t material_indx;
    vec4 joints;
    vec4 weight;
} ModelVertex3D;

#endif // E_VERTEX_H
