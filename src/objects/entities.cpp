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
	em->r = r;

	em->timerspawn1=0;
	em->timerspawn2=0;

	renderable_init(&em->r_missile, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(MISSILE_OBJ, r, &em->r_missile);
	renderable_sendbuffer(r, &em->r_missile);

	renderable_init(&em->r_mine, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(MINE_OBJ, r, &em->r_mine);
	renderable_sendbuffer(r, &em->r_mine);


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

		renderable_init(&pu->r_pickup, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
		objloader_load(PICKUP_OBJ, em->r, &pu->r_pickup);
		renderable_sendbuffer(em->r, &pu->r_pickup);


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
			em->pickups[i].body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
			em->pickups[i].body->release();
		}
		renderable_deallocate(&em->pickups[i].r_pickup);
	}

	for(i=0;i<ENTITY_MINE_COUNT;i++){
		if (em->mines[i].flags & ENTITY_MINE_FLAG_ENABLED){
			em->mines[i].body->release();
		}
	}

	renderable_deallocate(&em->r_missile);
	renderable_deallocate(&em->r_mine);
}

void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview)
{
	int i;

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
			renderable_render(r, &em->pickups[i].r_pickup, (float*)&physx::PxMat44(em->pickups[i].body->getGlobalPose()), worldview, 0);
}

void entitymanager_update(struct entitymanager* em, struct vehiclemanager* vm)
{
	int i;
	physx::PxTransform pose;

	if(em->pickupatspawn1==false){
		em->timerspawn1--;
		//printf("%d\n",em->timerspawn1);
		if(em->timerspawn1==0){
			entitymanager_newpickup(em, vm->track->pathpoints[PICKUP_SPAWN_LOC1].pos); 
			em->pickupatspawn1=true;
			em->timerspawn1 = PICKUP_TIMERS;
		}
	}


	if(em->pickupatspawn2==false){
		em->timerspawn2--;
		//printf("%d\n",em->timerspawn2);
		if(em->timerspawn2==0){
			entitymanager_newpickup(em, vm->track->pathpoints[PICKUP_SPAWN_LOC2].pos); 
			em->pickupatspawn2=true;
			em->timerspawn2 = PICKUP_TIMERS;
		}
	}


	//Check what custom collisions happened for each vehicle/entity and deal with them appropriately
	for (i = 0; i < VEHICLE_COUNT; i++) {
		if (vm->vehicles[i].hit_flag == 1) {
			vm->vehicles[i].body->clearForce();
			physx::PxRigidBodyExt::addForceAtLocalPos(*vm->vehicles[i].body, physx::PxVec3(0, 1500, 0), physx::PxVec3(0,0,0));
		}
	}
	for (i = 0; i < ENTITY_MISSILE_COUNT; i++) {
		if (vm->em->missiles[i].hit == 1) {
			entitymanager_removemissile(vm->em, &vm->em->missiles[i]);
		}
	}
	for (i = 0; i < ENTITY_MINE_COUNT; i++) {
		if (vm->em->mines[i].hit == 1) {
			entitymanager_removemine(vm->em, &vm->em->mines[i]);
		}
	}
	for (i = 0; i < ENTITY_PICKUP_COUNT; i++) {
		if (vm->em->pickups[i].hit > -1) {
			printf("index: %d\n", vm->em->pickups[i].hit);
			entitymanager_attachpickup(&vm->vehicles[vm->em->pickups[i].hit] , &vm->em->pickups[i],vm->em);
			/////
			//pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickup;


			vm->em->pickups[i].hit = -1;
		}
	}


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
			
			audiomanager_setsoundposition(em->missiles[i].missle_channel, em->missiles[i].pos);
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
	m->body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	m->body->userData = m;
	em->pm->scene->addActor(*m->body);
	vec3f_set(vel, 0.f, 0.f, -1.f);
	mat4f_transformvec3f(vel, (float*)&mat_pose);
	vec3f_scale(vel, ENTITY_MISSILE_SPEED);


	// update missle position
	vec3f_set(m->pos, pose.p.x, pose.p.y, pose.p.z);

	m->body->setLinearVelocity(physx::PxVec3(vel[VX], vel[VY], vel[VZ]));

	m->owner = v;

	m->timer = ENTITY_MISSILE_DESPAWNTIME;

	m->flags = ENTITY_MISSILE_FLAG_ENABLED;

	m->missle_channel = audiomanager_playsfx(em->am, em->sfx_missile, m->pos, -1,1.5);

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
			em->missiles[i].missle_channel = NULL;

		}

		
}


void entitymanager_attachpickup(struct vehicle* v, struct pickup* pu,struct entitymanager* em){

	for(int i=0;i<ENTITY_PICKUP_COUNT; i++){
		if((em->pickups+i)==pu && pu->holdingpu1==true){
			em->pickupatspawn1=false;
		}
		else if((em->pickups+i)==pu && pu->holdingpu2==true){
			em->pickupatspawn2=false;
		}
	}

	pu->owner = v;

	em->pm->scene->removeActor(*pu->body);
	
	v->haspickup = pu->typepickup;





	///THIS SHIT ISNT WORKING
	//0=mine, 1=missile, 2=speed
	if(pu->typepickup==0){
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMINE;
	}
	else if(pu->typepickup==1){
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMISSILE;
	}
	else if(pu->typepickup==2){
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupSPEED;
	}
	
}


struct pickup* entitymanager_newpickup(struct entitymanager* em, vec3f pos){
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	struct pickup* pu;
	vec3f spawn,min, max, avg, diff;
	int i;
	float temp;

	for (i = 0; i < ENTITY_PICKUP_COUNT; i++)
		if (!(em->pickups[i].flags & ENTITY_PICKUP_FLAG_ENABLED))
			break;

	if (i == ENTITY_PICKUP_COUNT)
		return NULL;

	pu = em->pickups + i;
	printf("pickup %d\n", i);


	// logic for attaching timers for respawning pickups
	pu->holdingpu1=false;
	pu->holdingpu2=false;
	if(em->timerspawn1==0){
		pu->holdingpu1=true;
		em->timerspawn1=PICKUP_TIMERS;
	}
	else if(em->timerspawn2==0){
		pu->holdingpu2=true;
		em->timerspawn2=PICKUP_TIMERS;
	}

	// find the limits of the loaded mesh
	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; (unsigned)i < pu->r_pickup.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, pu->r_pickup.buf_verts + i*em->r->vertsize[pu->r_pickup.type]);

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
	vec3f_scalen(pu->dim_pickup, diff, PICKUP_MESHSCALE);

	// swap x and z to get the correct vehicle dimensions
	temp = pu->dim_pickup[VX];
	pu->dim_pickup[VX] = pu->dim_pickup[VZ];
	pu->dim_pickup[VZ] = temp;

	mat4f_scalemul(pu->r_pickup.matrix_model, PICKUP_MESHSCALE, PICKUP_MESHSCALE, PICKUP_MESHSCALE);
	mat4f_rotateymul(pu->r_pickup.matrix_model, -1.57080f);

	if(pu->set!=1){
		mat4f_translatemul(pu->r_pickup.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);
	}
	pu->set=1;

	pu->avg[VX] = avg[VX];
	pu->avg[VY] = avg[VY];
	pu->avg[VZ] = avg[VZ];
		// initialize pickup textures, doing it here so no memory leaks
		texture_init(&pu->diffuse_pickupMINE);
		texture_loadfile(&pu->diffuse_pickupMINE, PICKUP_MINE_TEXTURE);
		texture_upload(&pu->diffuse_pickupMINE, RENDER_TEXTURE_DIFFUSE);

		texture_init(&pu->diffuse_pickupMISSILE);
		texture_loadfile(&pu->diffuse_pickupMISSILE, PICKUP_MISSILE_TEXTURE);
		texture_upload(&pu->diffuse_pickupMISSILE, RENDER_TEXTURE_DIFFUSE);

		texture_init(&pu->diffuse_pickupSPEED);
		texture_loadfile(&pu->diffuse_pickupSPEED, PICKUP_SPEED_TEXTURE);
		texture_upload(&pu->diffuse_pickupSPEED, RENDER_TEXTURE_DIFFUSE);

	int seed = static_cast<int>(time(0));
	srand(seed);
	seed = seed%3;
	if(seed==1){
		//Missile
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMISSILE;
		pu->typepickup=1;
	}
	else if(seed==2){
		//Mine
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMINE;
		pu->typepickup=0;
	}
	else{
		//Speed
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupSPEED;
		pu->typepickup=2;
	}

	// find spawn location
	vec3f_copy(pu->pos, pos);
	vec3f_copy(spawn, em->track->up);
	vec3f_scale(spawn, ENTITY_PICKUP_SPAWNHEIGHT);
	vec3f_add(pu->pos, spawn);

	// create a physics object and add it to the scene
	pu->body = physx::PxCreateDynamic(*em->pm->sdk, physx::PxTransform(pu->pos[VX], pu->pos[VY], pu->pos[VZ]), physx::PxBoxGeometry(pu->dim_pickup[VX] * 0.5f, pu->dim_pickup[VY] * 0.5f, pu->dim_pickup[VZ] * 0.5f), *em->pm->default_material, ENTITY_PICKUP_DENSITY);
	setupFiltering(pu->body, FilterGroup::ePickup, FilterGroup::ePickup);
	pu->body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	pu->body->userData = pu;
	em->pm->scene->addActor(*pu->body);

	//vec3f_negate(pu->pos, spawn);
	pu->flags = ENTITY_PICKUP_FLAG_ENABLED;
	pu->owner = NULL;

	return pu;
}

void entitymanager_removepickup(struct entitymanager* em, struct pickup* pu){
	int i;

	for(i=0;i<ENTITY_PICKUP_COUNT; i++){
		if(pu==em->pickups+i){

			//mat4f_translatemul(em->pickups[i].r_pickup.matrix_model, 1/-pu->avg[VX], 1/-pu->avg[VY], 1/-pu->avg[VZ]);
			//mat4f_rotateymul(em->pickups[i].r_pickup.matrix_model, 1/-1.57080f);
			mat4f_scalemul(em->pickups[i].r_pickup.matrix_model, 1/PICKUP_MESHSCALE, 1/PICKUP_MESHSCALE, 1/PICKUP_MESHSCALE);
			
			em->pickups[i].body->release();
			em->pickups[i].flags = ENTITY_PICKUP_FLAG_INIT;
			em->pickups[i].owner = NULL;
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
	//x->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	setupFiltering(x->body, FilterGroup::eMine, FilterGroup::eMine);
	x->body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	x->body->userData = x;
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
