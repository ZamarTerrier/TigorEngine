#ifndef E_PHYSICS_H
#define E_PHYSICS_H

#include "Variabels/engine_includes.h"

typedef enum{
    TIGOR_RIGIDBODY_TYPE_SPHERE,
    TIGOR_RIGIDBODY_TYPE_BOX,
} RigidBodyType;

typedef struct{
    mat3 orientation;
    vec3 position;
    vec3 size;
    vec3 rotating;
    float radius;
} ColParams;

typedef struct{
	vec3 velocity;
	vec3 angVel;

	vec3 forces; // sumForces
	vec3 torques; // Sum torques

	float mass;
	float cor; // Coefficient of restitution

	//float staticFriction;
	//float dynamicFriction;

	float friction;

    ColParams params;
    uint32_t type;
    bool isDynamic;
} RigidBody;

typedef struct CollisionManifold {
	bool colliding;
	vec3 normal;
	float depth;
	vec3 contacts[1024];
    uint32_t num_contacts;
} CollisionManifold;

typedef struct{
    vec3 points[2048];
    uint32_t num_points;
} Point;

typedef struct{
    vec3 start;
    vec3 end;
} Line;

typedef struct{
    Line lines[512];
    uint32_t num_lines;
} LineArr;

typedef struct{
    vec3 normal;
    float distance;
} Plane;

typedef struct{
    Plane planes[256];
    uint32_t num_planes;
} PlaneArr;

typedef struct Interval {
	float min;
	float max;
} Interval;

void RigidBodySynchCollisionVolumes(RigidBody *rb);

RigidBody *PhysicsInitObject(uint32_t type, bool isDynamic);
void PhysicsUpdate(float deltaTime);
void PhysicsClear();

#endif // E_PHYSICS_H