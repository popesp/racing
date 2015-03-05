#include	<PxPhysicsAPI.h>
#include	"physics.h"

using namespace physx;

//Filter groups for collisions
struct FilterGroup
{
	enum Enum
	{
		eGround			= (1 << 0),
		eVehicle		= (1 << 1),
		eMINE			= (1 << 2),
		eProjectile		= (1 << 3),
		//eHEIGHTFIELD	= (1 << 4),
	};
};

class collision : public PxSimulationEventCallback {

public:
	// Implements PxSimulationEventCallback
	virtual void							onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs);
	virtual void							onTrigger(PxTriggerPair* pairs, PxU32 count) {}
	virtual void							onConstraintBreak(PxConstraintInfo*, PxU32) {}
	virtual void							onWake(PxActor** , PxU32 ) {}
	virtual void							onSleep(PxActor** , PxU32 ){}

	void customizeSceneDesc(PxSceneDesc* sceneDesc);
};