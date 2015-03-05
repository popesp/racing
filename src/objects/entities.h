#ifndef ENTITIES
#define	ENTITIES


#include	"../objects/track.h"
#include	"../objects/vehicle.h"
#include	"../physics/physics.h"


#define	ENTITY_MISSILE_COUNT		32
#define ENTITY_PICKUP_COUNT			100
#define ENTITY_MINE_COUNT			1000

#define	ENTITY_MISSILE_DENSITY		1.f
#define	ENTITY_MISSILE_SPEED		80.f
#define	ENTITY_MISSILE_SIZE			1.f

#define	ENTITY_PICKUP_DENSITY		1.f
#define	ENTITY_PICKUP_WIDTH			.8f
#define ENTITY_PICKUP_HEIGHT		1.2f
#define ENTITY_PICKUP_LENGTH		0.1f

#define	ENTITY_MINE_DENSITY			1.f
#define	ENTITY_MINE_WIDTH			.8f
#define ENTITY_MINE_HEIGHT			.8f
#define ENTITY_MINE_LENGTH			.8f

#define	ENTITY_MISSILE_DESPAWNTIME	300

#define	ENTITY_MISSILE_SPAWNDIST	1.f
#define	ENTITY_PICKUP_SPAWNHEIGHT	0.f
#define ENTITY_MINE_SPAWNDIST		5.f

#define	ENTITY_MISSILE_FLAG_ENABLED	0x01
#define	ENTITY_MISSILE_FLAG_INIT	0x00

#define ENTITY_PICKUP_FLAG_INIT		0x00
#define ENTITY_PICKUP_FLAG_ENABLED	0x01

#define ENTITY_MINE_FLAG_INIT		0x00
#define ENTITY_MINE_FLAG_ENABLED	0x01

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
	vec3f pos;
	unsigned timer;

	unsigned char flags;
};

struct mine{
	physx::PxRigidDynamic* body;
	struct vehicle* owner;
	unsigned char flags;
};

struct entitymanager
{
	struct physicsmanager* pm;
	struct audiomanager* am;
	struct track* track;

	struct renderable r_missile;
	struct renderable r_mine;
	struct renderable r_pickup;

	vec3f dim_missile;
	vec3f dim_pickup;
	vec3f dim_mine;

	int sfx_missile;
	
	struct missile missiles[ENTITY_MISSILE_COUNT];
	struct mine mines[ENTITY_MINE_COUNT];
	struct pickup pickups[ENTITY_PICKUP_COUNT];
};


void entitymanager_startup(struct entitymanager* em, struct physicsmanager* pm, struct renderer* r,struct audiomanager* am, struct track* t);
void entitymanager_shutdown(struct entitymanager* em);

void entitymanager_update(struct entitymanager* em);

struct missile* entitymanager_newmissile(struct entitymanager* em, struct vehicle* v, vec3f dim);
void entitymanager_removemissile(struct entitymanager* em, struct missile* m);

void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview);

struct pickup* entitymanager_newpickup(struct entitymanager* em, vec3f offs);
void entitymanager_removepickup(struct entitymanager* em, struct pickup* pu);

struct mine* entitymanager_newmine(struct entitymanager* em, vec3f dim, struct vehicle* v);
void entitymanager_removemine(struct entitymanager* em, struct mine* x);

#endif
