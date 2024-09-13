#include "Objects/models.h"
#include "Objects/gameObject.h"
#include "Objects/render_texture.h"

#include <vulkan/vulkan.h>

#include "Core/e_memory.h"
#include "Core/e_camera.h"
#include "Core/e_buffer.h"
#include "Core/pipeline.h"
#include "Core/e_texture.h"

#include "math.h"

#include "Tools/e_math.h"
#include "Tools/glTFLoader.h"
#include "Tools/objLoader.h"
#include "Tools/fbxLoader.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_descriptors.h"

typedef void (*Update_Func)(ModelObject3D* mo, ModelNode *node, BluePrintDescriptor *descriptor);

extern ZEngine engine;

//Описание вертекса
EIVertexInputBindingDescription ModelObject3DGetBindingDescription() {
    EIVertexInputBindingDescription bindingDescription = {};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ModelVertex3D);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

void ModelDefaultDraw(ModelObject3D* mo, void *command){

    for(int i=0; i < mo->num_draw_nodes;i++){
        for(int j=0; j < mo->nodes[i].num_models; j++){

            ModelStruct *model = &mo->nodes[i].models[j];

            for(int l=0; l < model->graphObj.gItems.num_shader_packs;l++)
            {
                BluePrintPack *blue_pack = &model->graphObj.blueprints.blue_print_packs[l];

                if(blue_pack->render_point == engine.current_render)
                {
                    RenderTexture *render = engine.current_render;

                    ShaderPack *pack = &model->graphObj.gItems.shader_packs[l];

                    /*if(render->type == ENGINE_RENDER_TYPE_CUBEMAP){

                        mat4 res = MakeLookRender(render->currFrame, blue_pack->descriptors[k].indx_layer);

                        vkCmdPushConstants( command, pack->pipelines[k].layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &res);
                    }*/

                    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipeline.pipeline);

                    PipelineSetting *settings = &blue_pack->setting;

                    vertexParam *vParam = &model->graphObj.shapes[settings->vert_indx].vParam;
                    indexParam *iParam = &model->graphObj.shapes[settings->vert_indx].iParam;

                    if(settings->flags & ENGINE_PIPELINE_FLAG_DYNAMIC_VIEW){
                        vkCmdSetViewport(command, 0, 1, (const VkViewport *)&settings->viewport);
                        vkCmdSetScissor(command, 0, 1, (const VkRect2D *)&settings->scissor);
                    }

                    VkDeviceSize offsets[] = {0};
                    VkBuffer vertexBuffers[] = {vParam->buffer.buffer};

                    vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);

                    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pack->pipeline.layout, 0, 1, (const VkDescriptorSet *)&pack->descriptor.descr_sets[engine.imageIndex], 0, NULL);

                    if(settings->flags & ENGINE_PIPELINE_FLAG_DRAW_INDEXED){
                        vkCmdBindIndexBuffer(command, iParam->buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                        vkCmdDrawIndexed(command, iParam->indexesSize, 1, 0, 0, 0);
                    }else
                        vkCmdDraw(command, vParam->verticesSize, 1, 0, 0);
                }
            }
        }
    }
}

void ModelModelBufferUpdate(ModelObject3D* mo, uint32_t indx_node, BluePrintDescriptor *descriptor)
{
    Camera3D* cam = (Camera3D*) engine.cam3D;

    glTFStruct *gltf = mo->obj;

    RenderTexture *render = engine.current_render;

    ModelBuffer3D mbo = {};
    vec3 cameraUp = {0.0f,1.0f, 0.0f};

    engine_gltf_node *node = &gltf->nodes[mo->nodes[indx_node].id_node];

    mo->nodes[indx_node].model = mat4_mult_transform(node->global_matrix, m4_transform(mo->transform.position, mo->transform.scale, mo->transform.rotation));

    mbo.model = mo->nodes[indx_node].model;
    mbo.view = m4_look_at(cam->position, v3_add(cam->position, cam->rotation), cameraUp);
    mbo.proj = m4_perspective(render->width, render->height, cam->view_angle, cam->view_near, cam->view_distance);
    mbo.proj.m[1][1] *= -1;

    DescriptorUpdate(descriptor, (char *)&mbo, sizeof(mbo));
}

void ModelDirLightModelUpdate(ModelObject3D* mo, uint32_t indx_node, BluePrintDescriptor *descriptor)
{
    glTFStruct *gltf = mo->obj;

    ModelBuffer3D mbo = {};
    vec3 cameraUp = {0.0f,1.0f, 0.0f};

    RenderTexture *render = engine.current_render;

    DirLightBuffer dlb = {};
    memset(&dlb, 0, sizeof(DirLightBuffer));

    LightObjectFillDirLights(&dlb);

    //uint32_t layer_indx = descriptor->indx_layer;

    engine_gltf_node *node = &gltf->nodes[mo->nodes[indx_node].id_node];

    mo->nodes[indx_node].model = mat4_mult_transform(node->global_matrix, m4_transform(mo->transform.position, mo->transform.scale, mo->transform.rotation));

    mbo.model = mo->nodes[indx_node].model;
    mbo.view = m4_look_at(dlb.dir[0].position, v3_add(dlb.dir[0].position, dlb.dir[0].direction), cameraUp);

    if(render->flags & ENGINE_RENDER_FLAG_PERSPECTIVE){
        mbo.proj = m4_perspective(render->width, render->height, render->persp_view_angle, render->persp_view_near, render->persp_view_distance);
        mbo.proj.m[1][1] *= -1;
    }else if(render->flags & ENGINE_RENDER_FLAG_FRUSTRUM){
        mbo.proj = m4_frustum(-render->frust_side, render->frust_side, -render->frust_side, render->frust_side, render->frust_near, render->frust_far);
        mbo.proj.m[1][1] *= -1;
    }else
        mbo.proj = m4_ortho(-render->ortg_view_size, render->ortg_view_size, -render->ortg_view_size, render->ortg_view_size, -render->ortg_view_distance, render->ortg_view_distance);

    DescriptorUpdate(descriptor, (char *)&mbo, sizeof(mbo));
}

void ModelInvMatrixBuffer(ModelObject3D* mo, uint32_t indx_node, BluePrintDescriptor *descriptor)
{
    glTFStruct *glTF = mo->obj;

    InvMatrixsBuffer imb = {};
    memset(&imb, 0, sizeof(InvMatrixsBuffer));

    for(int k=0;k < glTF->num_join_mats;k++)
      imb.mats[k] = glTF->joint_mats[k].join_mat;

    imb.size = glTF->num_join_mats;

    DescriptorUpdate(descriptor, (char *)&imb, sizeof(imb));

    //Временно
    /*
    InvMatrixsBuffer imb = {};
    memset(&imb, 0, sizeof(InvMatrixsBuffer));

    DescriptorUpdate(descriptor, &imb, sizeof(imb));*/
}

void ModelDefaultUpdate(ModelObject3D *mo)
{
    /*for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            ModelStruct *obj = &mo->nodes[i].models[j];

            for(int l=0;l < obj->graphObj.blueprints.num_blue_print_packs;l++)
            {
                BluePrintPack *pack = &obj->graphObj.blueprints.blue_print_packs[l];

                if(pack->render_point == engine.current_render)
                {
                    for(int k=0;k < pack->num_descriptors;k++)
                    {
                        BluePrintDescriptor *descriptor = &pack->descriptors[k];

                        if(descriptor->update == NULL)
                            continue;

                        if(descriptor->descrType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                        {
                            Update_Func update = descriptor->update;
                            update(mo, i, descriptor);
                        }
                    }
                }

            }
        }
    }*/
}

void ModelClean(ModelObject3D* mo){
    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            GraphicsObjectClean(&mo->nodes[i].models[j].graphObj);
        }
    }
}

void ModelRecreate(ModelObject3D* mo){

    /*for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            ModelStruct *model = &mo->nodes[i].models[j];

            for(int l=0; l < model->graphObj.gItems.num_shader_packs;l++)
            {
                BluePrintPack *pack = &model->graphObj.blueprints.blue_print_packs[l];

                PipelineSetting *settings = &pack->setting;

                settings->scissor.offset.x = 0;
                settings->scissor.offset.y = 0;
                settings->scissor.extent.height = engine.height;
                settings->scissor.extent.width = engine.width;
                settings->viewport.x = 0;
                settings->viewport.y = 0;
                settings->viewport.height = engine.height;
                settings->viewport.width = engine.width;

                BuffersRecreateUniform(&model->graphObj.blueprints);
                GraphicsObjectCreateDrawItems(&model->graphObj);
                PipelineCreateGraphics(&model->graphObj);
            }
        }
    }*/
}

//Не корректно
void ModelSetSomeViewport(ModelObject3D* mo, float x, float y, float height, float width)
{
    /*
    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            ModelStruct *model = &mo->nodes[i].models[j];

            PipelineSetting *settings = (PipelineSetting *)model->graphObj.gItems.settings;

            for(int m=0; m < model->graphObj.gItems.settingsCount; m++)
            {
                settings[m].viewport.x = x;
                settings[m].viewport.y = y;
                settings[m].viewport.height = height;
                settings[m].viewport.width = width;
            }
        }
    }*/
}

void ModelDestroy(ModelObject3D* mo){
    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            GraphicsObjectDestroy(&mo->nodes[i].models[j].graphObj);
        }
    }
}

void ModelSetLightEnable(void *obj, bool enable)
{
    ModelObject3D *mo = (ModelObject3D *)obj;

    if(enable)
        mo->self.flags |= ENGINE_GAME_OBJECT_FLAG_LIGHT;
    else
        mo->self.flags &= ~(ENGINE_GAME_OBJECT_FLAG_LIGHT);
}

void ModelSetSelCameraEnable(void *obj, bool enable)
{
    ModelObject3D *mo = (ModelObject3D *)obj;

    if(enable)
        mo->self.flags |= ENGINE_GAME_OBJECT_FLAG_SELF_CAMERA;
    else
        mo->self.flags &= ~(ENGINE_GAME_OBJECT_FLAG_SELF_CAMERA);
}

void ModelSetShadowDescriptor(ModelStruct *model, uint32_t type, void *render, uint32_t layer_indx)
{
    /*uint32_t num = model->graphObj.blueprints.num_blue_print_packs;
    model->graphObj.blueprints.blue_print_packs[num].render_point = render;

    if(type == ENGINE_LIGHT_TYPE_DIRECTIONAL)
        BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(ModelBuffer3D), VK_SHADER_STAGE_VERTEX_BIT, (void *)ModelDirLightModelUpdate, layer_indx);
    else
        BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(ModelBuffer3D), VK_SHADER_STAGE_VERTEX_BIT, (void *)ModelSpotLightModelUpdate, layer_indx);

    PipelineSetting setting;

    PipelineSettingSetDefault(&setting);

    PipelineSettingSetShader(&setting, &_binary_shaders_depth_vert_spv_start, (size_t)(&_binary_shaders_depth_vert_spv_size), VK_SHADER_STAGE_VERTEX_BIT);
    PipelineSettingSetShader(&setting, &_binary_shaders_depth_frag_spv_start, (size_t)(&_binary_shaders_depth_frag_spv_size), VK_SHADER_STAGE_FRAGMENT_BIT);

    setting.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    setting.flags &= ~(ENGINE_PIPELINE_FLAG_DYNAMIC_VIEW);
    setting.vert_indx = 1;
    setting.cull_mode = VK_CULL_MODE_FRONT_BIT;

    ModelAddSettingPipeline(model, num, setting);

    model->graphObj.blueprints.num_blue_print_packs ++;*/
}

void ModelSetOmniShadowDescriptor(ModelStruct *model, void *render, uint32_t layer_indx)
{
    /*uint32_t num = model->graphObj.blueprints.num_blue_print_packs;
    model->graphObj.blueprints.blue_print_packs[num].render_point = render;

    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(ModelBuffer3D), VK_SHADER_STAGE_VERTEX_BIT, (void *)ModelOmniLightModelUpdate, layer_indx);
    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(LightPosBuff), VK_SHADER_STAGE_VERTEX_BIT, (void *)ModelDescriptorLightPosUpdate, layer_indx);

    BluePrintAddPushConstant(&model->graphObj.blueprints, num, sizeof(mat4), VK_SHADER_STAGE_VERTEX_BIT, 0);

    PipelineSetting setting;

    PipelineSettingSetDefault(&setting);

    PipelineSettingSetShader(&setting, &_binary_shaders_depth_vert_omni_spv_start, (size_t)(&_binary_shaders_depth_vert_omni_spv_size), VK_SHADER_STAGE_VERTEX_BIT);
    PipelineSettingSetShader(&setting, &_binary_shaders_depth_frag_omni_spv_start, (size_t)(&_binary_shaders_depth_frag_omni_spv_size), VK_SHADER_STAGE_FRAGMENT_BIT);

    setting.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    setting.flags &= ~(ENGINE_PIPELINE_FLAG_DYNAMIC_VIEW);
    setting.flags &= ~(ENGINE_PIPELINE_FLAG_ALPHA);
    setting.vert_indx = 1;
    setting.cull_mode = VK_CULL_MODE_FRONT_BIT;

    ModelAddSettingPipeline(model, num, setting);

    model->graphObj.blueprints.num_blue_print_packs ++;*/
}

void ModelSetShadowDefaultDescriptor(ModelStruct *model, void *render)
{
    uint32_t num = model->graphObj.blueprints.num_blue_print_packs;
    model->graphObj.blueprints.blue_print_packs[num].render_point = render;

    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(ModelBuffer3D), VK_SHADER_STAGE_VERTEX_BIT, (void *)ModelModelBufferUpdate, 0);
    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(InvMatrixsBuffer), VK_SHADER_STAGE_VERTEX_BIT, (void *)ModelInvMatrixBuffer, 0);

    /*RenderTexture **renders = engine.DataR.dir_shadow_array;

    if(engine.DataR.num_dir_shadows > 1)
        BluePrintAddRenderImageArray(&model->graphObj.blueprints, num, renders, engine.DataR.num_dir_shadows);
    else
        BluePrintAddRenderImage(&model->graphObj.blueprints, num, renders[0]);

    renders = engine.DataR.point_shadow_array;

    if(engine.DataR.num_point_shadows > 1)
        BluePrintAddRenderImageArray(&model->graphObj.blueprints, num, renders, engine.DataR.num_point_shadows);
    else
        BluePrintAddRenderImage(&model->graphObj.blueprints, num, renders[0]);

    renders = engine.DataR.spot_shadow_array;

    if(engine.DataR.num_spot_shadows > 1)
        BluePrintAddRenderImageArray(&model->graphObj.blueprints, num, renders, engine.DataR.num_spot_shadows);
    else
        BluePrintAddRenderImage(&model->graphObj.blueprints, num, renders[0]);*/

    /*BluePrintAddTextureImage(&model->graphObj.blueprints, num, model->diffuse, VK_SHADER_STAGE_FRAGMENT_BIT);
    //TextureImageAdd(&model->graphObj.blueprints, model->specular);
    BluePrintAddTextureImage(&model->graphObj.blueprints, num, model->normal, VK_SHADER_STAGE_FRAGMENT_BIT);

    PipelineSetting setting;

    PipelineSettingSetDefault(&setting);

    PipelineSettingSetShader(&setting, &_binary_shaders_model_shadow_vert_spv_start, (size_t)(&_binary_shaders_model_shadow_vert_spv_size), VK_SHADER_STAGE_VERTEX_BIT);
    PipelineSettingSetShader(&setting, &_binary_shaders_model_shadow_frag_spv_start, (size_t)(&_binary_shaders_model_shadow_frag_spv_size), VK_SHADER_STAGE_FRAGMENT_BIT);

    setting.vert_indx = 0;

    ModelAddSettingPipeline(model, num, setting);

    model->graphObj.blueprints.num_blue_print_packs ++;*/
}

void ModelSetDefaultDescriptor(ModelStruct *model, void *render)
{
    /*uint32_t num = model->graphObj.blueprints.num_blue_print_packs;
    model->graphObj.blueprints.blue_print_packs[num].render_point = render;

    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(ModelBuffer3D), VK_SHADER_STAGE_VERTEX_BIT, (void *)ModelModelBufferUpdate, 0);
    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(InvMatrixsBuffer), VK_SHADER_STAGE_VERTEX_BIT, (void *)ModelInvMatrixBuffer, 0);
    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(DirLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)ModelDescriptorDirLightsUpdate, 0);
    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(PointLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)ModelDescriptorPointLightsUpdate, 0);
    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(SpotLightBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)ModelDescriptorSpotLightsUpdate, 0);
    BluePrintAddUniformObject(&model->graphObj.blueprints, num, sizeof(LightStatusBuffer), VK_SHADER_STAGE_FRAGMENT_BIT, (void *)ModelLigtStatusBufferUpdate, 0);

    BluePrintAddTextureImage(&model->graphObj.blueprints, num, model->diffuse, VK_SHADER_STAGE_FRAGMENT_BIT);
    //TextureImageAdd(&model->graphObj.blueprints, model->specular);
    BluePrintAddTextureImage(&model->graphObj.blueprints, num, model->normal, VK_SHADER_STAGE_FRAGMENT_BIT);

    PipelineSetting setting;

    PipelineSettingSetDefault(&setting);

    PipelineSettingSetShader(&setting, &_binary_shaders_model_vert_spv_start, (size_t)(&_binary_shaders_model_vert_spv_size), VK_SHADER_STAGE_VERTEX_BIT);
    PipelineSettingSetShader(&setting, &_binary_shaders_model_frag_spv_start, (size_t)(&_binary_shaders_model_frag_spv_size), VK_SHADER_STAGE_FRAGMENT_BIT);

    setting.vert_indx = 0;
    setting.flags &= ~(ENGINE_PIPELINE_FLAG_ALPHA);

    ModelAddSettingPipeline(model, num, setting);

    model->graphObj.blueprints.num_blue_print_packs ++;*/
}

void ModelPopulateVertex3D(ModelStruct *model)
{
    uint32_t num = model->graphObj.num_shapes;

    model->graphObj.shapes[num].bindingDescription = &Bind3DDescription;
    model->graphObj.shapes[num].attr = cubeAttributeDescription;
    model->graphObj.shapes[num].countAttr = 3;

    ModelVertex3D *m_verts = model->graphObj.shapes[0].vParam.vertices;
    uint32_t count = model->graphObj.shapes[0].vParam.verticesSize;
    Vertex3D *verts = AllocateMemory(count, sizeof(Vertex3D));

    for(int i=0;i < count;i++)
    {
        verts[i].position = m_verts[i].position;
        verts[i].normal = m_verts[i].normal;
        verts[i].texCoord = m_verts[i].texCoord;
    }

    GraphicsObjectSetVertex(&model->graphObj, verts, count, sizeof(Vertex3D), model->graphObj.shapes[0].iParam.indices, model->graphObj.shapes[0].iParam.indexesSize, sizeof(uint32_t));

}

void ModelApplyShadows(ModelStruct *model, DrawParam *dParam)
{

    /*RenderTexture **renders = engine.DataR.dir_shadow_array;

    for(int i=0;i < engine.DataR.num_dir_shadows;i++)
        ModelSetShadowDescriptor(model, ENGINE_LIGHT_TYPE_DIRECTIONAL, renders[i], i);

    renders = engine.DataR.point_shadow_array;

    for(int i=0;i < engine.DataR.num_point_shadows;i++)
        ModelSetOmniShadowDescriptor(model, renders[i], i);

    renders = engine.DataR.spot_shadow_array;

    for(int i=0;i < engine.DataR.num_spot_shadows;i++)
        ModelSetShadowDescriptor(model, ENGINE_LIGHT_TYPE_SPOT, renders[i], i);*/

    ModelSetShadowDefaultDescriptor(model, dParam->render);

    ModelPopulateVertex3D(model);
}

void ModelDefaultInit(ModelStruct *model, DrawParam *dParam){

    if(dParam->flags & ENGINE_DRAW_PARAM_FLAG_ADD_SHADOW)
        ModelApplyShadows(model, dParam);
    else
        ModelSetDefaultDescriptor(model, dParam->render);


    GraphicsObjectCreateDrawItems(&model->graphObj);

    PipelineCreateGraphics(&model->graphObj);

}
