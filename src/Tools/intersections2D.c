#include "Tools/intersections2D.h"

#include "Objects/gameObject2D.h"

#include "TigorGUI.h"

#include "math.h"
#include "float.h"

#include "Data/e_resource_data.h"
#include "Data/e_resource_engine.h"

extern TEngine engine;

GJKObject gjkObject;

vec2 perpendicular (vec2 v) { vec2 p = { v.y, -v.x }; return p; }
float lengthSquared (vec2 v) { return v.x * v.x + v.y * v.y; }

vec2 tripleProduct(vec2 a, vec2 b, vec2 c){
    vec3 A = {a.x, a.y, 0};
    vec3 B = {b.x, b.y, 0};
    vec3 C = {c.x, c.y, 0};

    vec3 first = v3_cross(A, B);
    vec3 second = v3_cross(first, C);

    return (vec2){second.x, second.y};
}

size_t indexOfFurthestPoint (void *obj, vec2 direction) {

    Point2D *shape = obj;

    float maxdot = -1;
    size_t index = -1;

    for (int i=0;i < shape->num_points;i++) {
      float dot = v2_dot(shape->points[i], direction);
      if (dot > maxdot){
          maxdot = dot;
          index = i;
      }
    }

    return index;
}

vec2 supportFunc(void *obj, vec2 direction)
{
    Point2D *shape = obj;

    float maxdot = -1;
    vec2 base;

    for (int i=0;i < shape->num_points;i++) {
      float dot = v2_dot(shape->points[i], direction);
      if (dot > maxdot){
          maxdot = dot;
          base = v2_add(shape->points[i], shape->center);
      }
    }

    return base;
}

vec2 averagePoint (void *obj) {

    Point2D *shape = obj;

    vec2 avg = { 0.f, 0.f };
    for (size_t i = 0; i < shape->num_points; i++) {
        avg.x += shape->points[i].x + shape->center.x;
        avg.y += shape->points[i].y + shape->center.y;
    }
    avg.x /= shape->num_points;
    avg.y /= shape->num_points;

    return avg;
}

vec2 support (void *obj1, void *obj2, vec2 d) {

    Point2D *shape1 = obj1;
    Point2D *shape2 = obj2;

    vec2 *vertex1 = shape1->points;
    vec2 *vertex2 = shape2->points;

    // get furthest point of first body along an arbitrary direction
    size_t i = indexOfFurthestPoint (obj1, d);

    // get furthest point of second body along the opposite direction
    size_t j = indexOfFurthestPoint (obj2, v2_muls(d, -1));

    // subtract (Minkowski sum) the two points to see if bodies 'overlap'
    return v2_sub(v2_add(vertex1[i], shape1->center), v2_add(vertex2[j], shape2->center));
}

int TestGJK2D (GJKObject *gjk) {
    vec2 a, b, c, d, ao, ab, ac, abperp, acperp;

    vec2 position1 = averagePoint (gjk->obj1); // not a CoG but
    vec2 position2 = averagePoint (gjk->obj2); // it's ok for GJK )

    // initial direction from the center of 1st body to the center of 2nd body
    d = v2_sub(position1, position2);

    // if initial direction is zero – set it to any arbitrary axis (we choose X)
    if ((d.x == 0) && (d.y == 0))
        d.x = 1.f;

    // set the first support as initial point of the new simplex
    a = gjk->simplex[0] = v2_sub(gjk->support1(gjk->obj1, d), gjk->support2(gjk->obj2, v2_muls(d, -1)));

    if (v2_dot(a, d) <= 0)
        return 0; // no collision

    d = v2_muls(a, -1); // The next search direction is always towards the origin, so the next search direction is negate(a)

    while (1) {
        a = gjk->simplex[++gjk->num_verts] = v2_sub(gjk->support1(gjk->obj1, d), gjk->support2(gjk->obj2, v2_muls(d, -1)));

        if (v2_dot(a, d) <= 0)
            return 0; // no collision

        ao = v2_muls(a, -1); // from point A to Origin is just negative A

        // simplex has 2 points (a line segment, not a triangle yet)
        if (gjk->num_verts < 2) {
            b = gjk->simplex[0];
            ab = v2_sub(b, a); // from point A to B
            d = tripleProduct (ab, ao, ab); // normal to AB towards Origin
            if (lengthSquared (d) == 0)
                d = perpendicular (ab);
            continue; // skip to next iteration
        }

        b = gjk->simplex[1];
        c = gjk->simplex[0];
        ab = v2_sub(b, a); // from point A to B
        ac = v2_sub(c, a); // from point A to C

        acperp = tripleProduct (ab, ac, ac);

        if (v2_dot(acperp, ao) >= 0) {

            d = acperp; // new direction is normal to AC towards Origin

        } else {

            abperp = tripleProduct (ac, ab, ab);

            if (v2_dot(abperp, ao) < 0)
                return 1; // collision

            gjk->simplex[0] = gjk->simplex[1]; // swap first element (point C)

            d = abperp; // new direction is normal to AB towards Origin
        }

        gjk->simplex[1] = gjk->simplex[2]; // swap element in the middle (point B)
        --gjk->num_verts;
    }

    return 0;
}

bool IntersectGJK2D(GJKObject *gjk, Point2D *obj1, Point2D *obj2){
    // reset everything
    gjk->num_verts = 0;
    gjk->obj1 = obj1;
    gjk->obj2 = obj2;
    gjk->support1 = supportFunc;
    gjk->support2 = supportFunc;

    // do the actual test
    EvolveResult result = StillEvolving;
    while(result == StillEvolving) {
        result = TestGJK2D(gjk);
    }
    return result == FoundIntersection;
}

float Signed2DTriArea(vec2 a, vec2 b, vec2 c){
    return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}

int IntersectLineToLine(vec2 a, vec2 b, vec2 c, vec2 d, float *t, vec2 *p)
{
    float a1 = Signed2DTriArea(a, b, d);
    float a2 = Signed2DTriArea(a, b, c);

    if(a1 * a2 < 0.0f){
        float a3 = Signed2DTriArea(c, d, a);
        
        float a4 = a3 + a2 - a1;

        if(a3 * a4 < 0.0f){
            *t = a3 / (a3 - a4);
            *p = v2_add(a, v2_muls(v2_sub(b, a), *t));
            return 1;
        }
    }
    return 0;
}

float SqDistPointSquare(vec2 pos, InterSquareParam *box)
{
    vec2 up = {box->position.x + box->size, box->position.y + box->size};
    vec2 down = {box->position.x - box->size, box->position.y - box->size};

    float *p = (float *)&pos;
    float *max = (float *)&up;
    float *min = (float *)&down;

    float sqDist = 0.0f;
    for (int i = 0; i < 2; i++) {
        // For each axis count any excess distance outside box extents
        float v = p[i];
        if (v < min[i]) sqDist += (min[i] - v) * (min[i] - v);
        if (v > max[i]) sqDist += (v - max[i]) * (v - max[i]);
    }
    return sqDist;
}

int ComputeSideSquare(vec2 p, vec2 min, vec2 max)
{
    int code = 0;  // initialised as being inside of clip window

    if (p.x < min.x)           // to the left of clip window
        code |= 1;
    else if (p.x > max.x)      // to the right of clip window
        code |= 2;
    if (p.y < min.y)           // below the clip window
        code |= 4;
    else if (p.y > max.y)      // above the clip window
        code |= 8;

    return code;
}

bool ComputeTwoTrianles(InterTriangleParam t1, InterTriangleParam t2)
{


    return false;
}

bool IntersectionSegmentSquare(vec2 p1, vec2 p2, InterSquareParam *box)
{
    vec2 max_i = {box->position.x + box->size, box->position.y + box->size };
    vec2 min_i = {box->position.x - box->size, box->position.y - box->size };

    int outcode0 = ComputeSideSquare(p1, min_i, max_i);
    int outcode1 = ComputeSideSquare(p2, min_i, max_i);
        bool accept = false;

        while (true) {
            if (!(outcode0 | outcode1)) {
                // bitwise OR is 0: both points inside window; trivially accept and exit loop
                accept = true;
                break;
            } else if (outcode0 & outcode1) {
                // bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
                // or BOTTOM), so both must be outside window; exit loop (accept is false)
                break;
            } else {
                // failed both tests, so calculate the line segment to clip
                // from an outside point to an intersection with clip edge
                double x, y;

                // At least one endpoint is outside the clip rectangle; pick it.
                int outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

                // Now find the intersection point;
                // use formulas:
                //   slope = (y1 - y0) / (x1 - x0)
                //   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
                //   y = y0 + slope * (xm - x0), where xm is xmin or xmax
                // No need to worry about divide-by-zero because, in each case, the
                // outcode bit being tested guarantees the denominator is non-zero
                if (outcodeOut & 8) {           // point is above the clip window
                    x = p1.x + (p2.x - p1.x) * (max_i.y - p1.y) / (p2.y - p1.y);
                    y = max_i.y;
                } else if (outcodeOut & 4) { // point is below the clip window
                    x = p1.x + (p2.x - p1.x) * (min_i.y - p1.y) / (p2.y - p1.y);
                    y = min_i.y;
                } else if (outcodeOut & 2) {  // point is to the right of clip window
                    y = p1.y + (p2.y - p1.y) * (max_i.x - p1.x) / (p2.x - p1.x);
                    x = max_i.x;
                } else if (outcodeOut & 1) {   // point is to the left of clip window
                    y = p1.y + (p2.y - p1.y) * (min_i.x - p1.x) / (p2.x - p1.x);
                    x = min_i.x;
                }

                // Now we move outside point to intersection point to clip
                // and get ready for next pass.
                if (outcodeOut == outcode0) {
                    p1.x = x;
                    p1.y = y;
                    outcode0 = ComputeSideSquare(p1, min_i, max_i);
                } else {
                    p2.x = x;
                    p2.y = y;
                    outcode1 = ComputeSideSquare(p2, min_i, max_i);
                }
            }
        }
        return accept;
}

vec2 ClosestPtPointTriangle2D(vec2 p, vec2 p0, vec2 p1, vec2 p2){

    vec2 ab = v2_sub(p1, p0);
    vec2 ac = v2_sub(p2, p0);
    vec2 ap = v2_sub(p, p0);

    float d1 = v2_dot(ab, ap);
    float d2 = v2_dot(ac, ap);
    if (d1 <= 0.0f && d2 <= 0.0f)
        return p0;

    vec2 bp = v2_sub(p, p1);
    float d3 = v2_dot(ab, bp);
    float d4 = v2_dot(ac, bp);
    if (d3 >= 0.0f && d4 <= d3)
        return p1;

    float vc = d1*d4 - d3*d2;
    if(vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f){
        float v = d1 / (d1 - d3);
        return v2_add(p0, v2_muls(ab, v));
    }

    vec2 cp = v2_sub(p, p2);
    float d5 = v2_dot(ab, cp);
    float d6 = v2_dot(ac, cp);
    if (d6 >= 0.0f && d5 <= d6) return p2;

    float vb = d5*d2 - d1*d6;
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
        float w = d2 / (d2 - d6);
        return v2_add(p0,v2_muls(ac, w)); // barycentric coordinates (1-w,0,w)
    }
    // Check if P in edge region of BC, if so return projection of P onto BC
    float va = d3*d6 - d5*d4;
    if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
        float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return v2_add(p1,v2_muls(v2_sub(p2, p1), w)); // barycentric coordinates (0,1-w,w)
    }
    // P inside face region. Compute Q through its barycentric coordinates (u,v,w)
    float denom = 1.0f / (va + vb + vc);
    float v = vb * denom;
    float w = vc * denom;

    return v2_add(p0, v2_add(v2_muls(ab, v), v2_muls(ac, w)));

}

int IntersectionSquarePlane(InterSquareParam *box, vec2 n, float d)
{
    vec2 max = {box->position.x + box->size, box->position.y + box->size };
    vec2 min = {box->position.x - box->size, box->position.y - box->size };

    // These two lines not necessary with a (center, extents) AABB representation
    vec2 c = v2_muls(v2_add(max, min), 0.5f); // Compute AABB center
    vec2 e = v2_sub(max, c); // Compute positive extents
    // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
    float r = e.x*fabs(n.x) + e.y*fabs(n.y);
    // Compute distance of box center from plane
    float s = v2_dot(n, c) - d;
    // Intersection occurs when distance s falls within [-r,+r] interval
    return fabs(s) <= r;
}

int IntersectionCircleTriangle(vec2 sPos, float r, vec2 p0, vec2 p1, vec2 p2, vec2 *resPos, float *dist, float *depth, vec2 *dir)
{
    // Find point P on triangle ABC closest to sphere center
    *resPos = ClosestPtPointTriangle2D(sPos, p0, p1, p2);
    vec2 v = v2_sub(*resPos, sPos);

    *dist = v2_dot(v,v);

    int intr = *dist <= r * r;

    *dir = intr ? v2_add(*dir, v2_muls(v2_norm(v), -0.1f)) : *dir;
    *depth = intr ? r * r - *dist : 0;
    return intr;
}

int IntersectionCircleCircle(InterCircleParam *o1, InterCircleParam *o2, float *dist, float *depth, vec2 *dir)
{
    float r1 = o1->radius * 0.58f, r2 = o2->radius * 0.58f;

    vec2 v = v2_sub(o2->center, o1->center);

    *dist = v2_dot(v, v);

    float rSumm = r1 + r2;

    int intr = *dist <= rSumm * rSumm;

    *dir = intr ? v2_muls(v2_norm(v), -1) : *dir;

    *depth = intr ? rSumm * rSumm - *dist : 0;

    return intr;
}

int IntersectionCircleSquare(InterCircleParam *circle, InterSquareParam *quad, float *dist, float *depth, vec2 *dir)
{
    float sqDist = SqDistPointSquare(circle->center, quad);

    vec2 v = v2_sub(quad->position, circle->center);

    *dist = sqDist;
    *dir = v2_muls(v2_norm(v), -1);
    float r = circle->radius;
    *depth = r * r - *dist;
    // Sphere and AABB intersect if the (squared) distance
    // between them is less than the (squared) sphere radius
    return sqDist <= r * r;

}

int IntersectionSquareSquare(InterSquareParam *box1, InterSquareParam *box2, float *dist, float *depth, vec2 *dir)
{
    vec2 min1 = {box1->position.x - box1->size, box1->position.y - box1->size};
    vec2 max1 = {box1->position.x + box1->size, box1->position.y + box1->size};

    vec2 min2 = {box2->position.x - box2->size, box2->position.y - box2->size};
    vec2 max2 = {box2->position.x + box2->size, box2->position.y + box2->size};

    vec2 v = v2_sub(box2->position, box1->position);

    if (max1.x < min2.x || min1.x > max2.x) return 0;
    if (max1.y < min2.y || min1.y > max2.y) return 0;

    *dist = SqDistPointSquare(box1->position, box2);
    *dir = v2_muls(v2_norm(v), -1);
    *depth = 0.01f;

    return 1;
}

Point2D GetVertices2D(float rot, vec2 pos, vec2 size, vec3 color){
	Point2D v;
    memset(&v, 0, sizeof(Point2D));
    v.num_points = 4;

    mat2 orr = m2_rotate(rot);
	const float* o = orr.arr;
	vec2 A[] = {			// OBB Axis
		{o[0], o[1]},
		{o[3], o[4]},
	};
	
	vec2 t1 = v2_muls(A[0], size.x);
	vec2 t2 = v2_muls(A[1], size.y);

	v.points[0] = v2_add(pos, m2_v2_mult(orr, v2_muls(size, -1)));
	v.points[1] = v2_add(pos, m2_v2_mult(orr, vec2_f(-size.x, size.y)));
	v.points[2] = v2_add(pos, m2_v2_mult(orr, vec2_f(size.x, size.y)));
	v.points[3] = v2_add(pos, m2_v2_mult(orr, vec2_f(size.x, -size.y)));

    for(int i=0; i < 4;i++)
        GUIAddCircleFilled(v.points[i], 4, vec4_f(color.x, color.y, color.z, 1.0f), 0);

    v.center = pos;
	return v;
}

Point2D CheckContact(Point2D *a, Point2D *b, float *d_points){

    Point2D result;
    memset(&result, 0, sizeof(Point2D));

    int res = 0;
    float t;
    vec2 t_v;
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            res = IntersectLineToLine(a->points[i], a->points[i < 3 ? i + 1 : 0], b->points[j], b->points[j < 3 ? j + 1 : 0], &t, &t_v);

            if(res){
                result.points[result.num_points] = t_v;
                d_points[result.num_points] = t - FLT_EPSILON;
                result.num_points ++;
            }
        }
    }

    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++){
            res = IntersectLineToLine(b->points[i], b->points[i < 3 ? i + 1 : 0], a->points[j], a->points[j < 3 ? j + 1 : 0], &t, &t_v);

            if(res){
                result.points[result.num_points] = t_v;
                d_points[result.num_points] = t - FLT_EPSILON;
                result.num_points ++;
            }
        }
    }

    vec2 t_points[32];
    float t_d_points[32];
    uint32_t t_num_points = result.num_points;
    result.num_points = 0; 
    memcpy(t_points, result.points, sizeof(vec2) * 32);
    memcpy(t_d_points, d_points, sizeof(float) * 32);
    memset(result.points, 0, sizeof(vec2) * 32);
    memset(d_points, 0, sizeof(float) * 32);

    int finded = false;
    for(int i=0;i < t_num_points;i++){
        finded = false;
        for(int j=0;j < result.num_points;j++){
            if(v2_distance(t_points[i], result.points[j]) < 3.0f){
                finded = true;
                break;
            }
        }

        if(!finded){
            result.points[result.num_points] = t_points[i];
            d_points[result.num_points] = t_d_points[i];
            result.num_points ++;
        }        
    }

    return result;
}

Manfloid2D IntersectionSquareOOBSquareOOB(void *sq1, void *sq2)
{

    GameObject2D *go1 = (GameObject2D *)sq1;
    GameObject2D *go2 = (GameObject2D *)sq2;

    vec2 pos1 = Transform2DGetPosition(go1);
    vec2 pos2 = Transform2DGetPosition(go2);
    float rot1 = Transform2DGetRotation(go1);
    float rot2 = Transform2DGetRotation(go2);

    Point2D points1 = GetVertices2D(rot1, pos1, Transform2DGetScale(go1), vec3_f(1, 0, 0));
    Point2D points2 = GetVertices2D(rot2, pos2, Transform2DGetScale(go2), vec3_f(0, 1, 0));
    
    Manfloid2D result;
    result.collisions = CheckContact(&points1, &points2, result.depths);
    
    return result;
}

int IntersectionTriangleSquare(InterTriangleParam triangle, InterSquareParam *box)
{
    vec2 min_t = {box->position.x - box->size, box->position.y - box->size };
    vec2 max_t = {box->position.x + box->size, box->position.y + box->size };
    // Compute box center and extents (if not already given in that format)
    vec2 c = v2_muls(v2_add(min_t, max_t), 0.5f);

    // Translate triangle as conceptually moving AABB to origin
    vec2 v0 = v2_sub(triangle.p1, c);
    vec2 v1 = v2_sub(triangle.p2, c);
    vec2 v2 = v2_sub(triangle.p3, c);


    bool r1 = IntersectionSegmentSquare(v0, v1, box);
    bool r2 = IntersectionSegmentSquare(v1, v2, box);
    bool r3 = IntersectionSegmentSquare(v2, v0, box);

    if(r1 && r2 && r3)
        return true;
    else
        return false;
}

int IntersectionSphapeSquare(void *obj1, InterSquareParam *square)
{
    GameObject2D *shape = obj1;

    vertexParam *vParam = &shape->graphObj.shapes[0].vParam;
    indexParam *iParam = &shape->graphObj.shapes[0].iParam;


    InterTriangleParam triangle;

    vec2 wind_size = {engine.width, engine.height};

    for(int i=0;i< iParam->indexesSize; i+=3)
    {
        Vertex2D * vert = vParam->vertices;
        triangle.p1 = v2_mul(v2_add(vert[iParam->indices[i + 0]].position, shape->transform.position), wind_size);
        triangle.p2 = v2_mul(v2_add(vert[iParam->indices[i + 1]].position, shape->transform.position), wind_size);
        triangle.p3 = v2_mul(v2_add(vert[iParam->indices[i + 2]].position, shape->transform.position), wind_size);

        if(IntersectionTriangleSquare(triangle, square))
            return true;
    }

    return false;
}

int IntersectionShapeShape(void *obj1, void *obj2)
{
    GameObject2D *shape1 = obj1;
    GameObject2D *shape2 = obj2;

    vertexParam *vParam1 = &shape1->graphObj.shapes[0].vParam;
    vertexParam *vParam2 = &shape2->graphObj.shapes[0].vParam;

    indexParam *iParam1 = &shape1->graphObj.shapes[0].iParam;
    indexParam *iParam2 = &shape2->graphObj.shapes[0].iParam;

    InterTriangleParam triangle1, triangle2;

    for(int i=0;i < iParam1->indexesSize; i+=3)
    {
        Vertex2D *vert = vParam1->vertices;
        triangle1.p1 = v2_add(vert[iParam1->indices[i + 0]].position, shape1->transform.position);
        triangle1.p2 = v2_add(vert[iParam1->indices[i + 1]].position, shape1->transform.position);
        triangle1.p3 = v2_add(vert[iParam1->indices[i + 2]].position, shape1->transform.position);

        for(int j=0;j < iParam2->indexesSize; j+=3)
        {
            vert = vParam2->vertices;
            triangle2.p1 = v2_add(vert[iParam2->indices[j + 0]].position, shape2->transform.position);
            triangle2.p2 = v2_add(vert[iParam2->indices[j + 1]].position, shape2->transform.position);
            triangle2.p3 = v2_add(vert[iParam2->indices[j + 2]].position, shape2->transform.position);

            if(ComputeTwoTrianles(triangle1, triangle2))
                return true;

        }
    }

    return false;
}

