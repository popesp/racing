#include	"entities.h"
#include	"../render/objloader.h"
#include	<time.h>

void entitymanager_startup(struct entitymanager* em, struct physicsmanager* pm, struct renderer* r,struct audiomanager* am, struct track* t)
{
	struct missile* m;
	struct pickup* pu;
	struct mine* x;

	float temp;
	vec3f min, max, avg, diff;
	int i;

	em->pm = pm;
	em->track = t;

	renderable_init(&em->r_missile, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(MISSILE_OBJ, r, &em->r_missile);
	renderable_sendbuffer(r, &em->r_missile);

	renderable_init(&em->r_pickup, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(PICKUP_OBJ, r, &em->r_pickup);
	renderable_sendbuffer(r, &em->r_pickup);

	renderable_init(&em->r_mine, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(MINE_OBJ, r, &em->r_mine);
	renderable_sendbuffer(r, &em->r_mine);

	/////////////////////////////////////////////////////////////////////////
	//PICKUP
	// find the limits of the loaded mesh
	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; (unsigned)i < em->r_pickup.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, em->r_pickup.buf_verts + i*r->vertsize[em->r_pickup.type]);

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
	vec3f_scalen(em->dim_pickup, diff, PICKUP_MESHSCALE);

	// swap x and z to get the correct vehicle dimensions
	temp = em->dim_pickup[VX];
	em->dim_pickup[VX] = em->dim_pickup[VZ];
	em->dim_pickup[VZ] = temp;

	mat4f_scalemul(em->r_pickup.matrix_model, PICKUP_MESHSCALE, PICKUP_MESHSCALE, PICKUP_MESHSCALE);
	mat4f_rotateymul(em->r_pickup.matrix_model, -1.57080f);
	mat4f_translatemul(em->r_pickup.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);

	// initialize pickup texture
	texture_init(&em->diffuse_pickup);
	texture_loadfile(&em->diffuse_pickup, PICKUP_MINE_TEXTURE);
	texture_upload(&em->diffuse_pickup, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_pickup2);
	texture_loadfile(&em->diffuse_pickup2, PICKUP_MISSILE_TEXTURE);
	texture_upload(&em->diffuse_pickup2, RENDER_TEXTURE_DIFFUSE);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//MISSILE
	// find the limits of the loaded mesh
	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; (unsigned)i < em->r_missile.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, em->r_missile.buf_verts + i*r->vertsize[em->r_missile.type]);

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
	vec3f_scalen(em->dim_missile, diff, MISSILE_MESHSCALE);

	// swap x and z to get the correct vehicle dimensions
	temp = em->dim_missile[VX];
	em->dim_missile[VX] = em->dim_missile[VZ];
	em->dim_missile[VZ] = temp;

	mat4f_scalemul(em->r_missile.matrix_model, MISSILE_MESHSCALE, MISSILE_MESHSCALE, MISSILE_MESHSCALE);
	mat4f_rotateymul(em->r_missile.matrix_model, 3.f);
	mat4f_translatemul(em->r_missile.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);

	// initialize missile texture
	texture_init(&em->diffuse_missile);
	texture_loadfile(&em->diffuse_missile, MISSILE_TEXTURE);
	texture_upload(&em->diffuse_missile, RENDER_TEXTURE_DIFFUSE);
	em->r_missile.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_missile;

	/////////////////////////////////////////////////////////////////////////
	//MINE STUFF
	/////////////////////////////////////////////////////////////////////////
	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; (unsigned)i < em->r_mine.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, em->r_mine.buf_verts + i*r->vertsize[em->r_mine.type]);

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
	vec3f_scalen(em->dim_mine, diff, MINE_MESHSCALE);

	// swap x and z to get the correct vehicle dimensions
	temp = em->dim_mine[VX];
	em->dim_mine[VX] = em->dim_mine[VZ];
	em->dim_mine[VZ] = temp;

	mat4f_scalemul(em->r_mine.matrix_model, MINE_MESHSCALE, MINE_MESHSCALE, MINE_MESHSCALE);
	mat4f_rotateymul(em->r_mine.matrix_model, -1.57080f);
	mat4f_translatemul(em->r_mine.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);

	// initialize pickup texture
	texture_init(&em->diffuse_mine);
	texture_loadfile(&em->diffuse_mine, MINE_TEXTURE);
	texture_upload(&em->diffuse_mine, RENDER_TEXTURE_DIFFUSE);
	em->r_mine.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_mine;

	// initialize missile array
	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
	{
		m = em->missiles + i;

		m->body = NULL;
		m->owner = NULL;
		m->flags = ENTITY_MISSILE_FLAG_INIT;
	}

	// pickup array
	for(i=0;i<ENTITY_PICKUP_COUNT;i++){
		pu = em->pickups+i;

		pu->body = NULL;
		pu->owner = NULL;
		pu->flags = ENTITY_PICKUP_FLAG_INIT;
	}

	// mine array
	for(i=0;i<ENTITY_MINE_COUNT;i++){
		x = em->mines+i;

		x->body = NULL;
		x->owner = NULL;
		x->flags = ENTITY_MINE_FLAG_INIT;
	}

	// create sound for missles
	em->sfx_missile = audiomanager_newsfx(am, SFX_MISSLE_FILENAME);
}

void entitymanager_shutdown(struct entitymanager* em)
{
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED)
			em->missiles[i].body->release();

	for(i=0;i<ENTITY_PICKUP_COUNT;i++){
		if (em->pickups[i].flags & ENTITY_PICKUP_FLAG_ENABLED){
			em->pickups[i].body->release();
		}
	}

	for(i=0;i<ENTITY_MINE_COUNT;i++){
		if (em->mines[i].flags & ENTITY_MINE_FLAG_ENABLED){
			em->mines[i].body->release();
		}
	}

	renderable_deallocate(&em->r_missile);
	renderable_deallocate(&em->r_pickup);
	renderable_deallocate(&em->r_mine);
}

void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview)
{
	int i;
	//aiplayer_mw = physx::PxMat44(game->aiplayers[i].vehicle->body->getGlobalPose());
	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED)
			renderable_render(r, &em->r_missile, (float*)&physx::PxMat44(em->missiles[i].body->getGlobalPose()), worldview, 0);

	for(i=0; i<ENTITY_MINE_COUNT;i++){
		if (em->mines[i].flags & ENTITY_MINE_FLAG_ENABLED){
			renderable_render(r, &em->r_mine, (float*)&physx::PxMat44(em->mines[i].body->getGlobalPose()), worldview, 0);
		}
	}

	for (i = 0; i < ENTITY_PICKUP_COUNT; i++)
		if (em->pickups[i].flags & ENTITY_PICKUP_FLAG_ENABLED)
			renderable_render(r, &em->r_pickup, (float*)&physx::PxMat44(em->pickups[i].body->getGlobalPose()), worldview, 0);
}

void entitymanager_update(struct entitymanager* em)
{
	int i;
	physx::PxTransform pose;
	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED)
		{
			em->missiles[i].timer--;

			if (em->missiles[i].timer == 0){
				entitymanager_removemissile(em, em->missiles + i);
				continue;
			}

			pose = em->missiles[i].body->getGlobalPose();

			// update missle position
			vec3f_set(em->missiles[i].pos, pose.p.x, pose.p.y, pose.p.z);
			
			//audiomanager_setsoundposition(em->missiles[i].missle_channel, em->missiles[i].pos);
		}
}

struct missile* entitymanager_newmissile(struct entitymanager* em, struct vehicle* v, vec3f dim)
{
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	struct missile* m;
	vec3f zero, vel;
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (!(em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED))
			break;

	if (i == ENTITY_MISSILE_COUNT)
		return NULL;

	m = em->missiles + i;

	pose = v->body->getGlobalPose().transform(physx::PxTransform(0.f, 0.f, -(dim[VZ]*1.5f + ENTITY_MISSILE_SPAWNDIST)));
	mat_pose = physx::PxMat44(pose);
	vec3f_set(zero, 0.f, 0.f, 0.f);

	m->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxBoxGeometry(em->dim_missile[VX] * 0.5f, em->dim_missile[VY] * 0.5f, em->dim_missile[VZ] * 0.5f), *em->pm->default_material, ENTITY_MISSILE_DENSITY);
	m->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	setupFiltering(m->body, FilterGroup::eProjectile, FilterGroup::eProjectile);


	em->pm->scene->addActor(*m->body);
	vec3f_set(vel, 0.f, 0.f, -1.f);
	mat4f_transformvec3f(vel, (float*)&mat_pose);
	vec3f_scale(vel, ENTITY_MISSILE_SPEED);
	
	m->body->setLinearVelocity(physx::PxVec3(vel[VX], vel[VY], vel[VZ]));

	m->owner = v;

	m->timer = ENTITY_MISSILE_DESPAWNTIME;

	m->flags = ENTITY_MISSILE_FLAG_ENABLED;

	//audiomanager_playsfx(em->am, em->sfx_missile, m->pos, 0);

	return m;
}

void entitymanager_removemissile(struct entitymanager* em, struct missile* m)
{
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (m == em->missiles + i)
		{
			em->missiles[i].body->release();
			em->missiles[i].flags = ENTITY_MISSILE_FLAG_INIT;
		}
}

/*
struct pickup* entitymanager_attachpickup(struct entity* em, vec3f dim, struct vehicle* v){
	
	struct pickup* pu;

	pu->owner = v;
	
}
*/

struct pickup* entitymanager_newpickup(struct entitymanager* em, vec3f dim){
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	struct pickup* pu;
	vec3f spawn;
	int i;


	//assign a random spawnpoint
	unsigned int max = em->track->num_pathpoints;

	unsigned int seed = static_cast<unsigned int>(time(0))%max;
	srand(seed);

	for (i = 0; i < ENTITY_PICKUP_COUNT; i++)
		if (!(em->pickups[i].flags & ENTITY_PICKUP_FLAG_ENABLED))
			break;

	if (i == ENTITY_PICKUP_COUNT)
		return NULL;

	pu = em->pickups + i;

	if(seed%2 ==1){
		em->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_pickup2;
	}else{
		em->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_pickup;
	}

	// find spawn location
	vec3f_copy(pu->pos, em->track->pathpoints[seed].pos);
	vec3f_copy(spawn, em->track->up);
	vec3f_scale(spawn, ENTITY_PICKUP_SPAWNHEIGHT);
	vec3f_add(pu->pos, spawn);
	vec3f_add(pu->pos, dim);

	// create a physics object and add it to the scene
	pu->body = physx::PxCreateDynamic(*em->pm->sdk, physx::PxTransform(pu->pos[VX], pu->pos[VY], pu->pos[VZ]), physx::PxBoxGeometry(em->dim_pickup[VX] * 0.5f, em->dim_pickup[VY] * 0.5f, em->dim_pickup[VZ] * 0.5f), *em->pm->default_material, ENTITY_PICKUP_DENSITY);
	//pu->body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
	setupFiltering(pu->body, FilterGroup::ePickup, FilterGroup::ePickup);
	em->pm->scene->addActor(*pu->body);

	pu->flags = ENTITY_PICKUP_FLAG_ENABLED;

	return pu;
}

void entitymanager_removepickup(struct entitymanager* em, struct pickup* pu){
	int i;

	for(i=0;i<ENTITY_PICKUP_COUNT; i++){
		if(pu==em->pickups+i){
			em->pickups[i].body->release();
			em->pickups[i].flags = ENTITY_PICKUP_FLAG_INIT;
		}
	}
}

struct mine* entitymanager_newmine(struct entitymanager* em, vec3f dim, struct vehicle* v){
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	struct mine* x;
	int i;

	for (i = 0; i < ENTITY_MINE_COUNT; i++)
		if (!(em->mines[i].flags & ENTITY_MINE_FLAG_ENABLED))
			break;

	if (i == ENTITY_MINE_COUNT)
		return NULL;

	x = em->mines + i;

	// find spawn location
	pose = v->body->getGlobalPose().transform(physx::PxTransform(0.f, 0.f, -(dim[VZ]*0.5f - ENTITY_MINE_SPAWNDIST)));
	mat_pose = physx::PxMat44(pose);

	// create a physics object and add it to the scene
	x->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxBoxGeometry(em->dim_mine[VX] * 0.5f, em->dim_mine[VY] * 0.5f, em->dim_mine[VZ] * 0.5f), *em->pm->default_material, ENTITY_MINE_DENSITY);
	x->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	setupFiltering(x->body, FilterGroup::eMine, FilterGroup::eMine);
	em->pm->scene->addActor(*x->body);

	x->owner = v;
	x->flags = ENTITY_MINE_FLAG_ENABLED;

	return x;
}

void entitymanager_removemine(struct entitymanager* em, struct mine* x){
	int i;

	for(i=0;i<ENTITY_MINE_COUNT;i++){
		if(x==em->mines+i){
			em->mines[i].body->release();
			em->mines[i].flags = ENTITY_MINE_FLAG_INIT;
		}
	}
}