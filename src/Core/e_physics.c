#include "Core/e_physics.h"

#include "Tools/e_math.h"
#include "Tools/intersections3D.h"

#include <float.h>

#define CMP(x, y) \
	(fabsf(x - y) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))

#define GRAVITY_CONST vec3_f(0.0f, -9.82f, 0.0f)

typedef struct{
    RigidBody *rigids[32];
    uint32_t num_rigids;
} Collider;

typedef struct{
    CollisionManifold colls[32];
    uint32_t num_colls;
} ResultCollider;

Collider colliders1;
Collider colliders2;
ResultCollider results;

float LinearProjectionPercent = 0.45f;
float PenetrationSlack = 0.01f;
int ImpulseIteration = 5;

RigidBody phy_objs[32];
uint32_t num_phy_objs = 0;

void ResetCollisionManifold(CollisionManifold* result) {
    memset(result, 0, sizeof(CollisionManifold));
	if (result != 0) {
		result->colliding = false;
		result->normal = vec3_f(0, 0, 1);
		result->depth = FLT_MAX;
	}
}


vec3 RigidBodyGetMin(const ColParams *aabb) {
	vec3 p1 = v3_add(aabb->position, aabb->size);
	vec3 p2 = v3_sub(aabb->position, aabb->size);

	return vec3_f(fminf(p1.x, p2.x), fminf(p1.y, p2.y), fminf(p1.z, p2.z));
}
vec3 RigidBodyGetMax(const ColParams *aabb) {
	vec3 p1 = v3_add(aabb->position, aabb->size);
	vec3 p2 = v3_sub(aabb->position, aabb->size);

	return vec3_f(fmaxf(p1.x, p2.x), fmaxf(p1.y, p2.y), fmaxf(p1.z, p2.z));
}

vec3 RigidBodyClosestPoint(const ColParams *obb, const vec3 *point) {
	vec3 result = obb->position;
	vec3 dir = v3_sub(*point, obb->position);

    float *arr = &obb->size;

	for (int i = 0; i < 3; ++i) {
		const float* orientation = &obb->orientation.arr[i * 3];
		vec3 axis = vec3_f(orientation[0], orientation[1], orientation[2]);

		float distance = v3_dot(dir, axis);

		if (distance > arr[i]) {
			distance = arr[i];
		}
		if (distance < -arr[i]) {
			distance = -arr[i];
		}

		result = v3_add(result, v3_muls(axis, distance));
	}

	return result;
}

bool RigidBodyPointInOBB(const vec3 point, const ColParams *obb) {
	vec3 dir = v3_sub(point, obb->position);

    float *arr = &obb->size;

	for (int i = 0; i < 3; ++i) {
		const float* orientation = &obb->orientation.arr[i * 3];
		vec3 axis = vec3_f(orientation[0], orientation[1], orientation[2]);

		float distance = v3_dot(dir, axis);

		if (distance > arr[i]) {
			return false;
		}
		if (distance < -arr[i]) {
			return false;
		}
	}

	return true;
}

float RigidBodyInvMass(RigidBody *rb) {
	if (rb->mass == 0.0f) {
		return 0.0f;
	}
	return 1.0f / rb->mass;
}

void RigidBodySynchCollisionVolumes(RigidBody *rb) {    
	rb->params.orientation = m3_rotation_matrix(rb->params.rotating);
}


mat4 RigidBodyInvTensor(RigidBody *rb) {
	if (rb->mass == 0) {
		return mat4_rowsf(
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		);
	}
	float ix = 0.0f;
	float iy = 0.0f;
	float iz = 0.0f;
	float iw = 0.0f;

	if (rb->mass != 0 && rb->type == TIGOR_RIGIDBODY_TYPE_SPHERE) {
		float r2 = rb->params.radius * rb->params.radius;
		float fraction = (2.0f / 5.0f);

		ix = r2 * rb->mass * fraction;
		iy = r2 * rb->mass * fraction;
		iz = r2 * rb->mass * fraction;
		iw = 1.0f;
	}
	else if (rb->mass != 0 && rb->type == TIGOR_RIGIDBODY_TYPE_BOX) {
		vec3 size = v3_muls(rb->params.size, 2.0f);
		float fraction = (1.0f / 12.0f);

		float x2 = size.x * size.x;
		float y2 = size.y * size.y;
		float z2 = size.z * size.z;

		ix = (y2 + z2) * rb->mass * fraction;
		iy = (x2 + z2) * rb->mass * fraction;
		iz = (x2 + y2) * rb->mass * fraction;
		iw = 1.0f;
	}

	return m4_inv(mat4_rowsf(
		ix, 0, 0, 0,
		0, iy, 0, 0,
		0, 0, iz, 0,
		0, 0, 0, iw));
}

void RigidBodyUpdate(RigidBody *rb, float dt) {
	// Integrate velocity
	const float damping = 0.98f;

	vec3 acceleration = v3_muls(rb->forces, RigidBodyInvMass(rb));
	rb->velocity = v3_add(rb->velocity, v3_muls(acceleration, dt));
	rb->velocity = v3_muls(rb->velocity, damping);

	if (fabsf(rb->velocity.x) < 0.001f) {
		rb->velocity.x = 0.0f;
	}
	if (fabsf(rb->velocity.y) < 0.001f) {
		rb->velocity.y = 0.0f;
	}
	if (fabsf(rb->velocity.z) < 0.001f) {
		rb->velocity.z = 0.0f;
	}

	if (rb->type == TIGOR_RIGIDBODY_TYPE_BOX) {
		vec3 angAccel = m4_v3_mult(RigidBodyInvTensor(rb), rb->torques);
		rb->angVel = v3_add(rb->angVel, v3_muls(angAccel, dt));
		rb->angVel = v3_muls(rb->angVel, damping);

		if (fabsf(rb->angVel.x) < 0.001f) {
			rb->angVel.x = 0.0f;
		}
		if (fabsf(rb->angVel.y) < 0.001f) {
			rb->angVel.y = 0.0f;
		}
		if (fabsf(rb->angVel.z) < 0.001f) {
			rb->angVel.z = 0.0f;
		}
	}

	// Integrate position
	rb->params.position = v3_add(rb->params.position, v3_muls(rb->velocity, dt));

	if (rb->type == TIGOR_RIGIDBODY_TYPE_BOX) {
		rb->params.rotating = v3_add(rb->params.rotating, v3_muls(v3_muls(rb->angVel, dt), 180.0f / M_PI));
	}

	RigidBodySynchCollisionVolumes(rb);
}

Interval RigidBodyGetInterval(const ColParams *aabb, const vec3 *axis) {
	vec3 i = RigidBodyGetMin(aabb);
	vec3 a = RigidBodyGetMax(aabb);

	vec3 vertex[8] = {
		{i.x, a.y, a.z},
		{i.x, a.y, i.z},
		{i.x, i.y, a.z},
		{i.x, i.y, i.z},
		{a.x, a.y, a.z},
		{a.x, a.y, i.z},
		{a.x, i.y, a.z},
		{a.x, i.y, i.z}
	};

	Interval result;
	result.min = result.max = v3_dot(*axis, vertex[0]);

	for (int i = 1; i < 8; ++i) {
		float projection = v3_dot(*axis, vertex[i]);
		result.min = (projection < result.min) ? projection : result.min;
		result.max = (projection > result.max) ? projection : result.max;
	}

	return result;
}

float RigidBodyPenetrationDepth(const ColParams *o1, const ColParams *o2, const vec3 *axis, bool *outShouldFlip) {
    vec3 norm = v3_norm(*axis);
	Interval i1 = RigidBodyGetInterval(o1, &norm);
	Interval i2 = RigidBodyGetInterval(o2, &norm);

	if (!((i2.min <= i1.max) && (i1.min <= i2.max))) {
		return 0.0f; // No penerattion
	}

	float len1 = i1.max - i1.min;
	float len2 = i2.max - i2.min;
	float min = fminf(i1.min, i2.min);
	float max = fmaxf(i1.max, i2.max);
	float length = max - min;

	if (outShouldFlip != 0) {
		*outShouldFlip = (i2.min < i1.min);
	}

	return (len1 + len2) - length;
}

Point RigidBodyGetVertices(const ColParams *obb) {
	Point v;
    memset(&v, 0, sizeof(Point));
    v.num_points = 8;

	vec3 C = obb->position;	// OBB Center
	vec3 E = obb->size;		// OBB Extents
	const float* o = obb->orientation.arr;
	vec3 A[] = {			// OBB Axis
		{o[0], o[1], o[2]},
		{o[3], o[4], o[5]},
		{o[6], o[7], o[8]},
	};

	v.points[0] = v3_add(C, v3_add(v3_add(v3_muls(A[0], E.x), v3_muls(A[1], E.y)), v3_muls(A[2], E.z)));
	v.points[1] = v3_sub(C, v3_add(v3_add(v3_muls(A[0], E.x), v3_muls(A[1], E.y)), v3_muls(A[2], E.z)));
	v.points[2] = v3_add(C, v3_add(v3_sub(v3_muls(A[0], E.x), v3_muls(A[1], E.y)), v3_muls(A[2], E.z)));
	v.points[3] = v3_add(C, v3_sub(v3_add(v3_muls(A[0], E.x), v3_muls(A[1], E.y)), v3_muls(A[2], E.z)));
	v.points[4] = v3_sub(C, v3_sub(v3_sub(v3_muls(A[0], E.x), v3_muls(A[1], E.y)), v3_muls(A[2], E.z)));
	v.points[5] = v3_add(C, v3_sub(v3_sub(v3_muls(A[0], E.x), v3_muls(A[1], E.y)), v3_muls(A[2], E.z)));
	v.points[6] = v3_sub(C, v3_sub(v3_add(v3_muls(A[0], E.x), v3_muls(A[1], E.y)), v3_muls(A[2], E.z)));
	v.points[7] = v3_sub(C, v3_add(v3_sub(v3_muls(A[0], E.x), v3_muls(A[1], E.y)), v3_muls(A[2], E.z)));

	return v;
}

LineArr RigidBodyGetEdges(const ColParams *obb) {
	LineArr result;
    memset(&result, 0, sizeof(LineArr));
	result.num_lines = 12;
	Point v = RigidBodyGetVertices(obb);

	int index[][2] = { // Indices of edges
		{ 6, 1 },{ 6, 3 },{ 6, 4 },{ 2, 7 },{ 2, 5 },{ 2, 0 },
		{ 0, 1 },{ 0, 3 },{ 7, 1 },{ 7, 4 },{ 4, 5 },{ 5, 3 }
	};

	for (int j = 0; j < 12; ++j) {
		result.lines[j].start = v.points[index[j][0]];
		result.lines[j].end = v.points[index[j][1]];
	}

	return result;
}

PlaneArr RigidBodyGetPlanes(const ColParams *obb) {
	vec3 c = obb->position;	// OBB Center
	vec3 e = obb->size;		// OBB Extents
	const float* o = obb->orientation.arr;
	vec3 a[] = {			// OBB Axis
		{o[0], o[1], o[2]},
		{o[3], o[4], o[5]},
		{o[6], o[7], o[8]},
	};

	PlaneArr result;
    memset(&result, 0, sizeof(PlaneArr));
	result.num_planes = 6;

	result.planes[0].normal = a[0];
    result.planes[0].distance =  v3_dot(a[0], (v3_add(c, v3_muls(a[0], e.x))));
	result.planes[1].normal = v3_muls(a[0], -1.0f);
    result.planes[1].distance =  -v3_dot(a[0], (v3_sub(c, v3_muls(a[0], e.x))));
	result.planes[2].normal = a[1];
    result.planes[2].distance =  v3_dot(a[1], (v3_add(c, v3_muls(a[1], e.y))));
	result.planes[3].normal = v3_muls(a[1], -1.0);
    result.planes[3].distance =  -v3_dot(a[1], (v3_sub(c, v3_muls(a[1], e.y))));
	result.planes[4].normal = a[2];
    result.planes[4].distance =  v3_dot(a[2], (v3_add(c, v3_muls(a[2], e.z))));
	result.planes[5].normal = v3_muls(a[2], -1.0);
    result.planes[5].distance =  -v3_dot(a[2], (v3_sub(c, v3_muls(a[2], e.z))));

	return result;
}


bool RigidBodyClipToPlane(const Plane *plane, const Line *line, vec3 *outPoint) {
	vec3 ab = v3_sub(line->end,  line->start);

	float nA = v3_dot(plane->normal, line->start);
	float nAB = v3_dot(plane->normal, ab);

	if (CMP(nAB, 0)) {
		return false;
	}

	float t = (plane->distance - nA) / nAB;
	if (t >= 0.0f && t <= 1.0f) {
		if (outPoint != 0) {
			*outPoint = v3_add(line->start, v3_muls(ab, t));
		}
		return true;
	}

	return false;
}

Point RigidBodyClipEdgesToOBB(const LineArr *edges, const ColParams *obb) {
	Point result;
    memset(&result, 0, sizeof(Point));
	result.num_points = edges->num_lines * 3;
	vec3 intersection;

	PlaneArr planes = RigidBodyGetPlanes(obb);

    int iter = 0;
	for (int i = 0; i < planes.num_planes; ++i) {
		for (int j = 0; j < edges->num_lines; ++j) {
			if (RigidBodyClipToPlane(&planes.planes[i], &edges->lines[j], &intersection)) {
				if (RigidBodyPointInOBB(intersection, obb)) {
					result.points[iter] = intersection;
                    iter ++;
				}
			}
		}
	}

	return result;
}

void SomeEraseContact(CollisionManifold *coll, int indx){

    int diff = coll->num_contacts - indx;

    vec3 arr[diff];

    memcpy(arr, &coll->contacts[indx], diff);

    memcpy(&coll->contacts[indx], arr + 1, diff - 1);

    coll->num_contacts --;

}

CollisionManifold FindCollisionFeaturesOBBOOBB(const ColParams *A, const ColParams *B) {
	CollisionManifold result; // Will return result of intersection!
	ResetCollisionManifold(&result);

	InterSphereParam s1;
	InterSphereParam s2;

    s1.center = A->position;
    s1.radius = v3_magnitude(&A->size);
    
    s2.center = B->position;
    s2.radius = v3_magnitude(&B->size);

	if (!IntersectionSphereToSphereL(&s1, &s2)) {
		return result;
	}

	const float* o1 = A->orientation.arr;
	const float* o2 = B->orientation.arr;

	vec3 test[15] = {
		{o1[0], o1[1], o1[2]},
		{o1[3], o1[4], o1[5]},
		{o1[6], o1[7], o1[8]},
		{o2[0], o2[1], o2[2]},
		{o2[3], o2[4], o2[5]},
		{o2[6], o2[7], o2[8]}
	};

	for (int i = 0; i < 3; ++i) { // Fill out rest of axis
		test[6 + i * 3 + 0] = v3_cross(test[i], test[0]);
		test[6 + i * 3 + 1] = v3_cross(test[i], test[1]);
		test[6 + i * 3 + 2] = v3_cross(test[i], test[2]);
	}

	vec3* hitNormal = 0;
	bool shouldFlip;

	for (int i = 0; i < 15; ++i) {
		if (test[i].x < 0.000001f) test[i].x = 0.0f;
		if (test[i].y < 0.000001f) test[i].y = 0.0f;
		if (test[i].z < 0.000001f) test[i].z = 0.0f;
		if (v3_magnitudesq(&test[i])< 0.001f) {
			continue;
		}
		float depth = RigidBodyPenetrationDepth(A, B, &test[i], &shouldFlip);
		if (depth <= 0.0f) {
			return result;
		}
		else if (depth < result.depth) {
			if (shouldFlip) {
				test[i] = v3_muls(test[i], -1.0f);
			}
			result.depth = depth;
			hitNormal = &test[i];
		}
	}

	if (hitNormal == 0) {
		return result;
	}
	vec3 axis = v3_norm(*hitNormal);

    LineArr t1 = RigidBodyGetEdges(A);
    LineArr t2 = RigidBodyGetEdges(B);

	Point c1 = RigidBodyClipEdgesToOBB(&t1, A);
	Point c2 = RigidBodyClipEdgesToOBB(&t2, B);
    
    memcpy(&result.contacts[result.num_contacts], &c1.points, c1.num_points);
    result.num_contacts += c1.num_points;
    memcpy(&result.contacts[result.num_contacts], &c2.points, c2.num_points);
    result.num_contacts += c2.num_points;

	Interval i = RigidBodyGetInterval(A, &axis);
	float distance = (i.max - i.min)* 0.5f - result.depth * 0.5f;
	vec3 pointOnPlane = v3_add(A->position, v3_muls(axis, distance));

    vec3 temp;
	
	for (int i = result.num_contacts - 1; i >= 0; --i) {
		vec3 contact = result.contacts[i];
		result.contacts[i] = v3_add(contact, v3_muls(axis, v3_dot(axis, v3_sub(pointOnPlane, contact))));
		
		// This bit is in the "There is more" section of the book
		for (int j = result.num_contacts - 1; j > i; --j) {
            temp = v3_sub(result.contacts[j], result.contacts[i]);
			if (v3_magnitudesq(&temp) < 0.0001f) {
				SomeEraseContact(&result, j);
				break;
			}
		}
	}

	result.colliding = true;
	result.normal = axis;

	return result;
}

CollisionManifold FindCollisionFeaturesSphereSphere(const ColParams *A, const ColParams *B) {
	CollisionManifold result; // Will return result of intersection!
	ResetCollisionManifold(&result);

	float r = A->radius + B->radius;
	vec3 d = v3_sub(B->position, A->position);

	if (v3_magnitudesq(&d) - r * r > 0 || v3_magnitudesq(&d) == 0.0f) {
		return result;
	}
	v3_norm(d);

	result.colliding = true;
	result.normal = d;
	result.depth = fabsf(v3_magnitude(&d) - r) * 0.5f;
	
	// dtp - Distance to intersection point
	float dtp = A->radius - result.depth;
	vec3 contact = v3_add(A->position, v3_muls(d, dtp));
	
	result.contacts[result.num_contacts] = contact;
    result.num_contacts ++;

	return result;
}

CollisionManifold FindCollisionFeaturesOBBSphere(const ColParams *obb, const ColParams *sphere) {
	CollisionManifold result; // Will return result of intersection!
	ResetCollisionManifold(&result);

	vec3 closestPoint = RigidBodyClosestPoint(obb, &sphere->position);

    vec3 temp = v3_sub(closestPoint, sphere->position);

	float distanceSq = v3_magnitudesq(&temp);
	if (distanceSq > sphere->radius * sphere->radius) {
		return result;
	}

	vec3 normal; 
	if (CMP(distanceSq, 0.0f)) {
        temp = v3_sub(closestPoint, obb->position);
		if (CMP(v3_magnitudesq(&temp), 0.0f)) {
			return result;

		}
		// Closest point is at the center of the sphere
		normal = v3_norm(v3_sub(closestPoint, obb->position));
	}
	else {
		normal = v3_norm(v3_sub(sphere->position, closestPoint));
	}

	vec3 outsidePoint = v3_sub(sphere->position, v3_muls(normal, sphere->radius));

    temp = v3_sub(closestPoint, outsidePoint);
	float distance = v3_magnitude(&temp);

	result.colliding = true;
	result.contacts[result.num_contacts] = v3_add(closestPoint, v3_muls(v3_sub(outsidePoint, closestPoint), 0.5f));
    result.num_contacts ++;
	result.normal = normal;
	result.depth = distance * 0.5f;

	return result;
}

CollisionManifold FindCollisionFeatures(RigidBody *ra, RigidBody *rb) {
	CollisionManifold result;
	ResetCollisionManifold(&result);

	if (ra->type == TIGOR_RIGIDBODY_TYPE_SPHERE) {
		if (rb->type == TIGOR_RIGIDBODY_TYPE_SPHERE) {
			result = FindCollisionFeaturesSphereSphere(&ra->params, &rb->params);
		}
		else if (rb->type == TIGOR_RIGIDBODY_TYPE_BOX) {
			result = FindCollisionFeaturesOBBSphere(&rb->params, &ra->params);
			result.normal = v3_muls(result.normal, -1.0f);
		}
	}
	else if (ra->type == TIGOR_RIGIDBODY_TYPE_BOX) {
		if (rb->type == TIGOR_RIGIDBODY_TYPE_BOX) {
			result = FindCollisionFeaturesOBBOOBB(&ra->params, &rb->params);
		}
		else if (rb->type == TIGOR_RIGIDBODY_TYPE_SPHERE) {
			result = FindCollisionFeaturesOBBSphere(&ra->params, &rb->params);
		}
	}


	return result;
}

RigidBody *PhysicsInitObject(uint32_t type, bool isDynamic){
    RigidBody *res = &phy_objs[num_phy_objs];
    memset(res, 0, sizeof(RigidBody));

    res->type = type;
    res->isDynamic = isDynamic;

    res->params.radius = 1.0f;
    res->params.size = vec3_f(1, 1, 1);
    res->mass = 1.0f;
    res->friction = 0.6f;
    res->cor = 0.5f;

    num_phy_objs ++;

    return res;
}

void RigidBodyApplyForces(RigidBody *rb) {
	rb->forces = v3_muls(GRAVITY_CONST, rb->mass);
}


void RigidBodyApplyImpulse(RigidBody *A, RigidBody *B, const CollisionManifold *M, int c) {
	// Linear impulse
	float invMass1 = RigidBodyInvMass(A);
	float invMass2 = RigidBodyInvMass(B);
	float invMassSum = invMass1 + invMass2;

	if (invMassSum == 0.0f) {
		return; // Both objects have infinate mass!
	}

	vec3 r1 = v3_sub(M->contacts[c], A->params.position);
	vec3 r2 = v3_sub(M->contacts[c], B->params.position);
	mat4 i1 = RigidBodyInvTensor(A);
	mat4 i2 = RigidBodyInvTensor(B);

	vec3 relativeVel = v3_sub(v3_add(B->velocity, v3_cross(B->angVel, r2)), v3_add(A->velocity, v3_cross(A->angVel, r1)));

	// Relative collision normal
	vec3 relativeNorm = M->normal;
	relativeNorm = v3_norm(relativeNorm);

	// Moving away from each other? Do nothing!
	if (v3_dot(relativeVel, relativeNorm) > 0.0f) {
		return;
	}

	float e = fminf(A->cor, B->cor);

	float numerator = (-(1.0f + e) * v3_dot(relativeVel, relativeNorm));
	float d1 = invMassSum;
#ifndef LINEAR_ONLY
	vec3 d2 = v3_cross(m4_v3_mult(i1, v3_cross(r1, relativeNorm)), r1);
	vec3 d3 = v3_cross(m4_v3_mult(i2, v3_cross(r2, relativeNorm)), r2);
	float denominator = d1 + v3_dot(relativeNorm, v3_add(d2, d3));
#else
	float denominator = d1;
#endif

	float j = (denominator == 0.0f) ? 0.0f : numerator / denominator;
	if (M->num_contacts > 0.0f && j != 0.0f) {
		j /= (float)M->num_contacts;
	}

	vec3 impulse = v3_muls(relativeNorm, j);
	A->velocity = v3_sub(A->velocity, v3_muls(impulse, invMass1));
	B->velocity = v3_add(B->velocity, v3_muls(impulse, invMass2));

#ifndef LINEAR_ONLY
	A->angVel = v3_sub(A->angVel, m4_v3_mult(i1, v3_cross(r1, impulse)));
	B->angVel = v3_add(B->angVel, m4_v3_mult(i2, v3_cross(r2, impulse)));
#endif

	// Friction
	vec3 t = v3_sub(relativeVel, v3_muls(relativeNorm, v3_dot(relativeVel, relativeNorm)));
	if (CMP(v3_magnitudesq(&t), 0.0f)) {
		return;
	}
	t = v3_norm(t);

	numerator = -v3_dot(relativeVel, t);
	d1 = invMassSum;
#ifndef LINEAR_ONLY
	d2 = v3_cross(m4_v3_mult(i1, v3_cross(r1, t)), r1);
	d3 = v3_cross(m4_v3_mult(i2, v3_cross(r2, t)), r2);
	denominator = d1 + v3_dot(t, v3_add(d2, d3));
#else
	denominator = d1;
#endif

	float jt = (denominator == 0.0f) ? 0.0f : numerator / denominator;
	if (M->num_contacts > 0.0f && jt != 0.0f) {
		jt /= (float)M->num_contacts;
	}

	if (CMP(jt, 0.0f)) {
		return;
	}

	vec3 tangentImpuse;
#ifdef DYNAMIC_FRICTION
	float sf = sqrtf(A.staticFriction * B.staticFriction);
	float df = sqrtf(A.dynamicFriction * B.dynamicFriction);
	if (fabsf(jt) < j * sf) {
		tangentImpuse = t * jt;
	}
	else {
		tangentImpuse = t * -j * df;
	}
#else
	float friction = sqrtf(A->friction * B->friction);
	if (jt > j * friction) {
		jt = j * friction;
	}
	else if (jt < -j * friction) {
		jt = -j * friction;
	}
	tangentImpuse = v3_muls(t, jt);
#endif

	A->velocity = v3_sub(A->velocity, v3_muls(tangentImpuse, invMass1));
	B->velocity = v3_add(B->velocity, v3_muls(tangentImpuse, invMass2));

#ifndef LINEAR_ONLY
	A->angVel = v3_sub(A->angVel, m4_v3_mult(i1, v3_cross(r1, tangentImpuse)));
	B->angVel = v3_add(B->angVel, m4_v3_mult(i2, v3_cross(r2, tangentImpuse)));
#endif
}

void PhysicsUpdate(float deltaTime){
    memset(&colliders1, 0, sizeof(Collider));
    memset(&colliders2, 0, sizeof(Collider));
    memset(&results, 0, sizeof(ResultCollider));

	{ // Find objects whom are colliding
	  // First, build a list of colliding objects
		CollisionManifold result;
		for (int i = 0, size = num_phy_objs; i < size; ++i) {
			for (int j = i; j < size; ++j) {
				if (i == j) {
					continue;
				}
	            ResetCollisionManifold(&result);
				if (phy_objs[i].isDynamic && phy_objs[j].isDynamic) {
					result = FindCollisionFeatures(&phy_objs[i], &phy_objs[j]);
				}
				if (result.colliding) {
#if 0 
					bool isDuplicate = false;
					for (int k = 0, kSize = colliders1.size(); k < kSize; ++k) {
						if (colliders1[k] == bodies[i] || colliders1[k] == bodies[j]) {
							if (colliders2[k] == bodies[i] || colliders2[k] == bodies[j]) {
								isDuplicate = true;
								break;
							}
						}
					}

					if (!isDuplicate) {
						for (int k = 0, kSize = colliders2.size(); k < kSize; ++k) {
							if (colliders2[k] == bodies[i] || colliders2[k] == bodies[j]) {
								if (colliders1[k] == bodies[i] || colliders1[k] == bodies[j]) {
									isDuplicate = true;
									break;
								}
							}
						}
					}
					if (!isDuplicate)
#endif

					{
                        colliders1.rigids[colliders1.num_rigids] = &phy_objs[i];
                        colliders1.num_rigids ++;
                        colliders2.rigids[colliders2.num_rigids] = &phy_objs[j];
                        colliders2.num_rigids ++;                        
						results.colls[results.num_colls] = result;
                        results.num_colls ++;
					}
				}
			}
		}
	}

	// Calculate foces acting on the object
	for (int i = 0, size = num_phy_objs; i < size; ++i) {
		RigidBodyApplyForces(&phy_objs[i]);
	}

	// Apply impulses to resolve collisions
	for (int k = 0; k < ImpulseIteration; ++k) { // Apply impulses
		for (int i = 0, size = results.num_colls; i < size; ++i) {
			for (int j = 0, jSize = results.colls[i].num_contacts; j < jSize; ++j) {
				if (colliders2.rigids[i]->isDynamic && colliders2.rigids[i]->isDynamic) {
					RigidBodyApplyImpulse(colliders1.rigids[i], colliders2.rigids[i], &results.colls[i], j);
				}
			}
		}
	}

	// Integrate velocity and impulse of objects
	for (int i = 0, size = num_phy_objs; i < size; ++i) {
		RigidBodyUpdate(&phy_objs[i], deltaTime);
	}


	// Correct position to avoid sinking!
	//if (DoLinearProjection) 
    {
		for (int i = 0, size = results.num_colls; i < size; ++i) {
			if (!colliders1.rigids[i]->isDynamic && !colliders2.rigids[i]->isDynamic) {
				continue;
			}

			float totalMass = RigidBodyInvMass(colliders1.rigids[i]) + RigidBodyInvMass(colliders2.rigids[i]);

			if (totalMass == 0.0f) {
				continue;
			}

			float depth = fmaxf(results.colls[i].depth - PenetrationSlack, 0.0f);
			float scalar = (totalMass == 0.0f) ? 0.0f : depth / totalMass;
			vec3 correction = v3_muls(results.colls[i].normal, scalar * LinearProjectionPercent);

			colliders1.rigids[i]->params.position = v3_sub(colliders1.rigids[i]->params.position, v3_muls(correction, RigidBodyInvMass(colliders1.rigids[i])));
			colliders2.rigids[i]->params.position = v3_sub(colliders2.rigids[i]->params.position, v3_muls(correction, RigidBodyInvMass(colliders2.rigids[i])));

            RigidBodySynchCollisionVolumes(colliders1.rigids[i]);
            RigidBodySynchCollisionVolumes(colliders2.rigids[i]);
		}
	}

	// Solve constraints
	for (int i = 0, size = num_phy_objs; i < size; ++i) {
		//bodies[i]->SolveConstraints(constraints);
	}

}

void PhysicsClear(){
    memset(phy_objs, 0, sizeof(RigidBody) * 32);
    num_phy_objs = 0;
}