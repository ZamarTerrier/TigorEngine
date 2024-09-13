#include "builder_lang.h"

void BuilderLangParce(ShaderBuilder *builder, char * text_code){
    memset(builder, 0, sizeof(ShaderBuilder));

    ShaderBuilderInit(builder, SHADER_TYPE_VERTEX);
}