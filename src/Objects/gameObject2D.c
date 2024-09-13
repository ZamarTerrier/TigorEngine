#include "Objects/gameObject2D.h"

#include <vulkan/vulkan.h>
#include <math.h>

#include "Core/e_device.h"
#include "Core/e_buffer.h"
#include "Core/pipeline.h"
#include "Core/e_camera.h"

#include "Tools/e_math.h"
#include "Tools/e_shaders.h"

#include "Variabels/e_pipeline_variables.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern ZEngine engine;

void GameObject2DTransformBufferUpdate(GameObject2D *go, void *data)
{
    TransformBuffer2D tbo;

    tbo.position = v2_subs(v2_add(go->transform.position, go->transform.scale), 1.0f);
    tbo.rotation = go->transform.rotation;
    tbo.scale = go->transform.scale;

    
    memcpy(data, &tbo, sizeof(tbo));
}

void GameObject2DImageBuffer(GameObject2D *go, void *data)
{
    ImageBufferObjects ibo;
    ibo.offset = go->transform.img.offset;
    ibo.rotation = go->transform.img.rotation;
    ibo.scale = go->transform.img.scale;

    ibo.rotation.x = 0;
    ibo.rotation.y = 0;

    memcpy(data, &ibo, sizeof(ibo));
}

void GameObject2DDefaultUpdate(GameObject2D* go) {

    ZDevice *device = (ZDevice *)engine.device;
    
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

void GameObject2DDefaultDraw(GameObject2D* go){
    
    ZDevice *device = (ZDevice *)engine.device;
    
    VkCommandBuffer command = device->commandBuffers[engine.imageIndex];

    for(int i=0; i < go->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *pack = &go->graphObj.blueprints.blue_print_packs[i];

        if(pack->render_point == engine.current_render)
        {
            ShaderPack *pack = &go->graphObj.gItems.shader_packs[i];

            PipelineSetting *settings = &go->graphObj.blueprints.blue_print_packs[i].setting;
            
            vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipeline.pipeline);

            vertexParam *vParam = &go->graphObj.shapes[settings->vert_indx].vParam;
            indexParam *iParam = &go->graphObj.shapes[settings->vert_indx].iParam;

            if(vParam->verticesSize == 0)
                continue;

            if(settings->flags & ENGINE_PIPELINE_FLAG_DYNAMIC_VIEW){
                vkCmdSetViewport(command, 0, 1, (const VkViewport *)&settings->viewport);
                vkCmdSetScissor(command, 0, 1, (const VkRect2D *)&settings->scissor);
            }

            VkBuffer vertexBuffers[] = {vParam->buffer.buffer};
            VkDeviceSize offsets[] = {0};

            vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);
            vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipeline.layout, 0, 1, &pack->descriptor.descr_sets[engine.imageIndex], 0, NULL);

            if(settings->flags & ENGINE_PIPELINE_FLAG_DRAW_INDEXED){

                vkCmdBindIndexBuffer(command, iParam->buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(command, iParam->indexesSize, 1, 0, 0, 0);
            }else
                vkCmdDraw(command, vParam->verticesSize, 1, 0, 0);
        }
    }
}

void GameObject2DAddDescriptor(GameObject2D* go, uint32_t shader_indx, uint32_t size, uint32_t stage_bit, UpdateDescriptor Updater, uint32_t layer_indx){    
    BluePrintAddUniformObject(&go->graphObj.blueprints, shader_indx, size, stage_bit, Updater, layer_indx);
}

void GameObject2DAddDescriptorTexture(GameObject2D* go, uint32_t shader_indx, uint32_t stage_bit, GameObjectImage *image){
    BluePrintAddTextureImage(&go->graphObj.blueprints, shader_indx, image, stage_bit);
}

void GameObject2DSetDescriptorUpdate(GameObject2D* go, uint32_t shader_indx, uint32_t bind_index, UpdateDescriptor Updater){    
    BluePrintAddSomeUpdater(&go->graphObj.blueprints, shader_indx, bind_index, Updater);
}

void GameObject2DSetDescriptorTexture(GameObject2D* go, uint32_t shader_indx, uint32_t bind_index, Texture2D *texture){
    BluePrintSetTextureImage(&go->graphObj.blueprints, shader_indx, texture, bind_index);
}

void GameObject2DSetDescriptorTextureCreate(GameObject2D* go, uint32_t shader_indx, uint32_t bind_index, GameObjectImage *image){
    BluePrintSetTextureImageCreate(&go->graphObj.blueprints, shader_indx, image, bind_index);
}


void GameObject2DSetShader(GameObject2D *go, char *vert_path, char *frag_path){
    
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
    vert_code.flags |= ENGINE_SHADER_OBJECT_READED;
    ShaderObject frag_code = readFile(full_path_frag);
    frag_code.flags |= ENGINE_SHADER_OBJECT_READED;
    
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_code, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_code, num_pack);

    FreeMemory(currPath);
    FreeMemory(full_path_vert);
    FreeMemory(full_path_frag);    

    go->self.flags |= ENGINE_GAME_OBJECT_FLAG_SHADED;
}

void GameObject2DSetShaderSimple(GameObject2D *go, char *vert_path, char *frag_path){
    
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
    vert_code.flags |= ENGINE_SHADER_OBJECT_READED;
    ShaderObject frag_code = readFile(full_path_frag);
    frag_code.flags |= ENGINE_SHADER_OBJECT_READED;
    
    GraphicsObjectSetShader(&go->graphObj, &vert_code, num_pack, VK_SHADER_STAGE_VERTEX_BIT);
    GraphicsObjectSetShader(&go->graphObj, &frag_code, num_pack, VK_SHADER_STAGE_FRAGMENT_BIT);

    FreeMemory(currPath);
    FreeMemory(full_path_vert);
    FreeMemory(full_path_frag);    

    go->self.flags |= ENGINE_GAME_OBJECT_FLAG_SHADED;
}


void GameObject2DInitDefaultShader(GameObject2D *go){    
    
    if(go->self.flags & ENGINE_GAME_OBJECT_FLAG_SHADED)
        return;
    
    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);
    
    ShaderBuilder *vert = go->self.vert;
    ShaderBuilder *frag = go->self.frag;

    ShadersMakeDefault2DShader(vert, frag, go->num_images > 0);
    
    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);

    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_shader, num_pack);

    GameObject2DSetDescriptorUpdate((GameObject2D *)go, num_pack, 0, (UpdateDescriptor)GameObject2DTransformBufferUpdate);
    GameObject2DSetDescriptorUpdate((GameObject2D *)go, num_pack, 1, (UpdateDescriptor)GameObject2DImageBuffer);
    GameObject2DSetDescriptorTextureCreate(go, num_pack, 2, go->image);

    uint32_t flags = BluePrintGetSettingsValue(&go->graphObj.blueprints, num_pack, 3);
    BluePrintSetSettingsValue(&go->graphObj.blueprints, num_pack, 3, flags | ENGINE_PIPELINE_FLAG_FACE_CLOCKWISE);

    /*if(so->type == ENGINE_SHAPE_OBJECT_LINE)
    {
        flags = BluePrintGetSettingsValue(&so->go.graphObj.blueprints, num_pack, 3);

        BluePrintSetSettingsValue(&so->go.graphObj.blueprints, num_pack, 1, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
        BluePrintSetSettingsValue(&so->go.graphObj.blueprints, num_pack, 3, flags & ~(ENGINE_PIPELINE_FLAG_DRAW_INDEXED));
    }*/
    
    go->self.flags |= ENGINE_GAME_OBJECT_FLAG_SHADED;
}

void GameObject2DInitDraw(GameObject2D *go)
{
    if(!(go->self.flags & ENGINE_GAME_OBJECT_FLAG_SHADED))
        return;

    GraphicsObjectCreateDrawItems(&go->graphObj);

    PipelineCreateGraphics(&go->graphObj);

    go->self.flags |= ENGINE_GAME_OBJECT_FLAG_INIT;
}

void GameObject2DInitDefault(GameObject2D *go){
    GameObject2DInitDefaultShader(go);
    GameObject2DInitDraw(go);
}

void GameObject2DClean(GameObject2D* go){
    GraphicsObjectClean(&go->graphObj);
}

void GameObject2DRecreate(GameObject2D* go){

    GameObject2DClean(go);

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

    Transform2DReposition((struct GameObject2D_T *)go);
    Transform2DRescale((struct GameObject2D_T *)go);

}

void GameObject2DDestroy(GameObject2D* go){

    GraphicsObjectDestroy(&go->graphObj);

    if(go->image != NULL)
    {
        FreeMemory(go->image->path);

        if(go->image->size > 0)
            FreeMemory(go->image->buffer);

        FreeMemory(go->image);
    }

    for(int i=0; i < go->graphObj.num_shapes; i++)
    {
        if(go->graphObj.shapes[i].vParam.verticesSize)
            FreeMemory(go->graphObj.shapes[i].vParam.vertices);

        if(go->graphObj.shapes[i].iParam.indexesSize)
            FreeMemory(go->graphObj.shapes[i].iParam.indices);
    }

    FreeMemory(go->self.vert);
    FreeMemory(go->self.frag);
    
    go->self.flags &= ~(ENGINE_GAME_OBJECT_FLAG_INIT);
}

void GameObject2DInit(GameObject2D* go)
{
    GameObjectSetInitFunc((GameObject *)go, (void *)GameObject2DInitDefault);
    GameObjectSetUpdateFunc((GameObject *)go, (void *)GameObject2DDefaultUpdate);
    GameObjectSetDrawFunc((GameObject *)go, (void *)GameObject2DDefaultDraw);
    GameObjectSetCleanFunc((GameObject *)go, (void *)GameObject2DClean);
    GameObjectSetRecreateFunc((GameObject *)go, (void *)GameObject2DRecreate);
    GameObjectSetDestroyFunc((GameObject *)go, (void *)GameObject2DDestroy);

    go->self.obj_type = ENGINE_GAME_OBJECT_TYPE_2D;

    Transform2DInit(&go->transform);
    GraphicsObjectInit(&go->graphObj, ENGINE_VERTEX_TYPE_2D_OBJECT);

    go->self.vert = AllocateMemory(1, sizeof(ShaderBuilder));
    go->self.frag = AllocateMemory(1, sizeof(ShaderBuilder));
}

vec2 GameObject2DGetSize(GameObject2D* go)
{
    /*vec2 size;
    size.x = fabs((go->graphObj.shape.vParam.vertices[1].position.x - go->graphObj.shape.vParam.vertices[3].position.x) * (viewSize.x * diffSize.x)) * go->transform.scale.x;
    size.y = fabs((go->graphObj.shape.vParam.vertices[1].position.y - go->graphObj.shape.vParam.vertices[3].position.y) * (viewSize.y * diffSize.y)) * go->transform.scale.y;
    return size;*/
}
