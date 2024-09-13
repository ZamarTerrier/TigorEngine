#include <string.h>
#include <stdio.h>

#include "Objects/primitiveObject.h"

#include "Tools/e_shaders.h"
   
ShaderBuilder frag;
ShaderBuilder vert;

ShaderBuilder temp;
ShaderBuilder temp2;

PrimitiveObject po;

int main(){

    memset(&vert, 0, sizeof(ShaderBuilder));
    memset(&frag, 0, sizeof(ShaderBuilder));

    ShadersMakeDeafult3DShaderWithLight(&vert, &frag, false);
    
    ShaderBuilderWriteToFile(&vert, "D:\\Projects\\Temp\\vert.spv");
    ShaderBuilderWriteToFile(&frag, "D:\\Projects\\Temp\\frag.spv");

    return 0;
}