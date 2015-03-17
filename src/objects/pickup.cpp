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


void pickupmanager_startup(struct pickupmanager* pum, struct physicsmanager* pm, struct renderer* r, struct track* t, unsigned num_pickupgroups, int* track_indices)
{
	vec3f dim, center, offs;
	unsigned i;
	float w;

	// initialize pickup array
	pum->num_pickups = num_pickupgroups * 2;
	pum->pickups = (struct pickup*)mem_alloc(sizeof(struct pickup) * pum->num_pickups);

	// initialize pickup mesh
	renderable_init(&pum->renderable, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_L, RENDER_FLAG_NONE);
	objloader_load(PICKUP_MESH_FILENAME, r, &pum->renderable, dim, center);
	renderable_sendbuffer(r, &pum->renderable);

	// matrix model transformation
	mat4f_translatemul(pum->renderable.matrix_model, 0.f, -PICKUP_RADIUS, 0.f);
	mat4f_scalemul(pum->renderable.matrix_model, PICKUP_MESH_SCALE, PICKUP_MESH_SCALE, PICKUP_MESH_SCALE);
	mat4f_rotateymul(pum->renderable.matrix_model, PICKUP_MESH_YROTATE);
	mat4f_translatemul(pum->renderable.matrix_model, -center[VX], dim[VY]*0.5f - center[VY], -center[VZ]);

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
}

void pickupmanager_shutdown(struct pickupmanager* pum)
{
	unsigned i;

	// release physx actors
	for (i = 0; i < pum->num_pickups; i++)
		pum->pickups[i].body->release();

	// free the pickup array
	mem_free(pum->pickups);

	renderable_deallocate(&pum->renderable);

	// delete pickup textures
	for (i = 0; i < PICKUP_TYPE_COUNT; i++)
		texture_delete(pum->diffuse + i);
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
						v->powerup = VEHICLE_POWERUP_MISSILEX3;
					else
						v->powerup = p->type;
					break;

				case VEHICLE_POWERUP_MINE:
				case VEHICLE_POWERUP_TURRET:
					if (p->type == PICKUP_TYPE_MINE)
						v->powerup = VEHICLE_POWERUP_TURRET;
					else
						v->powerup = p->type;
					break;

				case VEHICLE_POWERUP_BOOST:
				case VEHICLE_POWERUP_LONGBOOST:
					if (p->type == PICKUP_TYPE_BOOST)
						v->powerup = VEHICLE_POWERUP_LONGBOOST;
					else
						v->powerup = p->type;
					break;
				}
			} else
				v->powerup = p->type;
			
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
		if (!(p->flags & PICKUP_FLAG_COLLECTED))
		{
			pum->renderable.textures[RENDER_TEXTURE_DIFFUSE] = pum->diffuse + p->type;
			renderable_render(r, &pum->renderable, (float*)&physx::PxMat44(p->body->getGlobalPose()), worldview, 0);
		}
	}
}