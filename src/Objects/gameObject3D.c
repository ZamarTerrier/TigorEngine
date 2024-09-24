#include "Objects/gameObject3D.h"

#include <vulkan/vulkan.h>

#include <direct.h>

#include "wManager/window_manager.h"

#include "Core/e_memory.h"
#include "Core/e_device.h"
#include "Core/e_camera.h"
#include "Core/pipeline.h"
#include "Core/e_buffer.h"
#include "Core/e_texture.h"

#include "Objects/render_texture.h"

#include "Tools/e_math.h"
#include "Tools/e_tools.h"
#include "Tools/e_shaders.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

#define VERTEX_BUFFER_BIND_ID 0
#define INSTANCE_BUFFER_BIND_ID 1

extern TEngine engine;

void GameObject3DDescriptorModelUpdate(GameObject3D* go, void *data)
{
    Camera3D* cam = (Camera3D*) engine.cam3D;

    RenderTexture *render = engine.current_render;

    ModelBuffer3D mbo = {};
    vec3 cameraUp = {0.0f,1.0f, 0.0f};

    go->transform.model = m4_transform(go->transform.position, go->transform.scale, go->transform.rotation);

    mbo.model = go->transform.model;
    mbo.view = m4_look_at(cam->position, v3_add(cam->position, cam->rotation), cameraUp);
    mbo.proj = m4_perspective(render->width, render->height, render->persp_view_angle, render->persp_view_near, render->persp_view_distance);
    mbo.proj.m[1][1] *= -1;

    memcpy(data, (char *)&mbo, sizeof(mbo));
}

void GameObject3DDefaultUpdate(GameObject3D* go) {

    TDevice *device = (TDevice *)engine.device;

    for(int i=0; i < go->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *pack = &go->graphObj.blueprints.blue_print_packs[i];

        if(pack->render_point == engine.current_render)
        {
            for(int j=0;j < pack->num_descriptors;j++)
            {
                BluePrintDescriptor *descriptor = &pack->descriptors[j];

                if(descriptor->descrType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                {
                    if(descriptor->uniform.count == 0 || descriptor->update == NULL)
                        continue;

                    void *point;
                    vkMapMemory(device->e_device, descriptor->uniform.buffers[engine.imageIndex].memory, 0, descriptor->buffsize, 0, &point);
                    UpdateDescriptor update = descriptor->update;
                    update(go, point);
                    vkUnmapMemory(device->e_device, descriptor->uniform.buffers[engine.imageIndex].memory);
                }
            }
        }
    }
}

void GameObject3DDefaultDraw(GameObject3D* go){
    
    TDevice *device = (TDevice *)engine.device;

    VkCommandBuffer command = device->commandBuffers[engine.imageIndex];

    for(int i=0; i < go->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *blue_pack = &go->graphObj.blueprints.blue_print_packs[i];

        if(blue_pack->render_point == engine.current_render)
        {
            RenderTexture *render = engine.current_render;

            ShaderPack *pack = &go->graphObj.gItems.shader_packs[i];

            /*if(render->type == TIGOR_RENDER_TYPE_CUBEMAP){

                mat4 res = MakeLookRender(render->currFrame, blue_pack->descriptors[j].indx_layer);

                vkCmdPushConstants( command, pack->pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &res);
            }*/

            PipelineSetting *settings = &blue_pack->setting;

            uint32_t num_instances = 1;
            uint32_t num_verts = go->graphObj.shapes[settings->vert_indx].vParam.num_verts;

            if(num_verts == 0)
                continue;

            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipeline.pipeline);

            if(settings->flags & TIGOR_PIPELINE_FLAG_DYNAMIC_VIEW){
                vkCmdSetViewport(command, 0, 1, (const VkViewport *)&settings->viewport);
                vkCmdSetScissor(command, 0, 1, (const VkRect2D *)&settings->scissor);
            }

            vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipeline.layout, 0, 1, (const VkDescriptorSet *)&pack->descriptor.descr_sets[engine.imageIndex], 0, NULL);

            if(num_verts > 0)
            {
                VkBuffer vertexBuffers[] = {go->graphObj.shapes[settings->vert_indx].vParam.buffer.buffer};
                VkDeviceSize offsets[] = {0};

                // Binding point 0 : Mesh vertex buffer
                vkCmdBindVertexBuffers(command, VERTEX_BUFFER_BIND_ID, 1, vertexBuffers, offsets);

                if(go->num_instances > 0){
                    // Binding point 1 : Instance data buffer
                    vkCmdBindVertexBuffers(command, INSTANCE_BUFFER_BIND_ID, 1, (const VkBuffer *)&go->buffer.buffer, offsets);
                    num_instances = go->num_instances;
                }
            }

            if(num_instances == 0)
                continue;

            if(settings->flags & TIGOR_PIPELINE_FLAG_DRAW_INDEXED && go->graphObj.shapes[settings->vert_indx].iParam.indexesSize > 0){
                vkCmdBindIndexBuffer(command, go->graphObj.shapes[settings->vert_indx].iParam.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(command, go->graphObj.shapes[settings->vert_indx].iParam.indexesSize, num_instances, 0, 0, 0);
            }else
                vkCmdDraw(command, go->graphObj.shapes[settings->vert_indx].vParam.num_verts, num_instances, 0, 0);
        }
    }
}

void GameObject3DAddDescriptor(GameObject3D* go, uint32_t shader_indx, uint32_t size, uint32_t stage_bit, UpdateDescriptor Updater, uint32_t layer_indx){    
    BluePrintAddUniformObject(&go->graphObj.blueprints, shader_indx, size, stage_bit, Updater, layer_indx);
}

void GameObject3DAddDescriptorTexture(GameObject3D* go, uint32_t shader_indx, uint32_t stage_bit, GameObjectImage *image){
    BluePrintAddTextureImage(&go->graphObj.blueprints, shader_indx, image, stage_bit);
}

void GameObject3DSetDescriptorUpdate(GameObject3D* go, uint32_t shader_indx, uint32_t bind_index, UpdateDescriptor Updater){    
    BluePrintAddSomeUpdater(&go->graphObj.blueprints, shader_indx, bind_index, Updater);
}

void GameObject3DSetDescriptorTexture(GameObject3D* go, uint32_t shader_indx, uint32_t bind_index, Texture2D *texture){
    BluePrintSetTextureImage(&go->graphObj.blueprints, shader_indx, texture, bind_index);
}

void GameObject3DSetDescriptorTextureCreate(GameObject3D* go, uint32_t shader_indx, uint32_t bind_index, GameObjectImage *image){
    BluePrintSetTextureImageCreate(&go->graphObj.blueprints, shader_indx, image, bind_index);
}

void GameObject3DSetShader(GameObject3D *go, char *vert_path, char *frag_path){
    
    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';
    
    char *full_path_vert = ToolsMakeString(currPath, vert_path);

    if(!DirectIsFileExist(full_path_vert)){
        FreeMemory(full_path_vert);            
        FreeMemory(currPath);
        return;
    }

    char *full_path_frag = ToolsMakeString(currPath, frag_path);

    if(!DirectIsFileExist(full_path_vert)){
        FreeMemory(full_path_vert);  
        FreeMemory(full_path_frag);            
        FreeMemory(currPath);
        return;
    }

    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);

    ShaderObject vert_code = readFile(full_path_vert);
    vert_code.flags |= TIGOR_SHADER_OBJECT_READED;
    ShaderObject frag_code = readFile(full_path_frag);
    frag_code.flags |= TIGOR_SHADER_OBJECT_READED;
    
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_code, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_code, num_pack);

    FreeMemory(currPath);
    FreeMemory(full_path_vert);
    FreeMemory(full_path_frag);    

    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void GameObject3DSetShaderSimple(GameObject3D *go, char *vert_path, char *frag_path){
    
    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';
    
    char *full_path_vert = ToolsMakeString(currPath, vert_path);

    if(!DirectIsFileExist(full_path_vert)){
        FreeMemory(full_path_vert);            
        FreeMemory(currPath);
        return;
    }

    char *full_path_frag = ToolsMakeString(currPath, frag_path);

    if(!DirectIsFileExist(full_path_vert)){
        FreeMemory(full_path_vert);  
        FreeMemory(full_path_frag);            
        FreeMemory(currPath);
        return;
    }

    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);

    ShaderObject vert_code = readFile(full_path_vert);
    vert_code.flags |= TIGOR_SHADER_OBJECT_READED;
    ShaderObject frag_code = readFile(full_path_frag);
    frag_code.flags |= TIGOR_SHADER_OBJECT_READED;
    
    GraphicsObjectSetShader(&go->graphObj, &vert_code, num_pack, VK_SHADER_STAGE_VERTEX_BIT);
    GraphicsObjectSetShader(&go->graphObj, &frag_code, num_pack, VK_SHADER_STAGE_FRAGMENT_BIT);

    FreeMemory(currPath);
    FreeMemory(full_path_vert);
    FreeMemory(full_path_frag);    

    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}


void GameObject3DInitDefaultShader(GameObject3D *go){    
    
    if(go->self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED)
        return;

    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);
    
    ShaderBuilder *vert = go->self.vert;
    ShaderBuilder *frag = go->self.frag;

    ShadersMakeDefault3DShader(vert, frag, go->num_images > 0);

    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);

    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_shader, num_pack);
    
    GameObject3DSetDescriptorUpdate(go, num_pack, 0, (UpdateDescriptor)GameObject3DDescriptorModelUpdate);
    GameObject3DSetDescriptorTextureCreate(go, num_pack, 1, go->num_images > 0 ? &go->images[0] : NULL);
    
    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void GameObject3DInitDraw(GameObject3D *go){

    if(!(go->self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED))
        return;

    GraphicsObjectCreateDrawItems(&go->graphObj);

    PipelineCreateGraphics(&go->graphObj);
    
    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_INIT;
}

void GameObject3DInitDefault(GameObject3D *go){

    GameObjectShaderInit(go);

    GameObject3DInitDraw(go);
}

void GameObject3DClean(GameObject3D* go){
    GraphicsObjectClean(&go->graphObj);
}

void GameObject3DRecreate(GameObject3D* go){

    for(int i=0; i < go->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *pack = &go->graphObj.blueprints.blue_print_packs[i];

        PipelineSetting *settings = &pack->setting;

        settings->scissor.offset.x = 0;
        settings->scissor.offset.y = 0;
        settings->scissor.extent.height = engine.height;
        settings->scissor.extent.width = engine.width;
        settings->viewport.x = 0;
        settings->viewport.y = 0;
        settings->viewport.height = engine.height;
        settings->viewport.width = engine.width;
    }

    BuffersRecreateUniform((struct BluePrints_T *)&go->graphObj.blueprints);

    GraphicsObjectCreateDrawItems(&go->graphObj);
    PipelineCreateGraphics(&go->graphObj);
}

void GameObject3DDestroy(GameObject3D* go){
    
    GraphicsObjectDestroy(&go->graphObj);

    for(int i=0;i < go->num_images;i++)
    {
        FreeMemory(go->images[i].path);

        if(go->images[i].size > 0)
            FreeMemory(go->images[i].buffer);
    }

    FreeMemory(go->images);

    for(int i=0; i < go->graphObj.num_shapes; i++)
    {
        if(go->graphObj.shapes[i].vParam.num_verts)
            FreeMemory(go->graphObj.shapes[i].vParam.vertices);

        if(go->graphObj.shapes[i].iParam.indexesSize)
            FreeMemory(go->graphObj.shapes[i].iParam.indices);
    }

    if(go->num_instances > 0)
        BuffersDestroyBuffer(&go->buffer);

    FreeMemory(go->self.vert);
    FreeMemory(go->self.frag);
    
    go->self.flags &= ~(TIGOR_GAME_OBJECT_FLAG_INIT);
}

int GameObject3DInitTextures(GameObject3D *go, DrawParam *dParam)
{
    go->images = AllocateMemoryP(3, sizeof(GameObjectImage), go);

    if(dParam == NULL)
        return 0;

    int iter = 0;
    
    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';

    if(dParam->diffuse != NULL){
        if(strlen(dParam->diffuse) != 0)
        {
            char *full_path = ToolsMakeString(currPath, dParam->diffuse);

            if(!DirectIsFileExist(full_path)){
                FreeMemory(full_path);            
                FreeMemory(currPath);
                return 0;
            }

            len = strlen(full_path);
            go->images[iter].path = AllocateMemoryP(len + 1, sizeof(char), go);
            memcpy(go->images[iter].path, full_path, len);
            go->images[iter].path[len] = '\0';
            //go->image->buffer = ToolsLoadImageFromFile(&go->image->size, dParam.filePath);
            go->num_images ++;
            iter++;

            FreeMemory(full_path);
        }
    }
    
    if(dParam->normal != NULL){
       if(strlen(dParam->normal) != 0)
        {
            char *full_path = ToolsMakeString(currPath, dParam->normal);

            if(!DirectIsFileExist(full_path)){
                FreeMemory(full_path);            
                FreeMemory(currPath);
                return 0;
            }

            len = strlen(full_path);
            go->images[iter].path = AllocateMemoryP(len + 1, sizeof(char), go);
            memcpy(go->images[iter].path, full_path, len);
            go->images[iter].path[len] = '\0';
            //go->image->buffer = ToolsLoadImageFromFile(&go->image->size, dParam.filePath);
            go->num_images ++;
            iter++;
            
            FreeMemory(full_path);
        }
    }
    

    if(dParam->specular != NULL){
        if(strlen(dParam->specular) != 0)
        {
            char *full_path = ToolsMakeString(currPath, dParam->normal);
            
            if(!DirectIsFileExist(full_path)){
                FreeMemory(full_path);            
                FreeMemory(currPath);
                return 0;
            }

            len = strlen(full_path);
            go->images[iter].path = AllocateMemoryP(len + 1, sizeof(char), go);
            memcpy(go->images[iter].path, full_path, len);
            go->images[iter].path[len] = '\0';
            //go->image->buffer = ToolsLoadImageFromFile(&go->image->size, dParam.filePath);
            go->num_images ++;
            iter++;
            
            FreeMemory(full_path);
        }
    }

    FreeMemory(currPath);

    return 1;
}

void GameObject3DInit(GameObject3D *go, GameObjectType type){

    GameObjectSetInitFunc((GameObject *)go, (void *)GameObject3DInitDefault);
    GameObjectSetUpdateFunc((GameObject *)go, (void *)GameObject3DDefaultUpdate);
    GameObjectSetDrawFunc((GameObject *)go, (void *)GameObject3DDefaultDraw);
    GameObjectSetCleanFunc((GameObject *)go, (void *)GameObject3DClean);
    GameObjectSetRecreateFunc((GameObject *)go, (void *)GameObject3DRecreate);
    GameObjectSetDestroyFunc((GameObject *)go, (void *)GameObject3DDestroy);
    
    GameObjectSetShaderInitFunc((GameObject *)go, (void *)GameObject3DInitDefaultShader);


    go->self.obj_type = type;

    Transform3DInit(&go->transform);
    go->num_instances = 0;

    switch(type){
        case TIGOR_GAME_OBJECT_TYPE_3D:
            GraphicsObjectInit(&go->graphObj, TIGOR_VERTEX_TYPE_3D_OBJECT);
            break;
        case TIGOR_GAME_OBJECT_TYPE_PARTICLE_3D:
            GraphicsObjectInit(&go->graphObj, TIGOR_VERTEX_TYPE_3D_PARTICLE);
            break;
        case TIGOR_GAME_OBJECT_TYPE_MODEL:
            GraphicsObjectInit(&go->graphObj, TIGOR_VERTEX_TYPE_MODEL_OBJECT);
            break;
    }

    go->graphObj.gItems.perspective = true;

    go->self.flags = 0;
    
    go->self.vert = AllocateMemory(1, sizeof(ShaderBuilder));
    go->self.frag = AllocateMemory(1, sizeof(ShaderBuilder));
}

void GameObject3DAddInstance(GameObject3D *go, VertexInstance3D vertex){

    if(go->num_instances + 1 >= MAX_INSTANCES){
        printf("Слишком много инстансов!\n");
        return;
    }

    go->instances[go->num_instances] = vertex;

    go->num_instances ++;
}

void GameObject3DSetInstance(GameObject3D *go, uint32_t indx, VertexInstance3D vertex){
    go->instances[indx] = vertex;
}

void GameObject3DRemoveInstance(GameObject3D *go, uint32_t indx){

    if(go->num_instances <= indx)
        return;

    VertexInstance3D instances[MAX_INSTANCES];
    memcpy(instances, go->instances, sizeof(VertexInstance3D) * MAX_INSTANCES);

    memset(go->instances, 0, sizeof(VertexInstance3D) * MAX_INSTANCES);

    go->num_instances --;

    uint32_t iter = 0;
    for(int i=0;i < go->num_instances;i++)
    {
        if(i != indx){
            go->instances[iter] = instances[i];
            iter++;
        }
    }
}

void GameObject3DInitInstances(GameObject3D *go){

    VkDeviceSize bufferSize;

    uint16_t num_verts = go->graphObj.shapes[0].vParam.num_verts;
    GraphicsObjectInit(&go->graphObj, TIGOR_VERTEX_TYPE_3D_INSTANCE);

    num_verts = go->graphObj.shapes[0].vParam.num_verts;
    memset(go->instances, 0, sizeof(VertexInstance3D) * MAX_INSTANCES);

    bufferSize = sizeof(VertexInstance3D) * MAX_INSTANCES;

    BuffersCreate(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &go->buffer, TIGOR_BUFFER_ALLOCATE_VERTEX);

}

void GameObject3DUpdateInstances(GameObject3D *go){
    TDevice *device = (TDevice *)engine.device;

    BufferObject stagingBuffer;
    VkDeviceSize bufferSize;

    bufferSize = sizeof(VertexInstance3D) * MAX_INSTANCES;

    BuffersCreate(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, TIGOR_BUFFER_ALLOCATE_STAGING);

    //Изменение памяти
    void* data;
    vkMapMemory(device->e_device, stagingBuffer.memory, 0, bufferSize, 0, &data);
    memset(data, 0, bufferSize);
    memcpy(data, go->instances, (size_t) go->num_instances * sizeof(VertexInstance3D));
    vkUnmapMemory(device->e_device, stagingBuffer.memory);

    //-------------

    BuffersCopy(&stagingBuffer, &go->buffer, bufferSize);

    BuffersDestroyBuffer(&stagingBuffer);

}

void GameObject3DEnableLight(GameObject3D *go, bool enable)
{
    if(enable)
        go->self.flags |= TIGOR_GAME_OBJECT_FLAG_LIGHT;
    else
        go->self.flags &= ~(TIGOR_GAME_OBJECT_FLAG_LIGHT);
}
