#ifndef	GAME
#define	GAME


#include	"input.h"
#include	"objects/camera.h"
#include	"objects/cart.h"
#include	"objects/track.h"
#include	"physics/physics.h"
#include	"render/render.h"
#include	"render/texture.h"
#include	"render/window.h"


#define	GAME_DEFAULT_WIDTH		600
#define	GAME_DEFAULT_HEIGHT		400
#define	GAME_DEFAULT_X			400
#define	GAME_DEFAULT_Y			400

#define	GAME_TITLE				"Racing Game"

#define	GAME_UPS				60.f
#define	GAME_SPU				1.f/GAME_UPS

#define	GAME_CLEARCOLOR			0.85f, 0.9f, 1.f

#define	GAME_POINTSIZE			10.f

#define	GAME_FLAG_INIT			0x00
#define	GAME_FLAG_TERMINATED	0x01
#define	GAME_FLAG_WIREFRAME		0x02	// TEMP
#define	GAME_FLAG_DEBUGCAM		0x04


struct game
{
	struct window window;
	struct renderer renderer;
	struct physicsmanager physics;
	struct inputmanager input;
	struct texturemanager textures;

	struct track track;
	struct cart player;
	struct camera cam_debug;
	struct camera cam_player;

	struct light track_lights[2];

	unsigned char flags;
};


int game_startup(struct game* game);
void game_mainloop(struct game* game);
void game_shutdown(struct game* game);


#endif