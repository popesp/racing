#include	"entities.h"


static float missile_pos[8][3] =
{
	{-1.f, -1.f, -1.f},
	{-1.f, 1.f, -1.f},
	{1.f, 1.f, -1.f},
	{1.f, -1.f, -1.f},
	{1.f, 1.f, 1.f},
	{-1.f, 1.f, 1.f},
	{-1.f, -1.f, 1.f},
	{1.f, -1.f, 1.f}
};

static float missile_nor[6][3] =
{
	{-1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{0.f, -1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 0.f, -1.f},
	{0.f, 0.f, 1.f}
};

static int missile_posindex[36] =
{
	0, 6, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 6,
	0, 1, 2, 0, 2, 3,
	4, 7, 3, 4, 3, 2,
	4, 2, 1, 4, 1, 5,
	4, 5, 6, 4, 6, 7
};

static int missile_norindex[36] =
{
	0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2,
	4, 4, 4, 4, 4, 4,
	1, 1, 1, 1, 1, 1,
	3, 3, 3, 3, 3, 3,
	5, 5, 5, 5, 5, 5
};


void entitymanager_startup(struct entitymanager* em, struct physicsmanager* pm, struct renderer* r, struct track* t)
{
	struct missile* m;
	float* ptr;
	int i;

	em->pm = pm;
	em->track = t;

	renderable_init(&em->r_missile, RENDER_MODE_TRIANGLES, RENDER_TYPE_MATS_L, RENDER_FLAG_NONE);
	renderable_allocate(r, &em->r_missile, 36);

	vec3f_set(em->dim_missile, 1.f, 1.f, 1.f);

	// generate cube for missile renderable
	ptr = em->r_missile.buf_verts;
	for (i = 0; i < 36; i++)
	{
		vec3f_copy(ptr, missile_pos[missile_posindex[i]]);
		ptr += RENDER_ATTRIBSIZE_POS;
		vec3f_copy(ptr, missile_nor[missile_norindex[i]]);
		ptr += RENDER_ATTRIBSIZE_NOR;
	}

	renderable_sendbuffer(r, &em->r_missile);

	vec3f_set(em->dim_missile, ENTITY_MISSILE_SIZE, ENTITY_MISSILE_SIZE, ENTITY_MISSILE_SIZE);
	mat4f_scalemul(em->r_missile.matrix_model, ENTITY_MISSILE_SIZE*0.5f, ENTITY_MISSILE_SIZE*0.5f, ENTITY_MISSILE_SIZE*0.5f);

	// initialize material properties
	vec3f_set(em->r_missile.material.amb, 1.8f, 0.15f, 0.1f);
	vec3f_set(em->r_missile.material.dif, 1.8f, 0.15f, 0.1f);
	vec3f_set(em->r_missile.material.spc, 1.8f, 0.5f, 0.5f);
	em->r_missile.material.shn = 100.f;

	// initialize missile array
	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
	{
		m = em->missiles + i;

		m->body = NULL;
		m->owner = NULL;
		m->flags = ENTITY_MISSILE_FLAG_INIT;
	}
}

void entitymanager_shutdown(struct entitymanager* em)
{
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED)
			em->missiles[i].body->release();

	renderable_deallocate(&em->r_missile);
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

	pose = v->body->getGlobalPose().transform(physx::PxTransform(0.f, 0.f, -(dim[VZ]*0.5f + ENTITY_MISSILE_SPAWNDIST)));
	mat_pose = physx::PxMat44(pose);
	vec3f_set(zero, 0.f, 0.f, 0.f);

	m->body = physx::PxCreateDynamic(*em->pm->sdk, pose, physx::PxBoxGeometry(em->dim_missile[VX] * 0.5f, em->dim_missile[VY] * 0.5f, em->dim_missile[VZ] * 0.5f), *em->pm->default_material, ENTITY_MISSILE_DENSITY);
	m->body->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	em->pm->scene->addActor(*m->body);

	vec3f_set(vel, 0.f, 0.f, -1.f);
	mat4f_transformvec3f(vel, (float*)&mat_pose);
	vec3f_scale(vel, ENTITY_MISSILE_SPEED);
	
	m->body->setLinearVelocity(physx::PxVec3(vel[VX], vel[VY], vel[VZ]));

	m->owner = v;

	m->flags = ENTITY_MISSILE_FLAG_ENABLED;

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


void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview)
{
	int i;

	for (i = 0; i < ENTITY_MISSILE_COUNT; i++)
		if (em->missiles[i].flags & ENTITY_MISSILE_FLAG_ENABLED)
			renderable_render(r, &em->r_missile, (float*)&physx::PxMat44(em->missiles[i].body->getGlobalPose()), worldview, 0);
}