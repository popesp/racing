#ifndef POWERUP
#define	POWERUP

#include	<PxPhysicsAPI.h>
#include	"../math/vec3f.h"
#include	"../render/render.h"
#include	"vehicle.h"
#include	"../core/player.h"
#include	"../render/objloader.h"
#include	"../render/texture.h"

#define PROJECTILE_WIDTH		1.2f
#define PROJECTILE_LENGTH		0.6f
#define PROJECTILE_HEIGHT		2.f

#define	PROJ_TARGETPOS	0.f, 0.f, -2.0f

#define	CART_FORWARD		0.f, 0.f, -1.f
#define	CART_FORCE_FORWARD	30.f

struct physicsmanager;

struct projectile
{
	struct physicsmanager* pm;
	physx::PxRigidDynamic* proj;

	struct renderable r_proj;
};

struct entitymanager
{
	struct physicsmanager* pm;
	struct texturemanager* tm;
	struct track* track;

	struct renderable r_proj;
	//struct renderable r_mine;
	//struct renderable r_pickup;

	vec3f dim;

	int tex_diffuse;

	struct projectile projectiles[1];
	int proj_flag;
	//struct mine mines[1];
	//struct pickup pickups[1];
};
void entitymanager_startup(struct entitymanager* em, struct renderer* r, struct texturemanager* tm, struct physicsmanager* pm, struct track* t);

void projectile_init(struct projectile* p, struct physicsmanager* pm, struct vehicle* v);

void projectile_delete(struct projectile* p, struct physicsmanager* pm);

void projectile_generatemesh(struct renderer* r, struct renderable r_proj);

#endif