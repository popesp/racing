#ifndef COLLISION
#define	COLLISION


#include	<PxPhysicsAPI.h>


#define	COLLISION_FILTER_STATIC		0x00000001
#define	COLLISION_FILTER_PICKUP		0x00000002
#define	COLLISION_FILTER_VEHICLE	0x00000004
#define	COLLISION_FILTER_MISSILE	0x00000008
#define	COLLISION_FILTER_MINE		0x00000010
#define COLLISION_FILTER_SLOWMINE	0x00000020


void collision_setupactor(physx::PxRigidActor* actor, unsigned filter_group, unsigned filter_mask);
physx::PxFilterFlags collision_filter(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize);


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