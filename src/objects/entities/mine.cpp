#include	"mine.h"

#include	"../../physics/collision.h"


void mine_init(struct entity* e, struct entitymanager* em, struct vehicle* v, physx::PxTransform pose)
{
	physx::PxMat44 mat_pose;

	mat_pose = physx::PxMat44(pose);

	// initialize the physx actor
	e->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxSphereGeometry(MINE_RADIUS), *em->pm->default_material, MINE_DENSITY);
	collision_setupactor(e->body, COLLISION_FILTER_MINE, COLLISION_FILTER_MISSILE | COLLISION_FILTER_MINE | COLLISION_FILTER_VEHICLE);
	e->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	e->body->userData = e;

	// store position
	vec3f_set(e->pos, pose.p.x, pose.p.y, pose.p.z);

	// add actor to scene
	em->pm->scene->addActor(*e->body);

	// store pointer to owner
	e->owner = v;

	// play sound effect and store the audio channel
	e->channel = audiomanager_playsfx(em->am, em->sfx_mine_idle, e->pos, -1);

	// set the despawn timer
	e->timer = MINE_DESPAWNTIME;

	// initialize mine flags
	e->type = ENTITY_TYPE_MINE;
	e->flags = ENTITY_FLAG_ENABLED;
}

void mine_delete(struct entity* e)
{
	e->body->release();
	soundchannel_stop(e->channel);
	e->flags = ENTITY_FLAG_INIT;
}

void mine_update(struct entity* e, struct entitymanager* em)
{
	physx::PxTransform pose;

	// check if the mine has hit anything
	if (e->flags & ENTITY_FLAG_HIT)
	{
		audiomanager_playsfx(em->am, em->sfx_mine_explode, e->pos, 0);
		mine_delete(e);
		return;
	}

	// decrement despawn timer
	e->timer--;
	if (e->timer == 0)
	{
		mine_delete(e);
		return;
	}
}