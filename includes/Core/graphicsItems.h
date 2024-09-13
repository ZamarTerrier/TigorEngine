#ifndef GRAPHICSITEMS_H
#define GRAPHICSITEMS_H

#include "Core/e_buffer.h"

#include "Variabels/engine_includes.h"

#define MAX_SHAPES 8

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    ENGINE_SHADER_OBJECT_NONE = 0x1,
    ENGINE_SHADER_OBJECT_READED = 0x2
} ShaderObjectFlags;

typedef struct{
    char* code;
    size_t size;
    ShaderObjectFlags flags;
} ShaderObject;

typedef struct{
    void *pipeline;
    void *layout;
} PipelineStruct;

typedef struct{
    void *descr_pool;
    void *descr_set_layout;
    VkDescriptorSet *descr_sets;
} ShaderDescriptor;

typedef struct{
    ShaderDescriptor descriptor;
    PipelineStruct pipeline;
} ShaderPack;

typedef struct{
    ShaderPack shader_packs[MAX_BLUE_PRINTS];
    uint32_t num_shader_packs;
    bool perspective;
} GraphicItems;

typedef struct{
    BufferObject buffer;
    uint32_t typeSize;
    uint32_t verticesSize;
    uint32_t bufferSize;
    bool extend;
    void* vertices;
} vertexParam;

typedef struct{
    BufferObject buffer;
    uint32_t typeSize;
    uint32_t indexesSize;
    uint32_t bufferSize;
    bool extend;
    uint32_t* indices;
} indexParam;

typedef struct{
    char vertShader[256];
    char fragShader[256];
} aboutShader;

typedef struct{
    indexParam iParam;
    vertexParam vParam;

    EIVertexInputBindingDescription* bindingDescription;
    EIVertexInputAttributeDescription* attr;
    uint32_t countBind;
    uint32_t countAttr;

    uint32_t type;

    bool init;
} Shape;

enum EngineDescriptorType{
    ENGINE_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    ENGINE_DESCRIPTOR_TYPE_IMAGE_SAMPLER,
    ENGINE_DESCRIPTOR_TYPE_IMAGE_ARRAY
};

#ifdef __cplusplus
}
#endif

#endif // GRAPHICSITEMS_H
