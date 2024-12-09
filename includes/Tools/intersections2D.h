#ifndef INTERSECTIONS2D_H
#define INTERSECTIONS2D_H

#include "Variabels/engine_includes.h"
#include "Variabels/std_intersect.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct{
    vec2 points[2048];
    vec2 center;
    uint32_t num_points;
} Point2D;

typedef struct{
    Point2D collisions;
    float depths[32];
} Manfloid2D;

bool IntersectGJK2D(GJKObject *gjk, Point2D *obj1, Point2D *obj2);
int IntersectLineToLine(vec2 a, vec2 b, vec2 c, vec2 d, float *t, vec2 *p);
int IntersectionCircleTriangle(vec2 sPos, float r, vec2 p0, vec2 p1, vec2 p2, vec2 *resPos, float *dist, float *depth, vec2 *dir);
int IntersectionCircleCircle(InterCircleParam *o1, InterCircleParam *o2, float *dist, float *depth, vec2 *dir);
int IntersectionCircleSquare(InterCircleParam *sph, InterSquareParam *box, float *dist, float *depth, vec2 *dir);
int IntersectionSquareSquare(InterSquareParam *box1, InterSquareParam *box2, float *dist, float *depth, vec2 *dir);
Manfloid2D IntersectionSquareOOBSquareOOB(GameObject2D *sq1, GameObject2D *sq2);
int IntersectionTriangleSquare(InterTriangleParam triangle, InterSquareParam *box);
int IntersectionSphapeSquare(void *obj1, InterSquareParam *square);
int IntersectionShapeShape(void *obj1, void *obj2);

#ifdef __cplusplus
}
#endif

#endif // INTERSECTIONS2D_H
