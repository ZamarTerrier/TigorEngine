#include "Core/graphicsObject.h"

#include <vulkan/vulkan.h>

#include "Core/e_memory.h"
#include "Core/e_device.h"
#include "Core/pipeline.h"
#include "Core/e_buffer.h"
#include "Core/e_texture.h"
#include "Core/e_descriptor.h"

#include "Tools/shader_builder.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_descriptors.h"
#include "Data/e_resource_engine.h"

extern TEngine engine;

void GraphicsObjectInit(GraphicsObject* graphObj, uint32_t type)
{
    graphObj->blueprints.num_blue_print_packs = 0;

    graphObj->blueprints.isShadow = false;

    if(graphObj->num_shapes == 0)
        memset(&graphObj->shapes, 0, sizeof(Shape) * MAX_SHAPES);

    switch(type)
    {
        case TIGOR_VERTEX_TYPE_2D_OBJECT:
            graphObj->shapes[0].bindingDescription = &Bind2DDescription;
            graphObj->shapes[0].attr = planeAttributeDescription;
            graphObj->shapes[0].countAttr = 3;
            graphObj->shapes[0].countBind = 1;
            graphObj->shapes[0].type = type;
            break;
        case TIGOR_VERTEX_TYPE_3D_OBJECT:
            graphObj->shapes[0].bindingDescription = &Bind3DDescription;
            graphObj->shapes[0].attr = cubeAttributeDescription;
            graphObj->shapes[0].countAttr = 3;
            graphObj->shapes[0].countBind = 1;
            graphObj->shapes[0].type = type;
            break;
        case TIGOR_VERTEX_TYPE_3D_INSTANCE:
            graphObj->shapes[0].bindingDescription = AllocateMemory(2, sizeof(EIVertexInputBindingDescription));
            graphObj->shapes[0].bindingDescription[0] = Bind3DDescription;
            graphObj->shapes[0].bindingDescription[1] = Bind3DInstanceDescription;
            graphObj->shapes[0].attr = instanceAttributeDescription;
            graphObj->shapes[0].countAttr = 6;
            graphObj->shapes[0].countBind = 2;
            graphObj->shapes[0].type = type;
            break;
        case TIGOR_VERTEX_TYPE_TREE_INSTANCE:
            graphObj->shapes[0].bindingDescription = AllocateMemory(2, sizeof(EIVertexInputBindingDescription));
            graphObj->shapes[0].bindingDescription[0] = BindTree3DDescription;
            graphObj->shapes[0].bindingDescription[1] = Bind3DInstanceDescription;
            graphObj->shapes[0].attr = treeInstanceAttributeDescription;
            graphObj->shapes[0].countAttr = 7;
            graphObj->shapes[0].countBind = 2;
            graphObj->shapes[0].type = type;
            break;
        case TIGOR_VERTEX_TYPE_MODEL_OBJECT:
            graphObj->shapes[0].bindingDescription = &BindModel3DDescription;
            graphObj->shapes[0].attr = modelAttributeDescription;
            graphObj->shapes[0].countAttr = 7;
            graphObj->shapes[0].countBind = 1;
            graphObj->shapes[0].type = type;
            break;
        case TIGOR_VERTEX_TYPE_2D_PARTICLE:
            graphObj->shapes[0].bindingDescription = &BindParticle2DDescription;
            graphObj->shapes[0].attr = particle2DAttributeDescription;
            graphObj->shapes[0].countAttr = 3;
            graphObj->shapes[0].countBind = 1;
            graphObj->shapes[0].type = type;
            break;
        case TIGOR_VERTEX_TYPE_3D_PARTICLE:
            graphObj->shapes[0].bindingDescription = &BindParticle3DDescription;
            graphObj->shapes[0].attr = particle3DAttributeDescription;
            graphObj->shapes[0].countAttr = 3;
            graphObj->shapes[0].countBind = 1;
            graphObj->shapes[0].type = type;
            break;
        case TIGOR_VERTEX_TYPE_TERRAIN:
            graphObj->shapes[0].bindingDescription = &BindTerrainDescription;
            graphObj->shapes[0].attr = TerrainAttributeDescription;
            graphObj->shapes[0].countAttr = 3;
            graphObj->shapes[0].countBind = 1;
            graphObj->shapes[0].type = type;
            break;
        case TIGOR_VERTEX_TYPE_SKY:
            graphObj->shapes[0].bindingDescription = &BindSkyDescription;
            graphObj->shapes[0].attr = SkyAttributeDescription;
            graphObj->shapes[0].countAttr = 2;
            graphObj->shapes[0].countBind = 1;
            graphObj->shapes[0].type = type;
            break;
        default:
            graphObj->shapes[0].type = 0;
            break;
    }
}

void GraphicsObjectSetVertex(GraphicsObject* graphObj, void *vert, int vertCount, uint32_t type_v_size, uint32_t *inx, int indxCount, uint32_t type_i_size){

    uint32_t num = graphObj->num_shapes;

    graphObj->shapes[num].vParam.typeSize = type_v_size;
    graphObj->shapes[num].iParam.typeSize = type_i_size;

    int res = 0;

    if(vert != NULL)
    {
        graphObj->shapes[num].vParam.vertices = AllocateMemoryP(vertCount, graphObj->shapes[num].vParam.typeSize, graphObj);
        memcpy(graphObj->shapes[num].vParam.vertices, vert, graphObj->shapes[num].vParam.typeSize * vertCount);
        graphObj->shapes[num].vParam.num_verts = vertCount;
    }

    if(inx != NULL)
    {
        graphObj->shapes[num].iParam.indices = AllocateMemoryP(indxCount, graphObj->shapes[num].iParam.typeSize, graphObj);
        memcpy(graphObj->shapes[num].iParam.indices, inx, graphObj->shapes[num].iParam.typeSize * indxCount);
        graphObj->shapes[num].iParam.indexesSize = indxCount;
    }

    if(!graphObj->shapes[num].init)
    {
        res = BuffersCreateVertex((struct VertexParam_T *)&graphObj->shapes[num].vParam);
        if(res)
            return;

        res = BuffersCreateIndex((struct IndexParam_T *)&graphObj->shapes[num].iParam);
        if(res)
            return;

        graphObj->shapes[num].init = true;
    }

    if(graphObj->shapes[num].vParam.num_verts > 0)
        BuffersUpdateVertex((struct VertexParam_T *)&graphObj->shapes[num].vParam);

    if(graphObj->shapes[num].iParam.indexesSize > 0)
        BuffersUpdateIndex((struct IndexParam_T *)&graphObj->shapes[num].iParam);

    graphObj->num_shapes ++;
}

void GraphicsObjectSetShaderWithUniform(GraphicsObject* graphObj, ShaderObject *shader, uint32_t pack_indx){
    ShaderBuilder *temp = AllocateMemory(1, sizeof(ShaderBuilder));

    ShaderBuilderMakeUniformsFromShader(temp, (uint32_t *)shader->code, shader->size, &graphObj->blueprints, pack_indx);

    PipelineSetting* setting = (PipelineSetting *)&graphObj->blueprints.blue_print_packs[pack_indx].setting;

    PipelineSettingSetShader(setting, shader, temp->type == SHADER_TYPE_VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT);

    FreeMemory(temp);
}

void GraphicsObjectSetShader(GraphicsObject* graphObj, ShaderObject *shader, uint32_t pack_indx, uint32_t shader_type){
    PipelineSetting* setting = (PipelineSetting *)&graphObj->blueprints.blue_print_packs[pack_indx].setting;

    PipelineSettingSetShader(setting, shader, shader_type);
}

void GraphicsObjectCreateDrawItems(GraphicsObject* graphObj){

    memset(graphObj->gItems.shader_packs, 0, sizeof(ShaderPack) * MAX_BLUE_PRINTS);

    for(int i=0;i< graphObj->blueprints.num_blue_print_packs;i++)
    {
        BluePrintPack *pack = &graphObj->blueprints.blue_print_packs[i];

        DescriptorCreate(&graphObj->gItems.shader_packs[i].descriptor, pack->descriptors, &graphObj->blueprints, pack->num_descriptors, engine.imagesCount);
    }    
}

void GraphicsObjectClean(GraphicsObject *graphObj)
{
    for(int i=0;i < graphObj->gItems.num_shader_packs;i++)
    {
        PipelineDestroy(&graphObj->gItems.shader_packs[i]);

        DescriptorDestroy(&graphObj->gItems.shader_packs[i].descriptor);
    }

    for(int i=0;i < graphObj->blueprints.num_blue_print_packs;i++)
    {
        BluePrintPack *pack = &graphObj->blueprints.blue_print_packs[i];

        for(int j=0;j < pack->num_descriptors;j++)
        {
            BluePrintDescriptor *descriptor = &pack->descriptors[j];

            if(descriptor->descrType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER){
                BuffersDestroyContainer(&descriptor->uniform);
            }
        }
    }
}

void GraphicsObjectDestroy(GraphicsObject* graphObj){

    TDevice *device = (TDevice *)engine.device;

    for(int i=0;i < graphObj->gItems.num_shader_packs;i++)
    {
        PipelineDestroy(&graphObj->gItems.shader_packs[i]);

        DescriptorDestroy(&graphObj->gItems.shader_packs[i].descriptor);
    }

    BluePrintClearAll(&graphObj->blueprints);    

    for(int i=0;i < graphObj->num_shapes;i++)
    {
        if(graphObj->shapes[i].iParam.indexesSize > 0)
            BuffersDestroyBuffer(&graphObj->shapes[i].iParam.buffer);

        BuffersDestroyBuffer(&graphObj->shapes[i].vParam.buffer);
    }
}

void GraphicsObjectSetShadersPath(GraphicsObject *graphObj, const char *vert, const char *frag)
{

    int len = strlen(vert);
    memset(graphObj->aShader.vertShader, 0, 256);
    memcpy(graphObj->aShader.vertShader, vert, len);


    len = strlen(frag);
    memset(graphObj->aShader.fragShader, 0, 256);
    memcpy(graphObj->aShader.fragShader, frag, len);
}
