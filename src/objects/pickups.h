#ifndef PICKUP
#define PICKUP

#include	"../math/vec3f.h"
#include	"../render/render.h"
#include	"../physics/physics.h"

#define PICKUP_WIDTH		.8f
#define PICKUP_HEIGHT		1.2f
#define	PICKUP_LENGTH		0.1f

#define	PICKUP_COUNT		10


struct physicsmanager;
struct power;


struct pickup{
		
	physx::PxRigidDynamic* body;
		
};

struct pickupmanager{

	struct physicsmanager* pm;

	struct renderable r_pickup;

	struct pickup pickups[PICKUP_COUNT];
};

void pickup_init(struct pickup*, struct physicsmanager*, vec3f);
void pickup_delete(struct pickup* pu);


#endif