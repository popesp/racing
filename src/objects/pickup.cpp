#include	"pickup.h"

#include	"../mem.h"
#include	"../physics/collision.h"
#include	"../random.h"
#include	"../render/objloader.h"
#include	"vehicle.h"


static char* pickup_texture_filename[PICKUP_TYPE_COUNT] =
{
	PICKUP_TEXTURE_FILENAME_MISSILE,
	PICKUP_TEXTURE_FILENAME_MINE,
	PICKUP_TEXTURE_FILENAME_BOOST
};


static void createpickup(struct pickup* p, struct physicsmanager* pm, struct track* t, int track_index, vec3f offs)
{
	mat4f basis;

	// get the pickup global transform
	track_transformindex(t, basis, track_index);

	// pickup spawn offset
	mat4f_translatemul(basis, offs[VX], offs[VY] + PICKUP_RADIUS, offs[VZ]);

	// initialize physx actor
	p->body = physx::PxCreateDynamic(*pm->sdk, physx::PxTransform((physx::PxMat44)basis), physx::PxSphereGeometry(PICKUP_RADIUS), *pm->default_material, PICKUP_DENSITY);
	collision_setupactor(p->body, COLLISION_FILTER_PICKUP, COLLISION_FILTER_VEHICLE);
	p->body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	p->body->userData = p;

	// add actor to the scene
	pm->scene->addActor(*p->body);

	p->type = (unsigned char)random_int(PICKUP_TYPE_COUNT);

	p->flags = PICKUP_FLAG_INIT;
}

void pickupmanager_startup(struct pickupmanager* pum, struct audiomanager* am, struct physicsmanager* pm, struct renderer* r, struct track* t, unsigned num_pickupgroups, int* track_indices)
{
	vec3f dim, center, offs;
	unsigned i;
	float w;

	pum->am = am;

	// initialize pickup array
	pum->num_pickups = num_pickupgroups * 2;
	pum->pickups = (struct pickup*)mem_alloc(sizeof(struct pickup) * pum->num_pickups);

	// initialize pickup mesh
	renderable_init(&pum->r_default, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(PICKUP_MESH_FILENAME_DEFAULT, r, &pum->r_default, dim, center);
	renderable_sendbuffer(r, &pum->r_default);
	// matrix model transformation
	mat4f_translatemul(pum->r_default.matrix_model, 0.f, -PICKUP_RADIUS, 0.f);
	mat4f_scalemul(pum->r_default.matrix_model, PICKUP_MESH_SCALE, PICKUP_MESH_SCALE, PICKUP_MESH_SCALE);
	mat4f_rotateymul(pum->r_default.matrix_model, PICKUP_MESH_YROTATE);
	mat4f_translatemul(pum->r_default.matrix_model, -center[VX], dim[VY]*0.5f - center[VY], -center[VZ]);

	// initialize empty pickup mesh
	renderable_init(&pum->r_empty, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(PICKUP_MESH_FILENAME_EMPTY, r, &pum->r_empty, dim, center);
	renderable_sendbuffer(r, &pum->r_empty);
	// matrix model transformation
	mat4f_translatemul(pum->r_empty.matrix_model, 0.f, -PICKUP_RADIUS, 0.f);
	mat4f_scalemul(pum->r_empty.matrix_model, PICKUP_MESH_SCALE, PICKUP_MESH_SCALE, PICKUP_MESH_SCALE);
	mat4f_rotateymul(pum->r_empty.matrix_model, PICKUP_MESH_YROTATE);
	mat4f_translatemul(pum->r_empty.matrix_model, -center[VX], dim[VY]*0.5f - center[VY], -center[VZ]);

	// initialize pickup textures
	for (i = 0; i < PICKUP_TYPE_COUNT; i++)
	{
		texture_init(pum->diffuse + i);
		texture_loadfile(pum->diffuse + i, pickup_texture_filename[i]);
		texture_upload(pum->diffuse + i, RENDER_TEXTURE_DIFFUSE);
	}

	// create pickups
	for (i = 0; i < num_pickupgroups; i++)
	{
		w = t->pathpoints[track_indices[i]].width * 0.25f;

		vec3f_set(offs, -w, 0.f, 0.f);
		createpickup(pum->pickups + i*2 + 0, pm, t, track_indices[i], offs);

		vec3f_set(offs, w, 0.f, 0.f);
		createpickup(pum->pickups + i*2 + 1, pm, t, track_indices[i], offs);
	}

	// create sound effects
	pum->sfx_collect = audiomanager_newsfx(am, PICKUP_SFX_FILENAME_COLLECT, true);
	pum->sfx_upgrade = audiomanager_newsfx(am, PICKUP_SFX_FILENAME_UPGRADE, true);
}

void pickupmanager_reset(struct pickupmanager* pum)
{
	unsigned i;

	for (i = 0; i < pum->num_pickups; i++)
	{
		pum->pickups[i].type = (unsigned char)random_int(PICKUP_TYPE_COUNT);
		pum->pickups[i].flags = PICKUP_FLAG_INIT;
	}
}

void pickupmanager_shutdown(struct pickupmanager* pum)
{
	unsigned i;

	// release physx actors
	for (i = 0; i < pum->num_pickups; i++)
		pum->pickups[i].body->release();

	// free the pickup array
	mem_free(pum->pickups);

	renderable_deallocate(&pum->r_default);
	renderable_deallocate(&pum->r_empty);

	// delete pickup textures
	for (i = 0; i < PICKUP_TYPE_COUNT; i++)
		texture_delete(pum->diffuse + i);

	// delete sound effects
	sound_delete(pum->sfx_collect);
	sound_delete(pum->sfx_upgrade);
}

void pickupmanager_update(struct pickupmanager* pum)
{
	struct vehicle* v;
	struct pickup* p;
	unsigned i;

	for (i = 0; i < pum->num_pickups; i++)
	{
		p = pum->pickups + i;

		if (p->flags & PICKUP_FLAG_COLLECTED)
		{
			p->timer--;

			// reset the pickup if the timer is up
			if (p->timer == 0)
			{
				p->type = (unsigned char)random_int(PICKUP_TYPE_COUNT);
				p->flags = PICKUP_FLAG_INIT;
			}
		} else if (p->flags & PICKUP_FLAG_VEHICLEHIT)
		{
			p->flags |= PICKUP_FLAG_COLLECTED;
			p->timer = PICKUP_SPAWNTIME;

			v = p->collector;

			// powerup upgrade logic
			if (v->flags & VEHICLE_FLAG_HASPOWERUP)
			{
				switch (v->powerup)
				{
				case VEHICLE_POWERUP_MISSILE:
				case VEHICLE_POWERUP_MISSILEX2:
				case VEHICLE_POWERUP_MISSILEX3:
					if (p->type == PICKUP_TYPE_MISSILE)
					{
						v->powerup = VEHICLE_POWERUP_MISSILEX3;
						audiomanager_playsfx(pum->am, pum->sfx_upgrade, v->pos, 0, true);
					} else if (p->type == PICKUP_TYPE_MINE)
					{
						v->powerup = VEHICLE_POWERUP_TURRET;
						audiomanager_playsfx(pum->am, pum->sfx_upgrade, v->pos, 0, true);
					} else
					{
						v->powerup = p->type;
						audiomanager_playsfx(pum->am, pum->sfx_collect, v->pos, 0, true);
					}
					break;

				case VEHICLE_POWERUP_MINE:
				case VEHICLE_POWERUP_MINEX2:
				case VEHICLE_POWERUP_MINEX3:
					if (p->type == PICKUP_TYPE_MINE)
					{
						v->powerup = VEHICLE_POWERUP_MINEX3;
						audiomanager_playsfx(pum->am, pum->sfx_upgrade, v->pos, 0, true);
					} else if (p->type == PICKUP_TYPE_MISSILE)
					{
						v->powerup = VEHICLE_POWERUP_TURRET;
						audiomanager_playsfx(pum->am, pum->sfx_upgrade, v->pos, 0, true);
					} else
					{
						v->powerup = p->type;
						audiomanager_playsfx(pum->am, pum->sfx_collect, v->pos, 0, true);
					}
					break;

				case VEHICLE_POWERUP_BOOST:
				case VEHICLE_POWERUP_LONGBOOST:
					if (p->type == PICKUP_TYPE_BOOST)
					{
						v->powerup = VEHICLE_POWERUP_LONGBOOST;
						audiomanager_playsfx(pum->am, pum->sfx_upgrade, v->pos, 0, true);
					} else
					{
						v->powerup = p->type;
						audiomanager_playsfx(pum->am, pum->sfx_collect, v->pos, 0, true);
					}
					break;
				}
			} else
			{
				v->powerup = p->type;
				audiomanager_playsfx(pum->am, pum->sfx_collect, v->pos, 0, true);
			}
			
			v->flags |= VEHICLE_FLAG_HASPOWERUP;
		}
	}
}


void pickupmanager_render(struct pickupmanager* pum, struct renderer* r, mat4f worldview)
{
	struct pickup* p;
	unsigned i;

	for (i = 0; i < pum->num_pickups; i++)
	{
		p = pum->pickups + i;
		if (p->flags & PICKUP_FLAG_COLLECTED)
		{
			// the pickup texture doesn't matter, since only the base is rendered
			pum->r_empty.textures[RENDER_TEXTURE_DIFFUSE] = pum->diffuse;
			renderable_render(r, &pum->r_empty, (float*)&physx::PxMat44(p->body->getGlobalPose()), worldview, 0);
		} else
		{
			pum->r_default.textures[RENDER_TEXTURE_DIFFUSE] = pum->diffuse + p->type;
			renderable_render(r, &pum->r_default, (float*)&physx::PxMat44(p->body->getGlobalPose()), worldview, 0);
		}
	}
}