#ifndef	PHYSICS
#define	PHYSICS


#include	<PxPhysicsAPI.h>


#define	PHYSICS_DEFAULT_GRAVITY	0.f, -1.f, 0.f


struct physicsmanager
{
	physx::PxDefaultAllocator default_alloc;
	physx::PxDefaultErrorCallback default_error;

	physx::PxPhysics* sdk;
	physx::PxFoundation* foundation;
	physx::PxCooking* cooking;

	physx::PxMaterial* default_material;

	physx::PxScene* scene;
};


void physics_startup(struct physicsmanager*);
void physics_update(struct physicsmanager*, float);
void physics_shutdown(struct physicsmanager*);

void physics_addstatic_trianglestrip(struct physicsmanager*, unsigned, unsigned, float*);


#endif