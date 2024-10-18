#include <string.h>
#include <stdio.h>

#include "Objects/primitiveObject.h"

#include "Tools/e_shaders.h"
   
ShaderBuilder frag;
ShaderBuilder tesc;
ShaderBuilder tese;
ShaderBuilder vert;

ShaderBuilder temp;
ShaderBuilder temp2;

PrimitiveObject po;

void WriteSomeShader(ShaderBuilder *shader, char *path){

    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    
    char *full_path = ToolsMakeString(currPath, path);
    
    ShaderBuilderWriteToFile(shader, full_path);
    
    FreeMemory(currPath);
    FreeMemory(full_path);
}

int main(){

    memset(&vert, 0, sizeof(ShaderBuilder));
    memset(&tesc, 0, sizeof(ShaderBuilder));
    memset(&tese, 0, sizeof(ShaderBuilder));
    memset(&frag, 0, sizeof(ShaderBuilder));

    ShadersMakeTerrainShader(&vert, &tesc, &tese, &frag);
    
    WriteSomeShader(&vert, "\\vert.spv");
    WriteSomeShader(&tesc, "\\tesc.spv");
    WriteSomeShader(&tese, "\\tese.spv");
    WriteSomeShader(&frag, "\\frag.spv");

    return 0;
}