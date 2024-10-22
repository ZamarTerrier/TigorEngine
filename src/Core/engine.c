#include "Core/engine.h"
#include "Core/e_memory.h"
#include "Core/e_device.h"
#include "Core/e_window.h"
#include "Core/pipeline.h"
#include "Core/e_buffer.h"
#include "Core/e_camera.h"
#include "Core/e_debuger.h"
#include "Core/swapchain.h"
#include "Core/e_texture.h"
#include "Core/e_texture.h"

#include <vulkan/vulkan.h>

#include "wManager/window_manager.h"
#include "wManager/manager_includes.h"

#include "Variabels/e_texture_variables.h"

#include "string.h"

#include "Objects/gameObject.h"
#include "Objects/render_texture.h"

#include "Tools/e_tools.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern TEngine engine;

void EngineCreateSilent(){

    memset(&engine, 0, sizeof(TEngine));  
    
    engine.window = AllocateMemoryP(1, sizeof(TWindow), &engine);
    engine.device = AllocateMemoryP(1, sizeof(TDevice), &engine);
    engine.swapchain = AllocateMemoryP(1, sizeof(TSwapChain), &engine);
    
    engine.cache.alloc_buffers_memory_head = calloc(1, sizeof(ChildStack));
    engine.cache.alloc_descriptor_head = calloc(1, sizeof(ChildStack));
    engine.cache.alloc_pipeline_head = calloc(1, sizeof(ChildStack));

    wManagerInit();
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    BuffersCreateCommandPool();
}

void EngineInitVulkan(){
    createInstance();
    createSurface();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    SwapChainCreate();
    SwapChainCreateImageViews();
    BuffersCreateCommandPool();
    ToolsCreateDepthResources();
    BuffersCreateCommand();
    EngineCreateSyncobjects();
}

void EngineDeviceWaitIdle(){
    TDevice *device = (TDevice *)engine.device;

    vkDeviceWaitIdle(device->e_device);
}

void EngineCreateSyncobjects() {

    TDevice *device = (TDevice *)engine.device;

    engine.Sync.imageAvailableSemaphores = (VkSemaphore *)AllocateMemoryP(engine.MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore), &engine);
    engine.Sync.renderFinishedSemaphores = (VkSemaphore *)AllocateMemoryP(engine.MAX_FRAMES_IN_FLIGHT, sizeof(VkSemaphore), &engine);
    engine.Sync.inFlightFences = (VkFence *)AllocateMemoryP(engine.MAX_FRAMES_IN_FLIGHT, sizeof(VkFence), &engine);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < engine.MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device->e_device, &semaphoreInfo, NULL, &engine.Sync.imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device->e_device, &semaphoreInfo, NULL, &engine.Sync.renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device->e_device, &fenceInfo, NULL, &engine.Sync.inFlightFences[i]) != VK_SUCCESS) {
            printf("failed to create synchronization objects for a frame!");
            exit(1);
        }
    }
}

void EngineWaitEvents()
{
    wManagerWaitEvents();
}
