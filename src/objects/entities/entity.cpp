#include	"entity.h"

#include	"mine.h"
#include	"missile.h"
#include	"../../render/objloader.h"
#include	"turret.h"


// initialization function pointers
static void (* initentity[ENTITY_TYPE_COUNT])(struct entity*, struct entitymanager*, struct vehicle*, physx::PxTransform) =
{
	missile_init,
	mine_init,
	turret_init
};

// deletion function pointers
static void (* deleteentity[ENTITY_TYPE_COUNT])(struct entity*) =
{
	missile_delete,
	mine_delete,
	turret_delete
};

// update function pointers
static void (* updateentity[ENTITY_TYPE_COUNT])(struct entity*, struct entitymanager*) =
{
	missile_update,
	mine_update,
	turret_update
};


static char* entity_mesh_filename[ENTITY_TYPE_COUNT] =
{
	MISSILE_MESH_FILENAME,
	MINE_MESH_FILENAME,
	TURRET_MESH_FILENAME
};

static float entity_mesh_scale[ENTITY_TYPE_COUNT] =
{
	MISSILE_MESH_SCALE,
	MINE_MESH_SCALE,
	TURRET_MESH_SCALE
};

static float entity_mesh_yrotate[ENTITY_TYPE_COUNT] =
{
	MISSILE_MESH_YROTATE,
	MINE_MESH_YROTATE,
	TURRET_MESH_YROTATE
};

static vec3f entity_mesh_offset[ENTITY_TYPE_COUNT] =
{
	MISSILE_MESH_OFFSET,
	MINE_MESH_OFFSET,
	TURRET_MESH_OFFSET
};


static char* entity_texture_filename_diffuse[ENTITY_TYPE_COUNT] =
{
	MISSILE_TEXTURE_FILENAME_DIFFUSE,
	MINE_TEXTURE_FILENAME_DIFFUSE,
	TURRET_TEXTURE_FILENAME_DIFFUSE
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
	em->sfx_missile_idle = audiomanager_newsfx(am, MISSILE_SFX_FILENAME_IDLE, true);
	em->sfx_missile_explode = audiomanager_newsfx(am, MISSILE_SFX_FILENAME_EXPLODE, true);
	em->sfx_mine_idle = audiomanager_newsfx(am, MINE_SFX_FILENAME_IDLE, true);
	em->sfx_mine_explode = audiomanager_newsfx(am, MINE_SFX_FILENAME_EXPLODE, true);
}

void entitymanager_reset(struct entitymanager* em)
{
	int i;

	// delete entities
	for (i = 0; i < ENTITY_MAX; i++)
		if (em->entities[i].flags & ENTITY_FLAG_ENABLED)
			deleteentity[em->entities[i].type](em->entities + i);
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

	// delete sound objects
	sound_delete(em->sfx_missile_idle);
	sound_delete(em->sfx_missile_explode);
	sound_delete(em->sfx_mine_idle);
	sound_delete(em->sfx_mine_explode);
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