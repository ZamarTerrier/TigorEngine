#include "Core/e_memory.h"
#include "Core/e_device.h"
#include "Core/e_buffer.h"
#include "Core/e_blue_print.h"

#include <vulkan/vulkan.h>

#include "Objects/gameObject2D.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern ZEngine engine;

void BuffersCreateCommandPool() {
    ZDevice *device = (ZDevice *)engine.device;

    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(device->e_physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device->e_device, &poolInfo, NULL, (struct VkCommandPool_T **)&device->commandPool) != VK_SUCCESS) {
        printf("failed to create command pool!");
        exit(1);
    }

}

void BuffersCreateCommand(){
    ZDevice *device = (ZDevice *)engine.device;

    device->commandBuffers = AllocateMemoryP(engine.imagesCount, sizeof(VkCommandBuffer), &device);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = device->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) engine.imagesCount;

    if (vkAllocateCommandBuffers(device->e_device, &allocInfo, (struct VkCommandBuffer_T **)device->commandBuffers) != VK_SUCCESS) {
        printf("failed to allocate command buffers!");
        exit(1);
    }

}

int BuffersCreateVertex(struct VertexParam_T* vert) {
    vertexParam *vertex = (vertexParam *)vert;

    //Выделение памяти
    VkDeviceSize bufferSize;

    bufferSize = vertex->typeSize * vertex->verticesSize;

    if(bufferSize == 0)
        return 1;

    BuffersCreate(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertex->buffer, ENGINE_BUFFER_ALLOCATE_VERTEX);

    vertex->bufferSize = bufferSize;
    vertex->extend = false;

    return 0;
}

int BuffersCreateVertexInst(struct VertexParam_T* vert) {
    vertexParam *vertex = (vertexParam *)vert;

    if(vertex->verticesSize >= MAX_VERTEX_COUNT)
    {
        printf("Очень много вершин!\n");
        return 1;
    }

    //Выделение памяти
    VkDeviceSize bufferSize;

    bufferSize = vertex->typeSize * MAX_VERTEX_COUNT;

    BuffersCreate(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertex->buffer, ENGINE_BUFFER_ALLOCATE_VERTEX);

    vertex->bufferSize = bufferSize;
    vertex->extend = true;

    return 0;
}

int BuffersUpdateVertex(struct VertexParam_T* vert) {
    vertexParam *vertex = (vertexParam *)vert;

    ZDevice *device = (ZDevice *)engine.device;

    BufferObject stagingBuffer;
    VkDeviceSize bufferSize;

    bufferSize = vertex->typeSize * vertex->verticesSize;

    if(!vertex->extend){
        if(bufferSize != vertex->bufferSize)
            return 1;
    }
    else{
        if(vertex->verticesSize >= MAX_VERTEX_COUNT)
        {
            printf("Очень много вершин!\n");
            return 1;
        }
    }

    if(vertex->extend)
        if(bufferSize != vertex->bufferSize)
            return 1;

    BuffersCreate(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, ENGINE_BUFFER_ALLOCATE_STAGING);

    //Изменение памяти
    void* data;
    vkMapMemory(device->e_device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memset(data, 0, vertex->bufferSize);
    memcpy(data, vertex->vertices, (size_t) bufferSize);
    vkUnmapMemory(device->e_device, stagingBuffer.memory);

    //-------------

    BuffersCopy(&stagingBuffer, &vertex->buffer, bufferSize);

    BuffersDestroyBuffer(&stagingBuffer);

    if(vertex->extend)
        vertex->bufferSize = bufferSize;

    return 0;
}

int BuffersCreateIndex(struct IndexParam_T* indx) {
    indexParam *index = (indexParam *)indx;

    VkDeviceSize bufferSize = index->typeSize * index->indexesSize;

    if(bufferSize == 0)
        return 1;

    BuffersCreate(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &index->buffer, ENGINE_BUFFER_ALLOCATE_INDEX);
    index->bufferSize = bufferSize;
    index->extend = false;

    return 0;
}

int BuffersCreateIndexInst(struct IndexParam_T* indx) {
    indexParam *index = (indexParam *)indx;

    if(index->typeSize >= MAX_INDEX_COUNT)
    {
        printf("Очень много индексов!\n");
        return 1;
    }

    VkDeviceSize bufferSize = index->typeSize * MAX_INDEX_COUNT;

    BuffersCreate(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &index->buffer, ENGINE_BUFFER_ALLOCATE_INDEX);

    index->bufferSize = bufferSize;
    index->extend = true;

    return 0;
}

int BuffersUpdateIndex(struct IndexParam_T* indx)
{
    indexParam *index = (indexParam *)indx;

    ZDevice *device = (ZDevice *)engine.device;

    BufferObject stagingBuffer;
    VkDeviceSize bufferSize;

    bufferSize = index->typeSize * index->indexesSize;

    if(!index->extend){
        if(bufferSize != index->bufferSize)
            return 1;
    }
    else{
        if(index->indexesSize >= MAX_INDEX_COUNT)
        {
            printf("Очень много индексов!\n");
            return 1;
        }
    }

    BuffersCreate(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, ENGINE_BUFFER_ALLOCATE_STAGING);

    void* data;
    vkMapMemory(device->e_device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memset(data, 0, index->bufferSize);
    memcpy(data, index->indices, (size_t) bufferSize);
    vkUnmapMemory(device->e_device, stagingBuffer.memory);

    BuffersCopy(&stagingBuffer, &index->buffer, bufferSize);

    BuffersDestroyBuffer(&stagingBuffer);

    if(index->extend)
        index->bufferSize = bufferSize;

    return 0;
}

void BuffersCreateUniform(BufferContainer* uniform) {

    uniform->buffers = AllocateMemoryP(uniform->count, sizeof(BufferObject), uniform);

    for (int i = 0; i < uniform->count; i++) {
        BuffersCreate(uniform->type_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &uniform->buffers[i], ENGINE_BUFFER_ALLOCATE_UNIFORM);
    }
}

void BuffersCreateStorage(BufferContainer* uniform){
    uniform->buffers = AllocateMemoryP(uniform->count, sizeof(BufferObject), uniform);

    for (int i = 0; i < uniform->count; i++) {
        BuffersCreate(uniform->type_size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &uniform->buffers[i], ENGINE_BUFFER_ALLOCATE_UNIFORM);
    }
}

void BuffersCreateStorageVertex(BufferContainer* uniform){
    uniform->buffers = AllocateMemoryP(uniform->count, sizeof(BufferObject), uniform);

    for (int i = 0; i < uniform->count; i++) {
        BuffersCreate(uniform->type_size,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &uniform->buffers[i], ENGINE_BUFFER_ALLOCATE_UNIFORM);
    }
}

uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    ZDevice *device = (ZDevice *)engine.device;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device->e_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    printf("failed to find suitable memory type!\n");
    exit(1);

}

uint32_t alloc_counter_b = 0;

void AcceptAllocBuffer(uint32_t type, BufferObject *buffer)
{
    buffer->type = type;

    if(engine.cache.alloc_buffers_memory_head->node == NULL){
        engine.cache.alloc_buffers_memory_head->next = calloc(1, sizeof(ChildStack));
        engine.cache.alloc_buffers_memory_head->node = buffer;
    }
    else{

        ChildStack *child = engine.cache.alloc_buffers_memory_head;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = calloc(1, sizeof(ChildStack));
        child->node = buffer;
    }

    alloc_counter_b ++;
}

void BuffersCreate(uint64_t size, uint32_t usage, uint32_t properties, BufferObject *buffer, uint32_t type) {
    ZDevice *device = (ZDevice *)engine.device;

    VkBufferCreateInfo bufferInfo = {};
    memset(&bufferInfo, 0, sizeof(VkBufferCreateInfo));
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->e_device, &bufferInfo, NULL, &buffer->buffer) != VK_SUCCESS) {
        printf("Error : Failed to create buffer!\n");
        exit(1);
    }

    VkMemoryRequirements memRequirements;
    memset(&memRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(device->e_device, buffer->buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    memset(&allocInfo, 0, sizeof(VkMemoryAllocateInfo));
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; 
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->e_device, &allocInfo, NULL, &buffer->memory) != VK_SUCCESS) {
        printf("Error : Failed to allocate buffer memory!\n");
        exit(1);
    }

    vkBindBufferMemory(device->e_device, buffer->buffer, buffer->memory, 0);

    AcceptAllocBuffer(type, buffer);    
}

void BuffersDestroyBuffer(BufferObject *buffer)
{
    if(buffer == NULL || buffer->buffer == NULL)
        return;

    ZDevice *device = (ZDevice *)engine.device;

    BufferObject *curr = NULL;

    ChildStack *child = engine.cache.alloc_buffers_memory_head;
    ChildStack *before = NULL;

    while(child != NULL)
    {
        curr = child->node;

        if(curr == buffer)
            break;

        before = child;
        child = child->next;
    }

    if(curr == NULL){
        printf("Can't find this memory 0x%x\n", buffer);
        return;
    }

    if(child->next != NULL){
        vkDestroyBuffer(device->e_device, curr->buffer, NULL);
        vkFreeMemory(device->e_device, curr->memory, NULL);
        curr->buffer = VK_NULL_HANDLE;
        curr->memory = VK_NULL_HANDLE;

        if(before != NULL)
            before->next = child->next;
        else
            engine.cache.alloc_buffers_memory_head = child->next;

        free(child);
        child = NULL;

    }else{
        
        if(before != NULL){
            free(child);
            child = NULL;  
        } 
    }

    buffer->buffer = VK_NULL_HANDLE;
    buffer->memory = VK_NULL_HANDLE;
    buffer->type = 0;
}

void BuffersClearAll()
{
    ChildStack *child = engine.cache.alloc_buffers_memory_head;
    
    if(child == NULL)
        return;

    ChildStack *next = NULL;
    
    uint32_t counter = 0;       

    while(child != NULL){    
        
        next = child->next;

        if(child->node != NULL)
            counter ++;

        if(child->node != NULL)
            BuffersDestroyBuffer(child->node);
            
        child = next;
    }
    
    if(engine.cache.alloc_buffers_memory_head != NULL){
        free(engine.cache.alloc_buffers_memory_head);
        engine.cache.alloc_buffers_memory_head = NULL;
    }

    if(counter > 0)
        printf("Autofree vkBuffers count : %i\n", counter);
}

void BuffersDestroyContainer(BufferContainer *container){
    for (int i = 0; i < container->count; i++) {
        BuffersDestroyBuffer(&container->buffers[i]);
    }
    FreeMemory(container->buffers);
    container->buffers = NULL;
    container->count = 0;
}

void BuffersCopy(BufferObject *srcBuffer, BufferObject *dstBuffer, uint64_t size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer->buffer, dstBuffer->buffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void BuffersRecreateUniform(struct  BluePrints_T *bPrints){

    Blueprints *blueprints = (Blueprints *)bPrints;

    for(int i=0; i < blueprints->num_blue_print_packs;i++)
    {
        BluePrintPack *pack = &blueprints->blue_print_packs[i];

        for(int j=0;j < pack->num_descriptors;j++){
            BluePrintDescriptor *descriptor = &pack->descriptors[j];

            if(descriptor->descrType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                descriptor->uniform.count = engine.imagesCount;
                descriptor->uniform.type_size = descriptor->buffsize;
                BuffersCreateUniform(&descriptor->uniform);
            }
        }
    }
}
