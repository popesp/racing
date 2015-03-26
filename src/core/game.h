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

#define GAME_WINCONDITION_LAPS	2

#define	GAME_DEFAULT_WIDTH		1280
#define	GAME_DEFAULT_HEIGHT		800
#define	GAME_DEFAULT_X			200
#define	GAME_DEFAULT_Y			200

#define	GAME_TITLE				"Warped Steel"

#define	GAME_UPS				60.f
#define	GAME_SPU				1.f/GAME_UPS

#define	GAME_CLEARCOLOR			0.85f, 0.9f, 1.f

#define	GAME_POINTSIZE			10.f

#define	GAME_AIPLAYER_COUNT		(VEHICLE_COUNT - 1)

#define	GAME_STARTINGPOS		-20.f, 1.5f, 0.f // TEMP
#define	GAME_AISTARTINGPOS		-20.f, 1.5f, -20.f // TEMP

#define	GAME_MUSIC_COUNT		4
#define	GAME_MUSIC_1_ID			0
#define	GAME_MUSIC_2_ID			1
#define	GAME_MUSIC_3_ID			2
#define	GAME_MUSIC_4_ID			3
#define	GAME_MUSIC_1_FILENAME	"res/music/Erasure Always.mp3"
#define	GAME_MUSIC_2_FILENAME	"res/music/Daft Punk & The Glitch Mob - Derezzed.mp3"
#define	GAME_MUSIC_3_FILENAME	"res/music/Full Force Forward.mp3"
#define	GAME_MUSIC_4_FILENAME	"res/music/Daft Punk & Boys Noize - End Of Line.mp3"

#define	GAME_FLAG_TERMINATED	0x01
#define	GAME_FLAG_WIREFRAME		0x02	// TEMP
#define	GAME_FLAG_DEBUGCAM		0x04
#define	GAME_FLAG_PAUSED		0x08
#define	GAME_FLAG_WINCONDITION	0x10
#define	GAME_FLAG_YOULOSE		0x20
#define	GAME_FLAG_YOUWIN		0x40
#define GAME_FLAG_SWITCHON		0x80
#define GAME_FLAG_MAINMENU		0x1b

#define	GAME_FLAG_INIT			(GAME_FLAG_WINCONDITION)

#define	MENU_FLAG_STARTGAME		0x01
#define	MENU_FLAG_SETTINGS		0x02
#define	MENU_FLAG_CREDITS		0x04
#define	MENU_FLAG_EXITGAME		0x08
#define	MENU_FLAG_SHUTDOWN		0x10
#define GAME_FLAG_INCREDITS		0x20
#define GAME_FLAG_INSETTINGS	0x40


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
	int num_aiplayers;
	struct aiplayer aiplayers[GAME_AIPLAYER_COUNT];

	struct camera cam_debug;

	struct light track_lights[2];

	bool soundon;
	FMOD_CHANNEL* currentchannel;
	int index_currentsong;
	int songs[GAME_MUSIC_COUNT];

	unsigned char flags;

	unsigned char menuflags;
};

int start_subsystems(struct game* game);
int game_startup(struct game* game);
void game_mainloop(struct game* game);
void game_shutdown(struct game* game);

#endif