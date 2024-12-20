/*
* Android Vulkan function pointer loader
*
* Copyright (C) 2016-2024 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "Core/vulkan_android.h"

#if defined(__ANDROID__)
	#include <android/log.h>
	#include <dlfcn.h>
	#include <android/native_window_jni.h>

struct android_app* androidApp;

const int32_t DOUBLE_TAP_TIMEOUT = 300 * 1000000;
const int32_t TAP_TIMEOUT = 180 * 1000000;
const int32_t DOUBLE_TAP_SLOP = 100;
const int32_t TAP_SLOP = 8;

PFN_vkCreateInstance vkCreateInstance;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkMapMemory vkMapMemory;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCmdCopyImage vkCmdCopyImage;
PFN_vkCmdBlitImage vkCmdBlitImage;
PFN_vkCmdClearAttachments vkCmdClearAttachments;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkCmdNextSubpass vkCmdNextSubpass;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
PFN_vkCmdClearColorImage vkCmdClearColorImage;
PFN_vkCreateImage vkCreateImage;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkCreateFence vkCreateFence;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkResetFences vkResetFences;
PFN_vkResetDescriptorPool vkResetDescriptorPool;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueueWaitIdle vkQueueWaitIdle;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkCreatePipelineCache vkCreatePipelineCache;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCreateComputePipelines vkCreateComputePipelines;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
PFN_vkCmdPushConstants vkCmdPushConstants;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
PFN_vkCmdDispatch vkCmdDispatch;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
PFN_vkDestroyRenderPass vkDestroyRenderPass;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
PFN_vkCreateQueryPool vkCreateQueryPool;
PFN_vkDestroyQueryPool vkDestroyQueryPool;
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
PFN_vkCmdBeginQuery vkCmdBeginQuery;
PFN_vkCmdEndQuery vkCmdEndQuery;
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
PFN_vkQueueBindSparse vkQueueBindSparse;
PFN_vkCmdBeginRendering vkCmdBeginRendering;
PFN_vkCmdEndRendering vkCmdEndRendering;

PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
PFN_vkCmdFillBuffer vkCmdFillBuffer;

PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkQueuePresentKHR vkQueuePresentKHR;

PFN_vkResetCommandBuffer vkResetCommandBuffer;

PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;

int32_t screenDensity;

void *libVulkan;

// Dynamically load Vulkan library and base function pointers
bool AndroidLoadVulkanLibrary()
{
    ///__android_log_print(ANDROID_LOG_INFO, "vulkanandroid", "Loading libvulkan.so...\n");

    // Load vulkan library
    libVulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!libVulkan)
    {
        ///__android_log_print(ANDROID_LOG_INFO, "vulkanandroid", "Could not load vulkan library : %s!\n", dlerror());
        return false;
    }

    // Load base function pointers
    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)(dlsym(libVulkan, "vkEnumerateInstanceExtensionProperties"));
    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)(dlsym(libVulkan, "vkEnumerateInstanceLayerProperties"));
    vkCreateInstance = (PFN_vkCreateInstance)(dlsym(libVulkan, "vkCreateInstance"));
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)(dlsym(libVulkan, "vkGetInstanceProcAddr"));
    vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)(dlsym(libVulkan, "vkGetDeviceProcAddr"));

    return true;
}

// Load instance based Vulkan function pointers
void AndroidLoadVulkanFunctions(VkInstance instance)
{
    ///__android_log_print(ANDROID_LOG_INFO, "vulkanandroid", "Loading instance based function pointers...\n");

    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)(vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));
    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties"));
    vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2"));
    vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)(vkGetInstanceProcAddr(instance, "vkEnumerateDeviceLayerProperties"));
    vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)(vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties"));
    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties"));
    vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures"));
    vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2"));
    vkCreateDevice = (PFN_vkCreateDevice)(vkGetInstanceProcAddr(instance, "vkCreateDevice"));
    vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties"));
    vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties"));

    vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)(vkGetInstanceProcAddr(instance, "vkCmdPipelineBarrier"));
    vkCreateShaderModule = (PFN_vkCreateShaderModule)(vkGetInstanceProcAddr(instance, "vkCreateShaderModule"));

    vkCreateBuffer = (PFN_vkCreateBuffer)(vkGetInstanceProcAddr(instance, "vkCreateBuffer"));
    vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)(vkGetInstanceProcAddr(instance, "vkGetBufferMemoryRequirements"));
    vkMapMemory = (PFN_vkMapMemory)(vkGetInstanceProcAddr(instance, "vkMapMemory"));
    vkUnmapMemory = (PFN_vkUnmapMemory)(vkGetInstanceProcAddr(instance, "vkUnmapMemory"));
    vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)(vkGetInstanceProcAddr(instance, "vkFlushMappedMemoryRanges"));
    vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)(vkGetInstanceProcAddr(instance, "vkInvalidateMappedMemoryRanges"));
    vkBindBufferMemory = (PFN_vkBindBufferMemory)(vkGetInstanceProcAddr(instance, "vkBindBufferMemory"));
    vkDestroyBuffer = (PFN_vkDestroyBuffer)(vkGetInstanceProcAddr(instance, "vkDestroyBuffer"));

    vkAllocateMemory = (PFN_vkAllocateMemory)(vkGetInstanceProcAddr(instance, "vkAllocateMemory"));
    vkFreeMemory = (PFN_vkFreeMemory)(vkGetInstanceProcAddr(instance, "vkFreeMemory"));
    vkCreateRenderPass = (PFN_vkCreateRenderPass)(vkGetInstanceProcAddr(instance, "vkCreateRenderPass"));
    vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)(vkGetInstanceProcAddr(instance, "vkCmdBeginRenderPass"));
    vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)(vkGetInstanceProcAddr(instance, "vkCmdEndRenderPass"));
    vkCmdNextSubpass = (PFN_vkCmdNextSubpass)(vkGetInstanceProcAddr(instance, "vkCmdNextSubpass"));
    vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands)(vkGetInstanceProcAddr(instance, "vkCmdExecuteCommands"));
    vkCmdClearColorImage = (PFN_vkCmdClearColorImage)(vkGetInstanceProcAddr(instance, "vkCmdClearColorImage"));

    vkCreateImage = (PFN_vkCreateImage)(vkGetInstanceProcAddr(instance, "vkCreateImage"));
    vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)(vkGetInstanceProcAddr(instance, "vkGetImageMemoryRequirements"));
    vkCreateImageView = (PFN_vkCreateImageView)(vkGetInstanceProcAddr(instance, "vkCreateImageView"));
    vkDestroyImageView = (PFN_vkDestroyImageView)(vkGetInstanceProcAddr(instance, "vkDestroyImageView"));
    vkBindImageMemory = (PFN_vkBindImageMemory)(vkGetInstanceProcAddr(instance, "vkBindImageMemory"));
    vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)(vkGetInstanceProcAddr(instance, "vkGetImageSubresourceLayout"));
    vkCmdCopyImage = (PFN_vkCmdCopyImage)(vkGetInstanceProcAddr(instance, "vkCmdCopyImage"));
    vkCmdBlitImage = (PFN_vkCmdBlitImage)(vkGetInstanceProcAddr(instance, "vkCmdBlitImage"));
    vkDestroyImage = (PFN_vkDestroyImage)(vkGetInstanceProcAddr(instance, "vkDestroyImage"));

    vkCmdClearAttachments = (PFN_vkCmdClearAttachments)(vkGetInstanceProcAddr(instance, "vkCmdClearAttachments"));

    vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)(vkGetInstanceProcAddr(instance, "vkCmdCopyBuffer"));
    vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)(vkGetInstanceProcAddr(instance, "vkCmdCopyBufferToImage"));

    vkCreateSampler = (PFN_vkCreateSampler)(vkGetInstanceProcAddr(instance, "vkCreateSampler"));
    vkDestroySampler = (PFN_vkDestroySampler)(vkGetInstanceProcAddr(instance, "vkDestroySampler"));;

    vkCreateSemaphore = (PFN_vkCreateSemaphore)(vkGetInstanceProcAddr(instance, "vkCreateSemaphore"));
    vkDestroySemaphore = (PFN_vkDestroySemaphore)(vkGetInstanceProcAddr(instance, "vkDestroySemaphore"));

    vkCreateFence = (PFN_vkCreateFence)(vkGetInstanceProcAddr(instance, "vkCreateFence"));
    vkDestroyFence = (PFN_vkDestroyFence)(vkGetInstanceProcAddr(instance, "vkDestroyFence"));
    vkWaitForFences = (PFN_vkWaitForFences)(vkGetInstanceProcAddr(instance, "vkWaitForFences"));
    vkResetFences = (PFN_vkResetFences)(vkGetInstanceProcAddr(instance, "vkResetFences"));;
    vkResetDescriptorPool = (PFN_vkResetDescriptorPool)(vkGetInstanceProcAddr(instance, "vkResetDescriptorPool"));

    vkCreateCommandPool = (PFN_vkCreateCommandPool)(vkGetInstanceProcAddr(instance, "vkCreateCommandPool"));
    vkDestroyCommandPool = (PFN_vkDestroyCommandPool)(vkGetInstanceProcAddr(instance, "vkDestroyCommandPool"));;

    vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)(vkGetInstanceProcAddr(instance, "vkAllocateCommandBuffers"));
    vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)(vkGetInstanceProcAddr(instance, "vkBeginCommandBuffer"));
    vkEndCommandBuffer = (PFN_vkEndCommandBuffer)(vkGetInstanceProcAddr(instance, "vkEndCommandBuffer"));

    vkGetDeviceQueue = (PFN_vkGetDeviceQueue)(vkGetInstanceProcAddr(instance, "vkGetDeviceQueue"));
    vkQueueSubmit = (PFN_vkQueueSubmit)(vkGetInstanceProcAddr(instance, "vkQueueSubmit"));
    vkQueueWaitIdle = (PFN_vkQueueWaitIdle)(vkGetInstanceProcAddr(instance, "vkQueueWaitIdle"));

    vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)(vkGetInstanceProcAddr(instance, "vkDeviceWaitIdle"));

    vkCreateFramebuffer = (PFN_vkCreateFramebuffer)(vkGetInstanceProcAddr(instance, "vkCreateFramebuffer"));

    vkCreatePipelineCache = (PFN_vkCreatePipelineCache)(vkGetInstanceProcAddr(instance, "vkCreatePipelineCache"));
    vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)(vkGetInstanceProcAddr(instance, "vkCreatePipelineLayout"));
    vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)(vkGetInstanceProcAddr(instance, "vkCreateGraphicsPipelines"));
    vkCreateComputePipelines = (PFN_vkCreateComputePipelines)(vkGetInstanceProcAddr(instance, "vkCreateComputePipelines"));

    vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)(vkGetInstanceProcAddr(instance, "vkCreateDescriptorPool"));
    vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)(vkGetInstanceProcAddr(instance, "vkCreateDescriptorSetLayout"));

    vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)(vkGetInstanceProcAddr(instance, "vkAllocateDescriptorSets"));
    vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)(vkGetInstanceProcAddr(instance, "vkUpdateDescriptorSets"));

    vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)(vkGetInstanceProcAddr(instance, "vkCmdBindDescriptorSets"));
    vkCmdBindPipeline = (PFN_vkCmdBindPipeline)(vkGetInstanceProcAddr(instance, "vkCmdBindPipeline"));
    vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)(vkGetInstanceProcAddr(instance, "vkCmdBindVertexBuffers"));
    vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)(vkGetInstanceProcAddr(instance, "vkCmdBindIndexBuffer"));

    vkCmdSetViewport = (PFN_vkCmdSetViewport)(vkGetInstanceProcAddr(instance, "vkCmdSetViewport"));
    vkCmdSetScissor = (PFN_vkCmdSetScissor)(vkGetInstanceProcAddr(instance, "vkCmdSetScissor"));
    vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth)(vkGetInstanceProcAddr(instance, "vkCmdSetLineWidth"));
    vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)(vkGetInstanceProcAddr(instance, "vkCmdSetDepthBias"));
    vkCmdPushConstants = (PFN_vkCmdPushConstants)(vkGetInstanceProcAddr(instance, "vkCmdPushConstants"));;

    vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)(vkGetInstanceProcAddr(instance, "vkCmdDrawIndexed"));
    vkCmdDraw = (PFN_vkCmdDraw)(vkGetInstanceProcAddr(instance, "vkCmdDraw"));
    vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)(vkGetInstanceProcAddr(instance, "vkCmdDrawIndexedIndirect"));
    vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect)(vkGetInstanceProcAddr(instance, "vkCmdDrawIndirect"));
    vkCmdDispatch = (PFN_vkCmdDispatch)(vkGetInstanceProcAddr(instance, "vkCmdDispatch"));

    vkDestroyPipeline = (PFN_vkDestroyPipeline)(vkGetInstanceProcAddr(instance, "vkDestroyPipeline"));
    vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)(vkGetInstanceProcAddr(instance, "vkDestroyPipelineLayout"));;
    vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)(vkGetInstanceProcAddr(instance, "vkDestroyDescriptorSetLayout"));
    vkDestroyDevice = (PFN_vkDestroyDevice)(vkGetInstanceProcAddr(instance, "vkDestroyDevice"));
    vkDestroyInstance = (PFN_vkDestroyInstance)(vkGetInstanceProcAddr(instance, "vkDestroyInstance"));
    vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)(vkGetInstanceProcAddr(instance, "vkDestroyDescriptorPool"));
    vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)(vkGetInstanceProcAddr(instance, "vkFreeCommandBuffers"));
    vkDestroyRenderPass = (PFN_vkDestroyRenderPass)(vkGetInstanceProcAddr(instance, "vkDestroyRenderPass"));
    vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer)(vkGetInstanceProcAddr(instance, "vkDestroyFramebuffer"));
    vkDestroyShaderModule = (PFN_vkDestroyShaderModule)(vkGetInstanceProcAddr(instance, "vkDestroyShaderModule"));
    vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache)(vkGetInstanceProcAddr(instance, "vkDestroyPipelineCache"));

    vkCreateQueryPool = (PFN_vkCreateQueryPool)(vkGetInstanceProcAddr(instance, "vkCreateQueryPool"));
    vkDestroyQueryPool = (PFN_vkDestroyQueryPool)(vkGetInstanceProcAddr(instance, "vkDestroyQueryPool"));
    vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults)(vkGetInstanceProcAddr(instance, "vkGetQueryPoolResults"));

    vkCmdBeginQuery = (PFN_vkCmdBeginQuery)(vkGetInstanceProcAddr(instance, "vkCmdBeginQuery"));
    vkCmdEndQuery = (PFN_vkCmdEndQuery)(vkGetInstanceProcAddr(instance, "vkCmdEndQuery"));
    vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool)(vkGetInstanceProcAddr(instance, "vkCmdResetQueryPool"));
    vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)(vkGetInstanceProcAddr(instance, "vkCmdCopyQueryPoolResults"));

    vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties"));
    vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements)(vkGetInstanceProcAddr(instance, "vkGetImageSparseMemoryRequirements"));
    vkQueueBindSparse = (PFN_vkQueueBindSparse)(vkGetInstanceProcAddr(instance, "vkQueueBindSparse"));

    vkCmdBeginRendering = (PFN_vkCmdBeginRendering)(vkGetInstanceProcAddr(instance, "vkCmdBeginRendering"));
    vkCmdEndRendering = (PFN_vkCmdEndRendering)(vkGetInstanceProcAddr(instance, "vkCmdEndRendering"));

    vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)(vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR"));
    vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)(vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR"));

    vkCmdFillBuffer = (PFN_vkCmdFillBuffer)(vkGetInstanceProcAddr(instance, "vkCmdFillBuffer"));

    vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
    vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)(vkGetInstanceProcAddr(instance, "vkCreateSwapchainKHR"));
    vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)(vkGetInstanceProcAddr(instance, "vkDestroySwapchainKHR"));
    vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)(vkGetInstanceProcAddr(instance, "vkGetSwapchainImagesKHR"));
    vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)(vkGetInstanceProcAddr(instance, "vkAcquireNextImageKHR"));
    vkQueuePresentKHR = (PFN_vkQueuePresentKHR)(vkGetInstanceProcAddr(instance, "vkQueuePresentKHR"));

    vkResetCommandBuffer = (PFN_vkResetCommandBuffer)(vkGetInstanceProcAddr(instance, "vkResetCommandBuffer"));

    vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties)(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties"));
}

void AndroidFreeVulkanLibrary()
{
    dlclose(libVulkan);
}

void AndroidGetDeviceConfig()
{
    // Screen density
    /*AConfiguration* config = AConfiguration_new();
    AConfiguration_fromAssetManager(config, androidApp->activity->assetManager);
    screenDensity = AConfiguration_getDensity(config);
    AConfiguration_delete(config);*/
}

// Displays a native alert dialog using JNI
void AndroidShowAlert(const char* message) {
    /*JNIEnv* jni;
    androidApp->activity->vm->AttachCurrentThread(&jni, NULL);

    jstring jmessage = jni->NewStringUTF(message);

    jclass clazz = jni->GetObjectClass(androidApp->activity->clazz);
    // Signature has to match java implementation (arguments)
    jmethodID methodID = jni->GetMethodID(clazz, "showAlert", "(Ljava/lang/String;)V");
    jni->CallVoidMethod(androidApp->activity->clazz, methodID, jmessage);
    jni->DeleteLocalRef(jmessage);

    androidApp->activity->vm->DetachCurrentThread();*/
    return;
}

#endif
