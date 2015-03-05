#include	"collision.h"


void collision::onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
{	
	printf("Here\n");
	for(PxU32 i=0; i < nbPairs; i++)
	{
		const PxContactPair& cp = pairs[i];

		if(cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{

			printf("Here\n");

			//Find if it's a projectile, deal with its collision, below is a sample from the submarine snippet

			/*if((pairHeader.actors[0] == mSubmarineActor) || (pairHeader.actors[1] == mSubmarineActor))
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

void collision::customizeSceneDesc(PxSceneDesc* sceneDesc)
{
	sceneDesc->gravity = PxVec3(PHYSICS_DEFAULT_GRAVITY);
	sceneDesc->cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	sceneDesc->filterShader = PxDefaultSimulationFilterShader;//myFilterShader;
	sceneDesc->simulationEventCallback	= this;
}