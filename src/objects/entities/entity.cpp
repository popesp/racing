#include	"entity.h"

#include	"mine.h"
#include	"missile.h"
#include	"../../render/objloader.h"
#include	"turret.h"
#include	"slowmine.h"


// initialization function pointers
static void (* initentity[ENTITY_TYPE_COUNT])(struct entity*, struct entitymanager*, struct vehicle*, physx::PxTransform) =
{
	missile_init,
	mine_init,
	turret_init,
	slowmine_init
};

// deletion function pointers
static void (* deleteentity[ENTITY_TYPE_COUNT])(struct entity*) =
{
	missile_delete,
	mine_delete,
	turret_delete,
	slowmine_delete
};

// update function pointers
static void (* updateentity[ENTITY_TYPE_COUNT])(struct entity*, struct entitymanager*) =
{
	missile_update,
	mine_update,
	turret_update,
	slowmine_update
};


static char* entity_mesh_filename[ENTITY_TYPE_COUNT] =
{
	MISSILE_MESH_FILENAME,
	MINE_MESH_FILENAME,
	TURRET_MESH_FILENAME,
	SLOWMINE_MESH_FILENAME
};

static float entity_mesh_scale[ENTITY_TYPE_COUNT] =
{
	MISSILE_MESH_SCALE,
	MINE_MESH_SCALE,
	TURRET_MESH_SCALE,
	SLOWMINE_MESH_SCALE
};

static float entity_mesh_yrotate[ENTITY_TYPE_COUNT] =
{
	MISSILE_MESH_YROTATE,
	MINE_MESH_YROTATE,
	TURRET_MESH_YROTATE,
	SLOWMINE_MESH_YROTATE
};

static vec3f entity_mesh_offset[ENTITY_TYPE_COUNT] =
{
	MISSILE_MESH_OFFSET,
	MINE_MESH_OFFSET,
	TURRET_MESH_OFFSET,
	SLOWMINE_MESH_OFFSET
};


static char* entity_texture_filename_diffuse[ENTITY_TYPE_COUNT] =
{
	MISSILE_TEXTURE_FILENAME_DIFFUSE,
	MINE_TEXTURE_FILENAME_DIFFUSE,
	TURRET_TEXTURE_FILENAME_DIFFUSE,
	SLOWMINE_TEXTURE_FILENAME_DIFFUSE
};


void entitymanager_startup(struct entitymanager* em, struct physicsmanager* pm, struct audiomanager* am, struct renderer* r)
{
	vec3f dim, center, offs;
	struct renderable* rb;
	int i;

	// manager pointers
	em->pm = pm;
	em->am = am;

	// initialize entity array
	for (i = 0; i < ENTITY_MAX; i++)
		em->entities[i].flags = ENTITY_FLAG_INIT;

	// initialize renderables
	for (i = 0; i < ENTITY_TYPE_COUNT; i++)
	{
		rb = em->renderables + i;

		renderable_init(rb, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
		objloader_load(entity_mesh_filename[i], r, rb, dim, center);
		renderable_sendbuffer(r, rb);

		// matrix model transformation
		mat4f_scalemul(rb->matrix_model, entity_mesh_scale[i], entity_mesh_scale[i], entity_mesh_scale[i]);
		mat4f_rotateymul(rb->matrix_model, entity_mesh_yrotate[i]);

		// get the mesh offset, scale by the mesh dimensions
		vec3f_copy(offs, entity_mesh_offset[i]);
		offs[VX] *= dim[VX];
		offs[VX] *= dim[VX];
		offs[VX] *= dim[VX];
		vec3f_subtract(offs, center);
		mat4f_translatemul(rb->matrix_model, offs[VX], offs[VY], offs[VZ]);

		// initialize texture
		texture_init(em->diffuse + i);
		texture_loadfile(em->diffuse + i, entity_texture_filename_diffuse[i]);
		texture_upload(em->diffuse + i, RENDER_TEXTURE_DIFFUSE);

		// assign texture to entity renderable
		rb->textures[RENDER_TEXTURE_DIFFUSE] = em->diffuse + i;
	}

	// initialize sound effects
	
	em->sfx_missile_launch = audiomanager_newsfx(am, MISSILE_SFX_FILENAME_LAUNCH);
	em->sfx_missile_idle = audiomanager_newsfx(am, MISSILE_SFX_FILENAME_IDLE);
	em->sfx_missile_explode = audiomanager_newsfx(am, MISSILE_SFX_FILENAME_EXPLODE);
	em->sfx_mine_idle = audiomanager_newsfx(am, MINE_SFX_FILENAME_IDLE);
	em->sfx_mine_explode = audiomanager_newsfx(am, MINE_SFX_FILENAME_EXPLODE);
	em->sfx_slowmine_idle = audiomanager_newsfx(am, SLOWMINE_SFX_FILENAME_IDLE);
	em->sfx_slowmine_explode = audiomanager_newsfx(am, SLOWMINE_SFX_FILENAME_EXPLODE);
}

void entitymanager_shutdown(struct entitymanager* em)
{
	int i;

	// delete entities
	for (i = 0; i < ENTITY_MAX; i++)
		if (em->entities[i].flags & ENTITY_FLAG_ENABLED)
			deleteentity[em->entities[i].type](em->entities + i);

	// deallocate entity resources
	for (i = 0; i < ENTITY_TYPE_COUNT; i++)
	{
		renderable_deallocate(em->renderables + i);
		texture_delete(em->diffuse + i);
	}
}

void entitymanager_update(struct entitymanager* em)
{
	int i;

	for (i = 0; i < ENTITY_MAX; i++)
		if (em->entities[i].flags & ENTITY_FLAG_ENABLED)
			updateentity[em->entities[i].type](em->entities + i, em);
}


void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview)
{
	struct entity* e;
	int i;

	for (i = 0; i < ENTITY_MAX; i++)
	{
		e = em->entities + i;

		if (e->flags & ENTITY_FLAG_ENABLED)
			renderable_render(r, em->renderables + e->type, (float*)&physx::PxMat44(e->body->getGlobalPose()), worldview, 0);
	}
}


void entitymanager_newentity(struct entitymanager* em, unsigned char type, struct vehicle* v, physx::PxTransform pose)
{
	int i;

	for (i = 0; i < ENTITY_MAX; i++)
		if (!(em->entities[i].flags & ENTITY_FLAG_ENABLED))
			break;

	if (i == ENTITY_MAX)
		return;

	initentity[type](em->entities + i, em, v, pose);
}




/*

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
		}
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
	}
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

*/