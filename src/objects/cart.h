#ifndef CART
#define	CART


#include	"../math/vec3f.h"
#include	"../render/render.h"


#define	CART_WIDTH		1.f
#define	CART_HEIGHT		1.f
#define	CART_LENGTH		2.f

#define	CART_UP			0.f, 1.f, 0.f
#define	CART_FORWARD	0.f, 0.f, -1.f


struct physicsmanager;
struct vehicle;


struct cart
{
	struct vehicle* vehicle;

	struct renderable r_cart;
};


void cart_init(struct cart*, struct physicsmanager*, vec3f);

void cart_generatemesh(struct renderer*, struct cart*);


#endif