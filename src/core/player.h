#ifndef AIPLAYER
#define	AIPLAYER


#include	"input.h"
#include	"../math/vec3f.h"
#include	"../objects/camera.h"
#include	"../objects/cart.h"
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


void player_init(struct player* p, struct physicsmanager* pm, struct renderer* r, struct controller* controller, vec3f pos);
void aiplayer_init(struct aiplayer* p, struct physicsmanager* pm, struct renderer* r, vec3f pos);

void player_delete(struct player* p);
void aiplayer_delete(struct aiplayer* p);

void aiplayer_updateinput(struct aiplayer* p);

void player_updatecamera(struct player* p);


#endif