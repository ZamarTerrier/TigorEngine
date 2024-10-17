#include "Objects/light_object.h"

#include "Tools/e_math.h"

extern TEngine engine;

void LightObjectInit(LightObject *light, LightObjectType type){
    
    light->color = vec3_f(1, 1, 1);
    light->intensity = 1.0f;

    switch(type){
        case TIGOR_LIGHT_OBJECT_TYPE_POINT_LIGHT:
            light->radius = 1.0f;
            break;
        case TIGOR_LIGHT_OBJECT_TYPE_DIRECTIONAL_LIGHT:
            light->color = vec3_f(0.5, 0.5, 0.5);
            break;
        case TIGOR_LIGHT_OBJECT_TYPE_SPOT_LIGHT:
            light->cutoff = 0.99f;
            light->outCutOff = 0.95f;
            light->radius = 100.0f;
            break;
    }

    light->type = type;
}

LightObject *LightObjectAdd(LightObjectType type){

    if(engine.lights.size + 1 >= 10){
        printf("Light Object Error : Too many objects\n");
        return NULL;
    }

    LightObject *obj = AllocateMemory(1, sizeof(LightObject));

    if(engine.lights.lights == NULL)
        engine.lights.lights = AllocateMemory(1, sizeof(ChildStack));

    if(engine.lights.lights->node == NULL){
        engine.lights.lights->next = AllocateMemory(1, sizeof(ChildStack));
        engine.lights.lights->node = obj;
    }
    else{

        ChildStack *child = engine.lights.lights->next;

        while(child->next != NULL)
        {
            child = child->next;
        }

        child->next = AllocateMemory(1, sizeof(ChildStack));
        child->node = obj;
    }

    LightObjectInit(obj, type);

    engine.lights.size ++;

    return obj;

}

void LightObjectRemove(LightObject *light){
    ChildStack *child = engine.lights.lights;
    ChildStack *next = NULL, *before = NULL;

    while(child != NULL){
        if(child->node == light)
            break;

        before = child->next;
        child = child->next;
    }

    if(child == NULL){
        printf("Light Object Error : Can't find that memory\n");
        return;
    }

    if(child->next != NULL){
        if(before != NULL){
            child->next->before = before;
            before->next = child->next;

            if(child->next->node == NULL)
               engine.lights.lights = before;

        }
        else{
            child->next->before = NULL;
            engine.lights.lights = child->next;
        }
    }else{
        if(before != NULL){
            before->next = NULL;
            engine.lights.lights = before;
        }
    }

    if(child->node != NULL)
        FreeMemory(child->node);

    FreeMemory(child);

    engine.lights.size --;
}

void LightObjectClear(){
    ChildStack *child = engine.lights.lights;
    ChildStack *next = NULL, *before = NULL;

    while(child != NULL){
        if(child->node != NULL)
            FreeMemory(child->node);

        next = child->next;
        FreeMemory(child);
        child = next;
    }

    engine.lights.size = 0;
}

void LightObjectSetLights(void *dst){

    ChildStack *child = engine.lights.lights;

    uint32_t iter = 0;
    while(child != NULL){

        if(child->node != NULL){
            memcpy(dst + iter, child->node, sizeof(LightObject));
            iter += sizeof(LightObject);
        }

        child = child->next;
    }
}

void LightObjectSetPosition(LightObject *light, float x, float y, float z){
    light->position = vec3_f(x, y, z);
}   

vec3 LightObjectGetPosition(LightObject *light){
    return light->position;
}

void LightObjectSetColor(LightObject *light, float r, float g, float b){
    light->color = vec3_f(r, g, b);
}

vec3 LightObjectGetColor(LightObject *light){
    return light->color;
}

void LightObjectSetDirection(LightObject *light, float x, float y, float z){
    light->direction = vec3_f(x, y, z);
}   

vec3 LightObjectGetDirection(LightObject *light){
    return light->direction;
}

void LightObjectSetIntensity(LightObject *light, float intensity){
    light->intensity = intensity;
}

float LightObjectGetIntensity(LightObject *light){
    return light->intensity;
}

void LightObjectSetCutOff(LightObject *light, float cutoff){
    light->cutoff = cutoff;
}

float LightObjectGetCutOff(LightObject *light){
    return light->cutoff;
}

void LightObjectSetOutCutOff(LightObject *light, float outCutOff){
    light->outCutOff = outCutOff;
}

float LightObjectGetOutCutOff(LightObject *light){
    return light->outCutOff;
}

void LightObjectSetRadius(LightObject *light, float radius){
    light->radius = radius;
}

float LightObjectGetRadius(LightObject *light){
    return light->radius;
}