#include "Core/swapchain.h"

#include <vulkan/vulkan.h>

#include "GUI/GUIManager.h"

#include "Core/e_memory.h"
#include "Core/e_device.h"
#include "Core/e_window.h"

#include "Objects/render_texture.h"

#include "Tools/e_math.h"

#include "Variabels/e_device_variables.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern TEngine engine;

void querySwapChainSupport(VkPhysicalDevice device, SwapChainSupportDetails* details) {
    TWindow *window = (TWindow *)engine.window;

#ifndef __ANDROID__
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window->surface, (VkSurfaceCapabilitiesKHR  *)&details->capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->surface, &details->sizeFormats, NULL);

    if (details->sizeFormats != 0) {
        details->formats = AllocateMemory(details->sizeFormats, sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->surface, &details->sizeFormats, (VkSurfaceFormatKHR *)details->formats);
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, (VkSurfaceKHR)window->surface, &details->sizeModes, NULL);

    if (details->sizeModes != 0) {
        details->presentModes = (VkPresentModeKHR*) AllocateMemory(details->sizeModes, sizeof(VkPresentModeKHR));
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->surface, &details->sizeModes, details->presentModes);
    }
#endif
}

EDSurfaceFormatKHR chooseSwapSurfaceFormat(const EDSurfaceFormatKHR* availableFormats, uint32_t sizeFormats) {

    for (int i=0; i < sizeFormats;i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }

    return availableFormats[0];
}

uint32_t chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes, uint32_t sizeModes) {

    for (int i=0; i < sizeModes;i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

EIExtent2D chooseSwapExtent(const EISurfaceCapabilitiesKHR capabilities) {

    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {

        VkExtent2D actualExtent = { engine.width, engine.height };

        actualExtent.width = clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return *(EIExtent2D*)&actualExtent;
    }
}

void SwapChainCreate() {
    TDevice *device = (TDevice *)engine.device;
    TWindow *window = (TWindow *)engine.window;
    TSwapChain *swapchain = (TSwapChain *)engine.swapchain;

    SwapChainSupportDetails swapChainSupport;

    querySwapChainSupport(device->e_physicalDevice, &swapChainSupport);

    EDSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.sizeFormats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.sizeModes);
    EIExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    engine.imagesCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && engine.imagesCount > swapChainSupport.capabilities.maxImageCount) {
        engine.imagesCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo;
    memset(&createInfo, 0, sizeof(VkSwapchainCreateInfoKHR));

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = (VkSurfaceKHR)window->surface;

    createInfo.minImageCount = engine.imagesCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = *(VkExtent2D*)&extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(device->e_physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device->e_device, &createInfo, NULL, (VkSwapchainKHR *)&swapchain->swapChain) != VK_SUCCESS) {
        printf("failed to create swap chain!");
        exit(1);
    }

    vkGetSwapchainImagesKHR(device->e_device, swapchain->swapChain, &engine.imagesCount, NULL);
    swapchain->swapChainImages = AllocateMemory(engine.imagesCount, sizeof(VkImage));
    vkGetSwapchainImagesKHR(device->e_device, swapchain->swapChain, &engine.imagesCount, (VkImage *)swapchain->swapChainImages);

    swapchain->swapChainImageFormat = surfaceFormat.format;
    swapchain->swapChainExtent = extent;

    
    if (swapChainSupport.sizeFormats != 0) {
        FreeMemory(swapChainSupport.formats);
    }

    if (swapChainSupport.sizeModes != 0) {
        FreeMemory(swapChainSupport.presentModes);
    }
}

void SwapChainCreateImageViews() {
    TSwapChain *swapchain = (TSwapChain *)engine.swapchain;

    swapchain->swapChainImageViews = AllocateMemory(engine.imagesCount, sizeof(VkImageView));

    for (size_t i = 0; i < engine.imagesCount; i++) {
        swapchain->swapChainImageViews[i] = TextureCreateImageView(swapchain->swapChainImages[i], VK_IMAGE_VIEW_TYPE_2D, swapchain->swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}


void RecreateSwapChain() {

    TWindow *window = (TWindow *)engine.window;
    TDevice *device = (TDevice *)engine.device;

#ifndef __ANDROID__
    wManagerGetFramebufferSize(window->e_window, &engine.width, &engine.height);

    while (engine.width == 0 || engine.height == 0) {
        wManagerGetFramebufferSize(window->e_window, &engine.width, &engine.height);
        wManagerWaitEvents();
    }
#endif

    engine.diffSize.x =  1;
    engine.diffSize.y =  1;

    vkDeviceWaitIdle(device->e_device);

    CleanupSwapChain();

    GUIManagerClear();
    
    for(int i=0; i < engine.gameObjects.size;i++)
    {
        GameObjectClean(engine.gameObjects.objects[i]);
    }
        
    SwapChainCreate();
    SwapChainCreateImageViews();
    ToolsCreateDepthResources();

    for(int i=0;i < engine.renders.size;i++)
    {
        RenderTextureRecreate(engine.renders.objects[i]);
    }

    GUIManagerRecreate();
        
    for(int i=0; i < engine.gameObjects.size;i++)
    {
        GameObjectRecreate(engine.gameObjects.objects[i]);
    }

    BuffersCreateCommand();    
}

void CleanupSwapChain() {

    TDevice *device = (TDevice *)engine.device;
    TSwapChain *swapchain = (TSwapChain *)engine.swapchain;

    vkFreeCommandBuffers(device->e_device, (VkCommandPool)device->commandPool, engine.imagesCount, (const VkCommandBuffer *) device->commandBuffers);
    FreeMemory(device->commandBuffers);
    device->commandBuffers = NULL;

    for (size_t i = 0; i < engine.imagesCount; i++) {
        vkDestroyImageView(device->e_device, swapchain->swapChainImageViews[i], NULL);
    }
    FreeMemory(swapchain->swapChainImages);
    FreeMemory(swapchain->swapChainImageViews);
    swapchain->swapChainImageViews = NULL;

    ImageDestroyTexture(&swapchain->depth_texture);

    vkDestroySwapchainKHR(device->e_device, swapchain->swapChain, NULL);

}
