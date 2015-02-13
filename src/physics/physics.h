#ifndef	PHYSICS
#define	PHYSICS


#include	<PxPhysicsAPI.h>
#include	"../math/vec3f.h"
#include	"scene_query.h"
#include	"wheel_query_results.h"

#define	PHYSICS_DEFAULT_GRAVITY	0.f, -10.f, 0.f



#define TIRE_TYPE_WETS		0
#define	TIRE_TYPE_SLICKS	1


#define eFRONT_LEFT_WHEEL	0
#define eFRONT_RIGHT_WHEEL	1
#define eREAR_LEFT_WHEEL	2
#define eREAR_RIGHT_WHEEL	3

#define MAX_NUM_SURFACE_TYPES 4
#define MAX_NUM_TIRE_TYPES 4

enum
	{
		MAX_NUM_INDEX_BUFFERS = 16
	};
	


	enum
	{
		MAX_NUM_4W_VEHICLES=8,
		
	};
struct physicsmanager
{
	physx::PxDefaultAllocator default_alloc;
	physx::PxDefaultErrorCallback default_error;

	physx::PxPhysics* sdk;
	physx::PxFoundation* foundation;
	physx::PxCooking* cooking;
	physx::PxVec3 gravity;
	physx::PxMaterial* default_material;

	physx::PxScene* scene;
	physx::PxVehicleDrivableSurfaceToTireFrictionPairs* mSurfaceTirePairs;
	//Array of all cars and report data for each car.


	//sdk raycasts (for the suspension lines).
	VehicleSceneQueryData* mSqData;
	physx::PxBatchQuery* mSqWheelRaycastBatchQuery;

	//Reports for each wheel.
	VehicleWheelQueryResults* mWheelQueryResults;

	
	//Array of all cars and report data for each car.
	physx::PxVehicleWheels* mVehicles[MAX_NUM_4W_VEHICLES];
	physx::PxVehicleWheelQueryResult mVehicleWheelQueryResults[MAX_NUM_4W_VEHICLES];
	physx::PxU32 mNumVehicles;

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


physx::PxRigidDynamic* physics_addcart(struct physicsmanager* pm, vec3f pos);
physx::PxRigidDynamic* physics_adddynamic_box(struct physicsmanager*, vec3f, vec3f);

/*	add a static triangle mesh stored in a triangle strip vertex buffer
	param:	pm				physics manager
	param:	num_verts		number of vertices in the buffer
	param:	stride			stride between vertices in the buffer
	param:	buf_verts		vertex buffer
*/
void physicsmanager_addstatic_trianglestrip(struct physicsmanager* pm, unsigned num_verts, unsigned stride, float* buf_verts);


#endif