#ifndef ENTITIES
#define	ENTITIES


#include	"../objects/track.h"
#include	"../objects/vehicle.h"
#include	"../physics/physics.h"
#include	"../physics/collisions.h"
#include	"../audio/audio.h"

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

#define	PICKUP_MESHSCALE			0.2f
#define MISSILE_MESHSCALE			0.35f
#define MINE_MESHSCALE				0.1f

#define PICKUP_OBJ					"res/models/powerup/powerup.obj"
#define PICKUP_MISSILE_TEXTURE		"res/models/powerup/powerup_rocketUV.png"
#define	PICKUP_MINE_TEXTURE			"res/models/powerup/powerup_mineUV.png"

#define	MINE_OBJ					"res/models/mine/mine.obj"
#define	MINE_TEXTURE				"res/models/mine/mineUV.png"

#define	MISSILE_OBJ					"res/models/Projectile/rocket.obj"
#define	MISSILE_TEXTURE				"res/models/Projectile/rocket_tex.png"

struct pickup{
	physx::PxRigidDynamic* body;

	struct vehicle* owner;

	unsigned char flags;

	vec3f pos;
};

struct missile
{
	physx::PxRigidDynamic* body;
	vec3f pos;
	struct vehicle* owner;
	unsigned timer;
	unsigned char flags;

	FMOD_CHANNEL* missle_channel;
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

	struct texture diffuse_pickup;
	struct texture diffuse_pickup2;
	struct texture diffuse_missile;
	struct texture diffuse_mine;

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
//void missileHit(physx::PxRigidBody* v);

void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview);

struct pickup* entitymanager_newpickup(struct entitymanager* em, vec3f offs);
void entitymanager_removepickup(struct entitymanager* em, struct pickup* pu);

struct mine* entitymanager_newmine(struct entitymanager* em, vec3f dim, struct vehicle* v);
void entitymanager_removemine(struct entitymanager* em, struct mine* x);

#endif
