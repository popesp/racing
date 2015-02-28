#ifndef	GAME
#define	GAME


#include	"../audio/audio.h"
#include	"input.h"
#include	"../objects/camera.h"
#include	"../objects/vehicle.h"
#include	"../objects/skybox.h"
#include	"../objects/track.h"
#include	"../physics/physics.h"
#include	"player.h"
#include	"../render/render.h"
#include	"../render/texture.h"
#include	"../render/window.h"
#include	"../objects/powerup.h"


#define	GAME_DEFAULT_WIDTH		1280
#define	GAME_DEFAULT_HEIGHT		800
#define	GAME_DEFAULT_X			200
#define	GAME_DEFAULT_Y			200

#define	GAME_TITLE				"Warped Steel"

#define	GAME_UPS				60.f
#define	GAME_SPU				1.f/GAME_UPS

#define	GAME_CLEARCOLOR			0.85f, 0.9f, 1.f

#define	GAME_POINTSIZE			10.f

#define	GAME_STARTINGPOS		-20.f, 1.5f, 0.f // TEMP
#define	GAME_AISTARTINGPOS		-20.f, 1.5f, -20.f // TEMP

#define	GAME_FLAG_INIT			0x00
#define	GAME_FLAG_TERMINATED	0x01
#define	GAME_FLAG_WIREFRAME		0x02	// TEMP
#define	GAME_FLAG_DEBUGCAM		0x04


struct game
{
	struct window window;
	struct renderer renderer;
	struct physicsmanager physicsmanager;
	struct inputmanager inputmanager;
	struct texturemanager texturemanager;
	struct audiomanager audiomanager;

	struct vehiclemanager vehiclemanager;

	struct skybox skybox;
	struct track track;

	struct renderable r_vehicle;
	struct renderable closestpoint; // temp

	struct player player;
	struct aiplayer aiplayer;

	struct camera cam_debug;

	struct light track_lights[2];

	int tex_trackbump;
	int tex_skybox;
	int tex_vehicle;

	int current_song;
	int song_1;
	int song_2;
	int song_3;

	unsigned char flags;
};


int game_startup(struct game* game);
void game_mainloop(struct game* game);
void game_shutdown(struct game* game);

#endif