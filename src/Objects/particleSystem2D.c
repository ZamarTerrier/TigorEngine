#include "Objects/particleSystem2D.h"

#include "TigorEngine.h"

#include <vulkan/vulkan.h>

#include "Core/e_memory.h"
#include "Core/e_buffer.h"
#include "Core/e_camera.h"
#include "Core/e_device.h"

#include "Tools/e_math.h"
#include "Tools/e_shaders.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

#define MAX_PARTICLES_COUNT 1024

extern TEngine engine;


Particle2D *Particle2DAddObject(ParticleObject2D * particle){

    if(particle->part_header == NULL)
        particle->part_header = AllocateMemory(1, sizeof(ChildStack));

    Particle2D *object = AllocateMemory(1, sizeof(Particle2D));

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

uint32_t Patricle2DObjCount(ParticleObject2D *particle){
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

void Particle2DDesctroyObj(ParticleObject2D *particle, Particle2D *part){
    if(part == NULL)
        return;

    TDevice *device = (TDevice *)engine.device;

    Particle2D *curr = NULL;

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

void Particle2DCopyToBuffer(ParticleObject2D *particle){

    TDevice *device = (TDevice *)engine.device;

    ChildStack *child = particle->part_header;
    
    Particle2D *obj = NULL;

    uint32_t counter = 0;     
    
    uint32_t num_part = Patricle2DObjCount(particle);   
    
    char *data;
    vkMapMemory(device->e_device, particle->go.graphObj.shapes[0].vParam.buffer.memory, 0, sizeof(ParticleVertex2D) * MAX_PARTICLES_COUNT, 0, (void **)&data);

    memset(data, 0, sizeof(ParticleVertex2D) * MAX_PARTICLES_COUNT);

    ParticleVertex2D temp;

    while(child != NULL){
        if(counter >= MAX_PARTICLES_COUNT)
            break;

        obj = child->node;

        if(obj != NULL){

            temp.position = obj->position;
            temp.size = obj->scale;
            temp.color = obj->color;
            
            memcpy(data, &temp, (size_t) sizeof(ParticleVertex2D));

            data += sizeof(ParticleVertex2D);
            counter ++;
        }

        child = child->next;
    }
        
    vkUnmapMemory(device->e_device, particle->go.graphObj.shapes[0].vParam.buffer.memory);

    particle->go.graphObj.shapes[0].vParam.num_verts = num_part;
}

void Particle2DClear(ParticleObject2D *particle){
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

void Particle2DSetDefaultShader(ParticleObject2D* particle);

void Particle2DFind(ParticleObject2D *particle){

    ChildStack *child = particle->part_header;

    ChildStack *before = NULL;

    Particle2D *obj = NULL;

    while(child != NULL){  

        obj = child->node;
        child = child->next;
        if(obj != NULL){
            if(obj->life <= 0){

                Particle2DDesctroyObj(particle, obj);
                continue;
            }
            
            obj->life -= 0.01;
            obj->direction.y -= 0.1f * obj->gravity * 10;
            obj->position = v2_sub(obj->position, v2_muls(v2_divs(obj->direction, 10), obj->speed)) ;
            obj->scale -= 0.001f * obj->speed;
        }        
    }
}

void Particle2DDefaultUpdate(ParticleObject2D* particle, void *data){

    uint32_t num_part = Patricle2DObjCount(particle);

    if(num_part == 0)
        return;

    Particle2DFind(particle);

    if(num_part == 0)
        return;

    Particle2DCopyToBuffer(particle);

    TransformBuffer2D tbo = {};

    vec2 posit = v2_subs(particle->go.transform.position, 1.0f);
    tbo.model = m4_transform(vec3_f(posit.x, posit.y, 0), vec3_f(particle->go.transform.scale.x, particle->go.transform.scale.y, 0), vec3_f(particle->go.transform.rotation, 0, 0));

    memcpy(data, &tbo, sizeof(tbo));
}


void Particle2DDestroy(ParticleObject2D* particle){

    Particle2DClear(particle);

    GraphicsObjectDestroy(&particle->go.graphObj);

    if(particle->go.image != NULL)
    {
        FreeMemory(particle->go.image->path);

        if(particle->go.image->size > 0)
            FreeMemory(particle->go.image->buffer);

        FreeMemory(particle->go.image);
    }

    BuffersDestroyBuffer(&particle->go.graphObj.shapes[0].vParam.buffer);

    FreeMemory(particle->part_header);
    free(particle->go.graphObj.shapes[0].vParam.vertices);

    FreeMemory(particle->go.self.vert);
    FreeMemory(particle->go.self.frag);
    
    particle->go.self.flags &= ~(TIGOR_GAME_OBJECT_FLAG_INIT);
}

void Particle2DInit(ParticleObject2D* particle, DrawParam *dParam){
    TDevice *device = (TDevice *)engine.device;

    memset(particle, 0, sizeof(ParticleObject2D));

    GameObject2DInit((GameObject2D *)particle, TIGOR_GAME_OBJECT_TYPE_PARTICLE_2D);
    
    char *name = "Particle_Object";

    memcpy(particle->go.name, name, strlen(name));

    GameObjectSetDestroyFunc((GameObject *)particle, (void *)Particle2DDestroy);
    GameObjectSetShaderInitFunc((GameObject *)particle, (void *)Particle2DSetDefaultShader);

    BuffersCreate(sizeof(ParticleVertex2D) * MAX_PARTICLES_COUNT, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &particle->go.graphObj.shapes[0].vParam.buffer, TIGOR_BUFFER_ALLOCATE_VERTEX);
        
    particle->go.graphObj.shapes[0].vParam.bufferSize = sizeof(ParticleVertex2D) * MAX_PARTICLES_COUNT;
    particle->go.graphObj.shapes[0].vParam.typeSize = sizeof(ParticleVertex2D);
    particle->go.graphObj.shapes[0].vParam.num_verts = 0;
    particle->go.graphObj.num_shapes ++;

    particle->go.image = AllocateMemory(1, sizeof(GameObjectImage));

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

        particle->go.image->path = full_path;
        //go->image->buffer = ToolsLoadImageFromFile(&go->image->size, dParam.filePath);
        particle->go.num_images ++;
    }
    
    FreeMemory(currPath);
}

void Particle2DSetDefaultShader(ParticleObject2D* particle)
{    
    if(particle->go.self.flags & TIGOR_GAME_OBJECT_FLAG_SHADED)
        return;

    uint32_t num_pack = BluePrintInit(&particle->go.graphObj.blueprints);
    
    ShaderBuilder *vert = particle->go.self.vert;
    ShaderBuilder *frag = particle->go.self.frag;

    ShaderMakeDefaultParticle2DShader(vert, frag);
    
    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *)vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *)frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);
    
    GraphicsObjectSetShaderWithUniform(&particle->go.graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&particle->go.graphObj, &frag_shader, num_pack);

    GameObject2DSetDescriptorUpdate((GameObject2D *)particle, num_pack, 0, (UpdateDescriptor)Particle2DDefaultUpdate);
    GameObject2DSetDescriptorTextureCreate(particle, num_pack, 1, particle->go.image);

    particle->go.graphObj.gItems.perspective = true;
    
    uint32_t flags = BluePrintGetSettingsValue(&particle->go.graphObj.blueprints, num_pack, 3);
    BluePrintSetSettingsValue(&particle->go.graphObj.blueprints, num_pack, 1, VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    flags &= ~(TIGOR_PIPELINE_FLAG_DRAW_INDEXED);
    BluePrintSetSettingsValue(&particle->go.graphObj.blueprints, num_pack, 3, flags);
    
    particle->go.self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
}

void Particle2DAdd(ParticleObject2D* particle, vec2 position, vec2 direction, float speed, float gravity, float life){

    Particle2D *part = Particle2DAddObject(particle);

    part->position = v2_div(position, vec2_f(engine.width, engine.height));
    part->direction = v2_div(direction, vec2_f(engine.width / 100, engine.height / 100));
    part->life = life;
    part->speed = speed;
    part->gravity = gravity / 100;

    float min_size = 0.3f;
    float max_size = 1.0f;

    part->scale = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));

    part->color.x = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));
    part->color.y = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));
    part->color.z = min_size + (((float)rand() / (float)RAND_MAX) * (max_size - min_size));

}
