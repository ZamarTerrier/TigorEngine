#ifndef SHAPEOBJECT_H
#define SHAPEOBJECT_H

#include "gameObject2D.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    vec2 position;
    vec2 direction;
    float length;
    vec4 color;
} LineParams;

typedef struct{
    float size;
    vec4 color;
} QuadParams;

typedef struct{
    float radius;
    int freq;
    vec4 color;
} CircleParams;

typedef struct{
    vec2 *positions;
    uint32_t num_positions;
    vec4 color;
} ShapeParams;

typedef enum{
    TIGOR_SHAPE_OBJECT_LINE,
    TIGOR_SHAPE_OBJECT_QUAD,
    TIGOR_SHAPE_OBJECT_CIRCLE,
    TIGOR_SHAPE_OBJECT_SHAPE
} ShapeType;

typedef struct{
    GameObject2D go;
    uint32_t type;
} ShapeObject;
/// @brief Инициализация многоугольного двумерного объекта
/// @param so - объект многоугольника
/// @param dParam - параметры для рендера
/// @param type - тип многоугольника
/// @param param - параметры многоугольника
int ShapeObjectInit(ShapeObject *so, DrawParam *dParam, ShapeType type, void *param);

#ifdef __cplusplus
}
#endif

#endif // SHAPEOBJECT_H
