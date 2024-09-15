#ifndef PARTICLESYSTEM3D_H
#define PARTICLESYSTEM3D_H

#include "Variabels/engine_includes.h"

#include "gameObject3D.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
    vec2 mouse;
    float time;
    vec3 camRot __attribute__ ((aligned (16)));
    vec3 camPos __attribute__ ((aligned (16)));
} DataBuffer;

typedef struct{
    vec3 position;
    vec3 direction;
    vec3 color;
    float scale;
    float life;
    float speed;
    float gravity;
} Particle3D;

typedef struct{
    GameObject3D go;
    ChildStack* part_header;
} ParticleObject3D;

/// @brief Инициализация объекта частиц
/// @param particle - объект частиц
/// @param dParam - параметры для отрисовки
void Particle3DInit(ParticleObject3D* particle, DrawParam *dParam);
/// @brief Функция для добавления новой частицы
/// @param particle - объект частиц
/// @param position - положение частицы в пространстве
/// @param direction - направление движения частицы
/// @param speed - скорость движения частицы
/// @param gravity - гравитация цастицы
/// @param life - срок жизни частицы
void Particle3DAdd(ParticleObject3D* particle, vec3 position, vec3 direction, float speed, float gravity, float life);

#ifdef __cplusplus
}
#endif

#endif // PARTICLESYSTEM3D_H
