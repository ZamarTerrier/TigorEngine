#ifndef LIGHT_OBJECT_H
#define LIGHT_OBJECT_H

#include "Variabels/engine_includes.h"

typedef enum{
    TIGOR_LIGHT_OBJECT_TYPE_POINT_LIGHT,
    TIGOR_LIGHT_OBJECT_TYPE_DIRECTIONAL_LIGHT,
    TIGOR_LIGHT_OBJECT_TYPE_SPOT_LIGHT,
} LightObjectType;

typedef struct LightObject {
    vec3 position __attribute__ ((aligned (16)));
    vec3 color __attribute__ ((aligned (16)));
    vec3 direction __attribute__ ((aligned (16))); // Для направленного и фонарного света
    float intensity;
    int type;
    float cutoff; // Угол освещения фонарного света внутренний круг
    float outCutOff; // Угол освещения фонарного света внешний круг
    float radius;
} LightObject;

typedef struct LightBuffer {
    LightObject lights[10];
    int num_lights;
    int light_enable;
} LightBuffer;

LightObject *LightObjectAdd();

void LightObjectSetLights(void *dst);

void LightObjectSetPosition(LightObject *light, float x, float y, float z);
vec3 LightObjectGetPosition(LightObject *light);

void LightObjectSetColor(LightObject *light, float r, float g, float b);
vec3 LightObjectGetColor(LightObject *light);

void LightObjectSetDirection(LightObject *light, float x, float y, float z);
vec3 LightObjectGetDirection(LightObject *light);

void LightObjectSetIntensity(LightObject *light, float intensity);
float LightObjectGetIntensity(LightObject *light);

void LightObjectSetCutOff(LightObject *light, float cutoff);
float LightObjectGetCutOff(LightObject *light);

void LightObjectSetOutCutOff(LightObject *light, float outCutOff);
float LightObjectGetOutCutOff(LightObject *light);

void LightObjectSetRadius(LightObject *light, float radius);
float LightObjectGetRadius(LightObject *light);

#endif //LIGHT_OBJECT_H