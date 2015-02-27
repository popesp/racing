#ifndef AIPLAYER
#define	AIPLAYER


#include	"input.h"
#include	"../math/vec3f.h"
#include	"../objects/camera.h"
#include	"../objects/cart.h"
#include	"../objects/track.h"
#include	"../physics/physics.h"


#define	PLAYER_CAMERA_TARGETPOS	0.f, 1.f, 5.f
#define	PLAYER_CAMERA_EASING	0.1f


struct player
{
	struct cart cart;

	struct camera camera;
};

struct aiplayer
{
	struct cart cart;

	struct controller controller;
};


void player_init(struct player* p, struct physicsmanager* pm, struct renderable* obj, struct controller* controller, struct track* t, int index_track);
void aiplayer_init(struct aiplayer* p, struct physicsmanager* pm, struct renderable* obj, struct track* t, int index_track);

void player_delete(struct player* p);
void aiplayer_delete(struct aiplayer* p);

void aiplayer_updateinput(struct aiplayer* p);

void player_updatecamera(struct player* p);

void player_update(struct player* p, struct track* t);
void aiplayer_update(struct aiplayer* p, struct track* t);


#endif