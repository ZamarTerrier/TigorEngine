#ifndef STDINC_H
#define STDINC_H

#define _USE_MATH_DEFINES

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Core/e_memory.h"

#include "Tools/e_direct.h"

#include "e_vertex.h"
#include "e_buffers_variables.h"

#include "e_math_variables.h"

#include "wManager/window_manager.h"

#define TIGOR_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;

#define MAX_FONTS 256
#define MAX_IMAGES 256
#define MAX_PIPELINES 256
#define MAX_UNIFORMS 256
#define MAX_BLUE_PRINTS 256

#define MAX_DRAW_OBJECTS 256

#define MAX_GUI_CALLBACKS 64

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __cplusplus
    #include <stdbool.h>
#endif

struct GameObject_T;
struct RenderTexture_T;

struct TWindow_T;
struct TDevice_T;
struct TSwapChain_T;

typedef struct ChildStack{
    struct ChildStack* next;
    struct ChildStack* before;
    void *node;
} ChildStack;

typedef struct{
    struct RenderTexture *objects[MAX_DRAW_OBJECTS];
    uint32_t size;
} EngineRenderItems;

typedef struct{
    ChildStack *lights;
    uint32_t size;
} EngineLightItems;

typedef struct{
    struct GameObject *objects[MAX_DRAW_OBJECTS];
    uint32_t size;    
} EngineGameObjects;

typedef struct FontCache{
    char path[256];
    void *cdata;
    void *info;
    void *texture;
} FontCache;

typedef struct{
    char app_name[256];
    int width;
    int height;
    void* cam2D;
    void* cam3D;
    
    vec2 viewSize;
    vec2 diffSize;

    struct TWindow_T *window;
    struct TDevice_T *device;
    struct TSwapChain_T *swapchain;

    EngineRenderItems renders;
    EngineLightItems  lights;

    EngineGameObjects gameObjects;    

    struct{
        void (*DrawFunc)(void);
        void (*RecreateFunc)(void);

        e_charCallback *charCallbacks;
        int charCallbackSize;

        e_keyCallback *keyCallbacks;
        int keyCallbackSize;
    } func;
    
    struct{
        ChildStack *alloc_buffers_memory_head;
        ChildStack *alloc_descriptor_head;
        ChildStack *alloc_pipeline_head;
    } cache;

    
    void *e_var_current_entry;

    void *main_render;    
    void *current_render;

    size_t currentFrame;
    uint32_t imageIndex;
    
    uint32_t extensionCount;
    uint32_t imagesCount;
    uint32_t wManagerExtensionCount;

    bool framebufferResized;
    bool framebufferwasResized;

    void* debugMessenger;

    bool present;

    uint32_t MAX_FRAMES_IN_FLIGHT;

    struct SynC{  
        VkSemaphore* imageAvailableSemaphores;
        VkSemaphore* renderFinishedSemaphores;

        VkFence* inFlightFences;
    }Sync;

    struct DataR{        
        char *font_path;
        void *e_var_images;
        int e_var_num_images;

        FontCache *e_var_fonts;
        uint32_t e_var_num_fonts;

        int define_font_loaded;
    } DataR;

} TEngine;

typedef enum{
    TIGOR_DRAW_PARAM_FLAG_ADD_SHADOW = 0x1,
    TIGOR_DRAW_PARAM_FLAG_DRAW_INDEXED = 0x2
} EngineDrawParamFlags;

typedef struct EIExtent2D {
    uint32_t    width;
    uint32_t    height;
} EIExtent2D;

typedef struct EIExtent3D {
    uint32_t    width;
    uint32_t    height;
    uint32_t    depth;
} EIExtent3D;

typedef struct EIOffset2D {
    int32_t    x;
    int32_t    y;
} EIOffset2D;

typedef struct EIOffset3D {
    int32_t    x;
    int32_t    y;
    int32_t    z;
} EIOffset3D;

typedef struct EIRect2D {
    EIOffset2D    offset;
    EIExtent2D    extent;
} EIRect2D;

typedef struct{
    char *path;
    char *buffer;
    int size;
    int imgWidth;
    int imgHeight;
    uint32_t img_type;
    uint32_t flags;
} GameObjectImage;

typedef struct {
    vec4 color;
    float depth;
    uint32_t next;
} Node;

typedef struct {
    uint32_t count;
    uint32_t maxNodeCount;
} GeometrySBO;

typedef struct PipelineCache{
    void *GraphicsPipeline;
    void *GraphicsPipelineLayout;
    void *setting;
} PipelineCache;

typedef struct EIVertexInputBindingDescription {
    uint32_t             binding;
    uint32_t             stride;
    uint32_t             inputRate;
} EIVertexInputBindingDescription;

typedef struct EIVertexInputAttributeDescription {
    uint32_t    location;
    uint32_t    binding;
    uint32_t    format;
    uint32_t    offset;
} EIVertexInputAttributeDescription;

typedef struct {
    EIVertexInputAttributeDescription* inputDescription;
    uint32_t size;
} attrDescr;

typedef struct{
    vec3 position;
    vec3 rotation;
} SomeDirLight;

typedef struct{
    uint32_t type;
    vec3 orig;
    vec3 dir;
    float angle;
} TreePeace;

typedef struct{
    uint32_t v_index;
    uint32_t i_index;
} VertextIterator;

typedef struct{
    char *diffuse;
    char *specular;
    char *normal;
    char second[256];
    char font[256];
    char vertShader[256];
    char fragShader[256];
    char topology;
    char flags;
    void *render;
    void *geom;
} DrawParam;

typedef struct EISurfaceCapabilitiesKHR {
    uint32_t                         minImageCount;
    uint32_t                         maxImageCount;
    EIExtent2D                       currentExtent;
    EIExtent2D                       minImageExtent;
    EIExtent2D                       maxImageExtent;
    uint32_t                         maxImageArrayLayers;
    uint32_t       supportedTransforms;
    uint32_t    currentTransform;
    uint32_t         supportedCompositeAlpha;
    uint32_t                supportedUsageFlags;
} EISurfaceCapabilitiesKHR;

typedef struct{
    uint32_t FindLetter;
    uint16_t IndexLetter;
} FontIndexes;

#ifdef __cplusplus
}
#endif

#endif
