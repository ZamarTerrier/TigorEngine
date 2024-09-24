#ifndef PRIMITIVEOBJECT_H
#define PRIMITIVEOBJECT_H

#include "Variabels/engine_includes.h"

#include "gameObject3D.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum PrimitiveObjectType{
    TIGOR_PRIMITIVE3D_LINE,
    TIGOR_PRIMITIVE3D_TRIANGLE,
    TIGOR_PRIMITIVE3D_QUAD,
    TIGOR_PRIMITIVE3D_PLANE,
    TIGOR_PRIMITIVE3D_CUBE,
    TIGOR_PRIMITIVE3D_CUBESPHERE,
    TIGOR_PRIMITIVE3D_ICOSPHERE,
    TIGOR_PRIMITIVE3D_SPHERE,
    TIGOR_PRIMITIVE3D_CAPSULE,
    TIGOR_PRIMITIVE3D_CONE,
    TIGOR_PRIMITIVE3D_CYLINDER,
    TIGOR_PRIMITIVE3D_SKYBOX
};

typedef struct{
    GameObject3D go;
    void *params;
    uint32_t type;
} PrimitiveObject;

typedef struct{
    float radius;
    int sectorCount;
    int stackCount;
} SphereParam;

typedef struct{
    float radius;
    int verperrow;
} CubeSphereParam;

typedef struct{
    int sectorCount;
    int stackCount;
    uint32_t texture_width;
    uint32_t texture_height;
} PlaneParam;

typedef struct{
    float height;
    int sectorCount;
    int stackCount;
} ConeParam;

/// @brief Инциализация объекта-примитива
/// @param po - объект-примитив
/// @param dParam - параметры для рендера
/// @param type - тип примитива
/// @param params - параметры примитива
/// @return - прошла ли инициализация успешно (true) либо (false)
int PrimitiveObjectInit(PrimitiveObject *po, DrawParam *dParam, char type, void *params);

#ifdef __cplusplus
}
#endif

#endif // PRIMITIVEOBJECT_H
