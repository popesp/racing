#include	"collisions.h"

using namespace physx;

void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask)
{
	physx::PxFilterData filterData;
	filterData.word0 = filterGroup; // word0 = own ID
	filterData.word1 = filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
	const physx::PxU32 numShapes = actor->getNbShapes();
	physx::PxShape** shapes = (physx::PxShape**)malloc(sizeof(physx::PxShape*)*numShapes);
	actor->getShapes(shapes, numShapes);
	for(physx::PxU32 i = 0; i < numShapes; i++)
	{
		physx::PxShape* shape = shapes[i];
		shape->setSimulationFilterData(filterData);
	}
	free(shapes);
}

physx::PxFilterFlags OurFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)

{
	// let triggers through
	if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
	{
		printf("IsTraigger\n");
		pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}
	// generate contacts for all that were not filtered above
	pairFlags = PxPairFlag::eCONTACT_DEFAULT;

	// trigger the contact callback for pairs (A,B) where 
	// the filtermask of A contains the ID of B and vice versa.
	if(filterData0.word0 == FilterGroup::eProjectile && filterData1.word0 == FilterGroup::eVehicle) {
		//pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		printf("Projectile with Vehicle Collision\n");
	}
	else if(filterData0.word0 == FilterGroup::eMine && filterData1.word0 == FilterGroup::eVehicle) {
		//pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		printf("Mine with Vehicle Collision\n");
	}
	else if(filterData0.word0 == FilterGroup::ePickup && filterData1.word0 == FilterGroup::eVehicle) {
		//pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		printf("Pickup with Vehicle Collision\n");
	}
	
	return PxFilterFlag::eDEFAULT;

	//pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
	//return physx::PxFilterFlag::eDEFAULT;
	//pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
	//return physx::PxFilterFlag::eDEFAULT;
}

void CustomCollisions::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	unsigned i;

	printf("Entered contact callback\n");
	for(i = 0; i < nbPairs; i++)
	{
		
		const PxContactPair& cp = pairs[i];

		if(cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{

			printf("Here\n");

			//Find if it's a projectile, deal with its collision, below is a sample from the submarine snippet
			/*
			if((pairHeader.actors[0] == mSubmarineActor) || (pairHeader.actors[1] == mSubmarineActor))
			{
				PxActor* otherActor = (mSubmarineActor == pairHeader.actors[0]) ? pairHeader.actors[1] : pairHeader.actors[0];			
				Seamine* mine =  reinterpret_cast<Seamine*>(otherActor->userData);
				// insert only once
				if(std::find(mMinesToExplode.begin(), mMinesToExplode.end(), mine) == mMinesToExplode.end())
					mMinesToExplode.push_back(mine);

				break;
			}*/
		}
		
	}
}