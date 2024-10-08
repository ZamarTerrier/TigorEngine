#include "Tools/objLoader.h"
#include "Tools/e_math.h"
#include "Tools/e_shaders.h"

#include "Core/e_camera.h"

#include "Objects/render_texture.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader.h"

extern TEngine engine;

typedef struct{
    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes;
    tinyobj_material_t* materials;
    uint64_t num_shapes;
    uint64_t num_materials;
    char* warn;
    char* err;
} OBJStruct;

static void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
  float v10[3];
  float v20[3];
  float len2;

  v10[0] = v1[0] - v0[0];
  v10[1] = v1[1] - v0[1];
  v10[2] = v1[2] - v0[2];

  v20[0] = v2[0] - v0[0];
  v20[1] = v2[1] - v0[1];
  v20[2] = v2[2] - v0[2];

  N[0] = v20[1] * v10[2] - v20[2] * v10[1];
  N[1] = v20[2] * v10[0] - v20[0] * v10[2];
  N[2] = v20[0] * v10[1] - v20[1] * v10[0];

  len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
  if (len2 > 0.0f) {
    float len = (float)sqrt((double)len2);

    N[0] /= len;
    N[1] /= len;
  }
}

void ParseSomeStruct(ModelObject3D *mo, Vertex3D *vertexs){

    tinyobj_material_t* materials = NULL;
    size_t face_offset = 0;

    OBJStruct *obj = mo->obj;

    for(int i=0; i < obj->attrib.num_face_num_verts;i++){

        assert(obj->attrib.face_num_verts[i] % 3 ==
               0); /* assume all triangle faces. */

        for (int f = 0; f < (size_t)obj->attrib.face_num_verts[i] / 3; f++) {
              size_t k;
              float v[3][3];
              float n[3][3];
              float c[3];
              float uv[3][2];
              float len2;

              tinyobj_vertex_index_t idx0 = obj->attrib.faces[face_offset + 3 * f + 0];
              tinyobj_vertex_index_t idx1 = obj->attrib.faces[face_offset + 3 * f + 1];
              tinyobj_vertex_index_t idx2 = obj->attrib.faces[face_offset + 3 * f + 2];

              for (k = 0; k < 3; k++) {
                int f0 = idx0.v_idx;
                int f1 = idx1.v_idx;
                int f2 = idx2.v_idx;
                assert(f0 >= 0);
                assert(f1 >= 0);
                assert(f2 >= 0);

                v[0][k] = obj->attrib.vertices[3 * (size_t)f0 + k];
                v[1][k] = obj->attrib.vertices[3 * (size_t)f1 + k];
                v[2][k] = obj->attrib.vertices[3 * (size_t)f2 + k];
              }

              {
              int f0 = idx0.vt_idx;
              int f1 = idx1.vt_idx;
              int f2 = idx2.vt_idx;
              assert(f0 >= 0);
              assert(f1 >= 0);
              assert(f2 >= 0);

              assert(f0 < obj->attrib.num_texcoords);
              assert(f1 < obj->attrib.num_texcoords);
              assert(f2 < obj->attrib.num_texcoords);

              uv[0][0] = obj->attrib.texcoords[2 * (size_t)f0 + 0];
              uv[1][0] = obj->attrib.texcoords[2 * (size_t)f1 + 0];
              uv[2][0] = obj->attrib.texcoords[2 * (size_t)f2 + 0];

              uv[0][1] = 1.0f - obj->attrib.texcoords[2 * (size_t)f0 + 1];
              uv[1][1] = 1.0f - obj->attrib.texcoords[2 * (size_t)f1 + 1];
              uv[2][1] = 1.0f - obj->attrib.texcoords[2 * (size_t)f2 + 1];
              }


              if (obj->attrib.num_normals > 0) {
                int f0 = idx0.vn_idx;
                int f1 = idx1.vn_idx;
                int f2 = idx2.vn_idx;
                if (f0 >= 0 && f1 >= 0 && f2 >= 0) {
                  assert(f0 < (int)obj->attrib.num_normals);
                  assert(f1 < (int)obj->attrib.num_normals);
                  assert(f2 < (int)obj->attrib.num_normals);
                  for (k = 0; k < 3; k++) {
                    n[0][k] = obj->attrib.normals[3 * (size_t)f0 + k];
                    n[1][k] = obj->attrib.normals[3 * (size_t)f1 + k];
                    n[2][k] = obj->attrib.normals[3 * (size_t)f2 + k];
                  }
                } else { /* normal index is not defined for this face */
                  /* compute geometric normal */
                  CalcNormal(n[0], v[0], v[1], v[2]);
                  n[1][0] = n[0][0];
                  n[1][1] = n[0][1];
                  n[1][2] = n[0][2];
                  n[2][0] = n[0][0];
                  n[2][1] = n[0][1];
                  n[2][2] = n[0][2];
                }
              } else {
                /* compute geometric normal */
                CalcNormal(n[0], v[0], v[1], v[2]);
                n[1][0] = n[0][0];
                n[1][1] = n[0][1];
                n[1][2] = n[0][2];
                n[2][0] = n[0][0];
                n[2][1] = n[0][1];
                n[2][2] = n[0][2];
              }

              for (k = 0; k < 3; k++) {
                vertexs[3 * i + k].position.x = v[k][0];
                vertexs[3 * i + k].position.y = v[k][1];
                vertexs[3 * i + k].position.z = v[k][2];
                vertexs[3 * i + k].texCoord.x = uv[k][0];
                vertexs[3 * i + k].texCoord.y = uv[k][1];
                vertexs[3 * i + k].normal.x = n[k][0];
                vertexs[3 * i + k].normal.y = n[k][1];
                vertexs[3 * i + k].normal.z = n[k][2];

                /*if (obj->attrib.material_ids[i] >= 0) {
                  int matidx = obj->attrib.material_ids[i];
                  vertexs[3 * i + k].color.x = materials[matidx].diffuse[0];
                  vertexs[3 * i + k].color.y = materials[matidx].diffuse[1];
                  vertexs[3 * i + k].color.z = materials[matidx].diffuse[2];
                }else {
                    vertexs[3 * i + k].color.x = n[k][0];
                    vertexs[3 * i + k].color.y = n[k][1];
                    vertexs[3 * i + k].color.z = n[k][2];
                }*/


                /**/

                /*
                c[0] = n[k][0];
                c[1] = n[k][1];
                c[2] = n[k][2];
                len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
                if (len2 > 0.0f) {
                  float len = (float)sqrt((double)len2);

                  c[0] /= len;
                  c[1] /= len;
                  c[2] /= len;
                }

                vb[(3 * i + k) * stride + 6] = (c[0] * 0.5f + 0.5f);
                vb[(3 * i + k) * stride + 7] = (c[1] * 0.5f + 0.5f);
                vb[(3 * i + k) * stride + 8] = (c[2] * 0.5f + 0.5f);

                if (attrib.material_ids[i] >= 0) {
                      int matidx = attrib.material_ids[i];
                      vb[(3 * i + k) * stride + 9] = materials[matidx].diffuse[0];
                      vb[(3 * i + k) * stride + 10] = materials[matidx].diffuse[1];
                      vb[(3 * i + k) * stride + 11] = materials[matidx].diffuse[2];
                } else {
                      vb[(3 * i + k) * stride + 9] = vb[(3 * i + k) * stride + 6];
                      vb[(3 * i + k) * stride + 10] = vb[(3 * i + k) * stride + 7];
                      vb[(3 * i + k) * stride + 11] = vb[(3 * i + k) * stride + 8];
                }*/
              }
        }
        face_offset += (size_t)obj->attrib.face_num_verts[i];
    }

    return;
}

char* mmap_file(size_t* len, const char* filename) {

    FILE* fd;
    int size;

    fd = fopen(filename, "r");
    if (fd == NULL) {
        printf("File Not Found!\n");
        return 0;
    }


    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);

    char *buff = (char *)AllocateMemory(size, sizeof(char));

    fseek(fd, 0L, SEEK_SET);

    fread(buff, sizeof(char), size, fd);

    fclose(fd);

    *len = size;

    return buff;
}

static void get_file_data(void* ctx, const char* filename, const int is_mtl, const char* obj_filename, char** data, size_t* len) {
  // NOTE: If you allocate the buffer with malloc(),
  // You can define your own memory management struct and pass it through `ctx`
  // to store the pointer and free memories at clean up stage(when you quit an
  // app)
  // This example uses mmap(), so no free() required.
  (void)ctx;

  if (!filename) {
    fprintf(stderr, "null filename\n");
    (*data) = NULL;
    (*len) = 0;
    return;
  }

  size_t data_len = 0;

  *data = mmap_file(&data_len, filename);
  (*len) = data_len;
}

void ModelObjUpdate(ModelObject3D* mo, uint32_t indx_node, void *data){

    Camera3D* cam = (Camera3D*) engine.cam3D;

    RenderTexture *render = engine.current_render;

    TransformBuffer mbo;
    vec3 cameraUp = {0.0f,1.0f, 0.0f};

    mbo.model = m4_transform(mo->transform.position, mo->transform.scale, mo->transform.rotation);
    mbo.view = m4_look_at(cam->position, v3_add(cam->position, cam->rotation), cameraUp);
    mbo.proj = m4_perspective(render->width, render->height, cam->view_angle, cam->view_near, cam->view_distance);
    mbo.proj.m[1][1] *= -1;

    memcpy(data, (char *)&mbo, sizeof(mbo));
}

void ModelObjSetDefaultShader(GameObject3D *go)
{    
    if(go->self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED)
        return;

    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);
    
    ShaderBuilder *vert = go->self.vert;
    ShaderBuilder *frag = go->self.frag;

    ShadersMakeDefault3DShader(vert, frag, go->num_diffuses > 0);

    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);

    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_shader, num_pack);
    
    GameObject3DSetDescriptorUpdate(go, num_pack, 0, (UpdateDescriptor)ModelObjUpdate);

    if(go->num_diffuses > 1)
        GameObject3DSetDescriptorTextureArrayCreate(go, num_pack, 1, go->diffuses, go->num_diffuses);
    else
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 1, go->num_diffuses > 0 ? go->diffuses : NULL);
        
    
    uint32_t flags = BluePrintGetSettingsValue(&go->graphObj.blueprints, 0, 3);
    BluePrintSetSettingsValue(&go->graphObj.blueprints, 0, 3, flags | TIGOR_PIPELINE_FLAG_FACE_CLOCKWISE);
    
    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void DestroyOBJModel(ModelObject3D *mo){

    OBJStruct *obj = mo->obj;

    tinyobj_attrib_free(&obj->attrib);
    tinyobj_shapes_free(obj->shapes,  obj->num_shapes);
    tinyobj_materials_free(obj->materials, obj->num_materials);
    FreeMemory(mo->obj);

    GameObjectDestroy(&mo->nodes[0].models[0]);
    FreeMemory(mo->nodes[0].models);
    FreeMemory(mo->nodes);
}

void Load3DObjModel(ModelObject3D * mo, char *filepath, DrawParam *dParam){

  char *currPath = DirectGetCurrectFilePath();
  int len = strlen(currPath);
  currPath[len] = '\\';
    
  char *full_path = ToolsMakeString(currPath, filepath);

  if(!DirectIsFileExist(full_path)){
      FreeMemory(full_path);              
      FreeMemory(currPath);
      return;
  }

  mo->type = TIGOR_MODEL_TYPE_OBJ;

  Transform3DInit(&mo->transform);

  GameObjectSetInitFunc((GameObject *)mo, (void *)ModelDefaultInit);
  GameObjectSetUpdateFunc((GameObject *)mo, (void *)ModelDefaultUpdate);
  GameObjectSetDrawFunc((GameObject *)mo, (void *)ModelDefaultDraw);
  GameObjectSetCleanFunc((GameObject *)mo, (void *)ModelClean);
  GameObjectSetRecreateFunc((GameObject *)mo, (void *)ModelRecreate);
  GameObjectSetDestroyFunc((GameObject *)mo, (void *)DestroyOBJModel);

  mo->obj = (OBJStruct *) AllocateMemory(1, sizeof(OBJStruct));

  OBJStruct *obj = mo->obj;

  unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
  tinyobj_parse_obj(&obj->attrib, &obj->shapes, &obj->num_materials, &obj->materials, &obj->num_materials, full_path, (void *)get_file_data, NULL, flags);

  mo->nodes = AllocateMemory(1, sizeof(ModelNode));
  mo->num_draw_nodes = 1;

  mo->nodes[0].models = AllocateMemory(1, sizeof(GameObject3D));
  mo->nodes[0].num_models = 1;

  GameObject3D *model = &mo->nodes[0].models[0];
  
  GameObject3DInit(model, TIGOR_GAME_OBJECT_TYPE_3D);
  
  GameObjectSetUpdateFunc((GameObject *)model, NULL);
  
  GameObjectSetShaderInitFunc(model, ModelObjSetDefaultShader);

  model->graphObj.gItems.perspective = true;

  Vertex3D *vertices = (Vertex3D *) AllocateMemory(obj->attrib.num_face_num_verts * 3, sizeof(Vertex3D));
  uint32_t *indices = (uint32_t *) AllocateMemory(obj->attrib.num_face_num_verts * 3, sizeof(uint32_t));

  ParseSomeStruct(mo, vertices );

  uint32_t num_verts = obj->attrib.num_face_num_verts * 3;
  uint32_t indexesSize = obj->attrib.num_face_num_verts * 3;

  for(int i=0; i < indexesSize;i++)
      indices[i] = i;
  
  GraphicsObjectSetVertex(&model->graphObj, vertices, num_verts, sizeof(Vertex3D), indices, indexesSize, sizeof(uint32_t));

  FreeMemory(vertices);
  FreeMemory(indices);

  GameObject3DInitTextures(model, dParam);
  
  FreeMemory(full_path);              
  FreeMemory(currPath);
}


