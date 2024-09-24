#include "Tools/fbxLoader.h"
#include "Tools/e_math.h"

#include "Core/e_camera.h"
#include "Core/e_buffer.h"
#include "Core/pipeline.h"
#include "Core/e_texture.h"
#include "Core/e_transform.h"
#include "Core/graphicsObject.h"

#include "ufbx.h"

#include "Objects/gameObject.h"
#include "Objects/render_texture.h"

#include "Tools/e_shaders.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern TEngine engine;

#define MAX_BLEND_SHAPES 128

size_t clamp_sz(size_t a, size_t min_a, size_t max_a) { return e_min(e_max(a, min_a), max_a); }

mat4 ufbx_to_mat4(ufbx_matrix m) {
    return mat4_rowsf(
		(float)m.m00, (float)m.m01, (float)m.m02, (float)m.m03,
		(float)m.m10, (float)m.m11, (float)m.m12, (float)m.m13,
		(float)m.m20, (float)m.m21, (float)m.m22, (float)m.m23,
		0, 0, 0, 1
    );
}

void read_node(engine_fbx_node *vnode, ufbx_node *node)
{
    vnode->self_index = node->typed_id;
    vnode->parent_index = node->parent ? node->parent->typed_id : -1;

    vnode->node_to_parent = ufbx_to_mat4(node->node_to_parent);
    vnode->node_to_world = ufbx_to_mat4(node->node_to_world);
    vnode->geometry_to_node = ufbx_to_mat4(node->geometry_to_node);
    vnode->geometry_to_world = ufbx_to_mat4(node->geometry_to_world);
    vnode->normal_to_world = ufbx_to_mat4(ufbx_matrix_for_normals(&node->geometry_to_world));
}

void read_mesh(engine_model_mesh *vBuffer, ufbx_mesh *mesh, FBXStruct *fbx)
{
    size_t max_parts = 0;
    size_t max_triangles = 0;

    if(mesh->materials.count > 0){
        if(mesh->materials.count > 0){
            vBuffer->image = AllocateMemory(1, sizeof(GameObjectImage));
                        
            vBuffer->image->path = AllocateMemory(mesh->materials.data[0]->pbr.base_color.texture->name.length, sizeof(char));
            memcpy(vBuffer->image->path, mesh->materials.data[0]->pbr.base_color.texture->name.data, sizeof(char) * mesh->materials.data[0]->pbr.base_color.texture->name.length);

            vBuffer->image->buffer = AllocateMemory(mesh->materials.data[0]->pbr.base_color.texture->content.size, sizeof(char));
            memcpy(vBuffer->image->buffer, mesh->materials.data[0]->pbr.base_color.texture->content.data, mesh->materials.data[0]->pbr.base_color.texture->content.size * sizeof(char));
            vBuffer->image->size = mesh->materials.data[0]->pbr.base_color.texture->content.size;
        }
    }

    size_t num_tri_indices = mesh->max_face_triangles * 3;
    uint32_t *tri_indices = AllocateMemory(num_tri_indices, sizeof(uint32_t));

    for (size_t face_ix = 0; face_ix < mesh->num_faces; face_ix++) {
        ufbx_face face = mesh->faces.data[face_ix];
        max_triangles += ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);
    }

    vBuffer->num_indices = vBuffer->num_verts = max_triangles * 3;

    vBuffer->verts = (ModelVertex3D *) AllocateMemory(  vBuffer->num_verts, sizeof(ModelVertex3D));    
    ModelVertex3D *mesh_skin_vertices = (ModelVertex3D *)AllocateMemory( mesh->num_vertices, sizeof(ModelVertex3D));
    vBuffer->indices = (uint32_t *) AllocateMemory( vBuffer->num_indices, sizeof(uint32_t));

    // In FBX files a single mesh can be instanced by multiple nodes. ufbx handles the connection
    // in two ways: (1) `ufbx_node.mesh/light/camera/etc` contains pointer to the data "attribute"
    // that node uses and (2) each element that can be connected to a node contains a list of
    // `ufbx_node*` instances eg. `ufbx_mesh.instances`.
    vBuffer->num_instances = mesh->instances.count;
    vBuffer->instance_node_indices = AllocateMemory(mesh->instances.count, sizeof(int32_t));

    for (size_t i = 0; i < mesh->instances.count; i++) {
        vBuffer->instance_node_indices[i] = (int32_t)mesh->instances.data[i]->typed_id;
    }
	ufbx_skin_deformer *skin = NULL;
            
    if (mesh->skin_deformers.count > 0) {
        skin = mesh->skin_deformers.data[0];   
        
        for(int j=0;j < skin->clusters.count;j++)
        {       
            
            if(fbx->num_join_mats >= MAX_BONES)
                continue;

            join_mat_struct *j_mat = &fbx->joint_mats[fbx->num_join_mats];  
			ufbx_skin_cluster *cluster = skin->clusters.data[j]; 
            uint32_t temp = 0, find = 0; 

            
            for(int i=0;i < fbx->num_join_mats;i++)
            {   
                if(fbx->joint_mats[i].id_node == (int32_t)cluster->bone_node->typed_id)
                    find = 1;
            }   

            if(find)
                continue;

            fbx->joint_mats[fbx->num_join_mats].id_node = (int32_t)cluster->bone_node->typed_id;   
            fbx->joint_mats[fbx->num_join_mats].inv_mat = ufbx_to_mat4(cluster->geometry_to_bone);  
            fbx->num_join_mats ++;
        }

        for (size_t vi = 0; vi < mesh->num_vertices; vi++) {
                ModelVertex3D *vert = &vBuffer->verts[vi];
                
                size_t num_weights = 0;
                float total_weight = 0.0f;
                float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                uint8_t clusters[4] = { 0, 0, 0, 0 };    
                // `ufbx_skin_vertex` contains the offset and number of weights that deform the vertex
                // in a descending weight order so we can pick the first N weights to use and get a
                // reasonable approximation of the skinning.
                ModelVertex3D *skin_vert = &mesh_skin_vertices[vi];
                ufbx_skin_vertex vertex_weights = skin->vertices.data[vi];
                for (size_t wi = 0; wi < vertex_weights.num_weights; wi++) {
                    if (num_weights >= 4) break;
                    ufbx_skin_weight weight = skin->weights.data[vertex_weights.weight_begin + wi]; 
                    // Since we only support a fixed amount of bones up to `MAX_BONES` and we take the
                    // first N ones we need to ignore weights with too high `cluster_index`.
                    if (weight.cluster_index < MAX_BONES) {
                        total_weight += (float)weight.weight;
                        clusters[num_weights] = (uint8_t)weight.cluster_index;
                        weights[num_weights] = (float)weight.weight;
                        num_weights++;
                    }
                }   
                float *joint = (float *)&skin_vert->joints;
                float *weight = (float *)&skin_vert->weight;
                // Normalize and quantize the weights to 8 bits. We need to be a bit careful to make
                // sure the _quantized_ sum is normalized ie. all 8-bit values sum to 255.
                if (total_weight > 0.0f) {

                    uint32_t quantized_sum = 0;
                    for (size_t i = 0; i < 4; i++) {
                        uint8_t quantized_weight = (uint8_t)((float)weights[i] / total_weight * 255.0f);
                        quantized_sum += quantized_weight;
                        joint[i] = clusters[i];
                        weight[i] = quantized_weight;
                    }
                    weight[0] += 255 - quantized_sum;
                }
            }   
    }

    int num_indices = 0;
    
    for (size_t face_ix = 0; face_ix < mesh->num_faces; face_ix++) {
        ufbx_face face = mesh->faces.data[face_ix];
        size_t num_tris = ufbx_triangulate_face(tri_indices, num_tri_indices, mesh, face);

        ufbx_vec2 default_uv = { 0, 0 };

        // Iterate through every vertex of every triangle in the triangulated result
        for (size_t tri_ix = 0; tri_ix < num_tris; tri_ix++) {
            
			for (size_t corner_ix = 0; corner_ix < 3; corner_ix++) {

				uint32_t ix = tri_indices[tri_ix*3 + corner_ix];
                ModelVertex3D *vert = &vBuffer->verts[num_indices];
                uint32_t *indx = vBuffer->indices;

                ufbx_vec3 pos = ufbx_get_vertex_vec3(&mesh->vertex_position, ix);
                ufbx_vec2 uv = mesh->vertex_uv.exists ? ufbx_get_vertex_vec2(&mesh->vertex_uv, ix) : default_uv;
                ufbx_vec3 normal = ufbx_get_vertex_vec3(&mesh->vertex_normal, ix);

                vert->position = vec3_f(pos.x, pos.y, pos.z);
                vert->normal = vec3_f(normal.x, normal.y, normal.z);
                vert->texCoord = vec2_f(uv.x - 0.5f, 1.0f - uv.y);

                // The skinning vertex stream is pre-calculated above so we just need to
                // copy the right one by the vertex index.
                if (skin) {
                    uint32_t s_v = mesh->vertex_indices.data[ix];
                    vert->joints = mesh_skin_vertices[s_v].joints;
                    vert->weight = mesh_skin_vertices[s_v].weight;
                }
                num_indices++;
            }
       }
    }
    
    
	ufbx_vertex_stream streams[2];
	size_t num_streams = 1;

	streams[0].data = vBuffer->verts;
	streams[0].vertex_count = num_indices;
	streams[0].vertex_size = sizeof(ModelVertex3D);

	// Optimize the flat vertex buffer into an indexed one. `ufbx_generate_indices()`
	// compacts the vertex buffer and returns the number of used vertices.
	ufbx_error error;
	size_t num_vertices = ufbx_generate_indices(streams, num_streams, vBuffer->indices, num_indices, NULL, &error);

    FreeMemory(mesh_skin_vertices);
    FreeMemory(tri_indices);
}

void read_blend_channel(engine_fbx_blend_channel *vchan, ufbx_blend_channel *chan)
{
    vchan->weight = (float)chan->weight;
}

void read_node_anim(engine_fbx_anim *va, engine_fbx_node_anim *vna, ufbx_anim_stack *stack, ufbx_node *node)
{
    vna->rot = AllocateMemory(va->num_frames, sizeof(vec4));
    vna->pos = AllocateMemory(va->num_frames, sizeof(vec3));
    vna->scale = AllocateMemory(va->num_frames, sizeof(vec3));

    bool const_rot = true, const_pos = true, const_scale = true;

    // Sample the node's transform evenly for the whole animation stack duration
    for (size_t i = 0; i < va->num_frames; i++) {
        double time = stack->time_begin + (double)i / va->framerate;

        ufbx_transform transform = ufbx_evaluate_transform(stack->anim, node, time);
        vna->pos[i] = vec3_f(transform.translation.x, transform.translation.y, transform.translation.z);
        vna->rot[i] = vec4_f(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
        vna->scale[i] = vec3_f(transform.scale.x, transform.scale.y, transform.scale.z);

        if (i > 0) {
            // Negated quaternions are equivalent, but interpolating between ones of different
            // polarity takes a the longer path, so flip the quaternion if necessary.
            if (v4_dot(vna->rot[i], vna->rot[i - 1]) < 0.0f) {
                vna->rot[i] = v4_neg(vna->rot[i]);
            }

            // Keep track of which channels are constant for the whole animation as an optimization
            if (!v4_equal(vna->rot[i - 1], vna->rot[i])) const_rot = false;
            if (!v3_equal(vna->pos[i - 1], vna->pos[i])) const_pos = false;
            if (!v3_equal(vna->scale[i - 1], vna->scale[i])) const_scale = false;
        }
    }

    if (const_rot) { vna->const_rot = vna->rot[0]; FreeMemory(vna->rot); vna->rot = NULL; }
    if (const_pos) { vna->const_pos = vna->pos[0]; FreeMemory(vna->pos); vna->pos = NULL; }
    if (const_scale) { vna->const_scale = vna->scale[0]; FreeMemory(vna->scale); vna->scale = NULL; }
}

void read_blend_channel_anim(engine_fbx_anim *va, engine_fbx_blend_channel_anim *vbca, ufbx_anim_stack *stack, ufbx_blend_channel *chan)
{
    vbca->weight = AllocateMemory( va->num_frames, sizeof(float));

    bool const_weight = true;

    // Sample the blend weight evenly for the whole animation stack duration
    for (size_t i = 0; i < va->num_frames; i++) {
        double time = stack->time_begin + (double)i / va->framerate;

        ufbx_real weight = ufbx_evaluate_blend_weight(stack->anim, chan, time);
        vbca->weight[i] = (float)weight;

        // Keep track of which channels are constant for the whole animation as an optimization
        if (i > 0) {
            if (vbca->weight[i - 1] != vbca->weight[i]) const_weight = false;
        }
    }

    if (const_weight) { vbca->const_weight = vbca->weight[0]; FreeMemory(vbca->weight); vbca->weight = NULL; }
}

void read_anim_stack(engine_fbx_anim *va, ufbx_anim_stack *stack, ufbx_scene *scene)
{
    const float target_framerate = 30.0f;
    const size_t max_frames = 4096;

    // Sample the animation evenly at `target_framerate` if possible while limiting the maximum
    // number of frames to `max_frames` by potentially dropping FPS.
    float duration = (float)stack->time_end - (float)stack->time_begin;
    size_t num_frames = clamp_sz((size_t)(duration * target_framerate), 2, max_frames);
    float framerate = (float)(num_frames - 1) / duration;

    va->name = AllocateMemory(stack->name.length + 1, sizeof(char));
    memcpy(va->name, stack->name.data, stack->name.length + 1);
    va->time_begin = (float)stack->time_begin;
    va->time_end = (float)stack->time_end;
    va->framerate = framerate;
    va->num_frames = num_frames;

    // Sample the animations of all nodes and blend channels in the stack
    va->nodes = AllocateMemory(scene->nodes.count, sizeof(engine_fbx_node_anim));
    va->num_nodes = scene->nodes.count;
    for (size_t i = 0; i < scene->nodes.count; i++) {
        ufbx_node *node = scene->nodes.data[i];
        read_node_anim(va, &va->nodes[i], stack, node);
    }

    va->blend_channels = AllocateMemory(scene->blend_channels.count, sizeof(engine_fbx_blend_channel_anim));
    va->num_blend_channels = scene->blend_channels.count;
    for (size_t i = 0; i < scene->blend_channels.count; i++) {
        ufbx_blend_channel *chan = scene->blend_channels.data[i];
        read_blend_channel_anim(va, &va->blend_channels[i], stack, chan);
    }
}

void read_scene(ModelObject3D *mo, ufbx_scene *scene)
{
    FBXStruct *fbx = mo->obj;

    fbx->num_nodes = scene->nodes.count;
    fbx->nodes = AllocateMemory(fbx->num_nodes, sizeof(engine_fbx_node));
    for (size_t i = 0; i < fbx->num_nodes; i++) {
        read_node(&fbx->nodes[i], scene->nodes.data[i]);
    }

    fbx->num_meshes = scene->meshes.count;
    fbx->meshes = (engine_model_mesh *)AllocateMemory(fbx->num_meshes, sizeof(engine_model_mesh));
    for (size_t i = 0; i < fbx->num_meshes; i++) {
        read_mesh(&fbx->meshes[i], scene->meshes.data[i], fbx);
    }

    fbx->num_blend_channels = scene->blend_channels.count;
    fbx->blend_channels = AllocateMemory(fbx->num_blend_channels, sizeof(engine_fbx_blend_channel));
    for (size_t i = 0; i < fbx->num_blend_channels; i++) {
        read_blend_channel(&fbx->blend_channels[i], scene->blend_channels.data[i]);
    }

    fbx->num_animations = scene->anim_stacks.count;
    fbx->animations = AllocateMemory(fbx->num_animations, sizeof(engine_fbx_anim));
    for (size_t i = 0; i < fbx->num_animations; i++) {
        read_anim_stack(&fbx->animations[i], scene->anim_stacks.data[i], scene);
    }
}


void update_animation(ModelObject3D *vs, engine_fbx_anim *va, float time)
{
    FBXStruct *fbx = vs->obj;

    float frame_time = (time - va->time_begin) * va->framerate;
    size_t f0 = e_min((size_t)frame_time + 0, va->num_frames - 1);
    size_t f1 = e_min((size_t)frame_time + 1, va->num_frames - 1);
    float t = e_min(frame_time - (float)f0, 1.0f);

    for (size_t i = 0; i < fbx->num_nodes; i++) {
        engine_fbx_node *vn = &fbx->nodes[i];
        engine_fbx_node_anim *vna = &va->nodes[i];

        vec4 rot = vna->rot ? v4_lerp(vna->rot[f0], vna->rot[f1], t) : vna->const_rot;
        vec3 pos = vna->pos ? v3_lerp(vna->pos[f0], vna->pos[f1], t) : vna->const_pos;
        vec3 scale = vna->scale ? v3_lerp(vna->scale[f0], vna->scale[f1], t) : vna->const_scale;

        vn->node_to_parent = m4_transform_quaternion(pos, scale, rot);
    }

    for (size_t i = 0; i < fbx->num_blend_channels; i++) {
        engine_fbx_blend_channel *vbc = &fbx->blend_channels[i];
        engine_fbx_blend_channel_anim *vbca = &va->blend_channels[i];

        vbc->weight = vbca->weight ? lerp(vbca->weight[f0], vbca->weight[f1], t) : vbca->const_weight;
    }
}


void update_hierarchy(ModelObject3D* mo)
{
    FBXStruct *fbx = mo->obj;

    for(int i=0; i < fbx->num_nodes;i++)
    {
        engine_fbx_node *node = &fbx->nodes[i];

        if(node->parent_index != -1)
            node->node_to_world = m4_mult(fbx->nodes[node->parent_index].node_to_world, node->node_to_parent);
        else
            node->node_to_world = node->node_to_parent;

        node->geometry_to_world = m4_mult(node->node_to_world, node->geometry_to_node);
        node->normal_to_world = mat4_transpose(mat4_inverse(node->geometry_to_world));
    }
}

void Load3DFBXNextFrame(ModelObject3D *mo, double time, int num_animation)
{
  FBXStruct *fbx = mo->obj;

  if(fbx != NULL)
  {
      engine_fbx_anim *anim = fbx->num_animations > 0 ? ( num_animation < fbx->num_animations ? &fbx->animations[num_animation] : NULL) : NULL;

      if (anim) {
          fbx->anim_time += time;
          if (fbx->anim_time >= anim->time_end) {
              fbx->anim_time -= anim->time_end - anim->time_begin;
          }
          update_animation(mo, anim, fbx->anim_time);
      }

      update_hierarchy(mo);
  }
}

void ModelFBXDestroy(ModelObject3D *mo){

    FBXStruct *fbx = mo->obj;

    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            GameObjectDestroy(&mo->nodes[i].models[j]);
            
        }
            
        FreeMemory(mo->nodes[i].models);
    }
    FreeMemory(mo->nodes);

    for(int i=0; i < fbx->num_meshes;i++)
    {
        FreeMemory(fbx->meshes[i].image);
        FreeMemory(fbx->meshes[i].verts);
        FreeMemory(fbx->meshes[i].indices);
        FreeMemory(fbx->meshes[i].instance_node_indices);
    }
    FreeMemory(fbx->meshes);
    
    for (size_t i = 0; i < fbx->num_animations; i++) {
        FreeMemory(fbx->animations[i].name);

        for(int j=0;j <  fbx->animations[i].num_nodes;j++){
            if(fbx->animations[i].nodes[j].pos != NULL)
                FreeMemory(fbx->animations[i].nodes[j].pos);
            if(fbx->animations[i].nodes[j].rot != NULL)
                FreeMemory(fbx->animations[i].nodes[j].rot);
            if(fbx->animations[i].nodes[j].scale != NULL)
                FreeMemory(fbx->animations[i].nodes[j].scale);
        }
        
        FreeMemory(fbx->animations[i].nodes);

        for(int j=0;j <  fbx->animations[i].num_blend_channels;j++){
            if(fbx->animations[i].blend_channels[j].weight != NULL)
                FreeMemory(fbx->animations[i].blend_channels[j].weight);
        }

        FreeMemory(fbx->animations[i].blend_channels);
    }

    FreeMemory(fbx->nodes);
    FreeMemory(fbx->animations);

    FreeMemory(fbx->blend_channels);

    FreeMemory(fbx);

}

void ModelFBXInvMatrixBuffer(ModelObject3D* mo, uint32_t indx_node, void *data)
{
    FBXStruct *fbx = mo->obj;

    if(fbx == NULL)
        return;

    InvMatrixsBuffer imb = {};
    memset(&imb, 0, sizeof(InvMatrixsBuffer));
    
	for (size_t i = 0; i < fbx->num_join_mats; i++) {
        engine_fbx_node *node = &fbx->nodes[fbx->joint_mats[i].id_node];
		imb.mats[i] = mat4_mult_transform(fbx->joint_mats[i].inv_mat, node->node_to_world);
	}

    imb.size = fbx->num_join_mats;

    memcpy(data, (char *)&imb, sizeof(imb));
}

void ModelFBXUpdate(ModelObject3D* mo, uint32_t indx_node, void *data){

    Camera3D* cam = (Camera3D*) engine.cam3D;

    FBXStruct *fbx = mo->obj;

    if(fbx == NULL)
        return;

    RenderTexture *render = engine.current_render;

    ModelBuffer3D mbo = {};
    vec3 cameraUp = {0.0f,1.0f, 0.0f};
    
    engine_fbx_node *node = &fbx->nodes[mo->nodes[indx_node].id_node];

    mbo.model = mat4_mult_transform(node->geometry_to_world, m4_transform(mo->transform.position, mo->transform.scale, mo->transform.rotation));
    mbo.view = m4_look_at(cam->position, v3_add(cam->position, cam->rotation), cameraUp);
    mbo.proj = m4_perspective(render->width, render->height, cam->view_angle, cam->view_near, cam->view_distance);
    mbo.proj.m[1][1] *= -1;

    memcpy(data, (char *)&mbo, sizeof(mbo));
}

void ModelFBXSetDefaultShader(GameObject3D *go)
{    
    if(go->self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED)
        return;

    uint32_t num_pack = BluePrintInit(&go->graphObj.blueprints);
    
    ShaderBuilder *vert = go->self.vert;
    ShaderBuilder *frag = go->self.frag;

    ShadersMakeDefault3DModelShader(vert, frag);

    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);

    GraphicsObjectSetShaderWithUniform(&go->graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&go->graphObj, &frag_shader, num_pack);
    
    GameObject3DSetDescriptorUpdate(go, num_pack, 0, (UpdateDescriptor)ModelFBXUpdate);
    GameObject3DSetDescriptorUpdate(go, num_pack, 1, (UpdateDescriptor)ModelFBXInvMatrixBuffer);
    GameObject3DSetDescriptorTextureCreate(go, num_pack, 2, go->num_images > 0 ? &go->images[0] : NULL);
    GameObject3DSetDescriptorTextureCreate(go, num_pack, 3, go->num_images > 0 ? &go->images[1] : NULL);
    GameObject3DSetDescriptorTextureCreate(go, num_pack, 4, go->num_images > 0 ? &go->images[2] : NULL);
    
    /*uint32_t flags = BluePrintGetSettingsValue(&go->graphObj.blueprints, 0, 3);
    BluePrintSetSettingsValue(&go->graphObj.blueprints, 0, 3, flags | TIGOR_PIPELINE_FLAG_FACE_CLOCKWISE);*/
    
    go->self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void Load3DFBXModel(ModelObject3D * mo, char *filepath, DrawParam *dParam)
{
    
    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';
    
    char *full_path = ToolsMakeString(currPath, filepath);
    
    if(!DirectIsFileExist(full_path)){
        FreeMemory(currPath);
        FreeMemory(full_path);
        return;
    }

    mo->type = TIGOR_MODEL_TYPE_FBX;

    Transform3DInit(&mo->transform);

    GameObjectSetInitFunc(mo, (void *)ModelDefaultInit);
    GameObjectSetUpdateFunc(mo, (void *)ModelDefaultUpdate);
    GameObjectSetDrawFunc(mo, (void *)ModelDefaultDraw);
    GameObjectSetCleanFunc(mo, (void *)ModelClean);
    GameObjectSetRecreateFunc(mo, (void *)ModelRecreate);
    GameObjectSetDestroyFunc(mo, (void *)ModelFBXDestroy);

    int vSize = 0, iSize = 0;

    mo->obj = (FBXStruct *) AllocateMemory(1, sizeof(FBXStruct));

    ufbx_error error; // Optional, pass NULL if you don't care about errors
    ufbx_scene *scene = ufbx_load_file(full_path, NULL, &error);
    
    FreeMemory(currPath);
    FreeMemory(full_path);

    if (!scene) {
        fprintf(stderr, "Failed to load: %s\n", error.description.data);
        exit(1);
    }

    read_scene( mo, scene);
    
    FBXStruct *fbx = mo->obj;
    
    mo->nodes = (ModelNode *) AllocateMemory(fbx->num_meshes, sizeof(ModelNode));
    mo->num_draw_nodes = fbx->num_meshes;

    if(mo->num_draw_nodes > 0)
    {
        for(int i=0; i < fbx->num_meshes;i++)
        {

            mo->nodes[i].models = AllocateMemory(1, sizeof(GameObject3D));
            mo->nodes[i].num_models = 1;

            GameObject3DInit(mo->nodes[i].models, TIGOR_GAME_OBJECT_TYPE_MODEL);

            engine_model_mesh *mesh = &fbx->meshes[i];

            GraphicsObjectSetVertex(&mo->nodes[i].models->graphObj, mesh->verts, mesh->num_verts, sizeof(ModelVertex3D), mesh->indices, mesh->num_indices, sizeof(uint32_t));
            
            if(mesh->image != NULL){   
                mo->nodes[i].models->images = AllocateMemory(3, sizeof(GameObjectImage));
                mo->nodes[i].models->images[0] = *mesh->image;

                mo->nodes[i].models->num_images = 1;
            }else{
                GameObject3DInitTextures(mo->nodes[i].models, dParam);
            }

            GameObjectSetUpdateFunc((GameObject *)mo->nodes[i].models, NULL);

            GameObjectSetShaderInitFunc(mo->nodes[i].models, ModelFBXSetDefaultShader);
        }
    } 

	ufbx_free_scene(scene);
}
