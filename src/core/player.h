#ifndef AIPLAYER
#define	AIPLAYER


#include	"input.h"
#include	"../math/vec3f.h"
#include	"../objects/camera.h"
#include	"../objects/vehicle.h"
#include	"../objects/track.h"
#include	"../physics/physics.h"


#define	PLAYER_CAMERA_TARGETPOS	0.f, 1.f, 5.f
#define	PLAYER_CAMERA_EASING	0.1f


struct player
{
	struct vehicle* vehicle;

	struct camera camera;
};

struct aiplayer
{
	struct vehicle* vehicle;

	struct controller controller;
};


void player_init(struct player* p, struct vehiclemanager* vm, controller* controller, int index_track, vec3f offs);
void aiplayer_init(struct aiplayer* p, struct vehiclemanager* vm, int index_track, vec3f offs);

void player_delete(struct player* p, struct vehiclemanager* vm);
void aiplayer_delete(struct aiplayer* p, struct vehiclemanager* vm);

void aiplayer_updateinput(struct aiplayer* p);

void player_updatecamera(struct player* p);


#endif