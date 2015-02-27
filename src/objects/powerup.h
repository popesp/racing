#ifndef POWERUP
#define	POWERUP

#include	<PxPhysicsAPI.h>
#include	"../math/vec3f.h"
#include	"../render/render.h"
#include	"vehicle.h"
#include	"../core/player.h"

#define PROJECTILE_WIDTH		1.2f
#define PROJECTILE_LENGTH		0.6f
#define PROJECTILE_HEIGHT		2.f

#define	PROJ_TARGETPOS	0.f, 0.f, -2.0f

struct physicsmanager;

struct projectile
{
	struct physicsmanager* pm;
	physx::PxRigidDynamic* proj;

	struct renderable r_proj;
};

void projectile_init(struct projectile* p, struct physicsmanager* pm, struct player* c);

void projectile_delete(struct projectile* p, struct physicsmanager* pm);

void projectile_generatemesh(struct renderer* r, struct projectile* p);

#endif