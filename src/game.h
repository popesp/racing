#ifndef	GAME
#define	GAME


#include	"input.h"
#include	"objects/camera.h"
#include	"objects/cart.h"
#include	"objects/track.h"
#include	"physics/physics.h"
#include	"render/window.h"
#include	"render/render.h"


#define	GAME_DEFAULT_WIDTH		600
#define	GAME_DEFAULT_HEIGHT		400
#define	GAME_DEFAULT_X			400
#define	GAME_DEFAULT_Y			400

#define	GAME_TITLE				"Racing Game"

#define	GAME_UPS				60.f
#define	GAME_SPU				1.f/GAME_UPS

#define	GAME_CLEARCOLOR			0.85f, 0.9f, 1.f

#define	GAME_POINTSIZE			10.f

#define	GAME_ROTATEXSPEED		0.6		// TEMP
#define	GAME_ROTATEYSPEED		0.6		// TEMP
#define	GAME_ROTATEXMAX			80.f	// TEMP
#define	GAME_ROTATEXMIN			-80.f	// TEMP

#define	GAME_ZOOMSPEED			0.1f	// TEMP
#define	GAME_ZOOMSCALE			20.f	// TEMP
#define	GAME_ZOOMMAX			10.f	// TEMP
#define	GAME_ZOOMMIN			0.1f	// TEMP

#define	GAME_FLAG_INIT			0x00
#define	GAME_FLAG_TERMINATED	0x01
#define	GAME_FLAG_WIREFRAME		0x02	// TEMP
#define	GAME_FLAG_ROTATING		0x04	// TEMP


struct game
{
	struct window window;
	struct renderer renderer;
	struct physicsmanager physics;
	struct inputmanager input;

	struct track track;
	struct cart player;
	struct freecamera player_camera;

	struct light track_lights[2];

	unsigned char flags;

	char PADDING[3];
};


int game_startup(struct game* game);
void game_mainloop(struct game* game);
void game_shutdown(struct game* game);


#endif