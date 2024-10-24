#include "Objects/terrain_object.h"
#include "Objects/render_texture.h"

#include "Core/e_camera.h"

#include "Tools/e_math.h"
#include "Tools/e_tools.h"
#include "Tools/e_shaders.h"

extern TEngine engine;


void InitTerrain(TerrainObject *to, TerrainParam *tParam){

    vertexParam vParam;
    indexParam iParam;

    vParam.num_verts = tParam->size_patch * tParam->size_patch;
    vParam.vertices = calloc(vParam.num_verts, sizeof(Vertex3D));

    Vertex3D *verts = vParam.vertices;

    const float wx = 2.0f;
    const float wy = 2.0f;

    for(int x=0; x < tParam->size_patch;x++){
        for(int z=0; z < tParam->size_patch;z++){
            uint32_t index = (z + x * tParam->size_patch);
            verts[index].position.x = (x - (float)tParam->size_patch / 2.0f) * tParam->vertex_step;
            verts[index].position.y = tParam->vertex_step;
            verts[index].position.z = (z - (float)tParam->size_patch / 2.0f) * tParam->vertex_step;
            verts[index].normal = vec3_f( 0, 1, 0);
            verts[index].texCoord = v2_muls(vec2_f((float)x / tParam->size_patch, (float)z / tParam->size_patch), 1.0f);
        }
    }

    const uint32_t w = (tParam->size_patch - 1);
    iParam.indexesSize = w * w * 4;
    iParam.indices = (uint32_t *) calloc(iParam.indexesSize, sizeof(uint32_t));

    // Indices
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < w; y++)
        {
            uint32_t index = (y + x * w) * 4;
            iParam.indices[index] = (x + y * tParam->size_patch);
            iParam.indices[index + 1] = iParam.indices[index] + tParam->size_patch;
            iParam.indices[index + 2] = iParam.indices[index + 1] + 1;
            iParam.indices[index + 3] = iParam.indices[index] + 1;
        }
    }
    
    GraphicsObjectSetVertex(&to->go.graphObj, vParam.vertices, vParam.num_verts, sizeof(Vertex3D), iParam.indices, iParam.indexesSize, sizeof(uint32_t));

    free(vParam.vertices);
    free(iParam.indices);
}

void TerrainObjectMakeDefaultParams(TerrainParam *tParam, uint32_t height_map_size)
{
    tParam->size_patch = 64;
    tParam->t_g_param.size_factor = 8;
    tParam->t_g_param.displaisment_factor = 128.0f;
    tParam->t_g_param.tesselation_factor = 0.75f;
    tParam->t_g_param.frequency = 1;
    tParam->t_g_param.amplitude = 1;
    tParam->t_g_param.octaves = 2;
    tParam->vertex_step = 40.0;
    tParam->t_t_param.height_map_scale = height_map_size;
    tParam->t_t_param.texture_scale = 180.0f;
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

    tb.displacementFactor = to->t_param.t_g_param.displaisment_factor;
    tb.tessellationFactor = to->t_param.t_g_param.tesselation_factor;
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
    /*uint32_t size_texture = to->t_param.t_t_param.texture_width * to->t_param.t_t_param.texture_height;

    uint16_t *some_map = buffer; //[size_texture];

    uint16_t temp = 0;

    int num = 2, iter = 0;

    float t_noise = 0;

    float x_del = (float)to->t_param.t_t_param.texture_width / to->width;
    float y_del = (float)to->t_param.t_t_param.texture_height / to->height;

    for(int i = 0;i < to->t_param.t_t_param.texture_width; i++)
    {
        float n_val_x = ((float)i * to->t_shift) / to->t_param.t_g_param.size_factor / x_del;

        for(int j = 0;j < to->t_param.t_t_param.texture_height; j++)
        {
            float n_val_y = ((float)j * to->t_shift) / to->t_param.t_g_param.size_factor / y_del;

            iter = i * to->t_param.t_t_param.texture_width + j;

            if(to->flags & ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS_PERLIN)
                t_noise = PerlinOctave2D(to->t_param.t_g_param.octaves, n_val_x, n_val_y, to->t_param.t_g_param.frequency, to->t_param.t_g_param.amplitude);
            else
                t_noise = SimplexOctave2D(to->t_param.t_g_param.octaves, n_val_x, n_val_y, to->t_param.t_g_param.frequency, to->t_param.t_g_param.amplitude);

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
    uint32_t size_texture = to->t_param.t_t_param.height_map_scale * to->t_param.t_t_param.height_map_scale;

    to->height_map = AllocateMemory(size_texture, sizeof(float));

    float *heightMap = to->height_map;

    uint32_t temp = 0;

    int num = 2, iter = 0;

    float t_noise = 0;

    float x_del = (float)to->t_param.t_t_param.height_map_scale / to->width;
    float y_del = (float)to->t_param.t_t_param.height_map_scale / to->height;

    for(int i = 0;i < to->t_param.t_t_param.height_map_scale; i++)
    {
        float n_val_x = ((float)i * to->t_shift) / to->t_param.t_g_param.size_factor / x_del;

        for(int j = 0;j < to->t_param.t_t_param.height_map_scale; j++)
        {
            float n_val_y = ((float)j * to->t_shift) / to->t_param.t_g_param.size_factor / y_del;

            iter = i * to->t_param.t_t_param.height_map_scale + j;

            if(to->flags & ENGINE_TERRIAN_FLAGS_GENERATE_HEIGHTS_PERLIN)
                t_noise = PerlinOctave2D(to->t_param.t_g_param.octaves, n_val_x, n_val_y, to->t_param.t_g_param.frequency, to->t_param.t_g_param.amplitude);
            else
                t_noise = SimplexOctave2D(to->t_param.t_g_param.octaves, n_val_x, n_val_y, to->t_param.t_g_param.frequency, to->t_param.t_g_param.amplitude);

            heightMap[iter] = t_noise * 2;
        }
    }

    TextureUpdate(descr, heightMap, size_texture * sizeof(float), 0);
}

float TerrainObjectGetHeight(TerrainObject *to, float x, float y)
{
    if(to->height_map == NULL)
        return 0;
    
    float *heightMap = to->height_map;

    int iter = (to->t_param.t_t_param.height_map_scale / 2) + (to->t_param.t_t_param.height_map_scale / 2 * to->t_param.t_t_param.height_map_scale);

    float res = heightMap[iter] * to->t_param.t_g_param.displaisment_factor;

    if(x == 0 || y ==0)
        return res;

    x += to->t_param.size_patch / 2.0f * to->t_param.vertex_step - 1;
    y += to->t_param.size_patch / 2.0f * to->t_param.vertex_step - 1;
    
    float scale = to->t_param.t_t_param.height_map_scale / (float)(to->width * to->t_param.vertex_step);
    uint32_t map_scale = to->t_param.t_t_param.height_map_scale * to->t_param.t_t_param.height_map_scale;

    int x_iter = round(x) * scale, z_iter = round(y) * scale;

    if(x_iter < 0 || x_iter > map_scale - 1  ||
       z_iter < 0 || z_iter > map_scale - 1)
       return 0;
    
    iter = x_iter + z_iter * to->t_param.t_t_param.height_map_scale;

    if(iter >= map_scale)
        return 0;
    
    res = heightMap[iter] * to->t_param.t_g_param.displaisment_factor;

    return res;
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

        g_img.imgWidth = to->t_param.t_t_param.height_map_scale;
        g_img.imgHeight = to->t_param.t_t_param.height_map_scale;
        g_img.flags = TIGOR_TEXTURE_FLAG_R32_FLOAT | TIGOR_TEXTURE_FLAG_SPECIFIC;

        BluePrintDescriptor *descr = BluePrintSetTextureImageCreate(&to->go.graphObj.blueprints, num_pack, &g_img, 3);
        TerrainObjectGenerateTerrainHeightTextureMap(to, descr);
    }        

    uint32_t flags = BluePrintGetSettingsValue(&to->go.graphObj.blueprints, num_pack, 3);
    BluePrintSetSettingsValue(&to->go.graphObj.blueprints, num_pack, 3, flags | TIGOR_PIPELINE_FLAG_TESSELLATION_CONTROL_SHADER | TIGOR_PIPELINE_FLAG_TESSELLATION_EVALUATION_SHADER | TIGOR_PIPELINE_FLAG_FACE_CLOCKWISE);
    
    BluePrintSetSettingsValue(&to->go.graphObj.blueprints, num_pack, 2, VK_CULL_MODE_NONE);
    
    BluePrintSetSettingsValue(&to->go.graphObj.blueprints, num_pack, 1, VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);

    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void TearrainDefaultDestroy(TerrainObject *to)
{
    FreeMemory(to->height_map);

    GameObject3DDestroy(to);

    FreeMemory(to->tesc_shader);
    FreeMemory(to->tese_shader);
}

void TerrainObjectInit(TerrainObject *to, DrawParam *dParam, TerrainParam *tParam)
{
    GameObject3DInit((GameObject3D *)to, TIGOR_GAME_OBJECT_TYPE_3D);
    
    GameObjectSetDestroyFunc((GameObject *)to, (void *)TearrainDefaultDestroy);

    GameObjectSetShaderInitFunc((GameObject *)to, TerrainObjectInitDefaultShader);

    to->t_param = *tParam;
    to->flags = tParam->flags;
    to->width = tParam->size_patch;
    to->height = tParam->size_patch;
    to->t_shift = rand() % UINT16_MAX;
    to->t_shift = to->t_shift / UINT16_MAX;

    InitTerrain(to, tParam);

    GameObject3DInitTextures(to, dParam);

    to->go.graphObj.num_shapes = 1;
}