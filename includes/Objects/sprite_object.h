#ifndef SPRITEOBJECT_H
#define SPRITEOBJECT_H

#include "gameObject2D.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    vec3 color;
    char texturePath[256];
    char vertShader[256];
    char fragShader[256];
} SpriteParam;

typedef struct{
    GameObject2D go;
} SpriteObject;
/// @brief Инициализация объекта-спрайта
/// @param go - объект-спрайт
/// @param dParam - параметры для рендера
/// @return - прошла ли инициализация успешно (true) либо (false)
int SpriteObjectInit(SpriteObject *go, DrawParam *dParam);
/// @brief Установка смещения изображения по атласу текстуры
/// @param go - объект-спрайт
/// @param x - координата по x
/// @param y - координата по y
/// @param width - ширина квадрата
/// @param height - высота квадрата
void SpriteObjectSetOffsetRect(SpriteObject *so, float x, float y, float width, float height);

#ifdef __cplusplus
}
#endif

#endif // SPRITEOBJECT_H
