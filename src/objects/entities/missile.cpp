#include	"missile.h"

#include	"../../physics/collision.h"


void missile_init(struct entity* e, struct entitymanager* em, struct vehicle* v, physx::PxTransform pose)
{
	physx::PxMat44 mat_pose;
	vec3f vel;

	mat_pose = physx::PxMat44(pose);

	// initialize the physx actor
	e->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxSphereGeometry(MISSILE_RADIUS), *em->pm->default_material, MISSILE_DENSITY);
	collision_setupactor(e->body, COLLISION_FILTER_MISSILE, COLLISION_FILTER_MISSILE | COLLISION_FILTER_MINE | COLLISION_FILTER_VEHICLE);
	e->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	e->body->userData = e;

	// store position
	vec3f_set(e->pos, pose.p.x, pose.p.y, pose.p.z);

	// set the missile velocity
	vec3f_set(vel, ENTITY_FORWARD);
	mat4f_transformvec3f(vel, (float*)&mat_pose);
	vec3f_scale(vel, MISSILE_SPEED);
	e->body->setLinearVelocity(physx::PxVec3(vel[VX], vel[VY], vel[VZ]));

	// add actor to scene
	em->pm->scene->addActor(*e->body);

	// store pointer to owner
	e->owner = v;

	// play sound effect and store the audio channel
	e->channel = audiomanager_playsfx(em->am, em->sfx_missile_idle, e->pos, -1, true);

	// set the despawn timer
	e->timer = MISSILE_DESPAWNTIME;

	// initialize missile flags
	e->type = ENTITY_TYPE_MISSILE;
	e->flags = ENTITY_FLAG_ENABLED;
}

void missile_delete(struct entity* e)
{
	e->body->release();
	soundchannel_stop(e->channel);
	e->flags = ENTITY_FLAG_INIT;
}

void missile_update(struct entity* e, struct entitymanager* em)
{
	physx::PxTransform pose;

	// check if the missile has hit anything
	if (e->flags & ENTITY_FLAG_HIT)
	{
		audiomanager_playsfx(em->am, em->sfx_missile_explode, e->pos, 0, true);
		missile_delete(e);
		return;
	}

	// decrement despawn timer
	e->timer--;
	if (e->timer == 0)
	{
		missile_delete(e);
		return;
	}

	// update position
	pose = e->body->getGlobalPose();
	vec3f_set(e->pos, pose.p.x, pose.p.y, pose.p.z);

	soundchannel_setposition(e->channel, e->pos);
}