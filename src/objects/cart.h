#ifndef CART
#define	CART


#include	<PxRigidDynamic.h>
#include	"../math/vec3f.h"
#include	"../render/render.h"


#define	CART_WIDTH	1.f
#define	CART_HEIGHT	1.f
#define	CART_LENGTH	2.f


struct physicsmanager;


struct cart
{
	physx::PxRigidDynamic* p_cart;

	struct renderable r_cart;
};


void cart_init(struct cart*, struct physicsmanager*, vec3f);

void cart_generatemesh(struct renderer*, struct cart*);


#endif