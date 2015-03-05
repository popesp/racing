#ifndef ENTITIES
#define	ENTITIES


#include	"../objects/track.h"
#include	"../objects/vehicle.h"
#include	"../physics/physics.h"



#define	ENTITY_MISSILE_COUNT		32
#define ENTITY_PICKUP_COUNT			10

#define	ENTITY_MISSILE_DENSITY		1.f
#define	ENTITY_MISSILE_SPEED		80.f
#define	ENTITY_MISSILE_SIZE			1.f

#define	ENTITY_PICKUP_DENSITY		1.f
#define	ENTITY_PICKUP_WIDTH			.8f
#define ENTITY_PICKUP_HEIGHT		1.2f
#define ENTITY_PICKUP_LENGTH		0.1f

#define	ENTITY_MISSILE_SPAWNDIST	1.f
#define	ENTITY_PICKUP_SPAWNHEIGHT	0.f

#define	ENTITY_MISSILE_FLAG_ENABLED	0x01
#define	ENTITY_MISSILE_FLAG_INIT	0x00

#define ENTITY_PICKUP_FLAG_INIT		0x00
#define ENTITY_PICKUP_FLAG_ENABLED	0x01

struct pickup{
	physx::PxRigidDynamic* body;

	struct vehicle* owner;

	unsigned char flags;

	vec3f pos;
};

struct missile
{
	physx::PxRigidDynamic* body;

	struct vehicle* owner;

	unsigned char flags;
};

struct entitymanager
{
	struct physicsmanager* pm;
	struct track* track;

	struct renderable r_missile;
	//struct renderable r_mine;
	struct renderable r_pickup;

	vec3f dim_missile;
	vec3f dim_pickup;

	struct missile missiles[ENTITY_MISSILE_COUNT];
	//struct mine mines[1];
	struct pickup pickups[ENTITY_PICKUP_COUNT];
};


void entitymanager_startup(struct entitymanager* em, struct physicsmanager* pm, struct renderer* r, struct track* t);
void entitymanager_shutdown(struct entitymanager* em);

struct missile* entitymanager_newmissile(struct entitymanager* em, struct vehicle* v, vec3f dim);
void entitymanager_removemissile(struct entitymanager* em, struct missile* m);

void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview);

struct pickup* entitymanager_newpickup(struct entitymanager* em, vec3f offs);
void entitymanager_removepickup(struct entitymanager* em, struct pickup* pu);

#endif
