#ifndef SHADERS_H
#define SHADERS_H

#include "Variabels/engine_includes.h"
#include "shader_builder.h"

void ShadersMakeClear2DShader(ShaderBuilder *vert, ShaderBuilder *frag);
void ShadersMakeDefault2DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture);

void ShadersMakeDefault3DShader(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture);
void ShadersMakeDeafult3DShaderWithLight(ShaderBuilder *vert, ShaderBuilder *frag, bool hasTexture, bool hasNormal, bool hasSpecular);

void ShadersMakeDefault3DModelShader(ShaderBuilder *vert, ShaderBuilder *frag, uint32_t count_texture);
void ShadersMakeDeafult3DModelShaderWithLight(ShaderBuilder *vert, ShaderBuilder *frag, uint32_t count_texture, uint32_t count_normal, uint32_t count_specular);

void ShaderMakeDefaultParticle2DShader(ShaderBuilder *vert, ShaderBuilder *frag);
void ShaderMakeDefaultParticle3DShader(ShaderBuilder *vert, ShaderBuilder *frag);

void ShadersMakeDefault2DTextShader(ShaderBuilder *vert, ShaderBuilder *frag);

void ShadersMakeTerrainShader(ShaderBuilder *vert, ShaderBuilder *tesc, ShaderBuilder *tese, ShaderBuilder *frag);

#endif // SHADERS_H
