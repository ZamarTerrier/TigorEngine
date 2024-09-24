#include <GUI/e_widget_image.h>

#include "Objects/gameObject2D.h"

#include <vulkan/vulkan.h>

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

#include "Tools/e_math.h"
#include "Tools/e_shaders.h"

extern TEngine engine;


void ImageWidgetCreateQuad(GameObject2D *go)
{
    Vertex2D *verts = AllocateMemory(4, sizeof(Vertex2D));

    verts[0].position.x = -1;
    verts[0].position.y = -1;
    verts[0].texCoord.x = 0;
    verts[0].texCoord.y = 0;

    verts[1].position.x = 1;
    verts[1].position.y = -1;
    verts[1].texCoord.x = 1.0f;
    verts[1].texCoord.y = 0;

    verts[2].position.x = 1;
    verts[2].position.y = 1;
    verts[2].texCoord.x = 1.0f;
    verts[2].texCoord.y = 1.0f;

    verts[3].position.x = -1;
    verts[3].position.y = 1;
    verts[3].texCoord.x = 0;
    verts[3].texCoord.y = 1.0f;

    for(int i=0;i < 4;i++)
    {
        verts[i].color = vec3_f(1, 1, 1);
    }

    uint32_t *tIndx = AllocateMemory(6, sizeof(uint32_t));

    uint32_t indx[] = {
      0, 1, 2, 2, 3, 0
    };

    memcpy(tIndx, indx, 6 * sizeof(uint32_t));

    GraphicsObjectSetVertex(&go->graphObj, verts, 4, sizeof(Vertex2D), tIndx, 6, sizeof(uint32_t));

    FreeMemory(verts);
    FreeMemory(tIndx);
}


void ImageWidgetDraw(EWidgetImage *img){

    Transform2DSetScale((struct GameObject2D_T *)&img->image, img->widget.scale.x, img->widget.scale.y);
    Transform2DSetPosition((struct GameObject2D_T *)&img->image, img->widget.position.x + img->widget.base.x, img->widget.position.y + img->widget.base.y);

    if(img->widget.widget_flags & TIGOR_FLAG_WIDGET_VISIBLE){
        GameObjectDraw((GameObject *)&img->image);
    }
}

extern void WidgetDestroy(EWidget *widget);

void ImageWidgetDestroy(EWidgetImage *img){

    if(!(img->image.self.flags & TIGOR_GAME_OBJECT_FLAG_INIT))
        return;

    WidgetDestroy((EWidget *)img);

    GameObjectDestroy((GameObject *)&img->image);

    img->image.self.flags &= ~(TIGOR_GAME_OBJECT_FLAG_INIT);
}


void ImageWidgetInit(EWidgetImage *img, char *image_path, EWidget *parent){

    if(!GUIManagerIsInit())
        return;
        
    if(strlen(image_path) == 0)
        return;

    WidgetInit((EWidget *)img, parent);

    GameObjectSetDrawFunc((GameObject *)img, ImageWidgetDraw);
    GameObjectSetDestroyFunc((GameObject *)img, ImageWidgetDestroy);

    img->widget.type = TIGOR_WIDGET_TYPE_IMAGE;
    ///--------------------------------------------------

    GameObject2DInit(&img->image, TIGOR_GAME_OBJECT_TYPE_2D);

    ImageWidgetCreateQuad(&img->image);

    img->image.image = AllocateMemory(1, sizeof(GameObjectImage));

    int len = strlen(image_path);
    img->image.image->path = AllocateMemory(len + 1, sizeof(char));
    memcpy(img->image.image->path, image_path, len);
    img->image.image->path[len] = '\0';;
    img->image.num_images ++;

    uint32_t num_pack = BluePrintInit(&img->image.graphObj.blueprints);
    
    ShaderBuilder *vert = img->image.self.vert;
    ShaderBuilder *frag = img->image.self.frag;

    ShadersMakeDefault2DShader(vert, frag, img->image.num_images > 0);

    ShaderObject vert_shader, frag_shader;
    memset(&vert_shader, 0, sizeof(ShaderObject));
    memset(&frag_shader, 0, sizeof(ShaderObject));

    vert_shader.code = (char *) vert->code;
    vert_shader.size = vert->size * sizeof(uint32_t);
    
    frag_shader.code = (char *) frag->code;
    frag_shader.size = frag->size * sizeof(uint32_t);

    GraphicsObjectSetShaderWithUniform(&img->image.graphObj, &vert_shader, num_pack);
    GraphicsObjectSetShaderWithUniform(&img->image.graphObj, &frag_shader, num_pack);

    BluePrintAddSomeUpdater(&img->image.graphObj.blueprints, num_pack, 0, (UpdateDescriptor)GameObject2DTransformBufferUpdate);
    BluePrintAddSomeUpdater(&img->image.graphObj.blueprints, num_pack, 1, (UpdateDescriptor)GameObject2DImageBuffer);
    BluePrintSetTextureImageCreate(&img->image.graphObj.blueprints, num_pack, img->image.image, 0);

    uint32_t flags = BluePrintGetSettingsValue(&img->image.graphObj.blueprints, num_pack, 3);
    BluePrintSetSettingsValue(&img->image.graphObj.blueprints, num_pack, 3, flags | TIGOR_PIPELINE_FLAG_FACE_CLOCKWISE);

    img->image.self.flags |= TIGOR_GAME_OBJECT_FLAG_SHADED;
    //----------------------------------------------------

    GameObject2DInitDraw(&img->image);

}