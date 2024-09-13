#ifndef PIPELINE_H
#define PIPELINE_H

#include "Variabels/engine_includes.h"

#include "Variabels/e_pipeline_variables.h"

#include "graphicsObject.h"

#include "Tools/e_tools.h"
#include "e_descriptor.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    void *GraphicsPipeline;
    void *GraphicsPipelineLayout;
} PipelineStack;

void PipelineSettingSetShader(PipelineSetting *setting, ShaderObject *shader, uint32_t type);

void PipelineDestroyStack(void *pipeline);

void PipelineClearAll();

void PipelineSettingSetDefault(void *setting);

void PipelineCreateGraphics(GraphicsObject* graphObj);

void PipelineDestroy(ShaderPack *pipeline);

#ifdef __cplusplus
}
#endif

#endif // PIPELINE_H
