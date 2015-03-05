#include	"collisions.h"


physx::PxFilterFlags OurFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
	return physx::PxFilterFlag::eDEFAULT;
}

void CustomCollisions::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	unsigned i;

	printf("Entered contact callback\n");
	for(i = 0; i < nbPairs; i++)
	{
		/*
		const PxContactPair& cp = pairs[i];

		if(cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{

			printf("Here\n");

			//Find if it's a projectile, deal with its collision, below is a sample from the submarine snippet

			if((pairHeader.actors[0] == mSubmarineActor) || (pairHeader.actors[1] == mSubmarineActor))
			{
				PxActor* otherActor = (mSubmarineActor == pairHeader.actors[0]) ? pairHeader.actors[1] : pairHeader.actors[0];			
				Seamine* mine =  reinterpret_cast<Seamine*>(otherActor->userData);
				// insert only once
				if(std::find(mMinesToExplode.begin(), mMinesToExplode.end(), mine) == mMinesToExplode.end())
					mMinesToExplode.push_back(mine);

				break;
			}
		}
		*/
	}
}