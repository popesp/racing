#ifndef	GAME
#define	GAME


#include	"../audio/audio.h"
#include	"input.h"
#include	"win.h"
#include	"../objects/camera.h"
#include	"../objects/entities/entity.h"
#include	"../objects/pickup.h"
#include	"../objects/skybox.h"
#include	"../objects/track.h"
#include	"../objects/vehicle.h"
#include	"../physics/physics.h"
#include	"player.h"
#include	"../render/render.h"
#include	"../render/window.h"
#include	"ui.h"


#define	GAME_DEFAULT_WIDTH			1280
#define	GAME_DEFAULT_HEIGHT			800
#define	GAME_DEFAULT_X				200
#define	GAME_DEFAULT_Y				200

#define	GAME_TITLE					"Warped Steel"

#define	GAME_UPS					60.f
#define	GAME_SPU					1.f/GAME_UPS

#define	GAME_CLEARCOLOR				0.1f, 0.1f, 0.1f

#define	GAME_POINTSIZE				10.f

#define	GAME_AIPLAYER_COUNT			7

#define	GAME_DEFAULT_LAPS			2
#define GAME_DEFAULT_DIFF			.5f

#define	GAME_TIMER_RACESTART		300

#define	GAME_MUSIC_COUNT			4
#define	GAME_MUSIC_1_ID				0
#define	GAME_MUSIC_2_ID				1
#define	GAME_MUSIC_3_ID				2
#define	GAME_MUSIC_4_ID				3
#define	GAME_MUSIC_1_FILENAME		"res/music/Erasure Always.mp3"
#define	GAME_MUSIC_2_FILENAME		"res/music/Daft Punk & The Glitch Mob - Derezzed.mp3"
#define	GAME_MUSIC_3_FILENAME		"res/music/Full Force Forward.mp3"
#define	GAME_MUSIC_4_FILENAME		"res/music/Daft Punk & Boys Noize - End Of Line.mp3"

#define	GAME_FLAG_TERMINATED		0x01
#define	GAME_FLAG_WIREFRAME			0x02
#define	GAME_FLAG_DEBUGCAM			0x04
#define	GAME_FLAG_INIT				0x00

#define	GAME_STATE_MAINMENU			0x00
#define	GAME_STATE_LOADRACE			0x01
#define	GAME_STATE_RACE				0x03
#define	GAME_STATE_RACEDONE			0x04
#define	GAME_STATE_PAUSEMENU		0x05
#define	GAME_STATE_SETTINGS			0x06
#define	GAME_STATE_CREDITS			0x07
#define	GAME_STATE_PAUSESETTINGS	0x08


struct game
{
	struct window window;
	struct renderer renderer;

	struct physicsmanager physicsmanager;
	struct inputmanager inputmanager;
	struct audiomanager audiomanager;
	struct uimanager uimanager;

	struct entitymanager entitymanager;
	struct vehiclemanager vehiclemanager;
	struct pickupmanager pickupmanager;

	struct skybox skybox;
	struct track track;

	struct player player;
	struct aiplayer aiplayers[GAME_AIPLAYER_COUNT];

	struct vehicle* winningvehicle;

	struct controller* controller_main;

	struct camera cam_debug;

	struct light track_lights[2];

	FMOD_CHANNEL* currentchannel;
	int index_currentsong;
	FMOD_SOUND* songs[GAME_MUSIC_COUNT];

	unsigned timer_racestart;

	unsigned laps;
	float difficulty;

	unsigned char flags;
	unsigned char state;
};


int game_startup(struct game* game);
void game_mainloop(struct game* game);
void game_shutdown(struct game* game);


#endif