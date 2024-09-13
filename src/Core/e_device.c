#include "Core/e_window.h"
#include "Core/e_device.h"
#include "Core/e_memory.h"

#include <vulkan/vulkan.h>

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern ZEngine engine;

bool checkDeviceExtensionSupport(void* arg) {

    VkPhysicalDevice *device = arg;

    vkEnumerateDeviceExtensionProperties(*device, NULL, &engine.extensionCount, NULL);

    VkExtensionProperties* availableExtensions = (VkExtensionProperties*) AllocateMemory( engine.extensionCount, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(*device, NULL, &engine.extensionCount, availableExtensions);

    const char** requiredExtensions = (const char**) AllocateMemory(num_dev_extensions, sizeof(char*));

    for(int i=0;i < engine.extensionCount;i++)
    {
        for(int j=0;j<num_dev_extensions;j++)
        {
            char *name = availableExtensions[i].extensionName;

            if(strcmp(deviceExtensions[j], availableExtensions[i].extensionName) == 0)
            {
                requiredExtensions[j] = (char *) AllocateMemory(strlen(availableExtensions[i].extensionName), sizeof(char));
                memcpy((void *)requiredExtensions[j], (const void *)deviceExtensions[j], sizeof(char) * strlen(availableExtensions[i].extensionName));
            }
        }
    }

    bool empty = true;

    for(int i=0;i<num_dev_extensions;i++)
    {
        if(requiredExtensions[i] != NULL)
            empty = false;

        FreeMemory((void *)requiredExtensions[i]);
    }

    FreeMemory(availableExtensions);
    FreeMemory(requiredExtensions); 

    return !empty;
}

bool isDeviceSuitable(void* arg) {

    VkPhysicalDevice *device = arg;

    QueueFamilyIndices indices = findQueueFamilies(*device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = true;

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(*device, &supportedFeatures);

    return isComplete(indices) && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy && supportedFeatures.fillModeNonSolid && supportedFeatures.tessellationShader;

}

void pickPhysicalDevice() {
    ZWindow *window = (ZWindow *)engine.window;
    ZDevice *device = (ZDevice *)engine.device;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(window->instance, &deviceCount, NULL);

    if (deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!");
        exit(1);
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice *) AllocateMemory(deviceCount, sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(window->instance, &deviceCount, devices);

    VkPhysicalDevice tDevice;
    for (int i=0; i < deviceCount; i++){
        tDevice = devices[i];
        bool temp = isDeviceSuitable(&tDevice);
        if (temp) {
            device->e_physicalDevice = tDevice;
            break;
        }
    }

    if(device->e_physicalDevice == NULL) {
        printf("failed to find a suitable GPU!");
        exit(1);
    }
    
    FreeMemory(devices);
}

void createLogicalDevice() {
    ZDevice *device = (ZDevice *)engine.device;

    VkDeviceQueueCreateInfo* queueCreateInfos;
    uint32_t* uniqueQueueFamilies;

    QueueFamilyIndices indices = findQueueFamilies(device->e_physicalDevice);

    if(engine.present && indices.graphicsFamily != indices.presentFamily){
        queueCreateInfos = (VkDeviceQueueCreateInfo*) AllocateMemory(2, sizeof(VkDeviceQueueCreateInfo));
        uniqueQueueFamilies = (uint32_t*) AllocateMemory(2, sizeof(uint32_t));
        uniqueQueueFamilies[0] = indices.graphicsFamily;
        uniqueQueueFamilies[1] = indices.presentFamily;
    }else{
        queueCreateInfos = (VkDeviceQueueCreateInfo*) AllocateMemory(1, sizeof(VkDeviceQueueCreateInfo));
        uniqueQueueFamilies = (uint32_t*) AllocateMemory(1, sizeof(uint32_t));
        uniqueQueueFamilies[0] = indices.graphicsFamily;
    }

    float queuePriority = 1.0f;

    uint32_t queueCount = engine.present && indices.graphicsFamily != indices.presentFamily ? 2 : 1;
    for (int i=0;i<queueCount;i++) {
        VkDeviceQueueCreateInfo queueCreateInfo;
        memset(&queueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));

        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[i] = queueCreateInfo;
    }

    VkPhysicalDeviceFeatures deviceFeatures;
    memset(&deviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;
    deviceFeatures.tessellationShader = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;

    VkDeviceCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkDeviceCreateInfo));

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos;
    createInfo.queueCreateInfoCount =  queueCount;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = num_dev_extensions;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = num_valid_layers;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(device->e_physicalDevice, &createInfo, NULL, (VkDevice *) &device->e_device) != VK_SUCCESS) {
        printf("failed to create logical device!");
        exit(1);
    }

    if(engine.present)
        vkGetDeviceQueue(device->e_device, indices.presentFamily,  0, (VkQueue *)&device->presentQueue);
                
    vkGetDeviceQueue(device->e_device, indices.graphicsFamily, 0, (VkQueue *)&device->graphicsQueue);

    FreeMemory(queueCreateInfos);
    FreeMemory(uniqueQueueFamilies);
}
