#include "Objects/terrain_object.h"
#include "Objects/render_texture.h"

#include "Core/e_camera.h"

#include "Tools/e_math.h"
#include "Tools/e_shaders.h"

extern TEngine engine;

void TerrainObjectMakeDefaultParams(TerrainParam *tParam, uint32_t texture_width, uint32_t texture_height, uint32_t height_map_size)
{
    tParam->size_patch = 256;
    tParam->t_g_param.size_factor = 182;
    tParam->t_g_param.height_factor = 12;
    tParam->t_g_param.displaisment_factor = 512.0f;
    tParam->t_g_param.tesselation_factor = 0.75f;
    tParam->t_g_param.frequency = 1;
    tParam->t_g_param.amplitude = 1;
    tParam->t_g_param.octaves = 6;
    tParam->vertex_step = 3.0;
    tParam->t_t_param.height_map_scale = height_map_size;
    tParam->t_t_param.texture_scale = 180.0f;
    tParam->t_t_param.texture_width = texture_width;
    tParam->t_t_param.texture_height = texture_height;
    tParam->t_t_param.num_textures = 0;
    tParam->flags = ENGINE_TERRIAN_FLAGS_REPEATE_TEXTURE | ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS ;//| ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS_PERLIN;
}

enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, BACK = 4, FRONT = 5 };

void TerrainObjectDescriptorTesselationUpdate(TerrainObject *to, void *data)
{
    Camera3D* cam = (Camera3D*) engine.cam3D;
    RenderTexture *render = engine.current_render;

    TesselationBuffer tb;
    memset(&tb, 0, sizeof(TesselationBuffer));

    tb.displacementFactor = to->t_g_param.displaisment_factor;
    tb.tessellationFactor = to->t_g_param.tesselation_factor;
    tb.tessellatedEdgeSize = 20.0f;

    tb.lightPos.y = -0.5f - tb.displacementFactor; // todo: Not uesed yet
    tb.viewportDim = vec2_f((float)engine.width, (float)engine.height);

    mat4 proj = m4_perspective(render->width, render->height, 60.0f, 0.1f, 512.0f);
    mat4 view = m4_look_at(cam->position, v3_add(cam->position, cam->rotation), vec3_f(0.0f,1.0f, 0.0f));

    mat4 matrix = m4_mult(proj, view);

    tb.frustumPlanes[LEFT].x = matrix.m[0][3] + matrix.m[0][0];
    tb.frustumPlanes[LEFT].y = matrix.m[1][3] + matrix.m[1][0];
    tb.frustumPlanes[LEFT].z = matrix.m[2][3] + matrix.m[2][0];
    tb.frustumPlanes[LEFT].w = matrix.m[3][3] + matrix.m[3][0];

    tb.frustumPlanes[RIGHT].x = matrix.m[0][3] - matrix.m[0][0];
    tb.frustumPlanes[RIGHT].y = matrix.m[1][3] - matrix.m[1][0];
    tb.frustumPlanes[RIGHT].z = matrix.m[2][3] - matrix.m[2][0];
    tb.frustumPlanes[RIGHT].w = matrix.m[3][3] - matrix.m[3][0];

    tb.frustumPlanes[TOP].x = matrix.m[0][3] - matrix.m[0][1];
    tb.frustumPlanes[TOP].y = matrix.m[1][3] - matrix.m[1][1];
    tb.frustumPlanes[TOP].z = matrix.m[2][3] - matrix.m[2][1];
    tb.frustumPlanes[TOP].w = matrix.m[3][3] - matrix.m[3][1];

    tb.frustumPlanes[BOTTOM].x = matrix.m[0][3] + matrix.m[0][1];
    tb.frustumPlanes[BOTTOM].y = matrix.m[1][3] + matrix.m[1][1];
    tb.frustumPlanes[BOTTOM].z = matrix.m[2][3] + matrix.m[2][1];
    tb.frustumPlanes[BOTTOM].w = matrix.m[3][3] + matrix.m[3][1];

    tb.frustumPlanes[BACK].x = matrix.m[0][3] + matrix.m[0][2];
    tb.frustumPlanes[BACK].y = matrix.m[1][3] + matrix.m[1][2];
    tb.frustumPlanes[BACK].z = matrix.m[2][3] + matrix.m[2][2];
    tb.frustumPlanes[BACK].w = matrix.m[3][3] + matrix.m[3][2];

    tb.frustumPlanes[FRONT].x = matrix.m[0][3] - matrix.m[0][2];
    tb.frustumPlanes[FRONT].y = matrix.m[1][3] - matrix.m[1][2];
    tb.frustumPlanes[FRONT].z = matrix.m[2][3] - matrix.m[2][2];
    tb.frustumPlanes[FRONT].w = matrix.m[3][3] - matrix.m[3][2];


    for (int i = 0; i < 6; i++)
    {
        tb.frustumPlanes[i] = v4_normalize(tb.frustumPlanes[i]);
    }

    memcpy(data, &tb, sizeof(tb));
}

void TerrainObjectGenerateTerrainTextureMap(TerrainObject *to, void *buffer)
{
    /*uint32_t size_texture = to->t_t_param.texture_width * to->t_t_param.texture_height;

    uint16_t *some_map = buffer; //[size_texture];

    uint16_t temp = 0;

    int num = 2, iter = 0;

    float t_noise = 0;

    float x_del = (float)to->t_t_param.texture_width / to->width;
    float y_del = (float)to->t_t_param.texture_height / to->height;

    for(int i = 0;i < to->t_t_param.texture_width; i++)
    {
        float n_val_x = ((float)i * to->t_shift) / to->t_g_param.size_factor / x_del;

        for(int j = 0;j < to->t_t_param.texture_height; j++)
        {
            float n_val_y = ((float)j * to->t_shift) / to->t_g_param.size_factor / y_del;

            iter = i * to->t_t_param.texture_width + j;

            if(to->flags & ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS_PERLIN)
                t_noise = PerlinOctave2D(to->t_g_param.octaves, n_val_x, n_val_y, to->t_g_param.frequency, to->t_g_param.amplitude);
            else
                t_noise = SimplexOctave2D(to->t_g_param.octaves, n_val_x, n_val_y, to->t_g_param.frequency, to->t_g_param.amplitude);

            if(t_noise >= 0.5f)
                num = 2;
            else if(t_noise >= 0.0f)
                num = 1;
            else if(t_noise <= -0.5f)
                num = 0;
            else
                num = 3;

            some_map[iter] = num;
        }
    }

    TextureUpdate(to->texture_descr, some_map, size_texture * sizeof(uint16_t), 0);*/
}

void TerrainObjectGenerateTerrainHeightTextureMap(TerrainObject *to, BluePrintDescriptor *descr)
{
    uint32_t size_texture = to->t_t_param.height_map_scale * to->t_t_param.height_map_scale;

    to->height_map = calloc(size_texture, sizeof(uint16_t));

    uint16_t *heightMap = to->height_map;

    uint32_t temp = 0;

    int num = 2, iter = 0;

    float t_noise = 0;

    float x_del = (float)to->t_t_param.height_map_scale / to->width;
    float y_del = (float)to->t_t_param.height_map_scale / to->height;

    for(int i = 0;i < to->t_t_param.height_map_scale; i++)
    {
        float n_val_x = ((float)i * to->t_shift) / to->t_g_param.size_factor / x_del;

        for(int j = 0;j < to->t_t_param.height_map_scale; j++)
        {
            float n_val_y = ((float)j * to->t_shift) / to->t_g_param.size_factor / y_del;

            iter = i * to->t_t_param.height_map_scale + j;

            if(to->flags & ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS_PERLIN)
                t_noise = PerlinOctave2D(to->t_g_param.octaves, n_val_x, n_val_y, to->t_g_param.frequency, to->t_g_param.amplitude);
            else
                t_noise = SimplexOctave2D(to->t_g_param.octaves, n_val_x, n_val_y, to->t_g_param.frequency, to->t_g_param.amplitude);

            float s_val = (t_noise + 1.0) / 2;
            uint16_t t_val = UINT16_MAX * s_val;

            heightMap[iter] = -t_val;
        }
    }

    TextureUpdate(descr, heightMap, size_texture * sizeof(uint16_t), 0);
}

void TerrainObjectInitDefaultShader(GameObject3D *go){    
    
    if(go->self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED)
        return;

    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);

    TerrainObject *to = go;
    
    ShaderBuilder *vert = go->self.vert;
    ShaderBuilder *tesc = to->tesc_shader = AllocateMemory(1, sizeof(ShaderBuilder));
    ShaderBuilder *tese = to->tese_shader = AllocateMemory(1, sizeof(ShaderBuilder));
    ShaderBuilder *frag = go->self.frag;

    ShadersMakeTerrainShader(vert, tesc, tese, frag);

    ShaderObject vert_shader, tesc_shader, tese_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&tesc_shader, 0, sizeof(ShaderObject));
    memset(&tese_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    tesc_shader.code = (char *)tesc->code;
    tesc_shader.size = tesc->size * sizeof(uint32_t);
    
    tese_shader.code = (char *)tese->code;
    tese_shader.size = tese->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);

    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &tesc_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &tese_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_shader, num_pack);
    
    GameObject3DSetDescriptorUpdate(go, num_pack, 0, (UpdateDescriptor)GameObject3DDescriptorModelUpdate);
    GameObject3DSetDescriptorUpdate(go, num_pack, 1, (UpdateDescriptor)TerrainObjectDescriptorTesselationUpdate);
    
    GameObject3DSetDescriptorTextureCreate(go, num_pack, 2,  go->diffuses);

    if(go->num_normals > 0)
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 3,  go->normals);
    else{
            
        GameObjectImage g_img;
        memset(&g_img, 0, sizeof(GameObjectImage));

        g_img.imgWidth = to->t_t_param.texture_width;
        g_img.imgHeight = to->t_t_param.texture_height;
        g_img.flags = TIGOR_TEXTURE_FLAG_URGB | TIGOR_TEXTURE_FLAG_SPECIFIC;

        BluePrintDescriptor *descr = BluePrintSetTextureImageCreate(&to->go.graphObj.blueprints, num_pack, &g_img, 3);
        TerrainObjectGenerateTerrainHeightTextureMap(to, descr);
    }        
    
    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void TearrainDefaultDestroy(TerrainObject *to)
{
    free(to->height_map);

    GameObject3DDestroy(to);

    FreeMemory(to->tesc_shader);
    FreeMemory(to->tese_shader);
}

void TerrainObjectInit(TerrainObject *to, DrawParam *dParam, TerrainParam *tParam)
{
    GameObject3DInit((GameObject3D *)to, TIGOR_GAME_OBJECT_TYPE_3D);
    
    GameObjectSetDestroyFunc((GameObject *)to, (void *)TearrainDefaultDestroy);

    GameObjectSetShaderInitFunc((GameObject *)to, TerrainObjectInitDefaultShader);

    to->t_t_param = tParam->t_t_param;
    to->t_g_param = tParam->t_g_param;
    to->flags = tParam->flags;
    to->width = tParam->size_patch;
    to->height = tParam->size_patch;
    to->t_shift = rand() % UINT16_MAX;
    to->t_shift = to->t_shift / UINT16_MAX;

    vertexParam vParam;
    indexParam iParam;

    InitTerrain(&vParam, &iParam, tParam);

    GraphicsObjectSetVertex(&to->go.graphObj, vParam.vertices, vParam.num_verts, sizeof(TerrainVertex), iParam.indices, iParam.indexesSize, sizeof(uint32_t));

    free(vParam.vertices);
    free(iParam.indices);

    if((to->flags & ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS_OLD))
        TerrainObjectGenerateTerrainHeights(to);

    GameObject3DInitTextures(to, dParam);

    to->go.graphObj.num_shapes = 1;
}