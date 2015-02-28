#ifndef PICKUP
#define PICKUP

#include	"../core/game.h"
#include	"../math/vec3f.h"
#include	"../render/render.h"
#include	"../physics/physics.h"

#define PICKUP_WIDTH		.8f
#define PICKUP_HEIGHT		1.2f
#define	PICKUP_LENGTH		0.1f

struct physicsmanager;
struct power;

struct pickup{
		
	physx::PxRigidDynamic* body;
		
};

struct pickupmanager{

	struct physicsmanager* pm;
	struct texturemanager* tm;

	struct renderable r_pickup;

	struct pickup* pickup;
};

void pickup_init(struct pickupmanager*, struct physicsmanager*, vec3f);
void pickup_delete(struct pickupmanager*);


#endif