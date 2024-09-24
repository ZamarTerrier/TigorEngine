#ifndef E_DEVICE_H
#define E_DEVICE_H

#include "Variabels/engine_includes.h"

#include "swapchain.h"
#include "Tools/e_tools.h"

typedef struct{
    
    void* e_physicalDevice;
    void* e_device;

    void* graphicsQueue;
    void* presentQueue;
    
    void* commandPool;
    void** commandBuffers;

} TDevice;

bool checkDeviceExtensionSupport(void* device);

bool isDeviceSuitable(void* device);

void pickPhysicalDevice();
void createLogicalDevice();

#endif
