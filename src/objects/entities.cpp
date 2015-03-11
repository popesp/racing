#include	"entities.h"
#include	"../render/objloader.h"
#include	<time.h>

void entitymanager_startup(struct entitymanager* em, struct physicsmanager* pm, struct renderer* r,struct audiomanager* am, struct track* t)
{
	struct missile* m;
	struct pickup* pu;
	struct mine* x;
	struct blimp* b;
	struct turret* tu;

	int i;

	em->pm = pm;
	em->track = t;
	em->r = r;

	//initialize to 0 so the pickups know to attach
	em->timerspawn1=0;
	em->timerspawn2=0;
	em->timerspawn3=0;
	em->timerspawn4=0;

	//keeps track of how much stuff is on our map
	em->num_blimps=0;
	em->num_mines=0;
	em->num_missiles=0;
	em->num_pickups=0;
	em->num_turrets=0;
	
	renderable_init(&em->r_missile, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(MISSILE_OBJ, r, &em->r_missile);
	renderable_sendbuffer(r, &em->r_missile);

	renderable_init(&em->r_turretmissile, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(MISSILE_OBJ, r, &em->r_turretmissile);
	renderable_sendbuffer(r, &em->r_turretmissile);

	renderable_init(&em->r_mine, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(MINE_OBJ, r, &em->r_mine);
	renderable_sendbuffer(r, &em->r_mine);

	renderable_init(&em->r_blimp, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(BLIMP_OBJ, em->r, &em->r_blimp);
	renderable_sendbuffer(em->r, &em->r_blimp);

	renderable_init(&em->r_blimplap, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(BLIMP_OBJ, em->r, &em->r_blimplap);
	renderable_sendbuffer(em->r, &em->r_blimplap);

	renderable_init(&em->r_turret, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(TURRET_OBJ, em->r, &em->r_turret);
	renderable_sendbuffer(r, &em->r_turret);

	//initialize all the textures for the objects
	entitymanager_textures(em, r);

	//initialize dimensions for the objects
	entitymanager_blimpinit(em);
	entitymanager_missileinit(em);
	entitymanager_mineinit(em);
	entitymanager_turretinit(em);

	em->r_missile.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_missile;
	em->r_turretmissile.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_missile;
	em->r_mine.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_mine;
	em->r_turret.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_turret;

	// initialize missile array
	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
	{
		m = em->missiles + i;
		m->body = NULL;
		m->owner = NULL;
		m->turretowner = NULL;
		m->flags = ENTITY_MISSILE_FLAG_INIT;
	}

	// pickup array
	for(i=0;i<ENTITY_PICKUP_COUNT;i++){
		pu = em->pickups+i;
		pu->body = NULL;
		pu->owner = NULL;
		pu->flags = ENTITY_PICKUP_FLAG_INIT;
		pu->hit=-1;

		texture_init(&pu->diffuse_pickupMISSILE);
		texture_init(&pu->diffuse_pickupMINE);
		texture_init(&pu->diffuse_pickupSPEED);
	}

	// mine array
	for(i=0;i<ENTITY_MINE_COUNT;i++){
		x = em->mines+i;
		x->body = NULL;
		x->owner = NULL;
		x->flags = ENTITY_MINE_FLAG_INIT;
	}

	// blimp array
	for(i=0;i<BLIMP_COUNT;i++){
		b = em->blimps+i;
		b->body = NULL;
		b->owner = NULL;
		b->flags = BLIMP_FLAG_INIT;
	}

	// turret array
	for(i=0;i<ENTITY_TURRET_COUNT;i++){
		tu = em->turrets+i;
		tu->body = NULL;
		tu->owner = NULL;
		tu->flags = TURRET_FLAG_INIT;
	}

	// create sound for missles and mines
	em->sfx_missile = audiomanager_newsfx(am, SFX_MISSLE_FILENAME);
	em->sfx_missile_exp = audiomanager_newsfx(am, SFX_MISSLE_EXP_FILENAME);

	em->sfx_mine = audiomanager_newsfx(am, SFX_MINE_FILENAME);
	em->sfx_mine_exp = audiomanager_newsfx(am, SFX_MINE_EXP_FILENAME);	
}

void entitymanager_shutdown(struct entitymanager* em)
{
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED)
			em->missiles[i].body->release();

	for(i=0;i<ENTITY_PICKUP_COUNT;i++){
		if (em->pickups[i].flags & ENTITY_PICKUP_FLAG_ENABLED){
			//this is currently broken
			//em->pickups[i].body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
			//em->pickups[i].body->release();
		}
		renderable_deallocate(&em->pickups[i].r_pickup);
	}

	for(i=0;i<ENTITY_MINE_COUNT;i++){
		if (em->mines[i].flags & ENTITY_MINE_FLAG_ENABLED){
			em->mines[i].body->release();
		}
	}

	for(i=0;i<BLIMP_COUNT;i++){
		if(em->blimps[i].flags & BLIMP_FLAG_ENABLED){
			//this is probably broken too
			em->blimps[i].body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
			em->blimps[i].body->release();
		}
		
	}

	renderable_deallocate(&em->r_blimp);
	renderable_deallocate(&em->r_missile);
	renderable_deallocate(&em->r_mine);
}

void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview)
{
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED){
			renderable_render(r, &em->r_missile, (float*)&physx::PxMat44(em->missiles[i].body->getGlobalPose()), worldview, 0);}
		else if(em->missiles[i].flags & ENTITY_TURRETMISSILE_FLAG_ENABLED){
			renderable_render(r, &em->r_turretmissile, (float*)&physx::PxMat44(em->missiles[i].body->getGlobalPose()), worldview, 0);
		}

	for(i=0; i<ENTITY_MINE_COUNT;i++){
		if (em->mines[i].flags & ENTITY_MINE_FLAG_ENABLED){
			renderable_render(r, &em->r_mine, (float*)&physx::PxMat44(em->mines[i].body->getGlobalPose()), worldview, 0);
		}
	}

	for(i=0; i<ENTITY_TURRET_COUNT;i++){
		if (em->turrets[i].flags & TURRET_FLAG_ENABLED){
			renderable_render(r, &em->r_turret, (float*)&physx::PxMat44(em->turrets[i].body->getGlobalPose()), worldview, 0);
		}
	}

	for (i = 0; i < ENTITY_PICKUP_COUNT; i++)
		if (em->pickups[i].flags & ENTITY_PICKUP_FLAG_ENABLED)
			renderable_render(r, &em->pickups[i].r_pickup, (float*)&physx::PxMat44(em->pickups[i].body->getGlobalPose()), worldview, 0);

	for (i=0;i<BLIMP_COUNT;i++){
		if(em->blimps[i].flags&BLIMP_FLAG_ENABLED){
			if(em->blimps[i].typeblimp&BLIMP_TYPE_PLACE){
				renderable_render(r, &em->r_blimp, (float*)&physx::PxMat44(em->blimps[i].body->getGlobalPose()),worldview,0);
			}
			else{
				renderable_render(r, &em->r_blimplap, (float*)&physx::PxMat44(em->blimps[i].body->getGlobalPose()),worldview,0);
			}
		}
	}
}

void entitymanager_update(struct entitymanager* em, struct vehiclemanager* vm)
{
	int i;
	physx::PxTransform pose,mine_pose;

	if(em->pickupatspawn1==false){
		em->timerspawn1--;
		//printf("%d\n",em->timerspawn1);
		if(em->timerspawn1<=0){
			entitymanager_newpickup(em, vm->track->pathpoints[PICKUP_SPAWN_LOC1].pos); 
			em->pickupatspawn1=true;
			em->timerspawn1 = PICKUP_TIMERS;
		}
	}

	if(em->pickupatspawn2==false){
		em->timerspawn2--;
		//printf("%d\n",em->timerspawn2);
		if(em->timerspawn2<=0){
			entitymanager_newpickup(em, vm->track->pathpoints[PICKUP_SPAWN_LOC2].pos); 
			em->pickupatspawn2=true;
			em->timerspawn2 = PICKUP_TIMERS;
		}
	}

	/*if(em->pickupatspawn3==false){
		em->timerspawn3--;
		printf("%d\n",em->timerspawn3);
		if(em->timerspawn3==0){
			entitymanager_newpickup(em, vm->track->pathpoints[PICKUP_SPAWN_LOC3].pos); 
			em->pickupatspawn3=true;
			em->timerspawn3 = PICKUP_TIMERS;
		}
	}*/
	
	if(em->pickupatspawn4==false){
		em->timerspawn4--;
		printf("%d\n",em->timerspawn4);
		if(em->timerspawn4<=0){
			//printf("new pikcup\n");
			entitymanager_newpickup(em, vm->track->pathpoints[PICKUP_SPAWN_LOC3].pos); 
			em->pickupatspawn4=true;
			em->timerspawn4 = PICKUP_TIMERS;
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
			audiomanager_playsfx(vm->em->am,vm->em->sfx_missile_exp,vm->em->missiles[i].pos,0);
			entitymanager_removemissile(vm->em, &vm->em->missiles[i]);
		}
	}
	for (i = 0; i < ENTITY_MINE_COUNT; i++) {
		if (vm->em->mines[i].hit == 1) {
			audiomanager_playsfx(vm->em->am,vm->em->sfx_mine_exp,vm->em->mines[i].pos,0);
			entitymanager_removemine(vm->em, &vm->em->mines[i]);
			continue;
		}
		if (em->mines[i].flags & ENTITY_MINE_FLAG_ENABLED){
			mine_pose = em->mines[i].body->getGlobalPose();

			// update mine position
			vec3f_set(em->mines[i].pos, mine_pose.p.x, mine_pose.p.y, mine_pose.p.z);
			
			audiomanager_setsoundposition(em->mines[i].mine_channel, em->mines[i].pos);
		}

	}
	for (i = 0; i < ENTITY_PICKUP_COUNT; i++) {
		if (vm->em->pickups[i].hit > -1) {
						
			if(vm->vehicles[vm->em->pickups[i].hit].haspickup!=100){
				entitymanager_removepickup(vm->em,vm->vehicles[vm->em->pickups[i].hit].owns);
			}

			entitymanager_attachpickup(&vm->vehicles[vm->em->pickups[i].hit] , &vm->em->pickups[i],vm->em);
			
			vm->em->pickups[i].hit = -1;
		}
	}

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED || em->missiles[i].flags & ENTITY_TURRETMISSILE_FLAG_ENABLED)
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
	
	for (i=0;i<ENTITY_TURRET_COUNT; i++){
		if(em->turrets[i].flags & TURRET_FLAG_ENABLED){
			em->turrets[i].timer--;

			//remove turret
			if(em->turrets[i].timer==0){
				entitymanager_removeturret(em,em->turrets+i);
				continue;
			}
			if(em->turrets[i].timer%20==0){
				entitymanager_turretmissile(em,em->turrets+i,vm->dim);
			}
		}
	}
}

struct missile* entitymanager_newmissile(struct entitymanager* em, struct vehicle* v, vec3f dim)
{
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	struct missile* m;
	struct missile* m1;
	struct missile* m2;
	vec3f zero, vel;
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (!(em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED))
			break;

	if (i == ENTITY_MISSILE_COUNT)
		return NULL;

	m = em->missiles + i;
	em->num_missiles++;

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

	//missile 2

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (!(em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED))
			break;

	if (i == ENTITY_MISSILE_COUNT)
		return NULL;

	m1 = em->missiles + i;
	em->num_missiles++;

	pose = v->body->getGlobalPose().transform(physx::PxTransform(2.f, 0.f, -(dim[VZ]*1.5f + ENTITY_MISSILE_SPAWNDIST)));
	mat_pose = physx::PxMat44(pose);
	vec3f_set(zero, 0.f, 0.f, 0.f);

	m1->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxBoxGeometry(em->dim_missile[VX] * 0.5f, em->dim_missile[VY] * 0.5f, em->dim_missile[VZ] * 0.5f), *em->pm->default_material, ENTITY_MISSILE_DENSITY);
	m1->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	setupFiltering(m1->body, FilterGroup::eProjectile, FilterGroup::eProjectile);
	m1->body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	m1->body->userData = m1;
	em->pm->scene->addActor(*m1->body);
	vec3f_set(vel, 0.f, 0.f, -1.f);
	mat4f_transformvec3f(vel, (float*)&mat_pose);
	vec3f_scale(vel, ENTITY_MISSILE_SPEED);

	// update missle position
	vec3f_set(m1->pos, pose.p.x, pose.p.y, pose.p.z);

	m1->body->setLinearVelocity(physx::PxVec3(vel[VX], vel[VY], vel[VZ]));

	m1->owner = v;

	m1->timer = ENTITY_MISSILE_DESPAWNTIME;

	m1->flags = ENTITY_MISSILE_FLAG_ENABLED;

	m1->missle_channel = audiomanager_playsfx(em->am, em->sfx_missile, m1->pos, -1,1.5);

	//missile 3

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (!(em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED))
			break;

	if (i == ENTITY_MISSILE_COUNT)
		return NULL;

	m2 = em->missiles + i;
	em->num_missiles++;

	pose = v->body->getGlobalPose().transform(physx::PxTransform(-2.f, 0.f, -(dim[VZ]*1.5f + ENTITY_MISSILE_SPAWNDIST)));
	mat_pose = physx::PxMat44(pose);
	vec3f_set(zero, 0.f, 0.f, 0.f);

	m2->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxBoxGeometry(em->dim_missile[VX] * 0.5f, em->dim_missile[VY] * 0.5f, em->dim_missile[VZ] * 0.5f), *em->pm->default_material, ENTITY_MISSILE_DENSITY);
	m2->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	setupFiltering(m2->body, FilterGroup::eProjectile, FilterGroup::eProjectile);
	m2->body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	m2->body->userData = m2;
	em->pm->scene->addActor(*m2->body);
	vec3f_set(vel, 0.f, 0.f, -1.f);
	mat4f_transformvec3f(vel, (float*)&mat_pose);
	vec3f_scale(vel, ENTITY_MISSILE_SPEED);

	// update missle position
	vec3f_set(m2->pos, pose.p.x, pose.p.y, pose.p.z);

	m2->body->setLinearVelocity(physx::PxVec3(vel[VX], vel[VY], vel[VZ]));

	m2->owner = v;

	m2->timer = ENTITY_MISSILE_DESPAWNTIME;

	m2->flags = ENTITY_MISSILE_FLAG_ENABLED;

	m2->missle_channel = audiomanager_playsfx(em->am, em->sfx_missile, m2->pos, -1, 1.5);

	return m;
}

struct missile* entitymanager_turretmissile(struct entitymanager* em, struct turret* tu, vec3f dim)
{
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	struct missile* m;
	vec3f zero, vel;
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (!(em->missiles[i].flags & ENTITY_TURRETMISSILE_FLAG_ENABLED))
			break;

	if (i == ENTITY_MISSILE_COUNT)
		return NULL;

	m = em->missiles + i;
	em->num_missiles++;

	pose = tu->body->getGlobalPose().transform(physx::PxTransform(0.f, 0.f, (dim[VZ]*1.5f + ENTITY_MISSILE_SPAWNDIST)));
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

	m->body->setLinearVelocity(physx::PxVec3(-vel[VX], -vel[VY], -vel[VZ]));

	m->turretowner = tu;

	m->timer = ENTITY_MISSILE_DESPAWNTIME;

	m->flags = ENTITY_TURRETMISSILE_FLAG_ENABLED;

	m->missle_channel = audiomanager_playsfx(em->am, em->sfx_missile, m->pos, -1,1.5);

	return m;
}

void entitymanager_removemissile(struct entitymanager* em, struct missile* m)
{
	int i;
	em->num_missiles--;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (m == em->missiles + i)
		{
			em->missiles[i].body->release();
			em->missiles[i].flags = ENTITY_MISSILE_FLAG_INIT;
			audiomanager_stopsound(em->missiles[i].missle_channel);
		}	
}

void entitymanager_attachpickup(struct vehicle* v, struct pickup* pu,struct entitymanager* em){

	vec3f min, max, avg, diff;
	int i;

	for(i=0;i<=em->num_pickups; i++){
		if((em->pickups+i)==pu && pu->holdingpu1==true){
			em->pickupatspawn1=false;
		}
		else if((em->pickups+i)==pu && pu->holdingpu2==true){
			em->pickupatspawn2=false;
		}
		/*else if((em->pickups+i)==pu && pu->holdingpu3==true){
			em->pickupatspawn3=false;
		}*/
		else if((em->pickups+i)==pu && pu->holdingpu4==true){
			em->pickupatspawn4=false;
		}
	}

	pu->owner = v;
	v->owns = pu;

	em->pm->scene->removeActor(*pu->body);

	if(pu->typepickup==POWERUP_MINE && (v->haspickup==POWERUP_MINE || v->haspickup==POWERUP_TURRET)){
		v->haspickup=POWERUP_TURRET;
		pu->typepickup=POWERUP_TURRET;
	}
	else if(pu->typepickup==POWERUP_MISSILE && (v->haspickup==POWERUP_MISSILE || v->haspickup==POWERUP_MISSILEX2 || v->haspickup==POWERUP_MISSILEX3)){
		v->haspickup=POWERUP_MISSILEX3;
		pu->typepickup=POWERUP_MISSILEX3;
	}
	else if(pu->typepickup==POWERUP_SPEED && (v->haspickup==POWERUP_SPEED || v->haspickup==POWERUP_SPEEDUP)){
		v->haspickup=POWERUP_SPEEDUP;
		pu->typepickup=POWERUP_SPEEDUP;
	}
	else{
		v->haspickup = pu->typepickup;
	}

	renderable_init(&pu->r_pickup, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(PICKUP_ATTACHED_OBJ, em->r, &pu->r_pickup);
	renderable_sendbuffer(em->r, &pu->r_pickup);

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
	vec3f_scalen(pu->dim_pickup, diff, PICKUP_ATTACHED_MESHSCALE);

	mat4f_scalemul(pu->r_pickup.matrix_model, PICKUP_ATTACHED_MESHSCALE, PICKUP_ATTACHED_MESHSCALE, PICKUP_ATTACHED_MESHSCALE);
	mat4f_rotateymul(pu->r_pickup.matrix_model, -1.57080f);
	mat4f_translatemul(pu->r_pickup.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);

	if(pu->typepickup==POWERUP_MISSILE){
		//Missile
		texture_loadfile(&pu->diffuse_pickupMISSILE, PICKUP_ATTACHED_MISSILE_TEXTURE);
		texture_upload(&pu->diffuse_pickupMISSILE, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMISSILE;
	}
	else if(pu->typepickup==POWERUP_SPEED){
		
		//Speed
		texture_loadfile(&pu->diffuse_pickupSPEED, PICKUP_ATTACHED_SPEED_TEXTURE);
		texture_upload(&pu->diffuse_pickupSPEED, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupSPEED;
	}
	else if(pu->typepickup==POWERUP_TURRET){
		//TURRET
		texture_loadfile(&pu->diffuse_pickupMINE, PICKUP_ATTACHED_TURRET_TEXTURE);
		texture_upload(&pu->diffuse_pickupMINE, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMINE;
	}
	else if(pu->typepickup==POWERUP_MISSILEX3){
		//X3 MISSILES
		texture_loadfile(&pu->diffuse_pickupMINE, PICKUP_ATTACHED_MISSILEX3_TEXTURE);
		texture_upload(&pu->diffuse_pickupMINE, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMINE;
	}
	else if(pu->typepickup==POWERUP_SPEEDUP){
		//SPEED UP MORE
		texture_loadfile(&pu->diffuse_pickupMINE, PICKUP_ATTACHED_SPEEDX2_TEXTURE);
		texture_upload(&pu->diffuse_pickupMINE, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMINE;
	}
	else{
		//Mine
		texture_loadfile(&pu->diffuse_pickupMINE, PICKUP_ATTACHED_MINE_TEXTURE);
		texture_upload(&pu->diffuse_pickupMINE, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMINE;
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
	//printf("pickup %d\n", i);
	em->num_pickups++;

	// logic for attaching timers for respawning pickups
	pu->holdingpu1=false;
	pu->holdingpu2=false;
	pu->holdingpu3=false;
	pu->holdingpu4=false;
	if(em->timerspawn1<=0){
		pu->holdingpu1=true;
		em->timerspawn1=PICKUP_TIMERS;
	}
	else if(em->timerspawn2<=0){
		pu->holdingpu2=true;
		em->timerspawn2=PICKUP_TIMERS;
	}
	/*else if(em->timerspawn3<=0){
		pu->holdingpu3=true;
		em->timerspawn3=PICKUP_TIMERS;
	}*/
	else if(em->timerspawn4<=0){
		pu->holdingpu4=true;
		em->timerspawn4=PICKUP_TIMERS;
	}

	renderable_init(&pu->r_pickup, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(PICKUP_OBJ, em->r, &pu->r_pickup);
	renderable_sendbuffer(em->r, &pu->r_pickup);

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
	mat4f_translatemul(pu->r_pickup.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);

	int seed = static_cast<int>(time(0));
	srand(seed);
	seed = (seed)%3;
	if(seed==1){
		//Missile
		texture_loadfile(&pu->diffuse_pickupMISSILE, PICKUP_MISSILE_TEXTURE);
		texture_upload(&pu->diffuse_pickupMISSILE, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMISSILE;

		pu->typepickup=POWERUP_MISSILE;
	}
	else if(seed==2){
		//Mine
		texture_loadfile(&pu->diffuse_pickupMINE, PICKUP_MINE_TEXTURE);
		texture_upload(&pu->diffuse_pickupMINE, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupMINE;

		pu->typepickup=POWERUP_MINE;
	}
	else{
		//Speed
		texture_loadfile(&pu->diffuse_pickupSPEED, PICKUP_SPEED_TEXTURE);
		texture_upload(&pu->diffuse_pickupSPEED, RENDER_TEXTURE_DIFFUSE);
		pu->r_pickup.textures[RENDER_TEXTURE_DIFFUSE] = &pu->diffuse_pickupSPEED;

		pu->typepickup=POWERUP_SPEED;
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
	em->num_pickups--;

	for(i=0;i<ENTITY_PICKUP_COUNT; i++){
		if(pu==em->pickups+i){
			em->pickups[i].body->release();
			em->pickups[i].flags = ENTITY_PICKUP_FLAG_INIT;
			em->pickups[i].owner = NULL;

			renderable_deallocate(&em->pickups[i].r_pickup);
		}
	}
}

struct turret* entitymanager_newturret(struct entitymanager* em, vec3f dim, struct vehicle* v){
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	struct turret* tu;
	int i;

	for (i = 0; i < ENTITY_TURRET_COUNT; i++)
		if (!(em->turrets[i].flags & TURRET_FLAG_ENABLED))
			break;

	if (i == ENTITY_TURRET_COUNT)
		return NULL;

	tu = em->turrets + i;
	em->num_turrets++;

	// find spawn location
	pose = v->body->getGlobalPose().transform(physx::PxTransform(0.f, 0.f, -(dim[VZ]*0.5f - TURRET_SPAWNDIST)));
	mat_pose = physx::PxMat44(pose);

	// create a physics object and add it to the scene
	tu->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxBoxGeometry(em->dim_turret[VX] * 0.5f, em->dim_turret[VY] * 0.5f, em->dim_turret[VZ] * 0.5f), *em->pm->default_material, TURRET_DENSITY);
	//em->pm->scene->addActor(*tu->body);

	tu->owner = v;
	tu->flags = TURRET_FLAG_ENABLED;
	tu->timer = ENTITY_TURRET_DESPAWNTIME;

	return tu;
}

void entitymanager_removeturret(struct entitymanager* em, struct turret* tu){
	int i;
	em->num_turrets--;

	for(i=0;i<ENTITY_TURRET_COUNT;i++){
		if(tu==em->turrets+i){
			em->turrets[i].body->release();
			em->turrets[i].flags = TURRET_FLAG_INIT;
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
	em->num_mines++;

	// find spawn location
	pose = v->body->getGlobalPose().transform(physx::PxTransform(0.f, 0.f, -(dim[VZ]*0.5f - ENTITY_MINE_SPAWNDIST)));
	mat_pose = physx::PxMat44(pose);

	// create a physics object and add it to the scene
	x->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxBoxGeometry(em->dim_mine[VX] * 0.5f, em->dim_mine[VY] * 0.5f, em->dim_mine[VZ] * 0.5f), *em->pm->default_material, ENTITY_MINE_DENSITY);
	x->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	setupFiltering(x->body, FilterGroup::eMine, FilterGroup::eMine);
	x->body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	x->body->userData = x;
	em->pm->scene->addActor(*x->body);

	x->owner = v;
	x->flags = ENTITY_MINE_FLAG_ENABLED;
	
	x->mine_channel= audiomanager_playsfx(em->am, em->sfx_mine, x->pos, -1,1.5);

	return x;
}

void entitymanager_removemine(struct entitymanager* em, struct mine* x){
	int i;
	em->num_mines--;

	for(i=0;i<ENTITY_MINE_COUNT;i++){
		if(x==em->mines+i){
			em->mines[i].body->release();
			em->mines[i].flags = ENTITY_MINE_FLAG_INIT;
			
			audiomanager_stopsound(em->mines[i].mine_channel);
		}
	}
}

struct blimp* entitymanager_placeblimp(struct vehicle* v,struct entitymanager* em, vec3f pos){
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	int i;
	struct blimp* b;

	for (i = 0; i < BLIMP_COUNT; i++)
		if (!(em->blimps[i].flags & BLIMP_FLAG_ENABLED))
			break;

	if (i == BLIMP_COUNT)
		return NULL;

	b = em->blimps + i;
	em->num_blimps++;

	b->owner = v;
	v->ownblimp = b;
	v->hasblimp=true;
	
	em->r_blimp.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_blimp;

	pose = v->body->getGlobalPose().transform(physx::PxTransform(-.3f, 1.f, -(pos[VZ]*0.5f - BLIMP_SPAWNDIST)));
	mat_pose = physx::PxMat44(pose);

	// create a physics object and add it to the scene
	b->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxBoxGeometry(em->dim_blimp[VX] * 0.5f, em->dim_blimp[VY] * 0.5f, em->dim_blimp[VZ] * 0.5f), *em->pm->default_material, BLIMP_DENSITY);

	b->flags = ENTITY_MINE_FLAG_ENABLED;
	b->typeblimp = BLIMP_TYPE_PLACE;

	return b;
}

struct blimp* entitymanager_lapblimp(struct entitymanager* em, vec3f pos){
	physx::PxTransform pose;
	physx::PxMat44 mat_pose;
	int i;
	struct blimp* b;
	vec3f spawn;

	for (i = 0; i < BLIMP_COUNT; i++)
		if (!(em->blimps[i].flags & BLIMP_FLAG_ENABLED))
			break;

	if (i == BLIMP_COUNT)
		return NULL;

	b = em->blimps + i;
	em->num_blimps++;

	em->r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &em->diffuse_welcome;

	// find spawn location
	vec3f_copy(b->pos, pos);

	// create a physics object and add it to the scene
	b->body = physx::PxCreateDynamic(*em->pm->sdk, physx::PxTransform(b->pos[VX], b->pos[VY], b->pos[VZ]), physx::PxBoxGeometry(em->dim_blimp[VX] * 0.5f, em->dim_blimp[VY] * 0.5f, em->dim_blimp[VZ] * 0.5f), *em->pm->default_material, BLIMP_DENSITY);

	b->flags = BLIMP_FLAG_ENABLED;
	b->typeblimp = BLIMP_TYPE_LAP;

	return b;
}

void entitymanager_removeblimp(struct entitymanager* em, struct blimp* b,struct vehicle* v){
	v->ownblimp=NULL;
	em->num_blimps--;
	
	int i;
	for(i=0;i<BLIMP_COUNT;i++){
		if(b==em->blimps+i){
			b->body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
			em->blimps[i].body->release();
			em->blimps[i].flags = BLIMP_FLAG_INIT;
			em->blimps[i].owner = NULL;
		}
	}
}

void entitymanager_textures(struct entitymanager* em, struct renderer* r){

	// initialize turret texture
	texture_init(&em->diffuse_turret);
	texture_loadfile(&em->diffuse_turret, TURRET_TEXTURE);
	texture_upload(&em->diffuse_turret, RENDER_TEXTURE_DIFFUSE);

	// initialize mine texture
	texture_init(&em->diffuse_mine);
	texture_loadfile(&em->diffuse_mine, MINE_TEXTURE);
	texture_upload(&em->diffuse_mine, RENDER_TEXTURE_DIFFUSE);

	// initialize missile texture
	texture_init(&em->diffuse_missile);
	texture_loadfile(&em->diffuse_missile, MISSILE_TEXTURE);
	texture_upload(&em->diffuse_missile, RENDER_TEXTURE_DIFFUSE);


	// initialize all the blimp textures
	texture_init(&em->diffuse_welcome);
	texture_loadfile(&em->diffuse_welcome, BLIMP_WELCOME_TEXTURE);
	texture_upload(&em->diffuse_welcome, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_blimp);
	texture_loadfile(&em->diffuse_blimp, BLIMP_REGULAR_TEXTURE);
	texture_upload(&em->diffuse_blimp, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_lap1);
	texture_loadfile(&em->diffuse_lap1, BLIMP_LAP1_TEXTURE);
	texture_upload(&em->diffuse_lap1, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_lap2);
	texture_loadfile(&em->diffuse_lap2, BLIMP_LAP2_TEXTURE);
	texture_upload(&em->diffuse_lap2, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_lap3);
	texture_loadfile(&em->diffuse_lap3, BLIMP_LAP3_TEXTURE);
	texture_upload(&em->diffuse_lap3, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_lap4);
	texture_loadfile(&em->diffuse_lap4, BLIMP_LAP4_TEXTURE);
	texture_upload(&em->diffuse_lap4, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_lap5);
	texture_loadfile(&em->diffuse_lap5, BLIMP_LAP5_TEXTURE);
	texture_upload(&em->diffuse_lap5, RENDER_TEXTURE_DIFFUSE);

	//blimp winning texture
	texture_init(&em->diffuse_win);
	texture_loadfile(&em->diffuse_win, BLIMP_WIN_TEXTURE);
	texture_upload(&em->diffuse_win, RENDER_TEXTURE_DIFFUSE);

	//blimp losing testure
	texture_init(&em->diffuse_lose);
	texture_loadfile(&em->diffuse_lose, BLIMP_LOSE_TEXTURE);
	texture_upload(&em->diffuse_lose, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_place1);
	texture_loadfile(&em->diffuse_place1, BLIMP_PLACE1_TEXTURE);
	texture_upload(&em->diffuse_place1, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_place2);
	texture_loadfile(&em->diffuse_place2, BLIMP_PLACE2_TEXTURE);
	texture_upload(&em->diffuse_place2, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_place3);
	texture_loadfile(&em->diffuse_place3, BLIMP_PLACE3_TEXTURE);
	texture_upload(&em->diffuse_place3, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_place4);
	texture_loadfile(&em->diffuse_place4, BLIMP_PLACE4_TEXTURE);
	texture_upload(&em->diffuse_place4, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_place5);
	texture_loadfile(&em->diffuse_place5, BLIMP_PLACE5_TEXTURE);
	texture_upload(&em->diffuse_place5, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_place6);
	texture_loadfile(&em->diffuse_place6, BLIMP_PLACE6_TEXTURE);
	texture_upload(&em->diffuse_place6, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_place7);
	texture_loadfile(&em->diffuse_place7, BLIMP_PLACE7_TEXTURE);
	texture_upload(&em->diffuse_place7, RENDER_TEXTURE_DIFFUSE);

	texture_init(&em->diffuse_place8);
	texture_loadfile(&em->diffuse_place8, BLIMP_PLACE8_TEXTURE);
	texture_upload(&em->diffuse_place8, RENDER_TEXTURE_DIFFUSE);

}

void entitymanager_blimpinit(struct entitymanager* em){
	float temp;
	vec3f min, max, avg, diff;
	int i;

	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; (unsigned)i < em->r_blimp.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, em->r_blimp.buf_verts + i*em->r->vertsize[em->r_blimp.type]);

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
	vec3f_scalen(em->dim_blimp, diff, BLIMP_MESHSCALE);

	// swap x and z to get the correct vehicle dimensions
	temp = em->dim_blimp[VX];
	em->dim_blimp[VX] = em->dim_blimp[VZ];
	em->dim_blimp[VZ] = temp;

	mat4f_scalemul(em->r_blimp.matrix_model, BLIMP_MESHSCALE, BLIMP_MESHSCALE, BLIMP_MESHSCALE);
	mat4f_rotateymul(em->r_blimp.matrix_model, -1.57080f);
	mat4f_translatemul(em->r_blimp.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);

	mat4f_scalemul(em->r_blimplap.matrix_model, BLIMP_LAP_MESHSCALE, BLIMP_LAP_MESHSCALE, BLIMP_LAP_MESHSCALE);
	mat4f_rotateymul(em->r_blimplap.matrix_model, -1.57080f);
	mat4f_translatemul(em->r_blimplap.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);
}

void entitymanager_missileinit(struct entitymanager* em){
	float temp;
	vec3f min, max, avg, diff;
	int i;

	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; (unsigned)i < em->r_missile.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, em->r_missile.buf_verts + i*em->r->vertsize[em->r_missile.type]);

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

	mat4f_scalemul(em->r_turretmissile.matrix_model, MISSILE_MESHSCALE, MISSILE_MESHSCALE, MISSILE_MESHSCALE);
	mat4f_rotateymul(em->r_turretmissile.matrix_model, 0.f);
	mat4f_translatemul(em->r_turretmissile.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);
}

void entitymanager_mineinit(struct entitymanager* em){
	float temp;
	vec3f min, max, avg, diff;
	int i;

	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; (unsigned)i < em->r_mine.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, em->r_mine.buf_verts + i*em->r->vertsize[em->r_mine.type]);

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

}

void entitymanager_turretinit(struct entitymanager* em){
	float temp;
	vec3f min, max, avg, diff;
	int i;

	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (i = 0; (unsigned)i < em->r_turret.num_verts; i++)
	{
		vec3f temp;

		// retrieve vertex from buffer
		vec3f_copy(temp, em->r_turret.buf_verts + i*em->r->vertsize[em->r_turret.type]);

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
	vec3f_scalen(em->dim_turret, diff, TURRET_MESHSCALE);

	// swap x and z to get the correct vehicle dimensions
	temp = em->dim_turret[VX];
	em->dim_turret[VX] = em->dim_turret[VZ];
	em->dim_turret[VZ] = temp;

	mat4f_scalemul(em->r_turret.matrix_model, TURRET_MESHSCALE, TURRET_MESHSCALE, TURRET_MESHSCALE);
	mat4f_rotateymul(em->r_turret.matrix_model, -1.5f);
	mat4f_translatemul(em->r_turret.matrix_model, -avg[VX], -avg[VY], -avg[VZ]);
}