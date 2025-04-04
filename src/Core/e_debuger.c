#include "Core/e_window.h"
#include "Core/e_debuger.h"

#include <vulkan/vulkan.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "Data/e_resource_engine.h"

extern TEngine engine;

uint32_t CreateDebugUtilsMessengerEXT(void* arg, const EdDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const EdAllocationCallbacks* pAllocator, void** messenger) {

    VkInstance *instance = arg;
    VkDebugUtilsMessengerEXT* pDebugMessenger = (struct VkDebugUtilsMessengerEXT_t **) messenger;

    PFN_vkCreateDebugUtilsMessengerEXT func1 = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func1 != NULL) {
        return func1((VkInstance)instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(void* arg, void* debugMessenger, const EdAllocationCallbacks* pAllocator) {

    VkInstance *instance = arg;

    PFN_vkDestroyDebugUtilsMessengerEXT func2 = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func2 != NULL) {
        func2((VkInstance)instance, (VkDebugUtilsMessengerEXT)debugMessenger, pAllocator);
    }
}


void *thread_f(const char *text){

    printf("validation layer : %s\n", text);

}

uint32_t debugCallback(
        uint32_t messageSeverity,
        uint32_t messageType,
        const void** CallbackData,
        void* pUserData){

    VkDebugUtilsMessengerCallbackDataEXT *pCallbackData = (struct VkDebugUtilsMessengerCallbackDataEXT *) CallbackData;

    if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        int i =0;
    }

    printf("validation layer : %s\n", pCallbackData->pMessage);

    /*
    pthread_t threader;
    pthread_create(&threader, NULL, thread_f, (void *)pCallbackData->pMessage);
    //переводим в отсоединенное состояние
    pthread_detach(threader);*/

    return VK_FALSE;

}

void populateDebugMessengerCreateInfo(EdDebugUtilsMessengerCreateInfoEXT* createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

void setupDebugMessenger(){

    TWindow *window = (TWindow *)engine.window;

    if(!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT debugInfo;
    memset(&debugInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));

    populateDebugMessengerCreateInfo((EdDebugUtilsMessengerCreateInfoEXT *) &debugInfo);
    if(CreateDebugUtilsMessengerEXT(window->instance, (const EdDebugUtilsMessengerCreateInfoEXT *)&debugInfo, NULL, &engine.debugMessenger) != VK_SUCCESS)
    {
        printf("failed create debug messnger");
        exit(1);
    }

}
