#ifndef COLLISIONS
#define	COLLISIONS


#include	<PxPhysicsAPI.h>


//Filter groups for collisions

struct FilterGroup
{
	enum Enum
	{
		ePickup			= (1 << 0),
		eVehicle		= (1 << 1),
		eMine			= (1 << 2),
		eProjectile		= (1 << 3),
		//eHEIGHTFIELD	= (1 << 4),
	};
};



physx::PxFilterFlags OurFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);

void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);

class CustomCollisions : public physx::PxSimulationEventCallback
{
public:
	virtual void onContact(const physx::PxContactPairHeader&, const physx::PxContactPair*, physx::PxU32);
	virtual void onTrigger(physx::PxTriggerPair*, physx::PxU32) {}
	virtual void onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) {}
	virtual void onWake(physx::PxActor**, physx::PxU32) {}
	virtual void onSleep(physx::PxActor**, physx::PxU32){}
};


#endif