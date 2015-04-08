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
#define	AI_CAMERA_EASING		1.1f

#define GAME_DIFFICULTY_EASY	1
#define GAME_DIFFICULTY_NORMAL	2
#define GAME_DIFFICULTY_HARD	3

struct player
{
	struct vehicle* vehicle;

	struct camera camera;
};

struct aiplayer
{
	struct vehicle* vehicle;

	struct track* track;

	struct controller controller;

	struct camera camera;

	float turn, speed;
	int next;
};


void player_init(struct player* p, struct vehiclemanager* vm, controller* controller, int index_track, vec3f offs);
void aiplayer_init(struct aiplayer* p, struct vehiclemanager* vm, int index_track, vec3f offs);

void player_delete(struct player* p, struct vehiclemanager* vm);
void aiplayer_delete(struct aiplayer* p, struct vehiclemanager* vm);

void aiplayer_updateinput(struct aiplayer* p, struct vehiclemanager* vm, int aidifficulty);

void player_updatecamera(struct player* p);
void aiwin_camera(struct aiplayer* aip);

#endif