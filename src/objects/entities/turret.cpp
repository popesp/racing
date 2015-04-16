#include	"turret.h"


void turret_init(struct entity* e, struct entitymanager* em, struct vehicle* v, physx::PxTransform pose)
{
	physx::PxMat44 mat_pose;

	mat_pose = physx::PxMat44(pose);

	// initialize the physx actor
	e->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxSphereGeometry(TURRET_RADIUS), *em->pm->default_material, TURRET_DENSITY);
	e->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	e->body->userData = e;

	// store position
	vec3f_set(e->pos, pose.p.x, pose.p.y, pose.p.z);

	// add actor to scene
	em->pm->scene->addActor(*e->body);

	// store pointer to owner
	e->owner = v;

	// set the despawn timer
	e->timer = TURRET_DESPAWNTIME;

	// initialize turret flags
	e->type = ENTITY_TYPE_TURRET;
	e->flags = ENTITY_FLAG_ENABLED;
}

void turret_delete(struct entity* e)
{
	e->body->release();
	e->flags = ENTITY_FLAG_INIT;
}

void turret_update(struct entity* e, struct entitymanager* em)
{
	physx::PxTransform pose_turret, pose_spawn;

	// decrement despawn timer
	e->timer--;
	if (e->timer == 0)
	{
		turret_delete(e);
		return;
	}

	if (e->timer % TURRET_COOLDOWN == 0)
	{
		pose_turret = e->body->getGlobalPose();
		pose_spawn = pose_turret.transform(physx::PxTransform(0.f, 0.f, -(TURRET_RADIUS + TURRET_MISSILE_SPAWNDIST)));

		entitymanager_newentity(em, ENTITY_TYPE_MISSILE, e->owner, pose_spawn);
	}
}