#include "Tools/glTFLoader.h"
#include "Tools/e_math.h"

#include <stdlib.h>

#include <vulkan/vulkan.h>

#include "Objects/models.h"

#include "Core/e_camera.h"
#include "Core/graphicsObject.h"
#include "Core/e_transform.h"
#include "Core/e_buffer.h"
#include "Core/e_descriptor.h"

#include "Objects/gameObject.h"
#include "Objects/gameObject3D.h"

#include "Tools/e_shaders.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

int frameCurr = 0;

void QuatToAngleAxis(vec4 q, float *outAngleDegrees, vec3 *axis) {

  float angleRadians = 2 * acos(q.w);
  if (angleRadians == 0.0) {
    *outAngleDegrees = 0.0;
    axis->x = 0.0;
    axis->y = 0.0;
    axis->z = 1.0;
    return;
  }

  float denom = sqrt(1-q.w*q.w);
  *outAngleDegrees = angleRadians * 180.0 / M_PI;
  axis->x = q.x / denom;
  axis->y = q.y / denom;
  axis->z = q.z / denom;
}

mat4 getLocalTransform(cgltf_node *node){

    cgltf_float matrix[16];

    cgltf_node_transform_local(node, matrix);

    mat4 mat = mat4_colmnsf(
                        matrix[0],  matrix[1],  matrix[2],  matrix[3],
                        matrix[4],  matrix[5],  matrix[6],  matrix[7],
                        matrix[8],  matrix[9],  matrix[10], matrix[11],
                        matrix[12], matrix[13], matrix[14], matrix[15]
                            );

    return mat;
}

mat4 getFullTransform(cgltf_node *node){

    mat4 mat = getLocalTransform(node);

    const cgltf_node* parent = node->parent;

    while (parent)
    {
        mat4 pm = getLocalTransform(parent);

        mat = mat4_mult_transform(mat, pm);

        parent = parent->parent;
    }

    return mat;
}

void readKeyframeValuesV3(engine_gltf_anim_keyframe *keyframes, cgltf_animation_channel *chanel)
{
    vec3 *fdata = chanel->sampler->output->buffer_view->buffer->data + chanel->sampler->output->buffer_view->offset;
    for (size_t i = 0; i < chanel->sampler->output->count; i++)
      keyframes[i].vector3 = fdata[i];
}

void readKeyframeValuesV4(engine_gltf_anim_keyframe *keyframes, cgltf_animation_channel *chanel)
{
    vec4 *fdata = chanel->sampler->output->buffer_view->buffer->data + chanel->sampler->output->buffer_view->offset;
    for (size_t i = 0; i < chanel->sampler->output->count; i++)
        keyframes[i].vector4 = fdata[i];
}

void readKeyframeValues(engine_gltf_anim_channel *a_channel, cgltf_animation_channel *chanel)
{
    if(chanel->sampler->output->type == cgltf_type_vec3)
    {
        a_channel->type = chanel->target_path;
        readKeyframeValuesV3(a_channel->keyframes, chanel);
    }else if(chanel->sampler->output->type == cgltf_type_vec4)
    {
        a_channel->type = chanel->target_path;
        readKeyframeValuesV4(a_channel->keyframes, chanel);
    }
}

void readKeyframeTimes(engine_gltf_anim_channel *a_channel, cgltf_animation_channel *chanel)
{

    float *fdata = chanel->sampler->input->buffer_view->buffer->data + chanel->sampler->input->buffer_view->offset;
    for (size_t i = 0; i < chanel->sampler->input->count; i++)
    {
        a_channel->keyframes[i].time = fdata[i];

        if(a_channel->min_time > fdata[i])
          a_channel->min_time = fdata[i];

        if(a_channel->max_time < fdata[i])
          a_channel->max_time = fdata[i];
    }
}

vec3 getValueV3(engine_gltf_anim_channel *channel, double time)
{
    if(channel->num_keyframes <= 0)
        return (vec3){0,0,0};

    if (time <= channel->keyframes[0].time)
        return channel->keyframes[0].vector3;

    if (time >= channel->keyframes[channel->num_keyframes - 1].time)
        return channel->keyframes[channel->num_keyframes - 1].vector3;

    size_t toFrameIndex = 0;
    for (size_t i = 0; i < channel->num_keyframes; ++i) {
        if (time <= channel->keyframes[i].time) {
            toFrameIndex = i;
            break;
        }
    }

    engine_gltf_anim_keyframe fromFrame = channel->keyframes[toFrameIndex - 1];
    engine_gltf_anim_keyframe toFrame = channel->keyframes[toFrameIndex];
    vec3 val1 = channel->keyframes[toFrameIndex - 1].vector3;
    vec3 val2 = channel->keyframes[toFrameIndex].vector3;
    // time <= toFrame.time, time > fromFrame.time
    uint32_t alpha = (time - fromFrame.time) / (toFrame.time - fromFrame.time);

    return v3_slerp(val1, val2, alpha);
}

vec4 getValueV4(engine_gltf_anim_channel *channel, double time)
{
    if(channel->num_keyframes <= 0)
      return (vec4){0,0,0,0};

    if (time <= channel->keyframes[0].time)
        return channel->keyframes[0].vector4;

    if (time >= channel->keyframes[channel->num_keyframes - 1].time)
        return channel->keyframes[channel->num_keyframes - 1].vector4;

    size_t toFrameIndex = 0;
    for (size_t i = 0; i < channel->num_keyframes; ++i) {
        if (time <= channel->keyframes[i].time) {
            toFrameIndex = i;
            break;
        }
    }

    engine_gltf_anim_keyframe fromFrame = channel->keyframes[toFrameIndex - 1];
    engine_gltf_anim_keyframe toFrame = channel->keyframes[toFrameIndex];
    vec4 val1 = channel->keyframes[toFrameIndex - 1].vector4;
    vec4 val2 = channel->keyframes[toFrameIndex].vector4;
    // time <= toFrame.time, time > fromFrame.time
    uint32_t alpha = (time - fromFrame.time) / (toFrame.time - fromFrame.time);

    return v4_lerp(val1, val2, alpha);
}

void SetupMeshState(glTFStruct *glTF, cgltf_data *model) {

    glTF->num_anims = model->animations_count;
    glTF->animations = AllocateMemory(model->animations_count, sizeof(engine_gltf_anim));

    for(int i=0;i < glTF->num_anims;i++ )
    {
        cgltf_animation* anim_gltf = &model->animations[i];
        engine_gltf_anim *animation = &glTF->animations[i];

        animation->channels = AllocateMemory( anim_gltf->channels_count, sizeof(engine_gltf_anim_channel));
        animation->num_channels = anim_gltf->channels_count;

        for(int j=0; j < anim_gltf->channels_count; j++)
        {
            cgltf_animation_channel *channel = &anim_gltf->channels[j];
            cgltf_animation_sampler *sampler = channel->sampler;

            engine_gltf_anim_channel *a_channel = &animation->channels[j];

            a_channel->keyframes = AllocateMemory(sampler->output->count, sizeof(engine_gltf_anim_keyframe));
            a_channel->num_keyframes = sampler->output->count;

            readKeyframeTimes(a_channel, channel);
            readKeyframeValues(a_channel, channel);

        }
    }

    glTF->num_nodes = model->nodes_count;
    glTF->nodes = AllocateMemory(glTF->num_nodes, sizeof(engine_gltf_node));

    int iter = 0;

    cgltf_node *skin_node = NULL;
    for(int i = 0; i < model->nodes_count; i++)
    {
        cgltf_node *node = &model->nodes[i];
        engine_gltf_node *g_node = &glTF->nodes[i];

        g_node->id_node = i;
        g_node->id_parent = -1;

        int len = strlen(node->name);

        memcpy(g_node->name, node->name, len >= 256 ? 256 : len);

        for(int j=0; j < model->nodes_count;j++)
        {
          if(node->parent == &model->nodes[j])
            g_node->id_parent = j;
        }

        if(node->mesh != NULL)
        {
            g_node->isModel = true;

            cgltf_mesh* mesh = node->mesh;

            g_node->mesh = AllocateMemory(mesh->primitives_count, sizeof(engine_model_mesh *));
            g_node->num_mesh = mesh->primitives_count;

            for(int j=0;j < mesh->primitives_count;j++)
            {
                g_node->mesh[j] = AllocateMemory(1, sizeof(engine_model_mesh));

                engine_model_mesh *g_mesh = g_node->mesh[j];

                if(mesh->primitives[j].type != cgltf_primitive_type_triangles)
                  continue;

                cgltf_primitive *primitive = &mesh->primitives[j];

                if(primitive->material != NULL)
                {
                    cgltf_texture *texture = primitive->material->pbr_metallic_roughness.base_color_texture.texture;

                    if(texture != NULL)
                    {
                        cgltf_image *image = texture->image;

                        g_mesh->image = AllocateMemory(1, sizeof(GameObjectImage));

                        if(image->uri != NULL)
                        {
                            int size_uri = strlen(image->uri);

                            g_mesh->image->path = AllocateMemory( 256, sizeof(char));
                            memset(g_mesh->image->path , 0, 256);

                            if(size_uri > 256){

                                ToolsAddStrings(g_mesh->image->path, 256, glTF->path, texture->name);
                                g_mesh->image->buffer = AllocateMemory(size_uri, sizeof(char));
                                memcpy(g_mesh->image->buffer, image->uri, size_uri);
                                g_mesh->image->size = size_uri;

                            }else{

                                int size = strlen(glTF->path) + strlen(image->uri);
                                ToolsAddStrings(g_mesh->image->path, size, glTF->path, image->uri);
                                g_mesh->image->size = 0;
                                //g_mesh->image->buffer = ToolsLoadImageFromFile(&g_mesh->image->size, buff);

                            }

                        }
                        else if(image->buffer_view->buffer != NULL)
                        {
                            char *name_point = image->name;
                            
                            if(name_point == NULL)
                                name_point = image->buffer_view->name;

                            if(name_point == NULL)
                                name_point = texture->name;
                            

                            int size = strlen(glTF->name) + strlen(name_point);
                            g_mesh->image->path = AllocateMemory( size + 1, sizeof(char));
                            ToolsAddStrings(g_mesh->image->path, size, glTF->name, name_point);
                            g_mesh->image->buffer = AllocateMemory(image->buffer_view->size, sizeof(char));
                            memcpy(g_mesh->image->buffer, image->buffer_view->buffer->data + image->buffer_view->offset, image->buffer_view->size);
                            g_mesh->image->size = image->buffer_view->size;
                        }
                    }

                    texture = primitive->material->normal_texture.texture;

                    if(texture != NULL)
                    {
                        cgltf_image *image = texture->image;

                        g_mesh->normal = AllocateMemory(1, sizeof(GameObjectImage));

                        if(image->uri != NULL)
                        {
                            int size = strlen(glTF->path) + strlen(image->uri);
                            g_mesh->normal->path = AllocateMemory( size + 1, sizeof(char));

                            
                            int size_uri = strlen(image->uri);

                            g_mesh->normal->path = AllocateMemory( 256, sizeof(char));
                            memset(g_mesh->normal->path , 0, 256);

                            if(size_uri > 256){

                                ToolsAddStrings(g_mesh->normal->path, 256, glTF->path, texture->name);
                                g_mesh->normal->buffer = AllocateMemory(size_uri, sizeof(char));
                                memcpy(g_mesh->normal->buffer, image->uri, size_uri);
                                g_mesh->normal->size = size_uri;

                            }else{
                                ToolsAddStrings(g_mesh->normal->path, size, glTF->path, image->uri);
                                g_mesh->normal->size = 0;
                                //g_mesh->image->buffer = ToolsLoadImageFromFile(&g_mesh->image->size, buff);

                            }

                        }
                        else if(image->buffer_view->buffer != NULL)
                        {
                            char *name_point = image->name;

                            if(name_point == NULL)
                                name_point = image->buffer_view->name;

                            if(name_point == NULL)
                                name_point = texture->name;

                            int size = strlen(glTF->name) + strlen(name_point);
                            g_mesh->normal->path = AllocateMemory( size + 1, sizeof(char));
                            ToolsAddStrings(g_mesh->normal->path, size, glTF->name, name_point);
                            g_mesh->normal->buffer = AllocateMemory(image->buffer_view->size, sizeof(char));
                            memcpy(g_mesh->normal->buffer, image->buffer_view->buffer->data + image->buffer_view->offset, image->buffer_view->size);
                            g_mesh->normal->size = image->buffer_view->size;
                            
                        }
                    }

                    texture = primitive->material->emissive_texture.texture;

                    if(texture != NULL)
                    {
                        cgltf_image *image = texture->image;

                        g_mesh->specular = AllocateMemory(1, sizeof(GameObjectImage));

                        if(image->uri != NULL)
                        {
                            int size = strlen(glTF->path) + strlen(image->uri);
                            g_mesh->specular->path = AllocateMemory( size + 1, sizeof(char));

                            
                            int size_uri = strlen(image->uri);

                            g_mesh->specular->path = AllocateMemory( 256, sizeof(char));
                            memset(g_mesh->specular->path , 0, 256);

                            if(size_uri > 256){
                                ToolsAddStrings(g_mesh->specular->path, 256, glTF->path, texture->name);
                                g_mesh->specular->buffer = AllocateMemory(size_uri, sizeof(char));
                                memcpy(g_mesh->specular->buffer, image->uri, size_uri);
                                g_mesh->specular->size = size_uri;

                            }else{
                                ToolsAddStrings(g_mesh->specular->path, size, glTF->path, image->uri);
                                g_mesh->specular->size = 0;
                                //g_mesh->image->buffer = ToolsLoadImageFromFile(&g_mesh->image->size, buff);
                            }

                        }
                        else if(image->buffer_view->buffer != NULL)
                        {

                            char *name_point = image->name;

                            if(name_point == NULL)
                                name_point = image->buffer_view->name;

                            if(name_point == NULL)
                                name_point = texture->name;

                            int size = strlen(glTF->name) + strlen(name_point);
                            g_mesh->specular->path = AllocateMemory( size + 1, sizeof(char));
                            ToolsAddStrings(g_mesh->specular->path, size, glTF->name, name_point);
                            g_mesh->specular->buffer = AllocateMemory(image->buffer_view->size, sizeof(char));
                            memcpy(g_mesh->specular->buffer, image->buffer_view->buffer->data + image->buffer_view->offset, image->buffer_view->size);
                            g_mesh->specular->size = image->buffer_view->size;
                        }
                    }
                }

                g_mesh->num_indices = primitive->indices->count;
                g_mesh->indices = (uint32_t *)AllocateMemory(g_mesh->num_indices, sizeof(uint32_t));

                g_mesh->num_verts = primitive->attributes->data->count;
                g_mesh->verts = (ModelVertex3D *)AllocateMemory(g_mesh->num_verts, sizeof(ModelVertex3D));


                for(int v=0;v < g_mesh->num_verts; v++)
                {
                    g_mesh->verts[v].weight = (vec4){1,1,1,1};
                }

                for(int k=0; k < primitive->attributes_count; k++)
                {
                    cgltf_attribute *attribute = &primitive->attributes[k];

                    vec2* v2_point;
                    vec3* v3_point;
                    vec4* v4_point;
                    vec4_u8 *v4_u8_point;

                    v4_u8_point = attribute->data->buffer_view->buffer->data + attribute->data->buffer_view->offset;
                    v4_point = attribute->data->buffer_view->buffer->data + attribute->data->buffer_view->offset;
                    v3_point = attribute->data->buffer_view->buffer->data + attribute->data->buffer_view->offset;
                    v2_point = attribute->data->buffer_view->buffer->data + attribute->data->buffer_view->offset;

                    vec3 tmp;
                    float f_tmp;

                    for(int v=0;v < g_mesh->num_verts; v++)
                    {
                        switch(attribute->type)
                        {
                            case cgltf_attribute_type_position:
                                tmp = v3_point[v];
                                g_mesh->verts[v].position = v3_point[v];
                                break;
                            case cgltf_attribute_type_normal:
                                g_mesh->verts[v].normal = v3_point[v];
                                break;
                            case cgltf_attribute_type_texcoord:
                                if(g_mesh->verts[v].texCoord.x == 0 && g_mesh->verts[v].texCoord.y ==0)
                                {
                                    g_mesh->verts[v].texCoord.x = v2_point[v].x;
                                    g_mesh->verts[v].texCoord.y = v2_point[v].y;
                                }
                                break;
                            case cgltf_attribute_type_color:
                                g_mesh->verts[v].color = v3_point[v];
                                break;
                            case cgltf_attribute_type_joints:
                                g_mesh->verts[v].joints.x = v4_u8_point[v].x;
                                g_mesh->verts[v].joints.y = v4_u8_point[v].y;
                                g_mesh->verts[v].joints.z = v4_u8_point[v].z;
                                g_mesh->verts[v].joints.w = v4_u8_point[v].w;
                                break;
                            case cgltf_attribute_type_weights:
                                g_mesh->verts[v].weight = v4_point[v];
                                break;
                            default:
                                break;
                        }                        
                    }
                }

                if(primitive->indices->component_type == cgltf_component_type_r_8u)
                {
                    uint8_t *ind_point = primitive->indices->buffer_view->buffer->data + primitive->indices->buffer_view->offset;

                    for(int ind=0; ind < g_mesh->num_indices; ind++)
                    {
                        g_mesh->indices[ind] = ind_point[ind];
                    }

                }
                else if(primitive->indices->component_type == cgltf_component_type_r_16u)
                {
                    uint16_t *ind_point = primitive->indices->buffer_view->buffer->data + primitive->indices->buffer_view->offset;

                    for(int ind=0; ind < g_mesh->num_indices; ind++)
                    {
                        g_mesh->indices[ind] = ind_point[ind];
                    }

                }
                else
                {
                    uint32_t *ind_point = primitive->indices->buffer_view->buffer->data + primitive->indices->buffer_view->offset;

                    for(int ind=0; ind < g_mesh->num_indices; ind++)
                    {
                        g_mesh->indices[ind] = ind_point[ind];
                    }
                }
            }

            iter++;
        }
        else
            g_node->isModel = false;

        for(int j=0;j < model->animations_count;j++ )
        {
            cgltf_animation* anim = &model->animations[j];

            for(int k=0; k < anim->channels_count; k++)
            {
                cgltf_animation_channel *channel = &anim->channels[k];
                if(channel->target_node == node)
                {
                    glTF->animations[j].channels[k].node_id = i;
                }
            }
        }

        if(node->skin != NULL)
        {
            skin_node = node;

            cgltf_accessor *accessor = skin_node->skin->inverse_bind_matrices;       
            mat4 *mat_point = accessor->buffer_view->offset + accessor->buffer_view->buffer->data;      

            for(int j=0;j < skin_node->skin->joints_count;j++)
            {       
                join_mat_struct *j_mat = &glTF->joint_mats[glTF->num_join_mats];   
                uint32_t temp = 0, find = 1;  

                for(int k=0;k < glTF->num_nodes;k++)
                {
                    if(skin_node->skin->joints[j] == &model->nodes[k])
                    {
                        temp = k;
                        break;
                    }
                }       
                
                for(int k=0;k < glTF->num_join_mats;k++){
                    if(glTF->joint_mats[k].id_node == temp)
                        find = 0;
                }

                if(find){
                    j_mat->id_node = temp;   
                    j_mat->inv_mat = mat_point[j];  
                    glTF->num_join_mats ++;
                }
            }

        }
        g_node->local_matrix = getLocalTransform(node);
        g_node->global_matrix = getFullTransform(node);
    }

    for(int i=0;i < glTF->num_join_mats;i++)
    {
        join_mat_struct *j_mat = &glTF->joint_mats[i];
        engine_gltf_node *node = &glTF->nodes[j_mat->id_node];
        j_mat->join_mat = mat4_mult_transform(j_mat->inv_mat, node->global_matrix);
    }

    glTF->num_meshes = iter;

}

void update_frame(ModelObject3D *mo, engine_gltf_anim * animation)
{

  glTFStruct *glTF = mo->obj;

  for(int i=0; i < glTF->num_nodes;i++)
  {
    engine_gltf_node * node = &glTF->nodes[i];

      for(int j=0;j< animation->num_channels;j++)
      {
          engine_gltf_anim_channel *channel = &animation->channels[j];

          if(channel->node_id == node->id_node)
          {
              float duration = channel->keyframes[channel->num_keyframes - 1].time;

              double wrappedTime = fmod(glTF->anim_time, duration);

              vec3 vector3;
              vec4 vector4;

              switch(channel->type)
              {
                case cgltf_animation_path_type_translation:
                    vector3 = getValueV3(channel, wrappedTime);
                    node->local_matrix = m4_translate_mat(node->local_matrix, vector3);
                    break;
                case cgltf_animation_path_type_rotation:
                    vector4 = getValueV4(channel, wrappedTime);
                    node->local_matrix = m4_rotation_mat_quternion(node->local_matrix, vector4);
                    break;
                case cgltf_animation_path_type_scale:
                    vector3 = getValueV3(channel, wrappedTime);
                    node->local_matrix = m4_scale(node->local_matrix, vector3);
                    break;
                default:
                    break;
              }
          }
      }
  }
}

void update_hierarhy(ModelObject3D *mo)
{
    glTFStruct *glTF = mo->obj;

    for(int i=0; i < glTF->num_nodes;i++)
    {
        engine_gltf_node * node = &glTF->nodes[i];

        mat4 mat = node->local_matrix;

        if(node->id_parent != -1)
        {
            engine_gltf_node *parent = &glTF->nodes[node->id_parent];

            while (parent)
            {
                mat4 pm = parent->local_matrix;

                mat = mat4_mult_transform(mat, pm);

                if(parent->id_parent != -1)
                    parent = &glTF->nodes[parent->id_parent];
                else
                    parent = NULL;
            }
        }

        node->global_matrix = mat;
    }

    for(int i=0;i < glTF->num_join_mats;i++)
    {
        join_mat_struct *j_mat = &glTF->joint_mats[i];
        engine_gltf_node *node = &glTF->nodes[j_mat->id_node];
        j_mat->join_mat = mat4_mult_transform(j_mat->inv_mat, node->global_matrix);
    }
}

void Load3DglTFNextFrame(void *ptr, double time, int num_animation)
{
  ModelObject3D *mo = ptr;

  if(mo->obj == NULL)
      return;

  glTFStruct *glTF = mo->obj;

  engine_gltf_anim *anim = glTF->num_anims > 0 ? ( num_animation < glTF->num_anims ? &glTF->animations[num_animation] : NULL) : NULL;

  if (anim != NULL) {
    glTF->anim_time += time;

    update_frame(mo, anim);

    update_hierarhy(mo);
  }

}

void ModelglTFDestroy(ModelObject3D* mo){

    glTFStruct *glTF = mo->obj;

    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            GameObject3D *model = &mo->nodes[i].models[j];
            
            GameObjectDestroy(model);
        }

        FreeMemory(mo->nodes[i].models);
    }
    FreeMemory(mo->nodes);

    for(int i=0; i < glTF->num_anims;i++)
    {
        for(int j=0;j < glTF->animations[i].num_channels;j++)
        {
            FreeMemory(glTF->animations[i].channels[j].keyframes);
        }
        FreeMemory(glTF->animations[i].channels);
    }
    FreeMemory(glTF->animations);

    for(int i=0; i < glTF->num_nodes;i++)
    {
        for(int j=0;j < glTF->nodes[i].num_mesh;j++)
        {
            FreeMemory(glTF->nodes[i].mesh[j]->instance_node_indices);

            engine_model_mesh *mesh = glTF->nodes[i].mesh[j];

            if(mesh->image != NULL){
                FreeMemory(mesh->image);
            }
            
            if(mesh->normal != NULL){
                FreeMemory(mesh->normal);
            }
            
            if(mesh->specular != NULL){
                FreeMemory(mesh->specular);
            }
            
            FreeMemory(mesh->indices);
            FreeMemory(mesh->verts);

            FreeMemory(mesh);
        }
        FreeMemory(glTF->nodes[i].mesh);
    }
    FreeMemory(glTF->nodes);
    FreeMemory(glTF->name);

    FreeMemory(glTF);
}

void ModelglTFnvMatrixBuffer(ModelObject3D* mo, uint32_t indx_node, void *data)
{
    glTFStruct *glTF = mo->obj;

    if(glTF == NULL)
        return;

    InvMatrixsBuffer imb = {};
    memset(&imb, 0, sizeof(InvMatrixsBuffer));

    imb.size = glTF->num_join_mats;
    for(int i=0;i < glTF->num_join_mats;i++)
        imb.mats[i] = glTF->joint_mats[i].join_mat;

    memcpy(data, (char *)&imb, sizeof(imb));
}

void ModelglTFSetDefaultShader(GameObject3D *go)
{    
    if(go->self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED)
        return;

    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);
    
    ShaderBuilder *vert = go->self.vert;
    ShaderBuilder *frag = go->self.frag;

    ShadersMakeDefault3DModelShader(vert, frag, go->num_diffuses > 0);

    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);

    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_shader, num_pack);
    
    GameObject3DSetDescriptorUpdate(go, num_pack, 0, (UpdateDescriptor)ModelModelBufferUpdate);
    GameObject3DSetDescriptorUpdate(go, num_pack, 1, (UpdateDescriptor)ModelglTFnvMatrixBuffer);
    
    if(go->num_diffuses > 1)
        GameObject3DSetDescriptorTextureArrayCreate(go, num_pack, 2, go->diffuses, go->num_diffuses);
    else
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 2, go->num_diffuses > 0 ? go->diffuses : NULL);
        
    if(go->num_normals > 1)
        GameObject3DSetDescriptorTextureArrayCreate(go, num_pack, 3, go->normals, go->num_normals);
    else
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 3, go->num_normals > 0 ? go->normals : NULL);
        
    if(go->num_speculars > 1)
        GameObject3DSetDescriptorTextureArrayCreate(go, num_pack, 4, go->speculars, go->num_speculars);
    else
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 4, go->num_speculars > 0 ? go->speculars : NULL);
    
    //uint32_t flags = BluePrintGetSettingsValue(&go->graphObj.blueprints, 0, 3);
    //BluePrintSetSettingsValue(&go->graphObj.blueprints, 0, 3, flags | TIGOR_PIPELINE_FLAG_FACE_CLOCKWISE);
    
    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void ModelglTFSetDefaultLightShader(GameObject3D *go)
{    
    if(go->self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED)
        return;

    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);
    
    ShaderBuilder *vert = go->self.vert;
    ShaderBuilder *frag = go->self.frag;

    ShadersMakeDeafult3DModelShaderWithLight(vert, frag, go->num_diffuses, go->num_normals, go->num_speculars);

    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);

    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_shader, num_pack);
    
    GameObject3DSetDescriptorUpdate(go, num_pack, 0, (UpdateDescriptor)ModelModelBufferUpdate);
    GameObject3DSetDescriptorUpdate(go, num_pack, 1, (UpdateDescriptor)ModelglTFnvMatrixBuffer);
    GameObject3DSetDescriptorUpdate(go, num_pack, 2, (UpdateDescriptor)ModelLightDescriptorUpdate);
    
    if(go->num_diffuses > 1)
        GameObject3DSetDescriptorTextureArrayCreate(go, num_pack, 3, go->diffuses, go->num_diffuses);
    else if(go->num_diffuses > 0)
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 3, go->diffuses);
    else
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 3, NULL);
        
    if(go->num_normals > 1)
        GameObject3DSetDescriptorTextureArrayCreate(go, num_pack, 4, go->normals, go->num_normals);
    else if(go->num_normals > 0)
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 4, go->normals);
        
    if(go->num_speculars > 1)
        GameObject3DSetDescriptorTextureArrayCreate(go, num_pack, 5, go->speculars, go->num_speculars);
    else if(go->speculars > 0)
        GameObject3DSetDescriptorTextureCreate(go, num_pack, 5, go->speculars);
    
    //uint32_t flags = BluePrintGetSettingsValue(&go->graphObj.blueprints, 0, 3);
    //BluePrintSetSettingsValue(&go->graphObj.blueprints, 0, 3, flags | TIGOR_PIPELINE_FLAG_FACE_CLOCKWISE);
    
    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void Load3DglTFModel(void *model, char *path, DrawParam *dParam){

    ModelObject3D *mo = (ModelObject3D *)model;

    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';
    
    char *full_path = ToolsMakeString(currPath, path);
    
    FreeMemory(currPath);
        
    mo->type = TIGOR_MODEL_TYPE_GLTF;

    Transform3DInit(&mo->transform);

    GameObjectSetInitFunc((GameObject *)mo, (void *)ModelDefaultInit);
    GameObjectSetUpdateFunc((GameObject *)mo, (void *)ModelDefaultUpdate);
    GameObjectSetDrawFunc((GameObject *)mo, (void *)ModelDefaultDraw);
    GameObjectSetCleanFunc((GameObject *)mo, (void *)ModelClean);
    GameObjectSetRecreateFunc((GameObject *)mo, (void *)ModelRecreate);
    
    mo->self.obj_type = TIGOR_GAME_OBJECT_TYPE_3D;
    mo->self.flags = 0;

    mo->obj = AllocateMemory(1, sizeof(glTFStruct));

    glTFStruct *glTF = mo->obj;

    glTF->path = full_path;

    glTF->name = DirectGetFileName(path);

    /*
    switch (type) {
        case 0://(models)
            ToolsAddStrings(ascii, 256, some_file, ".gltf");
            ToolsAddStrings(binary, 256, some_file, ".bin");
            break;
        case 1://(buffers, models)
            ToolsAddStrings(ascii, 256, some_file, ".gltf");
            ToolsAddStrings(binary, 256, some_file, ".gltf");
            break;
        case 2://(textures, buffers, models)
            ToolsAddStrings(ascii, 256, some_file, ".glb");
            ToolsAddStrings(binary, 256, some_file, ".glb");
            break;
        default:
            break;
    }*/
    
    if(!DirectIsFileExist(full_path)){
        GameObjectDestroy(model);
        FreeMemory(glTF->name);
        return;
    }
        
    GameObjectSetDestroyFunc((GameObject *)mo, (void *)ModelglTFDestroy);
  
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    cgltf_result result = cgltf_parse_file(&options, full_path, &data);
    if (result == cgltf_result_success)
    {
        result = cgltf_load_buffers(&options, data, full_path);

        if (result == cgltf_result_success)
        {
            SetupMeshState(glTF, data);

            mo->nodes = (ModelNode *) AllocateMemory(glTF->num_meshes, sizeof(ModelNode));
            mo->num_draw_nodes = glTF->num_meshes;

            int iter = 0;

            for(int i=0; i < glTF->num_nodes;i++)
            {

                engine_gltf_node *node = &glTF->nodes[i];

                if(node->isModel)
                {
                    mo->nodes[iter].id_node = node->id_node;


                    mo->nodes[iter].models = AllocateMemory(node->num_mesh, sizeof(GameObject3D));
                    mo->nodes[iter].num_models = node->num_mesh;

                    for(int j=0;j < node->num_mesh;j++)
                    {
                        GameObject3D *model = &mo->nodes[iter].models[j];
                        engine_model_mesh *mesh = node->mesh[j];

                        GameObject3DInit(model, TIGOR_GAME_OBJECT_TYPE_MODEL);

                        if(mesh->image || mesh->specular || mesh->normal){

                            if(mesh->image){
                                model->diffuses = AllocateMemory(1, sizeof(GameObjectImage));
                                model->diffuses[0] = *mesh->image;
                                model->num_diffuses = 1;
                            }

                            if(mesh->specular){
                                model->normals = AllocateMemory(1, sizeof(GameObjectImage));
                                model->normals[0] = *mesh->specular;
                                model->num_normals = 1;
                            }
                            if(mesh->normal){
                                model->speculars = AllocateMemory(1, sizeof(GameObjectImage));
                                model->speculars[0] = *mesh->normal;
                                model->num_speculars = 1;
                            }
                                
                        }else{
                            GameObject3DInitTextures(model, dParam);
                        }                          

                        model->graphObj.gItems.perspective = true;

                        GraphicsObjectSetVertex(&model->graphObj, mesh->verts, mesh->num_verts, sizeof(ModelVertex3D), mesh->indices, mesh->num_indices, sizeof(uint32_t));
                        

                        GameObjectSetUpdateFunc((GameObject *)model, NULL);

                        GameObjectSetShaderInitFunc((GameObject *)model, ModelglTFSetDefaultLightShader);
                    }

                    iter++;
                }

            }
        }

        FreeMemory(full_path);

        /* TODO make awesome stuff */
        cgltf_free(data);
    }

}