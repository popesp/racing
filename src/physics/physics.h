#ifndef	PHYSICS
#define	PHYSICS


#include	<PxPhysicsAPI.h>
#include	"../math/vec3f.h"


#define	PHYSICS_DEFAULT_GRAVITY				0.f, -10.f, 0.f

#define	PHYSICS_VEHICLE_RAYCAST_COUNT		4
#define	PHYSICS_VEHICLE_RAYCAST_MAXDIST		0.5f
#define	PHYSICS_VEHICLE_RAYCAST_MAXFORCE	20.f

#define	PHYSICS_VEHICLE_DAMP_LINEAR			1.2f
#define	PHYSICS_VEHICLE_DAMP_ANGULAR		5.f


struct vehicle
{
	physx::PxRigidDynamic* body;

	vec3f origins[PHYSICS_VEHICLE_RAYCAST_COUNT];
	vec3f dir[PHYSICS_VEHICLE_RAYCAST_COUNT];
};

struct physicsmanager
{
	physx::PxDefaultAllocator default_alloc;
	physx::PxDefaultErrorCallback default_error;

	physx::PxPhysics* sdk;
	physx::PxFoundation* foundation;
	physx::PxCooking* cooking;
	physx::PxMaterial* default_material;

	physx::PxScene* scene;

	int num_vehicles;
	struct vehicle* vehicles;
};


/*	start up the physics manager
	param:	pm				physics manager (modified)
*/
void physicsmanager_startup(struct physicsmanager* pm);

/*	shut down the physics manager
	param:	pm				physics manager (modified)
*/
void physicsmanager_shutdown(struct physicsmanager* pm);


/*	update the physics simulation
	param:	pm				physics manager
	param:	dt				delta time
*/
void physicsmanager_update(struct physicsmanager* pm, float dt);


/*	add a vehicle to the simulation
	param:	pm				physics manager
	param:	pos				position of the vehicle
	param:	dim				dimensions of the bounding box
	return:	int				index for the vehicle object
*/
int physicsmanager_addvehicle(struct physicsmanager* pm, vec3f pos, vec3f dim);


/*	probably temporary	*/
physx::PxRigidDynamic* physics_adddynamic_box(struct physicsmanager*, vec3f, vec3f);

/*	add a static triangle mesh stored in a triangle strip vertex buffer
	param:	pm				physics manager
	param:	num_verts		number of vertices in the buffer
	param:	stride			stride between vertices in the buffer
	param:	buf_verts		vertex buffer
*/
void physicsmanager_addstatic_trianglestrip(struct physicsmanager* pm, unsigned num_verts, unsigned stride, float* buf_verts);


#endif