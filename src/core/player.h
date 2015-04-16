#ifndef AIPLAYER
#define	AIPLAYER


#include	"input.h"
#include	"../math/vec3f.h"
#include	"../objects/camera.h"
#include	"../objects/vehicle.h"
#include	"../objects/track.h"
#include	"../physics/physics.h"


#define	PLAYER_CAMERA_TARGETPOS		0.f, 1.f, 5.f
#define	PLAYER_CAMERA_EASING		0.1f

#define	PLAYER_WINCAMERA_POS		0.f, 4.f, 10.f
#define	PLAYER_WINCAMERA_ROTATE		-0.01f

#define	AIPLAYER_MISSILE_COOLDOWN	60


struct player
{
	struct vehicle* vehicle;

	struct camera camera;
	float anglecamera;
};

struct aiplayer
{
	struct vehicle* vehicle;

	struct track* track;

	struct controller controller;

	unsigned timer_missile;
	unsigned timer_mine;

	float turn, speed;
};


void player_init(struct player* p, struct vehicle* v);
void aiplayer_init(struct aiplayer* p, struct vehicle* v, struct track* track);

void player_delete(struct player* p);
void aiplayer_delete(struct aiplayer* p);

void aiplayer_updateinput(struct aiplayer* p, struct vehiclemanager* vm);

void player_updatecamera(struct player* p);
void player_updatewincamera(struct player* p, struct vehicle* v);

#endif