#ifndef CART
#define	CART


#include	<PxRigidDynamic.h>
#include	"../math/vec3f.h"
#include	"../render/render.h"


#define	CART_WIDTH		1.f
#define	CART_HEIGHT		1.f
#define	CART_LENGTH		2.f



#define	CART_UP			0.f, 1.f, 0.f
#define	CART_FORWARD	0.f, 0.f, -1.f


struct physicsmanager;

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

static int cart_posindex[36] =
{
	0, 6, 5, 0, 5, 1,
	0, 3, 7, 0, 7, 6,
	0, 1, 2, 0, 2, 3,
	4, 7, 3, 4, 3, 2,
	4, 2, 1, 4, 1, 5,
	4, 5, 6, 4, 6, 7
};

struct cart
{
	physx::PxRigidDynamic* p_cart;

	struct renderable r_cart;
};


void cart_init(struct cart*, struct physicsmanager*, vec3f);

void cart_generatemesh(struct renderer*, struct cart*);


#endif