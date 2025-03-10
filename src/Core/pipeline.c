#include "Core/pipeline.h"

#include <vulkan/vulkan.h>

#include "Core/e_memory.h"
#include "Core/e_device.h"
#include "Core/swapchain.h"

#include "Objects/render_texture.h"

#include "Variabels/e_pipeline_variables.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_descriptors.h"

extern TEngine engine;

void PipelineSettingSetShader(PipelineSetting *setting, ShaderObject *shader, uint32_t type)
{
    uint32_t num = setting->num_stages;
    setting->stages[num].code_shader = shader->code;
    setting->stages[num].size_code_shader = shader->size;
    setting->stages[num].type_code_shader = type;
    setting->stages[num].flags = shader->flags;
    setting->num_stages ++;
}

void PipelineAcceptStack(void *pipeline, void *pipeline_layout)
{
    PipelineStack *stack;

    if(engine.cache.alloc_pipeline_head->node == NULL){
        engine.cache.alloc_pipeline_head->next = calloc(1, sizeof(ChildStack));
        engine.cache.alloc_pipeline_head->node = calloc(1, sizeof(PipelineStack));

        stack = engine.cache.alloc_pipeline_head->node;
        stack->GraphicsPipeline = pipeline;
        stack->GraphicsPipelineLayout = pipeline_layout;
    }
    else{

        ChildStack *child = engine.cache.alloc_pipeline_head->next;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = calloc(1, sizeof(ChildStack));
        child->node = calloc(1, sizeof(PipelineStack));

        stack = child->node;
        stack->GraphicsPipeline = pipeline;
        stack->GraphicsPipelineLayout = pipeline_layout;
    }

}

void PipelineClearAll()
{
    ChildStack *child = engine.cache.alloc_pipeline_head;
    ChildStack *next = NULL;

    PipelineStack *stack = NULL;

    uint32_t counter = 0;

    while(child != NULL)
    {
        next = child->next;
        
        if(child->node != NULL)
            counter ++;

        if(child->node != NULL){
            stack = child->node;
            PipelineDestroyStack(stack->GraphicsPipeline);
        }

        child = next;
    }

    if(engine.cache.alloc_pipeline_head != NULL){
        free(engine.cache.alloc_pipeline_head);
        engine.cache.alloc_pipeline_head = NULL;
    }

    if(counter > 0)
        printf("Autofree VkPipelines count : %i\n", counter);
}

void PipelineDestroyStack(void *pipeline)
{
    if(pipeline == NULL)
        return;

    TDevice *device = (TDevice *)engine.device;

    PipelineStack *stack = NULL;

    ChildStack *child = engine.cache.alloc_pipeline_head;
    ChildStack *before = NULL;

    while(child->next != NULL)
    {
        stack = child->node;

        if(stack->GraphicsPipeline == pipeline)
            break;

        before = child;
        child = child->next;
    }

    if(child->next != NULL){
        vkDestroyPipeline(device->e_device, (VkPipeline)stack->GraphicsPipeline, NULL);
        vkDestroyPipelineLayout(device->e_device, (VkPipelineLayout)stack->GraphicsPipelineLayout, NULL);
        free(child->node);
        child->node = NULL;

        if(before != NULL)
            before->next = child->next;
        else
            engine.cache.alloc_pipeline_head = child->next;
        
        free(child);
        child = NULL;
    }
}

void PipelineSettingSetDefault(void *arg){

    TSwapChain *swapchain = (TSwapChain *)engine.swapchain;

    PipelineSetting *setting = arg;

    memset(setting, 0, sizeof(PipelineSetting));

    setting->poligonMode = VK_POLYGON_MODE_FILL;
    setting->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    setting->scissor.offset.x = 0;
    setting->scissor.offset.y = 0;
    setting->scissor.extent = *(EIExtent2D*)&swapchain->swapChainExtent;
    setting->viewport.x = 0.0f;
    setting->viewport.y = 0.0f;
    setting->viewport.width = (float) swapchain->swapChainExtent.width;
    setting->viewport.height = (float) swapchain->swapChainExtent.height;
    setting->viewport.minDepth = 0.0f;
    setting->viewport.maxDepth = 1.0f;
    setting->flags = TIGOR_PIPELINE_FLAG_DYNAMIC_VIEW | TIGOR_PIPELINE_FLAG_DRAW_INDEXED | TIGOR_PIPELINE_FLAG_BIAS |\
                     TIGOR_PIPELINE_FLAG_ALPHA | TIGOR_PIPELINE_FLAG_FRAGMENT_SHADER | TIGOR_PIPELINE_FLAG_VERTEX_SHADER;
    setting->cull_mode = VK_CULL_MODE_BACK_BIT;
}

void PipelineMakePipeline(GraphicsObject *graphObj, uint32_t indx_pack)
{
    TDevice *device = (TDevice *)engine.device;

    BluePrintPack *pack = &graphObj->blueprints.blue_print_packs[indx_pack];
    PipelineSetting *setting = &graphObj->blueprints.blue_print_packs[indx_pack].setting;
    ShaderDescriptor *descriptor = &graphObj->gItems.shader_packs[indx_pack].descriptor;
    PipelineStruct *pipeline = &graphObj->gItems.shader_packs[indx_pack].pipeline;
    RenderTexture *render = graphObj->blueprints.blue_print_packs[indx_pack].render_point;

    if(render == NULL)
        render = graphObj->blueprints.blue_print_packs[indx_pack].render_point = engine.main_render;

    //Шейдеры
    VkPipelineShaderStageCreateInfo shaderStages[6];
    memset(shaderStages, 0, sizeof(VkPipelineShaderStageCreateInfo) * 6);

    uint32_t temp = 0x80;
    uint32_t count_stages = 0;
    while(temp < 0x1000){

        if(setting->flags & temp)
        {
            shaderStages[count_stages].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[count_stages].stage = setting->stages[count_stages].type_code_shader;
            shaderStages[count_stages].pName = "main";

            ShaderObject some_shader_code;

            some_shader_code.code = setting->stages[count_stages].code_shader;
            some_shader_code.size = setting->stages[count_stages].size_code_shader;

            shaderStages[count_stages].module = (VkShaderModule)createShaderModule(some_shader_code);

            count_stages ++;

        }

        temp *=2;
    }

    //-----------------
    //Информация для щейдеров

    VkPipelineInputAssemblyStateCreateInfo inputAssembly;
    memset(&inputAssembly, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = setting->topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    //-----------
    //Растеризатор
    VkPipelineRasterizationStateCreateInfo rasterizer;
    memset(&rasterizer, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = setting->poligonMode;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = setting->cull_mode;
    rasterizer.frontFace = setting->flags & TIGOR_PIPELINE_FLAG_FACE_CLOCKWISE ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = setting->flags & TIGOR_PIPELINE_FLAG_BIAS ? VK_TRUE : VK_FALSE;
    //-----------------
    //Колор блендинг
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    memset(&colorBlendAttachment, 0, sizeof(VkPipelineColorBlendAttachmentState));
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    if((render->type != TIGOR_RENDER_TYPE_DEPTH && !(render->flags & TIGOR_RENDER_FLAG_DEPTH)) || (setting->flags & TIGOR_PIPELINE_FLAG_ALPHA))
    {
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }else
        colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending;
    memset(&colorBlending, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    if(render->type == TIGOR_RENDER_TYPE_DEPTH || (render->flags & TIGOR_RENDER_FLAG_DEPTH))
    {
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;
    }

    //---------------
    VkPipelineDepthStencilStateCreateInfo depthStencil;
    memset(&depthStencil, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = graphObj->gItems.perspective;
    depthStencil.depthWriteEnable = graphObj->gItems.perspective;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional

    //---------------
    //Мультисэмплинг
    VkPipelineMultisampleStateCreateInfo multisampling;
    memset(&multisampling, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    //------------
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    memset(&vertexInputInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = graphObj->shapes[setting->vert_indx].countBind;
    vertexInputInfo.vertexAttributeDescriptionCount = graphObj->shapes[setting->vert_indx].countAttr;
    vertexInputInfo.pVertexBindingDescriptions = (const VkVertexInputBindingDescription *) graphObj->shapes[setting->vert_indx].bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = (const VkVertexInputAttributeDescription *)graphObj->shapes[setting->vert_indx].attr;


    VkPipelineDynamicStateCreateInfo dynamicState;
    memset(&dynamicState, 0, sizeof(VkPipelineDynamicStateCreateInfo));

    VkPipelineViewportStateCreateInfo viewportState;
    memset(&viewportState, 0, sizeof(VkPipelineViewportStateCreateInfo));
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    if(setting->flags & TIGOR_PIPELINE_FLAG_DYNAMIC_VIEW)
    {

        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

    }else{

        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = render->width;
        viewport.height = render->height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1.0;

        VkRect2D scissor;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = render->width;
        scissor.extent.height = render->height;

        viewportState.pViewports = &viewport;
        viewportState.pScissors = &scissor;
    }


    VkPushConstantRange *push_ranges = NULL;
    
    if(pack->num_push_constants > 0){
        push_ranges = AllocateMemory(pack->num_push_constants, sizeof(VkPushConstantRange));

        for(int l=0 ;l < pack->num_push_constants;l++)
        {
            push_ranges[l].offset = pack->push_constants[l].offset;
            push_ranges[l].size = pack->push_constants[l].size;
            push_ranges[l].stageFlags = pack->push_constants[l].stageFlags;
        }
    }


    VkPipelineLayoutCreateInfo pipelineLayoutInfo;
    memset(&pipelineLayoutInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Кол-во Дескирпторов для Юниформ баферов
    pipelineLayoutInfo.pSetLayouts = (const VkDescriptorSetLayout *) &descriptor->descr_set_layout; // Дескирпторы для Юниформ баферов
    pipelineLayoutInfo.pushConstantRangeCount = pack->num_push_constants; // Optional
    pipelineLayoutInfo.pPushConstantRanges = push_ranges;

    if (vkCreatePipelineLayout(device->e_device, &pipelineLayoutInfo, NULL, (VkPipelineLayout *)&pipeline->layout) != VK_SUCCESS) {
        printf("failed to create pipeline layout!");
        exit(1);
    }

    FreeMemory(push_ranges);

    //Сам пайплайн
    VkGraphicsPipelineCreateInfo pipelineInfo;
    memset(&pipelineInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

    VkPipelineTessellationStateCreateInfo *tessellationState;
    if(setting->flags & (TIGOR_PIPELINE_FLAG_TESSELLATION_CONTROL_SHADER | TIGOR_PIPELINE_FLAG_TESSELLATION_EVALUATION_SHADER))
    {
        tessellationState = AllocateMemory( 1, sizeof(VkPipelineTessellationStateCreateInfo));
        tessellationState->sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationState->patchControlPoints = 4;//patchControlPoints;

        pipelineInfo.pTessellationState = tessellationState;
    }

    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = count_stages;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;

    if(setting->flags & TIGOR_PIPELINE_FLAG_DYNAMIC_VIEW)
        pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = (VkPipelineLayout)pipeline->layout;
    pipelineInfo.renderPass = (VkRenderPass)render->render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(device->e_device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, (VkPipeline *)&pipeline->pipeline) != VK_SUCCESS) {
        printf("failed to create graphics pipeline!");
        exit(1);
    }

    if(setting->flags & (TIGOR_PIPELINE_FLAG_TESSELLATION_CONTROL_SHADER | TIGOR_PIPELINE_FLAG_TESSELLATION_EVALUATION_SHADER))
        FreeMemory(tessellationState);

    //-----------------------

    for(int i=0;i < count_stages;i++)
        vkDestroyShaderModule(device->e_device, shaderStages[i].module, NULL);

    PipelineAcceptStack(pipeline->pipeline, pipeline->layout);
}

void PipelineCreateGraphics(GraphicsObject* graphObj){

    graphObj->gItems.num_shader_packs = graphObj->blueprints.num_blue_print_packs;

    for(int i=0; i < graphObj->blueprints.num_blue_print_packs; i++){

        ShaderPack *pack = &graphObj->gItems.shader_packs[i];

        PipelineMakePipeline(graphObj, i);
    }
}

void PipelineDestroy(ShaderPack *pack)
{
    PipelineDestroyStack(pack->pipeline.pipeline);
}

