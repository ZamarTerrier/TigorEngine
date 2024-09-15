#include "Objects/shape_object.h"

#include <vulkan/vulkan.h>

#include "Core/e_buffer.h"
#include "Core/e_camera.h"
#include "Core/e_blue_print.h"

#include "Tools/e_math.h"
#include "Tools/e_shaders.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern ZEngine engine;

void ShapeObjectCreateQuad(ShapeObject *so, QuadParams *param)
{
    Vertex2D *verts = AllocateMemory(4, sizeof(Vertex2D));

    verts[0].position.x = -param->size / engine.width;
    verts[0].position.y = -param->size / engine.height;
    verts[0].texCoord.x = 0;
    verts[0].texCoord.y = 0;

    verts[1].position.x = param->size / engine.width;
    verts[1].position.y = -param->size / engine.height;
    verts[1].texCoord.x = 1.0f;
    verts[1].texCoord.y = 0;

    verts[2].position.x = param->size / engine.width;
    verts[2].position.y = param->size / engine.height;
    verts[2].texCoord.x = 1.0f;
    verts[2].texCoord.y = 1.0f;

    verts[3].position.x = -param->size / engine.width;
    verts[3].position.y = param->size / engine.height;
    verts[3].texCoord.x = 0;
    verts[3].texCoord.y = 1.0f;

    for(int i=0;i < 4;i++)
    {
        verts[i].color = param->color;
    }

    uint32_t *tIndx = AllocateMemory(6, sizeof(uint32_t));

    uint32_t indx[] = {
      0, 1, 2, 2, 3, 0
    };

    memcpy(tIndx, indx, 6 * sizeof(uint32_t));

    GraphicsObjectSetVertex(&so->go.graphObj, verts, 4, sizeof(Vertex2D), tIndx, 6, sizeof(uint32_t));
    
    FreeMemory(verts);
    FreeMemory(tIndx);
}

void ShapeObjectCreateCircle(ShapeObject *so, CircleParams *param)
{
    int freq = param->freq;

    float stackAngle, sectorAngle;

    if(freq < 3)
        freq = 3;


    uint32_t num_verts = (freq + 1);
    Vertex2D *verts = AllocateMemory( num_verts, sizeof(Vertex2D));

    float angInc = 2 * ((float)M_PI / freq);

    verts[0].position.x = 0;
    verts[0].position.y = 0;

    verts[0].texCoord.x = 0.5f;
    verts[0].texCoord.y = 0.5f;

    verts[0].color = param->color;

    float divW = param->radius / engine.width;
    float divH = param->radius / engine.height;

    for(int i=1;i <= freq;i++)
    {
        Vertex2D *vert = &verts[i];

        float x = cos(i * angInc);
        float y = sin(i * angInc);

        vert->position.x = x * divW;
        vert->position.y = y * divH;

        vert->texCoord.x = (float)(x + 1.0f) / 2;
        vert->texCoord.y = (float)(y + 1.0f) / 2;

        vert->color = param->color;
    }

    uint32_t indx_size = (freq + 1) * 3;
    uint32_t *indx = AllocateMemory( indx_size, sizeof(uint32_t));

    int k0 = 0, k1 = 1, k2 = 2, iter = 0;
    for(int i = 0; i <= freq; ++i)
    {
        indx[iter] = k0;
        indx[iter + 1] = k1;
        indx[iter + 2] = k2;

        k1 ++;
        k2 ++;

        if(k2 == num_verts)
            k2 = 1;

        iter +=3;
    }

    GraphicsObjectSetVertex(&so->go.graphObj, verts, num_verts, sizeof(Vertex2D), indx, indx_size, sizeof(uint32_t));
    
    FreeMemory(verts);
    FreeMemory(indx);
}

void ShapeObjectCreateShape(ShapeObject *so, ShapeParams *param)
{

    if(param->num_positions <= 2)
        return;

    uint32_t num_verts = param->num_positions + 1;
    Vertex2D *verts = AllocateMemory( num_verts, sizeof(Vertex2D));

    vec2 center = {0, 0};

    vec2 w_size = { engine.width, engine.height };

    vec2 min = param->positions[0];
    vec2 max = param->positions[0];
    for(int i=1; i < param->num_positions; i++)
    {
        if(min.x > param->positions[i].x)
            min.x = param->positions[i].x;
        else if(min.y > param->positions[i].y)
            min.y = param->positions[i].y;

        if(max.x < param->positions[i].x)
            max.x = param->positions[i].x;
        else if(max.y < param->positions[i].y)
            max.y = param->positions[i].y;
    }

    min = v2_div(min, w_size);
    max = v2_div(max, w_size);

    for(int i=0; i < param->num_positions; i++)
    {
        Vertex2D *vert = &verts[i + 1];

        vert->position = v2_div(param->positions[i], w_size);
        vert->texCoord.x = vert->position.x / max.x + min.x;
        vert->texCoord.y = vert->position.y / max.y + min.y;
        vert->color = param->color;

        center = v2_add(center, param->positions[i]);
    }

    center = v2_divs(center, param->num_positions);

    verts[0].position = v2_div(center, w_size);
    verts[0].texCoord.x = 0.5f;
    verts[0].texCoord.y = 0.5f;
    verts[0].color = param->color;


    uint32_t indx_size = (param->num_positions + 1) * 3;
    uint32_t *indx = AllocateMemory( indx_size, sizeof(uint32_t));

    int k0 = 0, k1 = 1, k2 = 2, iter = 0;
    for(int i = 0; i < param->num_positions; ++i)
    {
        indx[iter] = k0;
        indx[iter + 1] = k1;
        indx[iter + 2] = k2;

        k1 ++;
        k2 ++;

        if(k2 == num_verts)
            k2 = 1;

        iter +=3;
    }

    GraphicsObjectSetVertex(&so->go.graphObj, verts, num_verts, sizeof(Vertex2D), indx, indx_size, sizeof(uint32_t));
    
    FreeMemory(verts);
    FreeMemory(indx);
}

void ShapeObjectCreateLine(ShapeObject *so, LineParams *line)
{
    Vertex2D *verts = AllocateMemory(2, sizeof(Vertex2D));


    vec2 position;

    verts[0].position.x = line->position.x / engine.width;
    position.x = verts[0].position.x + (line->direction.x * (line->length / engine.width));

    verts[0].position.y = line->position.y / engine.height;
    position.y = verts[0].position.y + (line->direction.y * (line->length / engine.height));

    verts[0].color = line->color;

    verts[1].position = position;
    verts[1].color = line->color;


    GraphicsObjectSetVertex(&so->go.graphObj, verts, 2, sizeof(Vertex2D), NULL, 0, sizeof(uint32_t));
    
    FreeMemory(verts);
}

int ShapeObjectInit(ShapeObject *so, DrawParam *dParam, ShapeType type, void *param)
{
    char *name = "Shape_Object";

    memcpy(so->go.name, name, strlen(name));

    GameObject2DInit((GameObject2D *)so, ENGINE_GAME_OBJECT_TYPE_2D);

    switch(type)
    {
        case ENGINE_SHAPE_OBJECT_LINE:
            ShapeObjectCreateLine(so, param);
            break;
        case ENGINE_SHAPE_OBJECT_QUAD:
            ShapeObjectCreateQuad(so, param);
            break;
        case ENGINE_SHAPE_OBJECT_CIRCLE:
            ShapeObjectCreateCircle(so, param);
            break;
        case ENGINE_SHAPE_OBJECT_SHAPE:
            ShapeObjectCreateShape(so, param);
            break;
        default:
            printf("Error Shape Type!\n");
            break;
    }

    so->type = type;

    so->go.image = AllocateMemory(1, sizeof(GameObjectImage));

    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';

    if(dParam->diffuse != NULL){
        if(strlen(dParam->diffuse) != 0)
        {
            char *full_path = ToolsMakeString(currPath, dParam->diffuse);
            
            if(!DirectIsFileExist(full_path)){
                GameObjectDestroy((GameObject *)so);
                FreeMemory(full_path);            
                FreeMemory(currPath);
                return 0;
            }

            int len = strlen(full_path);
            so->go.image->path = AllocateMemory(len + 1, sizeof(char));
            memcpy(so->go.image->path, full_path, len);
            so->go.image->path[len] = '\0';
            //go->image->buffer = ToolsLoadImageFromFile(&go->image->size, dParam.filePath);

            so->go.num_images++;

            FreeMemory(full_path);
        }
    }    

    FreeMemory(currPath);

    so->go.graphObj.num_shapes = 1;

    return 1;
}
