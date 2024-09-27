#include "Objects/particleSystem3D.h"

#include "TigorEngine.h"

#include <vulkan/vulkan.h>

#include "Objects/render_texture.h"

#include "Core/e_memory.h"
#include "Core/e_camera.h"
#include "Core/e_device.h"
#include "Core/pipeline.h"
#include "Core/e_buffer.h"
#include "Core/e_texture.h"
#include "Core/e_transform.h"
#include "Core/graphicsObject.h"

#include "Tools/e_math.h"
#include "Tools/e_tools.h"
#include "Tools/e_shaders.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern TEngine engine;

#define MAX_PARTICLES_COUNT 1024

Particle3D *Particle3DAddObject(ParticleObject3D * particle){

    if(particle->part_header == NULL)
        particle->part_header = AllocateMemory(1, sizeof(ChildStack));

    Particle3D *object = AllocateMemory(1, sizeof(Particle3D));

    if(particle->part_header->node == NULL){
        particle->part_header->next = AllocateMemory(1, sizeof(ChildStack));
        particle->part_header->node = object;
    }
    else{

        ChildStack *child = particle->part_header;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = AllocateMemory(1, sizeof(ChildStack));
        child->node = object;
    }

    return object;
}

uint32_t Patricle3DObjCount(ParticleObject3D *particle){
    ChildStack *child = particle->part_header;

    uint32_t counter = 0;

    if(child == NULL)
        return counter;
    
    while(child->next != NULL){
        counter++;

        child = child->next;
    }

    return counter;   
}

void Particle3DDesctroyObj(ParticleObject3D *particle, Particle3D *part){
    if(part == NULL)
        return;

    TDevice *device = (TDevice *)engine.device;

    Particle3D *curr = NULL;

    ChildStack *child = particle->part_header;
    ChildStack *before = NULL;

    while(child != NULL)
    {
        curr = child->node;

        if(curr == part)
            break;

        before = child;
        child = child->next;
    }

    if(curr == NULL){
        printf("Can't find this memory 0x%x\n", part);
        return;
    }

    if(child->next != NULL){
        FreeMemory(child->node);

        if(before != NULL)
            before->next = child->next;
        else
            particle->part_header = child->next;

        FreeMemory(child);
        child = NULL;

    }else{
        
        if(before != NULL){
            FreeMemory(child->node);
            FreeMemory(child);
            child = NULL;  
        } 
    }
}

void Particle3DCopyToBuffer(ParticleObject3D *particle){

    TDevice *device = (TDevice *)engine.device;

    ChildStack *child = particle->part_header;
    
    Particle3D *obj = NULL;

    uint32_t counter = 0;     
    
    uint32_t num_part = Patricle3DObjCount(particle);   
    
    char *data;
    vkMapMemory(device->e_device, particle->go.graphObj.shapes[0].vParam.buffer.memory, 0, sizeof(ParticleVertex3D) * MAX_PARTICLES_COUNT, 0, (void **)&data);

    memset(data, 0, sizeof(ParticleVertex3D) * MAX_PARTICLES_COUNT);

    ParticleVertex3D temp;

    while(child != NULL){
        if(counter >= MAX_PARTICLES_COUNT)
            break;

        obj = child->node;

        if(obj != NULL){

            temp.position = obj->position;
            temp.size = obj->scale;
            temp.color = obj->color;
            
            memcpy(data, &temp, (size_t) sizeof(ParticleVertex3D));

            data += sizeof(ParticleVertex3D);
            counter ++;
        }

        child = child->next;
    }
        
    vkUnmapMemory(device->e_device, particle->go.graphObj.shapes[0].vParam.buffer.memory);

    particle->go.graphObj.shapes[0].vParam.num_verts = num_part;
}

void Particle3DClear(ParticleObject3D *particle){
    ChildStack *child = particle->part_header;
    
    if(child == NULL)
        return;

    ChildStack *next = NULL;
    ChildStack *before = NULL;
    
    uint32_t counter = 0;
       
    while(child != NULL){    
        
        if(child->node != NULL){
            FreeMemory(child->node);
        }

        before = child;  
        child = child->next;

        FreeMemory(before);
        before = NULL;
    }
    
    particle->part_header = NULL;
}

void Particle3DSetDefaultShader(ParticleObject3D* particle);

void Particle3DFind(ParticleObject3D *particle){

    ChildStack *child = particle->part_header;

    ChildStack *before = NULL;

    Particle3D *obj = NULL;

    while(child != NULL){  

        obj = child->node;
        child = child->next;
        if(obj != NULL){
            if(obj->life <= 0){

                Particle3DDesctroyObj(particle, obj);
                continue;
            }
            
            obj->life -= 0.01;
            obj->direction.y -= 0.1f * obj->gravity * 10;
            obj->position = v3_add(obj->position, v3_muls(v3_divs(obj->direction, 10), obj->speed)) ;
            obj->scale -= 0.001f * obj->speed;
        }        
    }
}

void Particle3DDefaultUpdate(ParticleObject3D* particle, void *data){

    uint32_t num_part = Patricle3DObjCount(particle);

    if(num_part == 0)
        return;

    Particle3DFind(particle);

    if(num_part == 0)
        return;

    Particle3DCopyToBuffer(particle);

    Camera3D* cam = (Camera3D*) engine.cam3D;

    RenderTexture *render = engine.current_render;

    ModelBuffer3D mbo = {};
    vec3 cameraUp = {0.0f,1.0f, 0.0f};

    particle->go.transform.model = m4_transform(particle->go.transform.position, particle->go.transform.scale, particle->go.transform.rotation);

    mbo.model = particle->go.transform.model;
    mbo.view = m4_look_at(cam->position, v3_add(cam->position, cam->rotation), cameraUp);
    mbo.proj = m4_perspective(render->width, render->height, render->persp_view_angle, render->persp_view_near, render->persp_view_distance);
    mbo.proj.m[1][1] *= -1;

    memcpy(data, &mbo, sizeof(mbo));
}

void Particle3DDestroy(ParticleObject3D* particle){

    Particle3DClear(particle);

    GraphicsObjectDestroy(&particle->go.graphObj);

    if(particle->go.diffuses != NULL)
    {
        FreeMemory(particle->go.diffuses->path);

        if(particle->go.diffuses->size > 0)
            FreeMemory(particle->go.diffuses->buffer);

        FreeMemory(particle->go.diffuses);
    }

    BuffersDestroyBuffer(&particle->go.graphObj.shapes[0].vParam.buffer);

    FreeMemory(particle->part_header);
    free(particle->go.graphObj.shapes[0].vParam.vertices);

    FreeMemory(particle->go.self.vert);
    FreeMemory(particle->go.self.frag);
    
    particle->go.self.flags &= ~(TIGOR_GAME_OBJECT_FLAG_INIT);
}

void Particle3DInit(ParticleObject3D* particle, DrawParam *dParam){

    TDevice *device = (TDevice *)engine.device;

    memset(particle, 0, sizeof(ParticleObject3D));

    GameObject3DInit((GameObject3D *)particle, TIGOR_GAME_OBJECT_TYPE_PARTICLE_3D);

    GameObjectSetDestroyFunc((GameObject *)particle, (void *)Particle3DDestroy);
    GameObjectSetShaderInitFunc((GameObject *)particle, (void *)Particle3DSetDefaultShader);

    BuffersCreate(sizeof(ParticleVertex3D) * MAX_PARTICLES_COUNT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &particle->go.graphObj.shapes[0].vParam.buffer, TIGOR_BUFFER_ALLOCATE_VERTEX);
        
    particle->go.graphObj.shapes[0].vParam.bufferSize = sizeof(ParticleVertex3D) * MAX_PARTICLES_COUNT;
    particle->go.graphObj.shapes[0].vParam.typeSize = sizeof(ParticleVertex3D);
    particle->go.graphObj.shapes[0].vParam.num_verts = 0;
    particle->go.graphObj.num_shapes ++;

    particle->go.diffuses = AllocateMemory(1, sizeof(GameObjectImage));

    char *currPath = DirectGetCurrectFilePath();
    int len = strlen(currPath);
    currPath[len] = '\\';

    if(strlen(dParam->diffuse) != 0)
    {

        char *full_path = ToolsMakeString(currPath, dParam->diffuse);

        if(!DirectIsFileExist(full_path)){
            FreeMemory(full_path);             
            FreeMemory(currPath);
            return;
        }

        particle->go.diffuses->path = full_path;
        //go->image->buffer = ToolsLoadImageFromFile(&go->image->size, dParam.filePath);
        particle->go.num_diffuses ++;
    }    

    FreeMemory(currPath);
}

void Particle3DSetDefaultShader(ParticleObject3D* particle)
{
    
    if(particle->go.self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED)
        return;

    uint32_t num_pack = BluePrintInit(&particle->go.graphObj.blueprints);
    
    ShaderBuilder *vert = particle->go.self.vert;
    ShaderBuilder *frag = particle->go.self.frag;

    ShaderMakeDefaultParticle3DShader(vert, frag);
    
    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);
    
    GraphicsObjectSetShaderWithUniform(&particle->go.graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&particle->go.graphObj, &frag_shader, num_pack);

    GameObject3DSetDescriptorUpdate((GameObject3D *)particle, num_pack, 0, (UpdateDescriptor)Particle3DDefaultUpdate);
    GameObject3DSetDescriptorTextureCreate((GameObject3D *)particle, num_pack, 1, particle->go.diffuses);
    
    uint32_t flags = BluePrintGetSettingsValue(&particle->go.graphObj.blueprints, num_pack, 3);
    BluePrintSetSettingsValue(&particle->go.graphObj.blueprints, num_pack, 1, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    flags &= ~(TIGOR_PIPELINE_FLAG_DRAW_INDEXED);
    BluePrintSetSettingsValue(&particle->go.graphObj.blueprints, num_pack, 3, flags);
    
    particle->go.self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void Particle3DAdd(ParticleObject3D* particle, vec3 position, vec3 direction, float speed, float gravity, float life){

    Particle3D *part = Particle3DAddObject(particle);

    part->position = position;
    part->direction = direction;
    part->life = life;
    part->speed = speed;
    part->gravity = gravity;

    float min_size = 0.3f;
    float max_size = 1.0f;

    part->scale = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));

    part->color.x = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));
    part->color.y = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));
    part->color.z = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));

}
