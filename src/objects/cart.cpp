#include	"cart.h"


static float cart_pos[8][3] =
{
	{-CART_WIDTH/2.f, -CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, -CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, -CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{CART_WIDTH/2.f, -CART_HEIGHT/2.f, CART_LENGTH/2.f}
};

static float cart_nor[6][3] =
{
	{-1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{0.f, -1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 0.f, -1.f},
	{0.f, 0.f, 1.f}
};

static int cart_posindex[36] =
{
	0, 6, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 6,
	0, 1, 2, 0, 2, 3,
	4, 7, 3, 4, 3, 2,
	4, 2, 1, 4, 1, 5,
	4, 5, 6, 4, 6, 7
};

static int cart_norindex[36] =
{
	0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2,
	4, 4, 4, 4, 4, 4,
	1, 1, 1, 1, 1, 1,
	3, 3, 3, 3, 3, 3,
	5, 5, 5, 5, 5, 5
};


/*	initialize a cart object
	param:	c				cart object to initialize (modified)
	param:	pm				physics manager
	param:	obj				renderable object pointer
	param:	t				track object
	param:	index_track		track point index on which to spawn cart
*/
void cart_init(struct cart* c, struct physicsmanager* pm, struct renderable* obj, struct track* t, int index_track)
{
	vec3f dim, spawn;

	c->pm = pm;

	// find spawn location
	vec3f_copy(c->pos, t->searchpoints[index_track]);
	vec3f_copy(spawn, t->up);
	vec3f_scale(spawn, CART_SPAWNHEIGHT);
	vec3f_add(c->pos, spawn);

	// create a new physics vehicle
	vec3f_set(dim, CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f);
	c->vehicle = physicsmanager_newvehicle(pm, c->pos, dim);

	c->renderable = obj;

	c->controller = NULL;

	c->index_track = index_track;
}

/*	delete a cart object
	param:	c				cart object to delete
*/
void cart_delete(struct cart* c)
{
	physicsmanager_removevehicle(c->pm, c->vehicle);
}


/*	update a cart object
	param:	c				cart object to update
	param:	t				track object
*/
void cart_update(struct cart* c, struct track* t)
{
	physx::PxVec3 pos;
	vec3f force, dist;
	float vel;

	// update global cart position
	pos = c->vehicle->body->getGlobalPose().p;
	vec3f_set(c->pos, pos.x, pos.y, pos.z);

	// update track index
	c->index_track = track_closestindex(t, c->pos, c->index_track);

	vec3f_subtractn(dist, c->pos, t->searchpoints[c->index_track]);
	if (vec3f_length(dist) > t->dist_boundary)
		cart_reset(c, t);

	if (c->controller != NULL && c->controller->flags & INPUT_FLAG_ENABLED)
	{
		int i;

		// if at least one raycast is touching the ground, apply an acceleration force
		for (i = 0; i < PHYSICS_VEHICLE_RAYCAST_COUNT; i++)
			if (c->vehicle->touching[i])
			{
				vec3f_set(force, CART_FORWARD);
				vec3f_scale(force, -CART_FORCE_FORWARD * c->controller->axes[INPUT_AXIS_TRIGGERS]);

				physx::PxRigidBodyExt::addLocalForceAtLocalPos(*c->vehicle->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, 0.f));

				break;
			}

		// turning force
		vel = vehicle_getspeed(c->vehicle);

		vec3f_set(force, CART_RIGHT);
		vec3f_scale(force, CART_FORCE_TURN * c->controller->axes[INPUT_AXIS_LEFT_LR]);

		if (c->controller->axes[INPUT_AXIS_TRIGGERS] > 0.1f && vel < 0.f)
			vec3f_negate(force);
		
		physx::PxRigidBodyExt::addLocalForceAtLocalPos(*c->vehicle->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, -CART_LENGTH/2.f));
	}
}


/*	reset a cart by placing it back on the track
	param:	c				cart object to reset
	param:	t				track object
*/
void cart_reset(struct cart* c, struct track* t)
{
	vec3f spawn;

	// find spawn location
	vec3f_copy(c->pos, t->searchpoints[c->index_track]);
	vec3f_copy(spawn, t->up);
	vec3f_scale(spawn, CART_SPAWNHEIGHT);
	vec3f_add(c->pos, spawn);

	c->vehicle->body->setGlobalPose(physx::PxTransform(physx::PxVec3(c->pos[VX], c->pos[VY], c->pos[VZ])));
	c->vehicle->body->setLinearVelocity(physx::PxVec3(0.f, 0.f, 0.f));
	c->vehicle->body->setAngularVelocity(physx::PxVec3(0.f, 0.f, 0.f));
}
