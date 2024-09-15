#include "Objects/models.h"
#include "Objects/gameObject.h"
#include "Objects/render_texture.h"

#include <vulkan/vulkan.h>

#include "Core/e_memory.h"
#include "Core/e_device.h"
#include "Core/e_camera.h"
#include "Core/e_buffer.h"
#include "Core/pipeline.h"
#include "Core/e_texture.h"

#include "math.h"

#include "Tools/e_math.h"
#include "Tools/glTFLoader.h"
#include "Tools/objLoader.h"
#include "Tools/fbxLoader.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"
#include "Data/e_resource_descriptors.h"

typedef void (*Update_Model)(ModelObject3D* mo, uint32_t indx_node, BluePrintDescriptor *descriptor);

extern ZEngine engine;


void ModelDefaultDraw(ModelObject3D* mo, void *command){

    for(int i=0; i < mo->num_draw_nodes;i++){
        for(int j=0; j < mo->nodes[i].num_models; j++){

            GameObject3D *model = &mo->nodes[i].models[j];

            GameObjectDraw(model);
        }
    }
}

void ModelModelBufferUpdate(ModelObject3D* mo, uint32_t indx_node, void *data)
{
    Camera3D* cam = (Camera3D*) engine.cam3D;

    glTFStruct *glTF = mo->obj;

    if(glTF == NULL)
        return;

    RenderTexture *render = engine.current_render;

    ModelBuffer3D mbo = {};
    vec3 cameraUp = {0.0f,1.0f, 0.0f};

    engine_gltf_node *node = &glTF->nodes[mo->nodes[indx_node].id_node];

    mo->nodes[indx_node].model = mat4_mult_transform(node->global_matrix, m4_transform(mo->transform.position, mo->transform.scale, mo->transform.rotation));

    mbo.model = mo->nodes[indx_node].model;
    mbo.view = m4_look_at(cam->position, v3_add(cam->position, cam->rotation), cameraUp);
    mbo.proj = m4_perspective(render->width, render->height, cam->view_angle, cam->view_near, cam->view_distance);
    mbo.proj.m[1][1] *= -1;

    memcpy(data, (char *)&mbo, sizeof(mbo));
}

void ModelUpdateModel(ModelObject3D *mo, GameObject3D *model){
    
    ZDevice *device = (ZDevice *)engine.device;

    for(int i=0; i < model->graphObj.gItems.num_shader_packs;i++)
    {
        BluePrintPack *pack = &model->graphObj.blueprints.blue_print_packs[i];

        if(pack->render_point == engine.current_render)
        {
            for(int j=0;j < pack->num_descriptors;j++)
            {
                BluePrintDescriptor *descriptor = &pack->descriptors[j];

                if(descriptor->descrType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                {
                    if(descriptor->uniform.count == 0 || descriptor->update == NULL)
                        continue;

                    void *point;
                    vkMapMemory(device->e_device, descriptor->uniform.buffers[engine.imageIndex].memory, 0, descriptor->buffsize, 0, &point);
                    Update_Model update = descriptor->update;
                    update(mo, i, point);
                    vkUnmapMemory(device->e_device, descriptor->uniform.buffers[engine.imageIndex].memory);
                }
            }
        }
    }
}

void ModelDefaultUpdate(ModelObject3D *mo)
{

    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            GameObject3D *model = &mo->nodes[i].models[j];

            ModelUpdateModel(mo, model);
        }
    }
}

void ModelClean(ModelObject3D* mo){
    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            GameObject3D *model = &mo->nodes[i].models[j];

            GameObjectClean(model);
        }
    }
}

void ModelRecreate(ModelObject3D* mo){

    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            GameObject3D *model = &mo->nodes[i].models[j];

            GameObjectRecreate(model);
        }
    }
}

//Не корректно
void ModelSetSomeViewport(ModelObject3D* mo, float x, float y, float height, float width)
{
    /*for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            ModelStruct *model = &mo->nodes[i].models[j];

            PipelineSetting *settings = (PipelineSetting *)&model->graphObj.gItems.setting;

            settings[m].viewport.x = x;
            settings[m].viewport.y = y;
            settings[m].viewport.height = height;
            settings[m].viewport.width = width;
        }
    }*/
}

void ModelDestroy(ModelObject3D* mo){
    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0;j < mo->nodes[i].num_models;j++)
        {
            GameObject3D *model = &mo->nodes[i].models[j];

            GameObjectDestroy(model);
        }
    }
}

void ModelSetLightEnable(void *obj, bool enable)
{
    ModelObject3D *mo = (ModelObject3D *)obj;

    if(enable)
        mo->self.flags |= ENGINE_GAME_OBJECT_FLAG_LIGHT;
    else
        mo->self.flags &= ~(ENGINE_GAME_OBJECT_FLAG_LIGHT);
}

void ModelSetSelCameraEnable(void *obj, bool enable)
{
    ModelObject3D *mo = (ModelObject3D *)obj;

    if(enable)
        mo->self.flags |= ENGINE_GAME_OBJECT_FLAG_SELF_CAMERA;
    else
        mo->self.flags &= ~(ENGINE_GAME_OBJECT_FLAG_SELF_CAMERA);
}

void ModelPopulateVertex3D(GameObject3D *model)
{
    uint32_t num = model->graphObj.num_shapes;

    model->graphObj.shapes[num].bindingDescription = &Bind3DDescription;
    model->graphObj.shapes[num].attr = cubeAttributeDescription;
    model->graphObj.shapes[num].countAttr = 3;

    ModelVertex3D *m_verts = model->graphObj.shapes[0].vParam.vertices;
    uint32_t count = model->graphObj.shapes[0].vParam.num_verts;
    Vertex3D *verts = AllocateMemory(count, sizeof(Vertex3D));

    for(int i=0;i < count;i++)
    {
        verts[i].position = m_verts[i].position;
        verts[i].normal = m_verts[i].normal;
        verts[i].texCoord = m_verts[i].texCoord;
    }

    GraphicsObjectSetVertex(&model->graphObj, verts, count, sizeof(Vertex3D), model->graphObj.shapes[0].iParam.indices, model->graphObj.shapes[0].iParam.indexesSize, sizeof(uint32_t));

}

void ModelDefautShader(GameObject3D *go){

    GameObject3DInitDefaultShader(go);

    uint32_t flags = BluePrintGetSettingsValue(&go->graphObj.blueprints, 0, 3);
    BluePrintSetSettingsValue(&go->graphObj.blueprints, 0, 3, flags | ENGINE_PIPELINE_FLAG_FACE_CLOCKWISE);
}

void ModelDefaultInit(ModelObject3D *mo, GameObjectType type){

    for(int i=0; i < mo->num_draw_nodes;i++)
    {
        for(int j=0; j < mo->nodes[i].num_models;j++)
        {
            GameObjectInit(&mo->nodes[i].models[j]);
        }
    }

    mo->self.flags |= ENGINE_GAME_OBJECT_FLAG_INIT;
}