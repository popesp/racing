#ifndef CART
#define	CART


#include	"../core/input.h"
#include	"../math/vec3f.h"
#include	"../physics/physics.h"
#include	"../render/render.h"


#define	CART_WIDTH			1.2f
#define	CART_HEIGHT			0.6f
#define	CART_LENGTH			2.f

#define	CART_UP				0.f, 1.f, 0.f
#define	CART_DOWN			0.f, -1.f, 0.f
#define	CART_FORWARD		0.f, 0.f, -1.f
#define	CART_RIGHT			1.f, 0.f, 0.f

#define	CART_FORCE_FORWARD	40.f
#define	CART_FORCE_TURN		5.f

#define	CART_SPAWNHEIGHT	1.5f


struct cart
{
	struct physicsmanager* pm;
	struct vehicle* vehicle;

	struct renderable r_cart;

	struct controller* controller;

	vec3f pos;

	int index_track;
};


/*	initialize a cart object
	param:	c				cart object to initialize (modified)
	param:	pm				physics manager
	param:	t				track object
	param:	index_track		track point index on which to spawn cart
*/
void cart_init(struct cart* c, struct physicsmanager* pm, struct track* t, int index_track);

/*	delete a cart object
	param:	c				cart object to delete
*/
void cart_delete(struct cart* c);


/*	update a cart object
	param:	c				cart object to update
	param:	t				track object
*/
void cart_update(struct cart* c, struct track* t);


/*	reset a cart by placing it back on the track
	param:	c				cart object to reset
	param:	t				track object
*/
void cart_reset(struct cart* c, struct track* t);


/*	generate a mesh for a cart
	param:	c				cart object
	param:	r				renderer
*/
void cart_generatemesh(struct cart* c, struct renderer* r);


#endif