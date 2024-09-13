#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include "e_texture.h"

#include "Variabels/engine_includes.h"

#include "e_texture.h"
#include "Tools/e_tools.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

typedef struct{
    VkSwapchainKHR  swapChain;
    VkImage *swapChainImages;
    VkImageView *swapChainImageViews;

    Texture2D depth_texture;
        
    uint32_t swapChainImageFormat;
    
    EIExtent2D swapChainExtent;
} ZSwapChain;

void querySwapChainSupport(VkPhysicalDevice device, SwapChainSupportDetails* details) ;

EDSurfaceFormatKHR chooseSwapSurfaceFormat(const EDSurfaceFormatKHR* availableFormats, uint32_t sizeFormats);

uint32_t chooseSwapPresentMode(const VkPresentModeKHR *availablePresentModes, uint32_t sizeModes);

EIExtent2D chooseSwapExtent(const EISurfaceCapabilitiesKHR capabilities);

void SwapChainCreate();

void SwapChainCreateImageViews();

void RecreateSwapChain();

void CleanupSwapChain();


#ifdef __cplusplus
}
#endif

#endif // SWAPCHAIN_H
