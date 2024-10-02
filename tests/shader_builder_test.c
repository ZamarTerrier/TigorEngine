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

    ShadersMakeClear2DShader(&vert, &frag);
    
    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    
    char *full_path_vert = ToolsMakeString(currPath, "\\vert.spv");
    char *full_path_frag = ToolsMakeString(currPath, "\\frag.spv");
    
    
    ShaderBuilderWriteToFile(&vert, full_path_vert);
    ShaderBuilderWriteToFile(&frag, full_path_frag);

    FreeMemory(currPath);
    FreeMemory(full_path_vert);
    FreeMemory(full_path_frag);

    return 0;
}