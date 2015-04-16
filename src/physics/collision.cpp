#include	"collision.h"

#include	"../mem.h"
#include	"../objects/entities/entity.h"
#include	"../objects/pickup.h"
#include	"../objects/vehicle.h"


void collision_setupactor(physx::PxRigidActor* actor, unsigned filter_group, unsigned filter_mask)
{
	physx::PxFilterData filterdata;
	physx::PxShape** shapes;
	unsigned i, num_shapes;

	// actor filter type
	filterdata.word0 = filter_group;

	// what collisions we are interested in
	filterdata.word1 = filter_mask;

	// allocate shape array
	num_shapes = actor->getNbShapes();
	shapes = (physx::PxShape**)mem_alloc(sizeof(physx::PxShape*) * num_shapes);

	// get the shapes
	actor->getShapes(shapes, num_shapes);

	// set the filter data for each shape
	for(i = 0; i < num_shapes; i++)
		shapes[i]->setSimulationFilterData(filterdata);

	free(shapes);
}

physx::PxFilterFlags collision_filter(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	(void)attributes0;
	(void)attributes1;
	(void)constantBlock;
	(void)constantBlockSize;

	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	else if (filterData0.word0 == COLLISION_FILTER_PICKUP || filterData1.word0 == COLLISION_FILTER_PICKUP)
		return physx::PxFilterFlag::eKILL;

	return physx::PxFilterFlag::eDEFAULT;
}

void CustomCollisions::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	physx::PxFilterData fd0, fd1;
	physx::PxContactPair cp;
	unsigned i;

	for(i = 0; i < nbPairs; i++)
	{
		 cp = pairs[i];

		if(cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			// get the filter data
			fd0 = cp.shapes[0]->getSimulationFilterData();
			fd1 = cp.shapes[1]->getSimulationFilterData();

			if (fd0.word0 == COLLISION_FILTER_MISSILE)
			{
				if (fd1.word0 == COLLISION_FILTER_MISSILE || fd1.word0 == COLLISION_FILTER_MINE || fd1.word0 == COLLISION_FILTER_SLOWMINE)
				{
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
					((struct entity*)pairHeader.actors[1]->userData)->flags |= ENTITY_FLAG_HIT;
				} else if (fd1.word0 == COLLISION_FILTER_VEHICLE)
				{
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
					((struct vehicle*)pairHeader.actors[1]->userData)->flags |= VEHICLE_FLAG_MISSILEHIT;
				} else if (fd1.word0 == COLLISION_FILTER_INVINCIBLE)
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
			} else if (fd0.word0 == COLLISION_FILTER_MINE)
			{
				if (fd1.word0 == COLLISION_FILTER_MISSILE || fd1.word0 == COLLISION_FILTER_MINE || fd1.word0 == COLLISION_FILTER_SLOWMINE)
				{
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
					((struct entity*)pairHeader.actors[1]->userData)->flags |= ENTITY_FLAG_HIT;
				} else if (fd1.word0 == COLLISION_FILTER_VEHICLE)
				{
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
					((struct vehicle*)pairHeader.actors[1]->userData)->flags |= VEHICLE_FLAG_MINEHIT;
				} else if (fd1.word0 == COLLISION_FILTER_INVINCIBLE)
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
			} else if (fd0.word0 == COLLISION_FILTER_VEHICLE)
			{
				if (fd1.word0 == COLLISION_FILTER_MISSILE)
				{
					((struct vehicle*)pairHeader.actors[0]->userData)->flags |= VEHICLE_FLAG_MISSILEHIT;
					((struct entity*)pairHeader.actors[1]->userData)->flags |= ENTITY_FLAG_HIT;
				} else if (fd1.word0 == COLLISION_FILTER_MINE)
				{
					((struct vehicle*)pairHeader.actors[0]->userData)->flags |= VEHICLE_FLAG_MINEHIT;
					((struct entity*)pairHeader.actors[1]->userData)->flags |= ENTITY_FLAG_HIT;
				} else if (fd1.word0 == COLLISION_FILTER_SLOWMINE)
				{
					((struct vehicle*)pairHeader.actors[0]->userData)->flags |= VEHICLE_FLAG_SLOWMINEHIT;
					((struct entity*)pairHeader.actors[1]->userData)->flags |= ENTITY_FLAG_HIT;
				} else if (fd1.word0 == COLLISION_FILTER_PICKUP)
				{
					((struct pickup*)pairHeader.actors[1]->userData)->flags |= PICKUP_FLAG_VEHICLEHIT;
					((struct pickup*)pairHeader.actors[1]->userData)->collector = ((struct vehicle*)pairHeader.actors[0]->userData);
				} else if (fd1.word0 == COLLISION_FILTER_INVINCIBLE)
					((struct vehicle*)pairHeader.actors[0]->userData)->flags |= VEHICLE_FLAG_MISSILEHIT;
			} else if (fd0.word0 == COLLISION_FILTER_PICKUP)
			{
				if (fd1.word0 == COLLISION_FILTER_VEHICLE || fd1.word0 == COLLISION_FILTER_INVINCIBLE)
				{
					((struct pickup*)pairHeader.actors[0]->userData)->flags |= PICKUP_FLAG_VEHICLEHIT;
					((struct pickup*)pairHeader.actors[0]->userData)->collector = ((struct vehicle*)pairHeader.actors[1]->userData);
				}
			} else if (fd0.word0 == COLLISION_FILTER_INVINCIBLE)
			{
				if (fd1.word0 == COLLISION_FILTER_PICKUP)
				{
					((struct pickup*)pairHeader.actors[1]->userData)->flags |= PICKUP_FLAG_VEHICLEHIT;
					((struct pickup*)pairHeader.actors[1]->userData)->collector = ((struct vehicle*)pairHeader.actors[0]->userData);
				} else if (fd1.word0 == COLLISION_FILTER_VEHICLE)
					((struct vehicle*)pairHeader.actors[1]->userData)->flags |= VEHICLE_FLAG_MISSILEHIT;
				else if (fd1.word0 == COLLISION_FILTER_MISSILE || fd1.word0 == COLLISION_FILTER_MINE || fd1.word0 == COLLISION_FILTER_SLOWMINE)
					((struct entity*)pairHeader.actors[1]->userData)->flags |= ENTITY_FLAG_HIT;
			} else if (fd0.word0 == COLLISION_FILTER_SLOWMINE)
			{
				if (fd1.word0 == COLLISION_FILTER_MISSILE || fd1.word0 == COLLISION_FILTER_MINE || fd1.word0 == COLLISION_FILTER_SLOWMINE)
				{
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
					((struct entity*)pairHeader.actors[1]->userData)->flags |= ENTITY_FLAG_HIT;
				} else if (fd1.word0 == COLLISION_FILTER_VEHICLE)
				{
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
					((struct vehicle*)pairHeader.actors[1]->userData)->flags |= VEHICLE_FLAG_SLOWMINEHIT;
				} else if (fd1.word0 == COLLISION_FILTER_INVINCIBLE)
					((struct entity*)pairHeader.actors[0]->userData)->flags |= ENTITY_FLAG_HIT;
			}
		}
	}
}