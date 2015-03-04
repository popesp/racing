#include	"vehicle.h"

#include	<float.h>
#include	"../render/objloader.h"


void vehiclemanager_startup(struct vehiclemanager* vm, struct physicsmanager* pm, struct entitymanager* em, struct renderer* r, struct track* t, const char* file_mesh, const char* file_diffuse)
{
	vec3f min, max, avg, diff;
	struct vehicle* v;
	float temp;
	unsigned i;

	vm->em = em;
	vm->pm = pm;
	vm->track = t;

	// initialize vehicle mesh
	renderable_init(&vm->r_vehicle, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(file_mesh, r, &vm->r_vehicle);
	renderable_sendbuffer(r, &vm->r_vehicle);

	// find the limits of the loaded mesh
	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (i = 0; i < vm->r_vehicle.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, vm->r_vehicle.buf_verts + i*r->vertsize[vm->r_vehicle.type]);

		// check for min and max vector positions
		if (temp[VX] < min[VX])
			min[VX] = temp[VX];
		if (temp[VX] > max[VX])
			max[VX] = temp[VX];

		if (temp[VY] < min[VY])
			min[VY] = temp[VY];
		if (temp[VY] > max[VY])
			max[VY] = temp[VY];

		if (temp[VZ] < min[VZ])
			min[VZ] = temp[VZ];
		if (temp[VZ] > max[VZ])
			max[VZ] = temp[VZ];
	}

	// find center point of model
	vec3f_addn(avg, min, max);
	vec3f_scale(avg, 0.5f);

	// find dimensions of model
	vec3f_subtractn(diff, max, min);
	vec3f_scalen(vm->dim, diff, VEHICLE_MESHSCALE);

	// swap x and z to get the correct vehicle dimensions
	temp = vm->dim[VX];
	vm->dim[VX] = vm->dim[VZ];
	vm->dim[VZ] = temp;

	mat4f_scalemul(vm->r_vehicle.matrix_model, VEHICLE_MESHSCALE, VEHICLE_MESHSCALE, VEHICLE_MESHSCALE);
	mat4f_rotateymul(vm->r_vehicle.matrix_model, -1.57080f);
	mat4f_translatemul(vm->r_vehicle.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);

	// initialize vehicle texture
	texture_init(&vm->diffuse);
	texture_loadfile(&vm->diffuse, file_diffuse);
	texture_upload(&vm->diffuse, RENDER_TEXTURE_DIFFUSE);
	vm->r_vehicle.textures[RENDER_TEXTURE_DIFFUSE] = &vm->diffuse;

	// initialize vehicle array
	for (i = 0; i < VEHICLE_COUNT; i++)
	{
		v = vm->vehicles + i;

		v->body = NULL;
		v->controller = NULL;
		v->flags = VEHICLE_FLAG_INIT;
	}
}

void vehiclemanager_shutdown(struct vehiclemanager* vm)
{
	int i;

	for (i = 0; i < VEHICLE_COUNT; i++)
		if (vm->vehicles[i].flags & VEHICLE_FLAG_ENABLED)
			vm->vehicles[i].body->release();

	renderable_deallocate(&vm->r_vehicle);
	texture_delete(&vm->diffuse);
}


struct vehicle* vehiclemanager_newvehicle(struct vehiclemanager* vm, int index_track, vec3f offs)
{
	struct vehicle* v;
	vec3f spawn;
	int i;

	for (i = 0; i < VEHICLE_COUNT; i++)
		if (!(vm->vehicles[i].flags & VEHICLE_FLAG_ENABLED))
			break;

	if (i == VEHICLE_COUNT)
		return NULL;

	v = vm->vehicles + i;

	// find spawn location
	vec3f_copy(v->pos, vm->track->pathpoints[index_track].pos);
	vec3f_copy(spawn, vm->track->up);
	vec3f_scale(spawn, VEHICLE_SPAWNHEIGHT);
	vec3f_add(v->pos, spawn);
	vec3f_add(v->pos, offs);

	// create a physics object and add it to the scene
	v->body = physx::PxCreateDynamic(*vm->pm->sdk, physx::PxTransform(v->pos[VX], v->pos[VY], v->pos[VZ]), physx::PxBoxGeometry(vm->dim[VX] * 0.5f, vm->dim[VY] * 0.5f, vm->dim[VZ] * 0.5f), *vm->pm->default_material, VEHICLE_DENSITY);
	vm->pm->scene->addActor(*v->body);

	// set damping forces
	v->body->setLinearDamping(VEHICLE_DAMP_LINEAR);
	v->body->setAngularDamping(VEHICLE_DAMP_ANGULAR);

	// create the raycast origins in "model" space
	vec3f_set(v->ray_origins[VEHICLE_RAYCAST_FRONTLEFT], -vm->dim[VX] * 0.5f + VEHICLE_RAYCAST_WIDTHOFFSET, -vm->dim[VY] * 0.5f, -vm->dim[VZ] * 0.5f + VEHICLE_RAYCAST_FRONTOFFSET);
	vec3f_set(v->ray_origins[VEHICLE_RAYCAST_FRONTRIGHT], vm->dim[VX] * 0.5f - VEHICLE_RAYCAST_WIDTHOFFSET, -vm->dim[VY] * 0.5f, -vm->dim[VZ] * 0.5f + VEHICLE_RAYCAST_FRONTOFFSET);
	vec3f_set(v->ray_origins[VEHICLE_RAYCAST_BACKLEFT], -vm->dim[VX] * 0.5f + VEHICLE_RAYCAST_WIDTHOFFSET, -vm->dim[VY] * 0.5f, vm->dim[VZ] * 0.5f - VEHICLE_RAYCAST_BACKOFFSET);
	vec3f_set(v->ray_origins[VEHICLE_RAYCAST_BACKRIGHT], vm->dim[VX] * 0.5f - VEHICLE_RAYCAST_WIDTHOFFSET, -vm->dim[VY] * 0.5f, vm->dim[VZ] * 0.5f - VEHICLE_RAYCAST_BACKOFFSET);

	// create the raycast directions in "model" space
	vec3f_set(v->ray_dirs[VEHICLE_RAYCAST_FRONTLEFT], 0.f, -1.f, 0.f);
	vec3f_set(v->ray_dirs[VEHICLE_RAYCAST_FRONTRIGHT], 0.f, -1.f, 0.f);
	vec3f_set(v->ray_dirs[VEHICLE_RAYCAST_BACKLEFT], 0.f, -1.f, 0.f);
	vec3f_set(v->ray_dirs[VEHICLE_RAYCAST_BACKRIGHT], 0.f, -1.f, 0.f);

	v->index_track = index_track;

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


static float getspeed(struct vehicle* v)
{
	vec3f vel, forward;

	physx::PxVec3 px_vel = v->body->getLinearVelocity();
	physx::PxMat44 transform(v->body->getGlobalPose());

	vec3f_set(vel, px_vel.x, px_vel.y, px_vel.z);

	vec3f_set(forward, VEHICLE_FORWARD);
	mat4f_transformvec3f(forward, (float*)&transform);

	return vec3f_dot(vel, forward);
}

static void vehicleinput(struct vehiclemanager* vm, struct vehicle* v, float speed)
{
	vec3f force;

	if (v->controller != NULL && v->controller->flags & INPUT_FLAG_ENABLED)
	{
		int i;

		// if at least one raycast is touching the ground, apply an acceleration force
		for (i = 0; i < VEHICLE_COUNT_RAYCASTS; i++)
			if (v->ray_touch[i])
			{
				vec3f_set(force, VEHICLE_FORWARD);
				vec3f_scale(force, -VEHICLE_ACCELERATION * v->controller->axes[INPUT_AXIS_TRIGGERS]);

				physx::PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, 0.f));

				break;
			}

		// turning force
		vec3f_set(force, VEHICLE_RIGHT);
		vec3f_scale(force, VEHICLE_TURNFORCE * v->controller->axes[INPUT_AXIS_LEFT_LR]);

		// TODO: this doesnt quite work right
		if (v->controller->axes[INPUT_AXIS_TRIGGERS] > 0.1f && speed < 0.f)
			vec3f_negate(force);
		
		physx::PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, -vm->dim[VZ]/2.f));
		
		// reset button
		if (v->controller->buttons[INPUT_BUTTON_BACK] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			vehicle_reset(vm, v);
		}

		// firing a projectile
		if (v->controller->buttons[INPUT_BUTTON_A] == (INPUT_STATE_DOWN | INPUT_STATE_CHANGED))
			entitymanager_newmissile(vm->em, v, vm->dim);
	}
}

void vehiclemanager_update(struct vehiclemanager* vm)
{
	physx::PxRaycastBuffer hit;
	physx::PxHitFlags outflags;
	physx::PxTransform pose;
	struct vehicle* v;
	vec3f dist, force;
	float speed;
	int i, j;

	outflags = physx::PxHitFlag::eDISTANCE;
	physx::PxQueryFilterData filterData(physx::PxQueryFlag::eSTATIC);

	for (i = 0; i < VEHICLE_COUNT; i++)
	{
		v = vm->vehicles + i;

		if (!(v->flags & VEHICLE_FLAG_ENABLED))
			continue;

		pose = v->body->getGlobalPose();

		// update vehicle position
		vec3f_set(v->pos, pose.p.x, pose.p.y, pose.p.z);

		// update track path index
		v->index_track = track_closestindex(vm->track, v->pos, v->index_track);

		// reset vehicle if it leaves the track
		vec3f_subtractn(dist, v->pos, vm->track->pathpoints[v->index_track].pos);
		if (vec3f_length(dist) > vm->track->dist_boundary)
			vehicle_reset(vm, v);

		// get the vehicle speed
		speed = getspeed(v);

		//monitors for flips
		
		if(speed<0.0001 && speed>-0.0005 && speed!=0){
			if(v->ray_touch[0]==false && v->ray_touch[1]==false && v->ray_touch[2]==false && v->ray_touch[3]==false){
				vehicle_reset(vm, v);
			}
		}
		
		// process controller input
		vehicleinput(vm, v, speed);

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
			v->ray_touch[j] = vm->pm->scene->raycast(physx::PxVec3(g_origin[VX], g_origin[VY], g_origin[VZ]), physx::PxVec3(g_dir[VX], g_dir[VY], g_dir[VZ]), VEHICLE_RAYCAST_MAXDIST, hit, outflags, filterData);

			if (v->ray_touch[j])
			{
				d = hit.block.distance;

				// calculate raycast force (quadratic)
				vec3f_scalen(force, v->ray_dirs[j], -VEHICLE_RAYCAST_MAXFORCE * (d*d/(VEHICLE_RAYCAST_MAXDIST * VEHICLE_RAYCAST_MAXDIST) - 2.f*d/VEHICLE_RAYCAST_MAXDIST + 1.f));

				physx::PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(v->ray_origins[j][VX], v->ray_origins[j][VY], v->ray_origins[j][VZ]));
			}
		}

		vec3f_set(force, VEHICLE_DOWN);
		vec3f_scale(force, VEHICLE_DOWNFORCE * fabs(speed));

		physx::PxRigidBodyExt::addLocalForceAtLocalPos(*v->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, 0.f));

	}
}


void vehicle_reset(struct vehiclemanager* vm, struct vehicle* v)
{
	vec3f nor, bin, tan, spawn;
	mat4f basis;

	// find negated tangent
	vec3f_copy(tan, vm->track->pathpoints[v->index_track].tan);
	vec3f_negate(tan);

	vec3f_cross(bin, vm->track->up, tan);
	vec3f_normalize(bin);

	vec3f_cross(nor, tan, bin);

	// find spawn location
	vec3f_copy(v->pos, vm->track->pathpoints[v->index_track].pos);
	vec3f_copy(spawn, vm->track->up);
	vec3f_scale(spawn, VEHICLE_SPAWNHEIGHT);
	vec3f_add(v->pos, spawn);

	// find the change of basis matrix
	mat4f_identity(basis);
	vec3f_copy(basis + C0, bin);
	vec3f_copy(basis + C1, nor);
	vec3f_copy(basis + C2, tan);
	vec3f_copy(basis + C3, v->pos); // translation

	// rotate to the track angle
	mat4f_rotatezmul(basis, -vm->track->pathpoints[v->index_track].angle);

	// TODO: set the orientation of the vehicle to the track gradient
	v->body->setGlobalPose(physx::PxTransform((physx::PxMat44)basis));
	v->body->setLinearVelocity(physx::PxVec3(0.f, 0.f, 0.f));
	v->body->setAngularVelocity(physx::PxVec3(0.f, 0.f, 0.f));
}
