#ifndef	GAME
#define	GAME


#include	"../audio/audio.h"
#include	"input.h"
#include	"../objects/camera.h"
#include	"../objects/vehicle.h"
#include	"../objects/skybox.h"
#include	"../objects/track.h"
#include	"../objects/pickups.h"
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

#define	AI_MAX_COUNT			152	//max is 9. buttons glitch over that.

#define	GAME_MUSIC_COUNT		4
#define	GAME_MUSIC_1_ID			0
#define	GAME_MUSIC_2_ID			1
#define	GAME_MUSIC_3_ID			2
#define	GAME_MUSIC_4_ID			3
#define	GAME_MUSIC_1_FILENAME	"res/music/Erasure Always.mp3"
#define	GAME_MUSIC_2_FILENAME	"res/music/Daft Punk & The Glitch Mob - Derezzed.mp3"
#define	GAME_MUSIC_3_FILENAME	"res/music/Full Force Forward.mp3"
#define	GAME_MUSIC_4_FILENAME	"res/music/Daft Punk & Boys Noize - End Of Line.mp3"

#define	GAME_SFX_1_ID			0
#define	GAME_SFX_2_ID			1
#define	GAME_SFX_3_ID			2
#define	GAME_SFX_1_FILENAME	"res/soundfx/143558__fight2flyphoto__bf-109-flyby.wav"
#define	GAME_SFX_2_FILENAME	"res/soundfx/engine.wav"
#define	GAME_SFX_3_FILENAME	"res/soundfx/crash.wav"



#define GAME_WIN_LAP			5
#define GAME_WINSTATE_OFF		0x0
#define GAME_WINSTATE_ON		0x1

#define GAME_PICKUP_COUNT		5

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
	struct aiplayer aiplayer[AI_MAX_COUNT];

	struct camera cam_debug;

	struct light track_lights[2];

	int tex_trackbump;
	int tex_skybox;
	int tex_vehicle;

	int index_currentsong;
	int songs[GAME_MUSIC_COUNT];
	int sfx[AUDIO_MAX_SFX];

	int amountAI;

	unsigned char flags;
	unsigned char winstate;
};

int game_startup(struct game* game);
void game_mainloop(struct game* game);
void game_shutdown(struct game* game);

#endif