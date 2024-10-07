#ifndef E_CAMERA_H
#define E_CAMERA_H

#include "Variabels/engine_includes.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    vec2 position;
    float angle;
    vec2 scale;
} Camera2D;

typedef struct{
    vec3 position __attribute__ ((aligned (16)));
    vec3 rotation __attribute__ ((aligned (16)));
    vec3 scale __attribute__ ((aligned (16)));

    double view_distance;
    double view_angle;
    double view_near;

    float yaw;
    float pitch;
    float sensitivity;

    float moveSpeed;
    float cameraSpeed;

    float lastX;
    float lastY;

    bool firstMouse;
    bool walk;
} Camera3D;
/// @brief Функция инициализации двумерной камеры
/// @param cam - объект камеры
void Camera2DInit(Camera2D *cam);
/// @brief Функция инициализации трёхмерной камеры
/// @param cam - объект камеры
void Camera3DInit(Camera3D *cam);
/// @brief Функция установки дальней плоскости сцены
/// @param cam - объект камеры
/// @param distance - расстояние до плоскости
void Camera3DSetViewDistance(Camera3D *cam, double distance);
/// @brief Функция установки угла обзора камеры
/// @param cam - объект камеры
/// @param angle - угол обзора
void Camera3DSetViewAngle(Camera3D *cam, double angle);
/// @brief Функция установки ближней плоскости сцены
/// @param cam - объект камеры
/// @param near - расстояние до плоскости
void Camera3DSetViewNear(Camera3D *cam, double near);
/// @brief Назначение активной камеры
/// @param cam - объект камеры
void Camera3DSetActive(Camera3D *cam);
/// @brief Назначение активной камеры
/// @param cam - объект камеры
void Camera2DSetActive(Camera2D *cam);
/// @brief Установка поворота двумерной камеры
/// @param angle - поворот камеры
void Camera2DSetRotation(float angle);
/// @brief Установка поворота камеры
/// @param x - поворот по х
/// @param y - поворот по у
/// @param z - поворот по z
void Camera3DSetRotation(float x, float y, float z);
/// @brief Функция получения угла поворота двумерной камеры
/// @return число с плавающей точкой
float Camra2DGetRotation();
/// @brief Функция получения значений поворота камеры
/// @return трёхмерный вектор координат x, y и z
vec3 Camera3DGetRotation();
/// @brief Установка положения камеры в пространстве
/// @param x - координаты по x
/// @param y - координаты по y
void Camera2DSetPosition(float x, float y);
/// @brief Установка положения камеры в пространстве
/// @param x - координаты по x
/// @param y - координаты по y
/// @param z - координаты по z
void Camera3DSetPosition(float x, float y, float z);
/// @brief Функция получения положения камеры в пространстве
/// @return ldevthysq вектор координат x и y
vec2 Camera2DGetPosition();
/// @brief Функция получения положения камеры в пространстве
/// @return трёхмерный вектор координат x, y и z
vec3 Camera3DGetPosition();
/// @brief Функция установки масштаба камеры
/// @param x - координаты по x
/// @param y - координаты по y
void Camera2DSetScale(float x, float y);
/// @brief Функция установки масштаба камеры
/// @param x - координаты по x
/// @param y - координаты по y
/// @param z - координаты по z
void Camera3DSetScale(float x, float y, float z);
/// @brief Функция вращения камеры
///     Производит вращение камеры в соответствии с положением мыши на экране
/// @param deltaTime - множитель скорости бработки кадров
void Camera3DMovementUpdate(float deltaTime);
/// @brief  Функция обработки нажатия клавиш
///     Позволяет изменять положение камеры с помощью клавиатуры
/// @param deltaTime - множитель скорости бработки кадров
void Camera3DUpdateInput(float deltaTime);
#ifdef __cplusplus
}
#endif

#endif // E_CAMERA_H
