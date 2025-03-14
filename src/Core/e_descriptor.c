#include "Core/e_device.h"
#include "Core/e_memory.h"
#include "Core/e_descriptor.h"

#include <vulkan/vulkan.h>

#include "Objects/gameObject2D.h"
#include "Objects/render_texture.h"

#include "Core/e_buffer.h"
#include "Core/e_descriptor.h"

#include "Variabels/e_texture_variables.h"
#include "Variabels/engine_includes.h"

#include "Data/e_resource_engine.h"

extern TEngine engine;

void DescriptorAcceptStack(ShaderDescriptor *descriptor)
{
    if(engine.cache.alloc_descriptor_head->node == NULL){
        engine.cache.alloc_descriptor_head->next = calloc(1, sizeof(ChildStack));

        engine.cache.alloc_descriptor_head->node = descriptor;
    }
    else{

        ChildStack *child = engine.cache.alloc_descriptor_head->next;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = calloc(1, sizeof(ChildStack));
        child->node = descriptor;
    }
}

void DescriptorClearAll()
{
    ChildStack *child = engine.cache.alloc_descriptor_head;
    ChildStack *next = NULL;
    
    if(child == NULL)
        return;

    uint32_t counter = 0;

    while(child != NULL)
    {
        next = child->next;

        if(child->node != NULL)
            counter ++;

        if(child->node != NULL)
            DescriptorDestroy(child->node);
            
        child = next;
    }

    if(engine.cache.alloc_descriptor_head != NULL){
        free(engine.cache.alloc_descriptor_head);
        engine.cache.alloc_descriptor_head = NULL;
    }

    if(counter > 0)
        printf("Autofree descriptors count : %i\n", counter);

}

void DescriptorDestroy(ShaderDescriptor *descriptor)
{
    if(descriptor == NULL)
        return;
    else if(descriptor->descr_pool == VK_NULL_HANDLE)
        return;

    TDevice *device = (TDevice *)engine.device;

    ChildStack *child = engine.cache.alloc_descriptor_head;
    ShaderDescriptor *curr = NULL;    
    ChildStack *before = NULL;

    while(child != NULL)
    {
        curr = child->node;

        if(curr != NULL){
            if(child->node == descriptor)
                break; 
        }

        before = child;
        child = child->next;
    }

    if(curr == NULL){
        printf("Can't find this memory : 0x%x\n");
        return;
    }

    if(child->next != NULL){
        vkFreeDescriptorSets(device->e_device, descriptor->descr_pool, engine.imagesCount, descriptor->descr_sets);
        vkDestroyDescriptorPool(device->e_device, descriptor->descr_pool, NULL);
        vkDestroyDescriptorSetLayout(device->e_device, descriptor->descr_set_layout, NULL);
        FreeMemory(descriptor->descr_sets);
        child->node = NULL;
        
        descriptor->descr_pool = VK_NULL_HANDLE;
        descriptor->descr_set_layout = VK_NULL_HANDLE;
        descriptor->descr_sets = VK_NULL_HANDLE;

        if(before != NULL)
            before->next = child->next;
        else
            engine.cache.alloc_descriptor_head = child->next;
            
        free(child);
        child = NULL;
    }

}

void DescriptorUpdateIndex(BluePrintDescriptor *descriptor, char *data, uint32_t size_data, uint32_t index){

    BufferObject stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    BuffersCreate(descriptor->uniform.type_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, TIGOR_BUFFER_ALLOCATE_STAGING);

    BuffersCopy(&stagingBuffer,  &descriptor->uniform.buffers[index], descriptor->uniform.type_size);

    BuffersDestroyBuffer(&stagingBuffer);
}

void DescriptorUpdate(BluePrintDescriptor *descriptor, char *data, uint32_t size_data)
{

}

void DescriptorSetImage(VkWriteDescriptorSet* descriptorWrites, VkDescriptorSet descr_set, uint32_t array_size, BluePrintDescriptor *blueprint_descriptor)
{
    descriptorWrites->pImageInfo = AllocateMemoryP(array_size, sizeof(VkDescriptorImageInfo), descriptorWrites);

    VkDescriptorImageInfo* imageInfo = (VkDescriptorImageInfo *)descriptorWrites->pImageInfo;

    Texture2D *textures = (Texture2D *)blueprint_descriptor->textures;

    for(int i=0;i < array_size;i++)
    {
        Texture2D *text = &textures[i];

        if(blueprint_descriptor->flags & TIGOR_BLUE_PRINT_FLAG_SINGLE_IMAGE){
            imageInfo[i].imageView = textures[i].image_view;
            imageInfo[i].sampler = textures[i].sampler;
            imageInfo[i].imageLayout = textures[i].imageLayout == 0 ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : textures[i].imageLayout;
        }
        else{
            imageInfo[i].imageView = textures[i].image_view;
            imageInfo[i].sampler = textures[i].sampler;
            imageInfo[i].imageLayout = textures[i].imageLayout == 0 ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : textures[i].imageLayout;
        }

    }

    descriptorWrites->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites->dstSet = (VkDescriptorSet)descr_set;
    descriptorWrites->dstBinding = blueprint_descriptor->binding;
    descriptorWrites->dstArrayElement = 0;
    descriptorWrites->descriptorCount = blueprint_descriptor->count;
    descriptorWrites->descriptorType = blueprint_descriptor->descrType;
}

void DescriptorSetBuffer(VkWriteDescriptorSet* descriptorWrites, VkDescriptorSet descr_set, VkBuffer uniform_buffer, BluePrintDescriptor *blueprint_descriptor)
{
    descriptorWrites->pBufferInfo = AllocateMemoryP(1, sizeof(VkDescriptorBufferInfo), descriptorWrites);

    VkDescriptorBufferInfo *bufferInfo = (VkDescriptorBufferInfo *)descriptorWrites->pBufferInfo;

    bufferInfo->buffer = (VkBuffer)uniform_buffer;//юнибавер
    bufferInfo->offset = 0;
    bufferInfo->range = blueprint_descriptor->uniform.type_size;//размер юниформ бафера

    descriptorWrites->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites->dstSet = (VkDescriptorSet)descr_set;
    descriptorWrites->dstBinding = blueprint_descriptor->binding;
    descriptorWrites->dstArrayElement = 0;
    descriptorWrites->descriptorCount = blueprint_descriptor->count;
    descriptorWrites->descriptorType = blueprint_descriptor->descrType;
}

void DescriptorCreate(ShaderDescriptor *descriptor, BluePrintDescriptor *descriptors, Blueprints *blueprints, size_t num_descr, size_t num_frame) {

    TDevice *device = (TDevice *)engine.device;

    //Создаем параметры дескриптора
    {
        VkDescriptorSetLayoutBinding bindings[num_descr];
        memset(bindings, 0, sizeof(VkDescriptorSetLayoutBinding) * num_descr);

        for(int i=0;i<num_descr;i++)
        {
            bindings[i].binding = descriptors[i].binding;
            bindings[i].descriptorType = descriptors[i].descrType;
            bindings[i].descriptorCount = descriptors[i].count;
            bindings[i].pImmutableSamplers = NULL;
            bindings[i].stageFlags = descriptors[i].stageflag;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = num_descr;
        layoutInfo.pBindings = bindings;

        if (vkCreateDescriptorSetLayout(device->e_device, &layoutInfo, NULL, (VkDescriptorSetLayout *)&descriptor->descr_set_layout) != VK_SUCCESS) {
            printf("failed to create descriptor set layout!");
            exit(1);
        }

    }

    //Создаем пулл дескрипторов для шейдера
    {
        VkDescriptorPoolSize poolSizes[num_descr];
        memset(poolSizes, 0, sizeof(VkDescriptorPoolSize) * num_descr);

        for(int i=0;i < num_descr; i++)
        {
            poolSizes[i].type = descriptors[i].descrType;
            //[Разобраться в корректном настраивании этого параметра]
            poolSizes[i].descriptorCount = descriptors[i].count * num_frame;
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = num_descr;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = num_frame;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        if (vkCreateDescriptorPool(device->e_device, &poolInfo, NULL, (VkDescriptorPool *) &descriptor->descr_pool) != VK_SUCCESS) {
            printf("failed to create descriptor pool!");
            exit(1);
        }

        //Создаем сами дескрипторы
        //-------------------------
        //Создаем идентичные друг другу сеты дескрипторов
        VkDescriptorSetLayout layouts[num_frame];
        memset(layouts, 0, sizeof(VkDescriptorSetLayout) * num_frame);

        for(int i=0; i < num_frame;i++)
        {
            layouts[i] = (VkDescriptorSetLayout)descriptor->descr_set_layout;
        }

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = (VkDescriptorPool)descriptor->descr_pool;
        allocInfo.descriptorSetCount = num_frame;
        allocInfo.pSetLayouts = layouts;

        descriptor->descr_sets = AllocateMemoryP(num_frame, sizeof(VkDescriptorSet), descriptor);
        if (vkAllocateDescriptorSets(device->e_device, &allocInfo, (VkDescriptorSet *) descriptor->descr_sets) != VK_SUCCESS) {
            printf("failed to allocate descriptor sets!");
            exit(1);
        }

    }

    VkWriteDescriptorSet descriptorWrites[num_descr];
    memset(descriptorWrites, 0, sizeof(VkWriteDescriptorSet) * num_descr);

    //-------------------------------------------------
    //Дескрипторы для всех изображений
    for (int i = 0; i < num_frame; i++) {
        for(int j=0;j < num_descr;j++)
        {
            BluePrintDescriptor *blueprint_descriptor = &descriptors[j];

            if(blueprint_descriptor->descrType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || blueprint_descriptor->descrType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER){

                //Дескриптор Юнибафферов

                DescriptorSetBuffer(&descriptorWrites[blueprint_descriptor->binding], descriptor->descr_sets[i], blueprint_descriptor->uniform.buffers[i].buffer, blueprint_descriptor);

             }else if(blueprint_descriptor->descrType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || blueprint_descriptor->descrType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE){

                //Дескриптор Изображений для шейдера
                Texture2D *textures = (Texture2D *)blueprint_descriptor->textures;

                DescriptorSetImage(&descriptorWrites[blueprint_descriptor->binding], descriptor->descr_sets[i], blueprint_descriptor->count, blueprint_descriptor);

            }
        }

        //--------------------------------------

        vkUpdateDescriptorSets(device->e_device, num_descr, descriptorWrites, 0, NULL);

        for(int j=0;j<num_descr; j++)
        {
            BluePrintDescriptor *blueprint_descriptor = &descriptors[j];
            if(blueprint_descriptor->descrType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                FreeMemory((void *)descriptorWrites[blueprint_descriptor->binding].pImageInfo);
            else
                FreeMemory((void *)descriptorWrites[blueprint_descriptor->binding].pBufferInfo);
        }
    }

    DescriptorAcceptStack(descriptor);

    //--------------------------------------

}
