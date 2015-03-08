#include	"collisions.h"
#include	"../objects/vehicle.h"
#include	"../objects/entities.h"
#include	<time.h>

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
	if((filterData0.word0 == FilterGroup::eProjectile && filterData1.word0 == FilterGroup::eVehicle)||(filterData1.word0 == FilterGroup::eProjectile && filterData0.word0 == FilterGroup::eVehicle)) {
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}
	else if((filterData0.word0 == FilterGroup::eMine && filterData1.word0 == FilterGroup::eVehicle)||(filterData1.word0 == FilterGroup::eMine && filterData0.word0 == FilterGroup::eVehicle)) {
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}
	else if((filterData0.word0 == FilterGroup::ePickup && filterData1.word0 == FilterGroup::eVehicle)||(filterData1.word0 == FilterGroup::ePickup && filterData0.word0 == FilterGroup::eVehicle)) {
		pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
	}
	
	return PxFilterFlag::eDEFAULT;
}
void missileHit(struct vehicle* v) {
	//printf("Poop %d \n", v->index_in_vm);
	//physx::PxRigidBodyExt::addForceAtLocalPos(*v->body, physx::PxVec3(0, 10, 0), physx::PxVec3(0, 0, 0));

}
void CustomCollisions::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	unsigned i;

	for(i = 0; i < nbPairs; i++)
	{
		
		const PxContactPair& cp = pairs[i];

		if(cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{

			const physx::PxU32 numShapes0 = pairHeader.actors[0]->getNbShapes();
			physx::PxShape** shapes0 = (physx::PxShape**)malloc(sizeof(physx::PxShape*)*numShapes0);
			pairHeader.actors[0]->getShapes(shapes0, numShapes0);
			physx::PxShape* shape0 = shapes0[i];
			physx::PxFilterData filterData0 = shape0->getSimulationFilterData();

			const physx::PxU32 numShapes1 = pairHeader.actors[0]->getNbShapes();
			physx::PxShape** shapes1 = (physx::PxShape**)malloc(sizeof(physx::PxShape*)*numShapes1);
			pairHeader.actors[0]->getShapes(shapes1, numShapes1);
			physx::PxShape* shape1 = shapes1[i];
			physx::PxFilterData filterData1 = shape1->getSimulationFilterData();

			if(filterData0.word0 == FilterGroup::eProjectile) {
				//printf("Projectile collision confirmed\n");

				physx::PxActor* vehicleActor = pairHeader.actors[1];
				struct vehicle* v = (struct vehicle*)vehicleActor->userData;
				v->hit_flag = 1;

				physx::PxActor* missileActor = pairHeader.actors[0];
				struct missile* m = (struct missile*)missileActor->userData;
				m->hit = 1;
			}
			else if(filterData1.word0 == FilterGroup::eProjectile) {
				//printf("Projectile collision confirmed\n");
				physx::PxActor* vehicleActor = pairHeader.actors[0];
				struct vehicle* v = (struct vehicle*)vehicleActor->userData;
				v->hit_flag = 1;

				physx::PxActor* missileActor = pairHeader.actors[1];
				struct missile* m = (struct missile*)missileActor->userData;
				m->hit = 1;
			}
			else if(filterData0.word0 == FilterGroup::eMine) {
				//printf("Mine collision confirmed\n");
				physx::PxActor* vehicleActor = pairHeader.actors[1];
				struct vehicle* v = (struct vehicle*)vehicleActor->userData;
				v->hit_flag = 1;

				physx::PxActor* mineActor = pairHeader.actors[0];
				struct mine* x = (struct mine*)mineActor->userData;
				x->hit = 1;
			}
			else if(filterData1.word0 == FilterGroup::eMine) {
				//printf("Mine collision confirmed\n");
				physx::PxActor* vehicleActor = pairHeader.actors[0];
				struct vehicle* v = (struct vehicle*)vehicleActor->userData;
				v->hit_flag = 1;

				physx::PxActor* mineActor = pairHeader.actors[1];
				struct mine* x = (struct mine*)mineActor->userData;
				x->hit = 1;
			}
			else if(filterData0.word0 == FilterGroup::ePickup) {
				//printf("Pickup collision confirmed\n");
				physx::PxActor* vehicleActor = pairHeader.actors[1];
				struct vehicle* v = (struct vehicle*)vehicleActor->userData;
				if (v->haspickup == 0) {
					int seed = static_cast<int>(time(0));
					srand(seed);
					seed = seed%3;
					v->haspickup = seed+1;
				}

				physx::PxActor* pickupActor = pairHeader.actors[0];
				struct pickup* pu = (struct pickup*)pickupActor->userData;
				pu->hit = v->index_in_vm;
			}
			else if(filterData1.word0 == FilterGroup::ePickup) {
				//printf("Pickup collision confirmed\n");
				physx::PxActor* vehicleActor = pairHeader.actors[0];
				struct vehicle* v = (struct vehicle*)vehicleActor->userData;
				if (v->haspickup == 0) {
					int seed = static_cast<int>(time(0));
					srand(seed);
					seed = seed%3;
					v->haspickup = seed+1;
				}

				physx::PxActor* pickupActor = pairHeader.actors[1];
				struct pickup* pu = (struct pickup*)pickupActor->userData;
				pu->hit = v->index_in_vm;
			}


			free(shapes0);
			free(shapes1);
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