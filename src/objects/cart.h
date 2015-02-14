#ifndef CART
#define	CART


#include	"../math/vec3f.h"
#include	"../render/render.h"


#define	CART_WIDTH		1.2f
#define	CART_HEIGHT		0.6f
#define	CART_LENGTH		2.f

#define	CART_UP			0.f, 1.f, 0.f
#define	CART_FORWARD	0.f, 0.f, -1.f
#define	CART_RIGHT		1.f, 0.f, 0.f


struct physicsmanager;


struct cart
{
	int index_vehicle;

	struct physicsmanager* pm;

	struct renderable r_cart;
};


void cart_init(struct cart*, struct physicsmanager*, vec3f);
void cart_delete(struct cart* c);

void cart_accelerate(struct cart* c, float d);
void cart_turn(struct cart* c, float d);
//void cart_shocks(struct cart *c, float d, unsigned bounce);

void cart_generatemesh(struct renderer*, struct cart*);


#endif