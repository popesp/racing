#include	"powerup.h"

#include	"../math/vec3f.h"
#include	"../physics/physics.h"
#include	"../render/render.h"

static float proj_pos[8][3] =
{
	{-PROJECTILE_WIDTH/2.f, -PROJECTILE_HEIGHT/2.f, -PROJECTILE_LENGTH/2.f},
	{-PROJECTILE_WIDTH/2.f, PROJECTILE_HEIGHT/2.f, -PROJECTILE_LENGTH/2.f},
	{PROJECTILE_WIDTH/2.f, PROJECTILE_HEIGHT/2.f, -PROJECTILE_LENGTH/2.f},
	{PROJECTILE_WIDTH/2.f, -PROJECTILE_HEIGHT/2.f, -PROJECTILE_LENGTH/2.f},
	{PROJECTILE_WIDTH/2.f, PROJECTILE_HEIGHT/2.f, PROJECTILE_LENGTH/2.f},
	{-PROJECTILE_WIDTH/2.f, PROJECTILE_HEIGHT/2.f, PROJECTILE_LENGTH/2.f},
	{-PROJECTILE_WIDTH/2.f, -PROJECTILE_HEIGHT/2.f, PROJECTILE_LENGTH/2.f},
	{PROJECTILE_WIDTH/2.f, -PROJECTILE_HEIGHT/2.f, PROJECTILE_LENGTH/2.f}
};

static float proj_nor[6][3] =
{
	{-1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{0.f, -1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 0.f, -1.f},
	{0.f, 0.f, 1.f}
};

static int proj_posindex[36] =
{
	0, 6, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 6,
	0, 1, 2, 0, 2, 3,
	4, 7, 3, 4, 3, 2,
	4, 2, 1, 4, 1, 5,
	4, 5, 6, 4, 6, 7
};

static int proj_norindex[36] =
{
	0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2,
	4, 4, 4, 4, 4, 4,
	1, 1, 1, 1, 1, 1,
	3, 3, 3, 3, 3, 3,
	5, 5, 5, 5, 5, 5
};

void entitymanager_startup(struct entitymanager* em, struct renderer* r, struct texturemanager* tm, struct physicsmanager* pm, struct track* t) {

	struct projectile* proj;

	em->pm = pm;
	em->tm = tm;
	em->track = t;
	em->proj_flag = 0;

	// initialize projectile mesh
	renderable_init(&em->r_proj, RENDER_MODE_TRIANGLES, RENDER_TYPE_MATS_L, RENDER_FLAG_NONE);

	vec3f_set(em->r_proj.material.amb, 1.8f, 0.15f, 0.1f);
	vec3f_set(em->r_proj.material.dif, 1.8f, 0.15f, 0.1f);
	vec3f_set(em->r_proj.material.spc, 1.8f, 0.5f, 0.5f);
	em->r_proj.material.shn = 100.f;

	projectile_generatemesh(r, em->r_proj);

	renderable_sendbuffer(r, &em->r_proj);
}

void projectile_init(struct projectile* p, struct physicsmanager* pm,struct vehicle* v) {
	
	vec3f dim;
	vec3f_set(dim, PROJECTILE_WIDTH/2.f, PROJECTILE_HEIGHT/2.f, PROJECTILE_LENGTH/2.f);

	p->pm = pm;

	vec3f pos;

	physx::PxMat44 t_player(v->body->getGlobalPose());
	physx::PxVec3 targetpos = t_player.transform(physx::PxVec3(PROJ_TARGETPOS));
			
	float proj_x = targetpos.x;
	float proj_y = targetpos.y;
	float proj_z = targetpos.z;
	vec3f_set(pos, PROJ_TARGETPOS);

	physx::PxTransform projtrans = v->body->getGlobalPose().transform(physx::PxTransform(PROJ_TARGETPOS));

	p->proj = physics_adddynamic_box(pm, pos, dim);

	p->proj->setGlobalPose(projtrans);

	vec3f force;

	vec3f_set(force, CART_FORWARD);
	vec3f_scale(force, CART_FORCE_FORWARD * 1000);

	physx::PxRigidBodyExt::addLocalForceAtLocalPos(*p->proj, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, 0.f));

	/*renderable_init(&p->r_proj, RENDER_MODE_TRIANGLES, RENDER_TYPE_MATS_L, RENDER_FLAG_NONE);

	vec3f_set(p->r_proj.material.amb, 0.8f, 0.15f, 0.1f);
	vec3f_set(p->r_proj.material.dif, 0.8f, 0.15f, 0.1f);
	vec3f_set(p->r_proj.material.spc, 0.8f, 0.5f, 0.5f);
	p->r_proj.material.shn = 100.f;*/
}

void projectile_delete(struct projectile* p, struct physicsmanager* pm)
{
	
	p->proj->release();
	renderable_deallocate(&p->r_proj);
	
}

void projectile_generatemesh(struct renderer* r, struct renderable r_proj) {
	
	float* ptr;
	int i;

	renderable_allocate(r, &r_proj, 36);

	ptr = r_proj.buf_verts;

	for (i = 0; i < 36; i++)
	{
		vec3f_copy(ptr, proj_pos[proj_posindex[i]]);
		ptr += RENDER_ATTRIBSIZE_POS;
		vec3f_copy(ptr, proj_nor[proj_norindex[i]]);
		ptr += RENDER_ATTRIBSIZE_NOR;
	}
	
}