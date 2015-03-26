#ifndef ENTITY
#define	ENTITY


#include	"../../audio/audio.h"
#include	"../../physics/physics.h"
#include	"../../render/render.h"
#include	"../../render/texture.h"
#include	"../vehicle.h"


#define	ENTITY_MAX			256
#define	ENTITY_FORWARD		0.f, 0.f, -1.f

#define	ENTITY_TYPE_COUNT	3
#define	ENTITY_TYPE_MISSILE	0
#define	ENTITY_TYPE_MINE	1
#define	ENTITY_TYPE_TURRET	2

#define	ENTITY_FLAG_ENABLED	0x01
#define	ENTITY_FLAG_HIT		0x02
#define	ENTITY_FLAG_INIT	0x00


struct entity
{
	physx::PxRigidDynamic* body;
	struct vehicle* owner;
	FMOD_CHANNEL* idle_channel;
	FMOD_CHANNEL* launch_channel;
	vec3f pos;

	unsigned timer;

	unsigned char type;
	unsigned char flags;
};


struct entitymanager
{
	struct physicsmanager* pm;
	struct audiomanager* am;

	struct entity entities[ENTITY_MAX];

	struct renderable renderables[ENTITY_TYPE_COUNT];
	struct texture diffuse[ENTITY_TYPE_COUNT];

	int sfx_missile_launch;
	int sfx_missile_idle;
	int sfx_missile_explode;
	int sfx_mine_idle;
	int sfx_mine_explode;
};


void entitymanager_startup(struct entitymanager* em, struct physicsmanager* pm, struct audiomanager* am, struct renderer* r);
void entitymanager_shutdown(struct entitymanager* em);
void entitymanager_update(struct entitymanager* em);

void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview);

void entitymanager_newentity(struct entitymanager* em, unsigned char type, struct vehicle* v, physx::PxTransform pose);


#endif
