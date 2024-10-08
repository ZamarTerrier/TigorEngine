#include "TigorEngine.h"

#include <vulkan/vulkan.h>

#include "Core/engine.h"
#include "Core/e_memory.h"
#include "Core/e_buffer.h"
#include "Core/e_window.h"
#include "Core/e_device.h"

#include "Objects/render_texture.h"

#include "GUI/GUIManager.h"

#include "wManager/window_manager.h"

#include "Tools/e_math.h"

#include "Variabels/engine_includes.h"

TEngine engine;

extern bool enableValidationLayers;

void EngineCharacterCallback(wManagerWindow* window, uint32_t codepoint)
{
    EntryWidgetCharacterCallback(window, codepoint);

    for(int i=0; i < engine.func.charCallbackSize;i++)
        engine.func.charCallbacks[i](window, codepoint);
}

void EngineKeyCallback(wManagerWindow* window,  unsigned int key, unsigned int scancode, unsigned int action, unsigned int mods)
{
    EntryWidgetKeyCallback(window, key, scancode, action, mods);

    for(int i=0; i < engine.func.keyCallbackSize;i++)
        engine.func.keyCallbacks[i](window, key, scancode, action, mods);
}


void TEngineInitSystem(int width, int height, const char* name){
    memset(&engine, 0, sizeof(TEngine));

    strcpy(engine.app_name, name);

    engine.present = true;

    engine.width = width;
    engine.height = height;

    engine.diffSize = vec2_f( 1.0f, 1.0f );

    engine.viewSize.x = engine.width;
    engine.viewSize.y = engine.height;

    #ifndef NDEBUG
        enableValidationLayers = true;
    #else
        enableValidationLayers = false;
    #endif

    //rootDirPath = e_GetCurrectFilePath();

    engine.window = AllocateMemoryP(1, sizeof(TWindow), &engine);
    engine.device = AllocateMemoryP(1, sizeof(TDevice), &engine);
    engine.swapchain = AllocateMemoryP(1, sizeof(TSwapChain), &engine);

    engine.cache.alloc_buffers_memory_head = calloc(1, sizeof(ChildStack));
    engine.cache.alloc_descriptor_head = calloc(1, sizeof(ChildStack));
    engine.cache.alloc_pipeline_head = calloc(1, sizeof(ChildStack));

    engine.e_var_current_entry = NULL;

    engine.MAX_FRAMES_IN_FLIGHT = 3;

    engine.DataR.e_var_images = AllocateMemoryP(MAX_IMAGES, sizeof(engine_buffered_image), &engine);
    engine.DataR.e_var_num_images = 0;

    engine.DataR.e_var_fonts = AllocateMemoryP(MAX_FONTS, sizeof(FontCache), &engine);
    engine.DataR.e_var_num_fonts = 0;
    
    InitWindow(engine.window);
    EngineInitVulkan();
    
    TWindow *window = (TWindow *)engine.window;
    wManagerSetCharCallback(window->e_window, EngineCharacterCallback);
    wManagerSetKeyCallback(window->e_window, EngineKeyCallback);
    
    char *text = "Null texture";
    engine_buffered_image *images = engine.DataR.e_var_images;
    TextureCreateEmptyDefault(&images[engine.DataR.e_var_num_images].texture);
    TextureCreateTextureImageView(&images[engine.DataR.e_var_num_images].texture, VK_IMAGE_VIEW_TYPE_2D);
    TextureCreateSampler(&images[engine.DataR.e_var_num_images].texture.sampler, images[engine.DataR.e_var_num_images].texture.textureType,  images[engine.DataR.e_var_num_images].texture.image_data.mip_levels);

    memcpy(images[engine.DataR.e_var_num_images].path, text, strlen(text));
    engine.DataR.e_var_num_images ++;
    
    memset(&engine.renders, 0, sizeof(EngineRenderItems));
    
    engine.main_render = AllocateMemory(1, sizeof(RenderTexture));
    
    RenderTextureInit(engine.main_render, TIGOR_RENDER_TYPE_WINDOW, 0, 0, 0);

    TEngineSetRender(engine.main_render, 1);

    GUIManagerInit(true);
}

void TEngineSetRender(void *obj, uint32_t count)
{
    if(count == 0)
        return;

    RenderTexture *some_render = obj;

    for (int i=0;i < engine.renders.size;i++){
        if(engine.renders.objects[i] == &some_render[0])
            return;
    }

    for(int i=0;i < count; i++ )
    {
        engine.renders.objects[engine.renders.size] = &some_render[i];

        engine.renders.size ++;
    }
}

void TEngineRender(){

    TDevice *device = (TDevice *)engine.device;
    TSwapChain *swapchain = (TSwapChain *)engine.swapchain;

    
    for( int i=0;i < engine.gameObjects.size;i++){
        if(!(engine.gameObjects.objects[i]->flags & TIGOR_GAME_OBJECT_FLAG_INIT))
            GameObjectInit(engine.gameObjects.objects[i]);
    }

    vkWaitForFences(device->e_device, 1, &engine.Sync.inFlightFences[engine.imageIndex], VK_TRUE, UINT64_MAX);
    VkResult result = vkAcquireNextImageKHR(device->e_device, swapchain->swapChain, UINT64_MAX, engine.Sync.imageAvailableSemaphores[engine.currentFrame], VK_NULL_HANDLE, &engine.imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        
        if(engine.func.RecreateFunc != NULL)
        {
            engine.func.RecreateFunc();
        }

        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("failed to acquire swap chain image!");
        exit(1);
    }

    //vkResetCommandPool(e_device, commandPool, 0);

    if(GUIManagerIsInit())
        GUIManagerUpdate();

    VkCommandBufferBeginInfo *beginInfo = AllocateMemory(1, sizeof(VkCommandBufferBeginInfo));
    beginInfo->sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //beginInfo->flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    if(vkBeginCommandBuffer(device->commandBuffers[engine.imageIndex], beginInfo) != VK_SUCCESS){
        printf("failed begin command buffer/n");
        exit(1);
    }

    FreeMemory(beginInfo);

    for(int i=0;i < engine.renders.size;i++)
    {
        RenderTexture *render;
        engine.current_render = render = engine.renders.objects[i];

        if((render->flags & TIGOR_RENDER_FLAG_ONE_SHOT) && (render->flags & TIGOR_RENDER_FLAG_SHOOTED))
            continue;

        if(render->type & TIGOR_RENDER_TYPE_CUBEMAP)
        {
            for(int k=0;k < 6;k++)
            {
                RenderTextureSetCurrentFrame(engine.current_render, k);

                RenderTextureBeginRendering(engine.current_render, device->commandBuffers[engine.imageIndex]);
                
                for( int i=0;i < engine.gameObjects.size;i++)
                    GameObjectDraw(engine.gameObjects.objects[i]);

                RenderTextureEndRendering(engine.current_render, device->commandBuffers[engine.imageIndex]);
            }
        }else{

            RenderTextureBeginRendering(engine.current_render, device->commandBuffers[engine.imageIndex]);

            for( int i=0;i < engine.gameObjects.size;i++)
                GameObjectDraw(engine.gameObjects.objects[i]);

            if(GUIManagerIsInit())
                GUIManagerDraw();

            RenderTextureEndRendering(engine.current_render, device->commandBuffers[engine.imageIndex]);
        }
    }

    if (vkEndCommandBuffer(device->commandBuffers[engine.imageIndex]) != VK_SUCCESS) {
        printf("failed to record command buffer!");
        exit(1);
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {engine.Sync.imageAvailableSemaphores[engine.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = (const VkCommandBuffer *)&device->commandBuffers[engine.imageIndex];

    VkSemaphore signalSemaphores[] = {engine.Sync.renderFinishedSemaphores[engine.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device->e_device, 1, (const VkFence *)&engine.Sync.inFlightFences[engine.currentFrame]);

    if (vkQueueSubmit(device->graphicsQueue, 1, &submitInfo, engine.Sync.inFlightFences[engine.currentFrame]) != VK_SUCCESS) {
        printf("failed to submit draw command buffer!");
        exit(1);
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &engine.imageIndex;

    result = vkQueuePresentKHR(device->presentQueue, &presentInfo);

    vkQueueWaitIdle(device->presentQueue);

    if(engine.framebufferwasResized){

        engine.framebufferwasResized = false;
    }

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || engine.framebufferResized) {
        engine.framebufferResized = false;

        RecreateSwapChain();

        engine.framebufferwasResized = true;
        
        if(engine.func.RecreateFunc != NULL)
        {
            engine.func.RecreateFunc();
        }

    } else if (result != VK_SUCCESS) {
        printf("failed to present swap chain image!");
        exit(1);
    }

    for(int i=0; i < engine.renders.size;i++)
    {
        RenderTexture *render = engine.renders.objects[i];

        if((render->flags & TIGOR_RENDER_FLAG_ONE_SHOT) && !(render->flags & TIGOR_RENDER_FLAG_SHOOTED))
            render->flags |= TIGOR_RENDER_FLAG_SHOOTED;
    }

    engine.currentFrame = (engine.currentFrame + 1) % engine.MAX_FRAMES_IN_FLIGHT;
    
    engine.gameObjects.size = 0;

    if(GUIManagerIsInit())
        GUIManagerClear();
}

void TEngineSetDrawFunc(DrawFunc_T func){
    engine.func.DrawFunc = func;
}

void TEngineSetRecreateFunc(RecreateFunc_T func){
    engine.func.RecreateFunc = func;
}

void TEngineDraw(GameObject *go){

    for( int i=0;i < engine.gameObjects.size;i++){
        if(engine.gameObjects.objects[i] == go)
            return;
    }

    engine.gameObjects.objects[engine.gameObjects.size] = go;
    engine.gameObjects.size ++;
}

void TEnginePoolEvents(){
    wManagerPoolEvents();
}

void TEngineSetKeyCallback(void *callback){
    engine.func.keyCallbackSize ++;

    engine.func.keyCallbacks = (e_keyCallback *)realloc(engine.func.keyCallbacks, engine.func.keyCallbackSize * sizeof(e_keyCallback));
    engine.func.keyCallbacks[engine.func.keyCallbackSize - 1] = (e_keyCallback)callback;
}

void TEngineSetCharCallback(void *callback){
    engine.func.charCallbackSize ++;

    engine.func.charCallbacks = (e_charCallback *)realloc(engine.func.charCallbacks, engine.func.charCallbackSize * sizeof(e_charCallback));
    engine.func.charCallbacks[engine.func.charCallbackSize - 1] = (e_charCallback)callback;
}

void TEngineSetMouseKeyCallback(void *callback){
    TWindow *window = (TWindow *)engine.window;

    wManagerSetMouseButtonCallback(window->e_window, callback);
}

void TEngineSetCursorPoscallback(void * callback){
    TWindow *window = (TWindow *)engine.window;

    wManagerSetCursorPosCallback(window->e_window, callback);
}

void TEngineGetWindowSize(int *width, int *height){

    *width = engine.width;
    *height = engine.height;
}

void TEngineFixedCursorCenter(){
    TWindow *window = (TWindow *)engine.window;

    wManagerSetCursorPos(window->e_window, engine.width / 2, engine.height / 2);
}

void TEngineGetCursorPos(double *xpos, double *ypos){
    TWindow *window = (TWindow *)engine.window;

    wManagerGetCursorPos(window->e_window, xpos, ypos);
}

void TEngineSetCursorPos(float xpos, float ypos){
    TWindow *window = (TWindow *)engine.window;

    wManagerSetCursorPos(window->e_window, xpos, ypos);
}

void TEngineHideCursor(char state){
    TWindow *window = (TWindow *)engine.window;

    switch(state){
        case 0 :
            wManagerSetInputMode(window->e_window, TIGOR_CURSOR, TIGOR_CURSOR_DISABLED);
            break;
        case 1 :
            wManagerSetInputMode(window->e_window, TIGOR_CURSOR, TIGOR_CURSOR_HIDDEN);
            break;
        case 2 :
            wManagerSetInputMode(window->e_window, TIGOR_CURSOR, TIGOR_CURSOR_NORMAL);
            break;
    }
}

int TEngineGetMousePress(int Key){
    TWindow *window = (TWindow *)engine.window;

    int state = wManagerGetMouseButton(window->e_window, Key);

    return state;
}

int TEngineWindowIsClosed(){
    return wManagerWindowIsClosed();
}

double TEngineGetTime(){
    return wManagerGetTime();
}

int TEngineGetKeyPress(int Key){
    TWindow *window = (TWindow *)engine.window;

    int res = wManagerGetKey(window->e_window, Key);

    return res;
}

const char *TEngineGetClipBoardString(){
    TWindow *window = (TWindow *)engine.window;

    return wManagerGetClipboardString(window->e_window);
}

void TEngineSetClipBoardString(const char *string){
    TWindow *window = (TWindow *)engine.window;

    wManagerSetClipboardString(window->e_window, string);
}

void TEngineSetFont(char *font_path){
    
    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';
    
    char *full_path = ToolsMakeString(currPath, font_path);

    if(!DirectIsFileExist(full_path)){
        FreeMemory(full_path);            
        FreeMemory(currPath);
        return;
    }

    engine.DataR.font_path = full_path;

    FreeMemory(currPath);
        
    if(!GUIManagerIsInit())
        GUIManagerInit(false);
    else{
        GUIManagerDestroy();
        GUIManagerInit(false);
    }
}

void TEngineCleanUp(){

    TDevice *device = (TDevice *)engine.device;
    TWindow *window = (TWindow *)engine.window;
    TSwapChain *swapchain = (TSwapChain *)engine.swapchain;

    vkDeviceWaitIdle(device->e_device);

    if(engine.present){
        CleanupSwapChain();

        for (size_t i = 0; i < engine.MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device->e_device, engine.Sync.renderFinishedSemaphores[i], NULL);
            vkDestroySemaphore(device->e_device, engine.Sync.imageAvailableSemaphores[i], NULL);
            vkDestroyFence(device->e_device, engine.Sync.inFlightFences[i], NULL);
        }

        FreeMemory(engine.Sync.renderFinishedSemaphores);
        FreeMemory(engine.Sync.imageAvailableSemaphores);
        FreeMemory(engine.Sync.inFlightFences);
    }

    vkDestroyCommandPool(device->e_device, device->commandPool, NULL);


   if(engine.DataR.e_var_num_fonts > 0)
    {
        FontCache *fonts = engine.DataR.e_var_fonts;    
        for(int i=0; i < engine.DataR.e_var_num_fonts;i++)
        {
            FontCache *font = &fonts[i];
            
            FreeMemory(fonts[i].cdata);
            FreeMemory(fonts[i].info);
            ImageDestroyTexture(fonts[i].texture);
            FreeMemory(fonts[i].texture);
        }
    }   
    FreeMemory(engine.DataR.e_var_fonts);
    engine.DataR.e_var_fonts = NULL;    
    
    if(engine.DataR.e_var_num_images > 0)
    {
        engine_buffered_image *images = engine.DataR.e_var_images;  
        for(int i=0; i < engine.DataR.e_var_num_images;i++)
        {
            ImageDestroyTexture(&images[i].texture);
        }
    }   
    FreeMemory(engine.DataR.e_var_images);
    engine.DataR.e_var_images = NULL;   

    RenderTextureDestroy(engine.main_render);
    FreeMemory(engine.main_render);

    if(engine.DataR.font_path != NULL)
        FreeMemory(engine.DataR.font_path);

    if(GUIManagerIsInit())
        GUIManagerDestroy();

    BuffersClearAll();
    DescriptorClearAll();
    PipelineClearAll();

    vkDestroyDevice(device->e_device, NULL);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(window->instance, engine.debugMessenger, NULL);
    }

    if(engine.present)
        vkDestroySurfaceKHR(window->instance, window->surface, NULL);

    vkDestroyInstance(window->instance, NULL);

    wManagerDestroyWindow(window->e_window);

    wManagerTerminate();

    FreeMemory(engine.window);
    FreeMemory(engine.device);
    FreeMemory(engine.swapchain);
        
    ClearAllAllocatedMemory();
}