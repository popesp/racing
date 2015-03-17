#include	"vehicle.h"

#include	"entities/mine.h"
#include	"entities/missile.h"
#include	"../physics/collision.h"
#include	"../random.h"
#include	"../render/objloader.h"


static char* vehicle_texture_filename[VEHICLE_TEXTURE_DIFFUSE_COUNT] =
{
	VEHICLE_TEXTURE_DIFFUSE_FILENAME0,
	VEHICLE_TEXTURE_DIFFUSE_FILENAME1,
	VEHICLE_TEXTURE_DIFFUSE_FILENAME2,
	VEHICLE_TEXTURE_DIFFUSE_FILENAME3,
	VEHICLE_TEXTURE_DIFFUSE_FILENAME4,
	VEHICLE_TEXTURE_DIFFUSE_FILENAME5,
	VEHICLE_TEXTURE_DIFFUSE_FILENAME6
};

static char* powerup_texture_filename[VEHICLE_POWERUP_COUNT] =
{
	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILE,
	VEHICLE_POWERUP_TEXTURE_FILENAME_MINE,
	VEHICLE_POWERUP_TEXTURE_FILENAME_BOOST,
	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILEX2,
	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILEX3,
	VEHICLE_POWERUP_TEXTURE_FILENAME_TURRET,
	VEHICLE_POWERUP_TEXTURE_FILENAME_LONGBOOST
};


static void missilefunction(struct vehicle* v)
{
	physx::PxTransform pose_vehicle, pose_spawn;
	vec3f dim;

	pose_vehicle = v->body->getGlobalPose();

	vec3f_set(dim, VEHICLE_DIMENSIONS);
	pose_spawn = pose_vehicle.transform(physx::PxTransform(0.f, 0.f, -(dim[VZ] + VEHICLE_POWERUP_MISSILE_SPAWNDIST)));

	entitymanager_newentity(v->vm->em, ENTITY_TYPE_MISSILE, v, pose_spawn);

	if (v->powerup == VEHICLE_POWERUP_MISSILEX3)
		v->powerup = VEHICLE_POWERUP_MISSILEX2;
	else if (v->powerup == VEHICLE_POWERUP_MISSILEX2)
		v->powerup = VEHICLE_POWERUP_MISSILE;
	else
		v->flags &= ~VEHICLE_FLAG_HASPOWERUP;
}

static void minefunction(struct vehicle* v)
{
	physx::PxTransform pose_vehicle, pose_spawn;
	vec3f dim;

	pose_vehicle = v->body->getGlobalPose();

	vec3f_set(dim, VEHICLE_DIMENSIONS);
	pose_spawn = pose_vehicle.transform(physx::PxTransform(0.f, 0.f, dim[VZ] + VEHICLE_POWERUP_MINE_SPAWNDIST));

	entitymanager_newentity(v->vm->em, ENTITY_TYPE_MINE, v, pose_spawn);

	v->flags &= ~VEHICLE_FLAG_HASPOWERUP;
}

static void boostfunction(struct vehicle* v)
{
	v->timer_boost = VEHICLE_POWERUP_BOOST_DURATION;
	v->flags |= VEHICLE_FLAG_BOOSTING;

	v->flags &= ~VEHICLE_FLAG_HASPOWERUP;
}

static void turretfunction(struct vehicle* v)
{
	physx::PxTransform pose_vehicle, pose_spawn;
	vec3f dim;

	pose_vehicle = v->body->getGlobalPose();

	vec3f_set(dim, VEHICLE_DIMENSIONS);
	pose_spawn = pose_vehicle.transform(physx::PxTransform(0.f, 0.f, dim[VZ] + VEHICLE_POWERUP_MINE_SPAWNDIST));

	entitymanager_newentity(v->vm->em, ENTITY_TYPE_TURRET, v, pose_spawn);

	v->flags &= ~VEHICLE_FLAG_HASPOWERUP;
}

static void longboostfunction(struct vehicle* v)
{
	v->timer_boost = VEHICLE_POWERUP_LONGBOOST_DURATION;
	v->flags |= VEHICLE_FLAG_BOOSTING;

	v->flags &= ~VEHICLE_FLAG_HASPOWERUP;
}

static void (* powerupfunction[VEHICLE_POWERUP_COUNT])(struct vehicle*) =
{
	missilefunction,
	minefunction,
	boostfunction,
	missilefunction,
	missilefunction,
	turretfunction,
	longboostfunction
};


void vehiclemanager_startup(struct vehiclemanager* vm, struct physicsmanager* pm, struct entitymanager* em, struct audiomanager* am, struct renderer* r, struct track* track)
{
	vec3f dim, center;
	int i;

	// manager pointers
	vm->pm = pm;
	vm->em = em;
	vm->am = am;
	vm->track = track;

	// initialize vehicle mesh
	renderable_init(&vm->r_vehicle, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(VEHICLE_MESH_FILENAME, r, &vm->r_vehicle, dim, center);
	renderable_sendbuffer(r, &vm->r_vehicle);
	// matrix model transformation
	mat4f_scalemul(vm->r_vehicle.matrix_model, VEHICLE_MESH_SCALE, VEHICLE_MESH_SCALE, VEHICLE_MESH_SCALE);
	mat4f_rotateymul(vm->r_vehicle.matrix_model, VEHICLE_MESH_YROTATE);
	mat4f_translatemul(vm->r_vehicle.matrix_model, -center[VX], -center[VY], -center[VZ]);

	// initialize powerup mesh
	renderable_init(&vm->r_powerup, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(VEHICLE_POWERUP_MESH_FILENAME, r, &vm->r_powerup, dim, center);
	renderable_sendbuffer(r, &vm->r_powerup);
	// matrix model transformation
	mat4f_scalemul(vm->r_powerup.matrix_model, VEHICLE_POWERUP_MESH_SCALE, VEHICLE_POWERUP_MESH_SCALE, VEHICLE_POWERUP_MESH_SCALE);
	mat4f_rotateymul(vm->r_powerup.matrix_model, VEHICLE_POWERUP_MESH_YROTATE);
	mat4f_translatemul(vm->r_powerup.matrix_model, -center[VX], -center[VY], -center[VZ]);

	// initialize vehicle diffuse textures
	for (i = 0; i < VEHICLE_TEXTURE_DIFFUSE_COUNT; i++)
	{
		texture_init(vm->diffuse_vehicle + i);
		texture_loadfile(vm->diffuse_vehicle + i, vehicle_texture_filename[i]);
		texture_upload(vm->diffuse_vehicle + i, RENDER_TEXTURE_DIFFUSE);
	}

	// initialize powerup diffuse textures
	for (i = 0; i < VEHICLE_POWERUP_COUNT; i++)
	{
		texture_init(vm->diffuse_powerup + i);
		texture_loadfile(vm->diffuse_powerup + i, powerup_texture_filename[i]);
		texture_upload(vm->diffuse_powerup + i, RENDER_TEXTURE_DIFFUSE);
	}

	// initialize sound effects
	vm->sfx_engine_start = audiomanager_newsfx(am, VEHICLE_SFX_FILENAME_ENGINESTART);
	vm->sfx_engine_idle = audiomanager_newsfx(am, VEHICLE_SFX_FILENAME_ENGINEIDLE);

	// initialize vehicle array
	for (i = 0; i < VEHICLE_COUNT; i++)
		vm->vehicles[i].flags = VEHICLE_FLAG_INIT;
}

void vehiclemanager_shutdown(struct vehiclemanager* vm)
{
	int i;

	renderable_deallocate(&vm->r_vehicle);

	// delete textures
	for (i = 0; i < VEHICLE_TEXTURE_DIFFUSE_COUNT; i++)
		texture_delete(vm->diffuse_vehicle + i);

	// delete vehicles
	for (i = 0; i < VEHICLE_COUNT; i++)
		if (vm->vehicles[i].flags & VEHICLE_FLAG_ENABLED)
		{
			vm->vehicles[i].body->release();
			soundchannel_stop(vm->vehicles[i].channel);
		}
}

static void vehicleinput(struct vehicle* v)
{
	vec3f force, boost, dim;
	float freq;
	int i;

	if (v->controller != NULL && v->controller->flags & INPUT_FLAG_ENABLED)
	{
		// if at least one raycast is touching the ground, apply an acceleration force
		for (i = 0; i < VEHICLE_COUNT_RAYCASTS; i++)
			if (v->ray_touch[i])
			{
				vec3f_set(force, VEHICLE_FORWARD);
				vec3f_scale(force, -VEHICLE_ACCELERATION * v->controller->axes[INPUT_AXIS_TRIGGERS]);

				// check if the vehicle is boosting
				if (v->flags & VEHICLE_FLAG_BOOSTING)
				{
					vec3f_set(boost, VEHICLE_FORWARD);
					vec3f_scale(boost, VEHICLE_POWERUP_BOOST_STRENGTH);
					vec3f_add(force, boost);

					v->timer_boost--;

					if (v->timer_boost == 0)
						v->flags &= ~VEHICLE_FLAG_BOOSTING;
				}

				// change engine sound frequency
				freq = vec3f_length2(force) * VEHICLE_FREQSCALE;
				soundchannel_setfreq(v->channel, 1.f + freq);
				
				// apply acceleration force
				physx::PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, 0.f));

				break;
			}

		// turning force TODO: change how this works
		vec3f_set(force, VEHICLE_RIGHT);
		vec3f_scale(force, VEHICLE_TURNFORCE * v->controller->axes[INPUT_AXIS_LEFT_LR]);

		// TODO: this doesnt quite work right
		if (v->controller->axes[INPUT_AXIS_TRIGGERS] > 0.1f && v->speed < 0.f)
			vec3f_negate(force);
		
		// apply turning force
		vec3f_set(dim, VEHICLE_DIMENSIONS);
		physx::PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, -dim[VZ]));

		// reset button
		if (v->controller->buttons[INPUT_BUTTON_BACK] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
			vehiclemanager_resetvehicle(v->vm, v);

		// using powerups
		if (v->controller->buttons[INPUT_BUTTON_A] == (INPUT_STATE_DOWN | INPUT_STATE_CHANGED))
		{
			if (v->flags & VEHICLE_FLAG_HASPOWERUP)
				powerupfunction[v->powerup](v);
		}
	}
}

static void updatespeed(struct vehicle* v, struct track* t)
{
	vec3f vel, forward, right, up, damp,  nor;
	mat4f basis;
	int i;

	physx::PxVec3 px_vel = v->body->getLinearVelocity();
	physx::PxMat44 transform(v->body->getGlobalPose());

	vec3f_set(vel, px_vel.x, px_vel.y, px_vel.z);

	// transform local vectors
	vec3f_set(forward, VEHICLE_FORWARD);
	mat4f_transformvec3f(forward, (float*)&transform);
	vec3f_set(right, VEHICLE_RIGHT);
	mat4f_transformvec3f(right, (float*)&transform);
	vec3f_set(up, VEHICLE_UP);
	mat4f_transformvec3f(up, (float*)&transform);

	// find track transformation
	track_transformindex(t, basis, v->index_track);

	vec3f_copy(nor, basis + C1);

	// reset the vehicle if its flipped
	if ((vec3f_length2(vel) < VEHICLE_RESETSPEEDTHRESHHOLD) && (vec3f_dot(up, nor) < VEHICLE_RESETTILTTHRESHHOLD))
		vehiclemanager_resetvehicle(v->vm, v);

	// get vehicle speed
	v->speed = vec3f_dot(vel, forward);

	// lateral damping
	for (i = 0; i < VEHICLE_COUNT_RAYCASTS; i++)
			if (v->ray_touch[i])
			{
				vec3f_scalen(damp, right, -VEHICLE_LATERALDAMPFORCE * vec3f_dot(vel, right));
				physx::PxRigidBodyExt::addForceAtLocalPos(*v->body, physx::PxVec3(damp[VX], damp[VY], damp[VZ]), physx::PxVec3(0.f, 0.f, 0.f));

				break;
			}
}

void vehiclemanager_update(struct vehiclemanager* vm)
{
	physx::PxQueryFilterData filterdata;
	physx::PxRaycastBuffer hit;
	physx::PxHitFlags outflags;
	physx::PxTransform pose;
	struct vehicle* v;
	vec3f dist, force;
	int i, j;

	// we are only interested in the raycast distance
	outflags = physx::PxHitFlag::eDISTANCE;

	// only test against static objects
	filterdata = physx::PxQueryFilterData(physx::PxQueryFlag::eSTATIC);

	for (i = 0; i < VEHICLE_COUNT; i++)
	{
		v = vm->vehicles + i;

		if (!(v->flags & VEHICLE_FLAG_ENABLED))
			continue;

		pose = v->body->getGlobalPose();

		// update vehicle position
		vec3f_set(v->pos, pose.p.x, pose.p.y, pose.p.z);

		// update channel position
		soundchannel_setposition(v->channel, v->pos);

		// update track path index
		v->index_track = track_closestindex(vm->track, v->pos, v->index_track);
		
		// reset vehicle if it leaves the track
		vec3f_subtractn(dist, v->pos, vm->track->pathpoints[v->index_track].pos);
		if (vec3f_length(dist) > vm->track->dist_boundary)
			vehiclemanager_resetvehicle(v->vm, v);

		updatespeed(v, vm->track);

		// process controller input
		vehicleinput(v);

		physx::PxMat44 transform(pose);

		// suspension raycasts
		for (j = 0; j < VEHICLE_COUNT_RAYCASTS; j++)
		{
			vec3f g_origin, g_dir;
			float d;

			// transform ray into global coordinates
			mat4f_fulltransformvec3fn(g_origin, v->ray_origins[j], (float*)&transform);
			mat4f_transformvec3fn(g_dir, v->ray_dirs[j], (float*)&transform);

			// raycast
			v->ray_touch[j] = vm->pm->scene->raycast(physx::PxVec3(g_origin[VX], g_origin[VY], g_origin[VZ]), physx::PxVec3(g_dir[VX], g_dir[VY], g_dir[VZ]), VEHICLE_RAYCAST_MAXDIST, hit, outflags, filterdata);

			if (v->ray_touch[j])
			{
				d = hit.block.distance;

				// calculate raycast force (quadratic)
				vec3f_scalen(force, v->ray_dirs[j], -VEHICLE_RAYCAST_MAXFORCE * (d*d/(VEHICLE_RAYCAST_MAXDIST * VEHICLE_RAYCAST_MAXDIST) - 2.f*d/VEHICLE_RAYCAST_MAXDIST + 1.f));

				physx::PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(v->ray_origins[j][VX], v->ray_origins[j][VY], v->ray_origins[j][VZ]));
			}
		}

		// down force
		vec3f_set(force, VEHICLE_DOWN);
		vec3f_scale(force, VEHICLE_DOWNFORCE * fabs(v->speed));

		physx::PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, 0.f));

		// check if a missile hit the vehicle
		if (v->flags & VEHICLE_FLAG_MISSILEHIT)
		{
			v->body->clearForce();
			v->body->setLinearVelocity(physx::PxVec3(0.f));
			physx::PxRigidBodyExt::addForceAtLocalPos(*v->body, physx::PxVec3(MISSILE_LOCALFORCE), physx::PxVec3(0.f, 0.f, 0.f));
			v->flags &= ~VEHICLE_FLAG_MISSILEHIT;
		}

		// check if a mine hit the vehicle
		if (v->flags & VEHICLE_FLAG_MINEHIT)
		{
			v->body->clearForce();
			v->body->setLinearVelocity(physx::PxVec3(0.f));
			physx::PxRigidBodyExt::addForceAtLocalPos(*v->body, physx::PxVec3(MINE_LOCALFORCE), physx::PxVec3(0.f, 0.f, 0.f));
			v->flags &= ~VEHICLE_FLAG_MINEHIT;
		}
	}
}


static FMOD_RESULT F_CALLBACK enginecallback(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void* commanddata1, void* commanddata2)
{
	struct vehicle* v;
	
	(void)commanddata1;
	(void)commanddata2;

	// retrieve pointer to vehicle from the channel user data
	FMOD_Channel_GetUserData(channel, (void**)&v);

	// if the channel ended, start the idle engine sound
	if (type == FMOD_CHANNEL_CALLBACKTYPE_END)
		v->channel = audiomanager_playsfx(v->vm->am, v->vm->sfx_engine_idle, v->pos, -1);

	return FMOD_OK;
}

struct vehicle* vehiclemanager_newvehicle(struct vehiclemanager* vm, struct controller* cntrl, int index_track, vec3f offs)
{
	physx::PxTransform pose;
	struct vehicle* v;
	vec3f spawn, dim;
	mat4f basis;
	int i;

	for (i = 0; i < VEHICLE_COUNT; i++)
		if (!(vm->vehicles[i].flags & VEHICLE_FLAG_ENABLED))
			break;

	if (i == VEHICLE_COUNT)
		return NULL;

	v = vm->vehicles + i;

	// store pointer to vehicle manager
	v->vm = vm;

	// find spawn global pose
	track_transformindex(vm->track, basis, index_track);

	// spawn offset
	vec3f_copy(spawn, vm->track->up);
	vec3f_scale(spawn, VEHICLE_SPAWNHEIGHT);
	vec3f_add(spawn, offs);
	mat4f_translatemul(basis, spawn[VX], spawn[VY], spawn[VZ]);

	pose = physx::PxTransform((physx::PxMat44)basis);

	// create a physics object and add it to the scene
	v->body = physx::PxCreateDynamic(*vm->pm->sdk, pose, physx::PxBoxGeometry(VEHICLE_DIMENSIONS), *vm->pm->default_material, VEHICLE_DENSITY);
	collision_setupactor(v->body, COLLISION_FILTER_VEHICLE, COLLISION_FILTER_MISSILE | COLLISION_FILTER_MINE | COLLISION_FILTER_PICKUP);
	v->body->userData = v;

	// store position
	vec3f_set(v->pos, pose.p.x, pose.p.y, pose.p.z);

	// add actor to scene
	vm->pm->scene->addActor(*v->body);

	// set damping forces
	v->body->setLinearDamping(VEHICLE_DAMP_LINEAR);
	v->body->setAngularDamping(VEHICLE_DAMP_ANGULAR);

	// set controller pointer
	v->controller = cntrl;

	// initialize the audio channel
	v->channel = audiomanager_playsfx(vm->am, vm->sfx_engine_start, v->pos, 0);
	FMOD_Channel_SetCallback(v->channel, enginecallback);
	FMOD_Channel_SetUserData(v->channel, v);

	// create the raycast origins in "model" space
	vec3f_set(dim, VEHICLE_DIMENSIONS);
	vec3f_set(v->ray_origins[VEHICLE_RAYCAST_FRONTLEFT], -dim[VX] + VEHICLE_RAYCAST_WIDTHOFFSET, -dim[VY], -dim[VZ] + VEHICLE_RAYCAST_FRONTOFFSET);
	vec3f_set(v->ray_origins[VEHICLE_RAYCAST_FRONTRIGHT], dim[VX] - VEHICLE_RAYCAST_WIDTHOFFSET, -dim[VY], -dim[VZ] + VEHICLE_RAYCAST_FRONTOFFSET);
	vec3f_set(v->ray_origins[VEHICLE_RAYCAST_BACKLEFT], -dim[VX] + VEHICLE_RAYCAST_WIDTHOFFSET, -dim[VY], dim[VZ] - VEHICLE_RAYCAST_BACKOFFSET);
	vec3f_set(v->ray_origins[VEHICLE_RAYCAST_BACKRIGHT], dim[VX] - VEHICLE_RAYCAST_WIDTHOFFSET, -dim[VY], dim[VZ] - VEHICLE_RAYCAST_BACKOFFSET);

	// create the raycast directions in "model" space
	vec3f_set(v->ray_dirs[VEHICLE_RAYCAST_FRONTLEFT], 0.f, -1.f, 0.f);
	vec3f_set(v->ray_dirs[VEHICLE_RAYCAST_FRONTRIGHT], 0.f, -1.f, 0.f);
	vec3f_set(v->ray_dirs[VEHICLE_RAYCAST_BACKLEFT], 0.f, -1.f, 0.f);
	vec3f_set(v->ray_dirs[VEHICLE_RAYCAST_BACKRIGHT], 0.f, -1.f, 0.f);

	v->index_track = index_track;

	// randomize diffuse texture
	v->index_diffuse = random_int(VEHICLE_TEXTURE_DIFFUSE_COUNT);

	// enabled flag
	v->flags = VEHICLE_FLAG_ENABLED;

	return v;
}

void vehiclemanager_removevehicle(struct vehiclemanager* vm, struct vehicle* v)
{
	int i;

	for (i = 0; i < VEHICLE_COUNT; i++)
		if (v == vm->vehicles + i)
		{
			vm->vehicles[i].body->release();
			vm->vehicles[i].flags = VEHICLE_FLAG_INIT;
		}
}


void vehiclemanager_render(struct vehiclemanager* vm, struct renderer* r, mat4f worldview)
{
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	struct vehicle* v;
	int i;

	for (i = 0; i < VEHICLE_COUNT; i++)
	{
		v = vm->vehicles + i;

		if (v->flags & VEHICLE_FLAG_ENABLED)
		{
			pose = v->body->getGlobalPose();

			mat_pose = physx::PxMat44(pose);
			vm->r_vehicle.textures[RENDER_TEXTURE_DIFFUSE] = vm->diffuse_vehicle + v->index_diffuse;
			renderable_render(r, &vm->r_vehicle, (float*)&mat_pose, worldview, 0);

			if (v->flags & VEHICLE_FLAG_HASPOWERUP)
			{
				mat_pose = physx::PxMat44(pose.transform(physx::PxTransform(VEHICLE_POWERUP_ATTACHLOCATION)));
				vm->r_powerup.textures[RENDER_TEXTURE_DIFFUSE] = vm->diffuse_powerup + v->powerup;
				renderable_render(r, &vm->r_powerup, (float*)&mat_pose, worldview, 0);
			}
		}
	}
}


void vehiclemanager_resetvehicle(struct vehiclemanager* vm, struct vehicle* v)
{
	mat4f basis;
	vec3f offs;

	// find transformation matrix
	track_transformindex(vm->track, basis, v->index_track);

	vec3f_copy(offs, vm->track->up);
	vec3f_scale(offs, VEHICLE_SPAWNHEIGHT);
	mat4f_translatemul(basis, offs[VX], offs[VY], offs[VZ]);

	// set the pose and velocities
	v->body->setGlobalPose(physx::PxTransform((physx::PxMat44)basis));
	v->body->setLinearVelocity(physx::PxVec3(0.f, 0.f, 0.f));
	v->body->setAngularVelocity(physx::PxVec3(0.f, 0.f, 0.f));
}