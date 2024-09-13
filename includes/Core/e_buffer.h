#ifndef E_BUFFER_H
#define E_BUFFER_H

#include "Variabels/engine_includes.h"

#define MAX_VERTEX_COUNT 65536
#define MAX_INDEX_COUNT MAX_VERTEX_COUNT * 3

struct BluePrints_T;
struct VertexParam_T;
struct IndexParam_T;

typedef enum{
    ENGINE_BUFFER_ALLOCATE_VERTEX,
    ENGINE_BUFFER_ALLOCATE_INDEX,
    ENGINE_BUFFER_ALLOCATE_UNIFORM,
    ENGINE_BUFFER_ALLOCATE_STAGING,
} BufferAllocType;

typedef struct BufferObject{
    BufferAllocType type;
    VkBuffer        buffer;
    VkDeviceMemory  memory;
} BufferObject;

typedef struct BufferContainer{
    BufferObject* buffers;
    uint32_t type_size;
    uint32_t count;
} BufferContainer;

void BuffersCreateCommandPool();

void BuffersCreateCommand();

void BuffersClearAll();
void BuffersDestroyContainer(BufferContainer *container);
void BuffersDestroyBuffer(BufferObject *buffer);

int BuffersCreateVertexInst(struct VertexParam_T* vert);
int BuffersCreateVertex(struct VertexParam_T* vert);
int BuffersUpdateVertex(struct VertexParam_T* vert);

int BuffersCreateIndexInst(struct IndexParam_T* ind);
int BuffersCreateIndex(struct IndexParam_T* ind);
int BuffersUpdateIndex(struct IndexParam_T* ind);

void BuffersRecreateUniform(struct BluePrints_T* blueprints);

void BuffersCreateStorage(BufferContainer* uniform);
void BuffersCreateUniform(BufferContainer* uniform);

uint32_t findMemoryType(uint32_t typeFilter, uint32_t properties);

void BuffersCreate(uint64_t size, uint32_t usage, uint32_t properties, BufferObject *buffer, uint32_t type);

void BuffersCopy(BufferObject* srcBuffer, BufferObject* dstBuffer, uint64_t size);


#endif // E_BUFFER_H
