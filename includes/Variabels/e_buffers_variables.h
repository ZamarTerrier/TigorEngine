#ifndef E_BUFFERS_VARIABLES_H
#define E_BUFFERS_VARIABLES_H

#include "e_math_variables.h"

#define MAX_BONES 128
#define MAX_LIGHTS 16

typedef struct{
    vec2 position;
} DefaultBuffer2D;

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
} TransformBuffer;

typedef struct{
    mat4 model;
    mat4 proj;
} ImageBufferObjects;

typedef struct{
    vec4 lightPos;
    vec4 frustumPlanes[6];
    float displacementFactor;
    float tessellationFactor;
    vec2 viewportDim;
    float tessellatedEdgeSize;
} TesselationBuffer;

typedef struct{
    mat4 proj;
    mat4 view;
} LightMatrix;

typedef struct{
    vec3 light_pos __attribute__ ((aligned (16)));
    vec3 view_pos __attribute__ ((aligned (16)));
} LightPosBuff;

typedef struct{
    mat4 mats[MAX_BONES];
    float size;
} InvMatrixsBuffer;

#endif // E_BUFFERS_VARIABLES_H
