#ifndef CART
#define	CART


#include	<PxRigidDynamic.h>
#include	"../math/vec3f.h"
#include	"../render/render.h"


#include "vehicle/PxVehicleSDK.h"
#include "vehicle/PxVehicleDrive4W.h"
#include "vehicle/PxVehicleDriveNW.h"
#include "vehicle/PxVehicleDriveTank.h"
#include "vehicle/PxVehicleUpdate.h"
#include "foundation/PxFoundation.h"

#if PX_DEBUG_VEHICLE_ON
#include "vehicle/PxVehicleUtilTelemetry.h"
#endif

using namespace physx;

namespace physx
{
	class PxScene;
	class PxBatchQuery;
	class PxCooking;
	class PxMaterial;
	class PxConvexMesh;
	struct PxVehicleDrivableSurfaceType;
}

class VehicleSceneQueryData;
class VehicleWheelQueryResults;

class PxVehicle4WAlloc;


#define	CART_WIDTH		1.f
#define	CART_HEIGHT		1.f
#define	CART_LENGTH		2.f



#define	CART_UP			0.f, 1.f, 0.f
#define	CART_FORWARD	0.f, 0.f, -1.f


struct physicsmanager;

static float cart_pos[8][3] =
{
	{-CART_WIDTH/2.f, -CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, -CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, -CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{CART_WIDTH/2.f, -CART_HEIGHT/2.f, CART_LENGTH/2.f}
};

static int cart_posindex[36] =
{
	0, 6, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 6,
	0, 1, 2, 0, 2, 3,
	4, 7, 3, 4, 3, 2,
	4, 2, 1, 4, 1, 5,
	4, 5, 6, 4, 6, 7
};

struct cart
{
	physx::PxRigidDynamic* p_cart;

	struct renderable r_cart;
};


void cart_init(struct cart*, struct physicsmanager*, vec3f);

void cart_generatemesh(struct renderer*, struct cart*);

//Collision types and flags describing collision interactions of each collision type.
enum
{
	COLLISION_FLAG_GROUND			=	1 << 0,
	COLLISION_FLAG_WHEEL			=	1 << 1,
	COLLISION_FLAG_CHASSIS			=	1 << 2,
	COLLISION_FLAG_OBSTACLE			=	1 << 3,
	COLLISION_FLAG_DRIVABLE_OBSTACLE=	1 << 4,

	COLLISION_FLAG_GROUND_AGAINST	=															COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_WHEEL_AGAINST	=									COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE,
	COLLISION_FLAG_CHASSIS_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_OBSTACLE_AGAINST	=			COLLISION_FLAG_GROUND | COLLISION_FLAG_WHEEL |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
	COLLISION_FLAG_DRIVABLE_OBSTACLE_AGAINST=	COLLISION_FLAG_GROUND 						 |	COLLISION_FLAG_CHASSIS | COLLISION_FLAG_OBSTACLE | COLLISION_FLAG_DRIVABLE_OBSTACLE,
};
	

//Id of drivable surface (used by suspension raycast filtering).
enum
{
	DRIVABLE_SURFACE_ID=0xffffffff
};

#define MAX_NUM_SURFACE_TYPES 4
#define MAX_NUM_TIRE_TYPES 4
//Drivable surface types.
enum
{
	SURFACE_TYPE_MUD=0,
	SURFACE_TYPE_TARMAC,
	SURFACE_TYPE_SNOW,
	SURFACE_TYPE_GRASS
};

struct SurfaceTypeNames
{
	static const char* getName(PxU32 type)
	{
		static char surfaceTypes[MAX_NUM_SURFACE_TYPES+1][64]=
		{
			"mud",
			"tarmac",
			"ice",
			"grass",
		};
		
		return surfaceTypes[type];
	}
};


struct TireFrictionMultipliers
{
	static float getValue(PxU32 surfaceType, PxU32 tireType)
	{
		//Tire model friction for each combination of drivable surface type and tire type.
		static PxF32 tireFrictionMultipliers[MAX_NUM_SURFACE_TYPES][MAX_NUM_TIRE_TYPES]=
		{
			//WETS	SLICKS	ICE		MUD
			{0.95f,	0.95f,	0.95f,	0.95f},		//MUD
			{1.10f,	1.15f,	1.10f,	1.10f},		//TARMAC
			{0.70f,	0.70f,	0.70f,	0.70f},		//ICE
			{0.80f,	0.80f,	0.80f,	0.80f}		//GRASS
		};
		return tireFrictionMultipliers[surfaceType][tireType];
	}
};

class Vehicle_VehicleManager
{
public:

	enum
	{
		MAX_NUM_4W_VEHICLES=8,
		
	};

	Vehicle_VehicleManager();
	~Vehicle_VehicleManager();

	void init(PxPhysics& physics, const PxMaterial** drivableSurfaceMaterials, const PxVehicleDrivableSurfaceType* drivableSurfaceTypes);

	void shutdown();


	PX_FORCE_INLINE	PxU32						getNbVehicles()					const	{ return mNumVehicles;		}
	PX_FORCE_INLINE	PxVehicleWheels*			getVehicle(const PxU32 i)				{ return mVehicles[i];		}
	PX_FORCE_INLINE const PxVehicleWheelQueryResult& getVehicleWheelQueryResults(const PxU32 i) const { return mVehicleWheelQueryResults[i]; }
					void						addVehicle(const PxU32 i, PxVehicleWheels* vehicle);		

	//Start the suspension raycasts (always call before calling update)
	void suspensionRaycasts(PxScene* scene);

	//Update vehicle dynamics and compute forces/torques to apply to sdk rigid bodies.
#if PX_DEBUG_VEHICLE_ON
	void updateAndRecordTelemetryData(const PxF32 timestep, const PxVec3& gravity, PxVehicleWheels* focusVehicleNW, PxVehicleTelemetryData* telemetryDataNW);
#else
	void update(const PxF32 timestep, const PxVec3& gravity);
#endif

	//Reset the car back to its rest state with a specified transform.
	void resetNWCar(const PxTransform& transform, const PxU32 vehicleId);

	//Switch the player's vehicle to 3-wheeled modes and back to 4-wheeled mode.
	void switchTo3WDeltaMode(const PxU32 vehicleId);
	void switchTo3WTadpoleMode(const PxU32 vehicleId);
	void switchTo4WMode(const PxU32 vehicleId);
	
	PxSerializationRegistry* getSerializationRegistry() { return mSerializationRegistry; }
private:

	//Array of all cars and report data for each car.
	PxVehicleWheels* mVehicles[MAX_NUM_4W_VEHICLES];
	PxVehicleWheelQueryResult mVehicleWheelQueryResults[MAX_NUM_4W_VEHICLES];
	PxU32 mNumVehicles;

	//sdk raycasts (for the suspension lines).
	VehicleSceneQueryData* mSqData;
	PxBatchQuery* mSqWheelRaycastBatchQuery;

	//Reports for each wheel.
	VehicleWheelQueryResults* mWheelQueryResults;

	//Cached simulation data of focus vehicle in 4W mode.
	PxVehicleWheelsSimData* mWheelsSimData4W;
	PxVehicleDriveSimData4W mDriveSimData4W;
	bool mIsIn3WMode;

	//Friction from combinations of tire and surface types.
	PxVehicleDrivableSurfaceToTireFrictionPairs* mSurfaceTirePairs;


	//Initialise a car back to its start transform and state.
	void resetNWCar(const PxTransform& startTransform, PxVehicleWheels* car);

	//Serialization
	PxSerializationRegistry*		mSerializationRegistry;
};


#endif