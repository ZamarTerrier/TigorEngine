#ifndef SHADERS_H
#define SHADERS_H

#include "Variabels/engine_includes.h"
#include "shader_builder.h"

void ShadersMakeClear2DShader(ShaderBuilder *vert, ShaderBuilder *frag);
void ShadersMakeDefault2DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture);
void ShaderMakeDefaultParticle2DShader(ShaderBuilder *vert, ShaderBuilder *frag);
void ShadersMakeDefault3DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture);
void ShadersMakeDefault3DModelShader(ShaderBuilder *vert, ShaderBuilder *frag, uint32_t count_texture);
void ShaderMakeDefaultParticle3DShader(ShaderBuilder *vert, ShaderBuilder *frag);
void ShadersMakeDeafult3DShaderWithLight(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture);
void ShadersMakeDefault2DTextShader(ShaderBuilder *vert, ShaderBuilder *frag);

#endif // SHADERS_H
