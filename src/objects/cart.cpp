#include	"cart.h"

#include	"../math/vec3f.h"
#include	"../physics/physics.h"
#include	"../render/render.h"
#include	"objloader.h"

static float cart_pos[8][3] =
{
	{-CART_WIDTH/2.f, -CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, -CART_HEIGHT/2.f, -CART_LENGTH/2.f},
	{CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{-CART_WIDTH/2.f, -CART_HEIGHT/2.f, CART_LENGTH/2.f},
	{CART_WIDTH/2.f, -CART_HEIGHT/2.f, CART_LENGTH/2.f}
};

static float cart_nor[6][3] =
{
	{-1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{0.f, -1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 0.f, -1.f},
	{0.f, 0.f, 1.f}
};

static int cart_posindex[36] =
{
	0, 6, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 6,
	0, 1, 2, 0, 2, 3,
	4, 7, 3, 4, 3, 2,
	4, 2, 1, 4, 1, 5,
	4, 5, 6, 4, 6, 7
};

static int cart_norindex[36] =
{
	0, 0, 0, 0, 0, 0,
	2, 2, 2, 2, 2, 2,
	4, 4, 4, 4, 4, 4,
	1, 1, 1, 1, 1, 1,
	3, 3, 3, 3, 3, 3,
	5, 5, 5, 5, 5, 5
};

using namespace physx;

void cart_init(struct cart* c, physicsmanager* pm, vec3f pos)
{
	vec3f dim;

	c->pm = pm;

	vec3f_set(dim, CART_WIDTH/2.f, CART_HEIGHT/2.f, CART_LENGTH/2.f);
	c->vehicle = physicsmanager_newvehicle(pm, pos, dim);

	renderable_init(&c->r_cart, RENDER_MODE_TRIANGLES, RENDER_TYPE_MATS_L, RENDER_FLAG_NONE);

	vec3f_set(c->r_cart.material.amb, 0.8f, 0.15f, 0.1f);
	vec3f_set(c->r_cart.material.dif, 0.8f, 0.15f, 0.1f);
	vec3f_set(c->r_cart.material.spc, 0.8f, 0.5f, 0.5f);
	c->r_cart.material.shn = 100.f;

	c->controller = NULL;
}

void cart_delete(struct cart* c)
{
	physicsmanager_removevehicle(c->pm, c->vehicle);
	renderable_deallocate(&c->r_cart);
}


void cart_update(struct cart* c)
{
	vec3f force;

	if (c->controller != NULL && c->controller->flags & INPUT_FLAG_ENABLED)
	{
		// acceleration force
		vec3f_set(force, CART_FORWARD);
		vec3f_scale(force, -CART_FORCE_FORWARD * c->controller->axes[INPUT_AXIS_TRIGGERS]);

		physx::PxRigidBodyExt::addLocalForceAtLocalPos(*c->vehicle->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, 0.f));

		// turning force
		vec3f_set(force, CART_RIGHT);
		PxVec3 velocity = PxRigidBodyExt::getLocalVelocityAtLocalPos(*c->vehicle->body, PxVec3(0.f, 0.f, 0.f));

		if ((c->controller->axes[INPUT_AXIS_TRIGGERS] > 0)){// || ((float)velocity.z > 0)){
			//printf("local velocity z: %d\n",velocity.z);
			vec3f_scale(force, -CART_FORCE_TURN * c->controller->axes[INPUT_AXIS_LEFT_LR]);
		}
		else
			vec3f_scale(force, CART_FORCE_TURN * c->controller->axes[INPUT_AXIS_LEFT_LR]);
		

		physx::PxRigidBodyExt::addLocalForceAtLocalPos(*c->vehicle->body, physx::PxVec3(force[VX], force[VY], force[VZ]), physx::PxVec3(0.f, 0.f, -CART_LENGTH/2.f));
	}
}


void cart_generatemesh(struct renderer* r, struct cart* c)
{
	float* ptr;
	int i;
	vec3f * cverts, *cuv, *cnorm;
	loadOBJ("res/Models/car/car.obj", &cverts, &cuv, &cnorm);
	renderable_allocate(r, &c->r_cart, 36);

	ptr = c->r_cart.buf_verts;

	for (i = 0; i < 36; i++)
	{
		vec3f_copy(ptr, cart_pos[cart_posindex[i]]);
		ptr += RENDER_ATTRIBSIZE_POS;
		vec3f_copy(ptr, cart_nor[cart_norindex[i]]);
		ptr += RENDER_ATTRIBSIZE_NOR;
	}




}
