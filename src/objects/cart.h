#ifndef CART
#define	CART


#include	"../core/input.h"
#include	"../math/vec3f.h"
#include	"../render/render.h"


#define	CART_WIDTH			1.2f
#define	CART_HEIGHT			0.6f
#define	CART_LENGTH			2.f

#define	CART_UP				0.f, 1.f, 0.f
#define	CART_FORWARD		0.f, 0.f, -1.f
#define	CART_RIGHT			1.f, 0.f, 0.f

#define	CART_FORCE_FORWARD	40.f
#define	CART_FORCE_TURN		4.f


struct physicsmanager;
struct vehicle;


struct cart
{
	struct physicsmanager* pm;
	struct vehicle* vehicle;

	struct renderable r_cart;

	struct controller* controller;
};


void cart_init(struct cart*, struct physicsmanager*, vec3f);
void cart_delete(struct cart* c);

void cart_update(struct cart* c);

void cart_generatemesh(struct renderer*, struct cart*);


#endif