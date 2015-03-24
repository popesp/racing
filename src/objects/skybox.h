#ifndef SKYBOX
#define	SKYBOX


#include	"../render/render.h"
#include	"../render/texture.h"


#define	SKYBOX_TEXTURE_COUNT	5
#define SKYBOX_TEXTURE0			"res/images/night.jpg"
#define	SKYBOX_TEXTURE1			"res/images/interstellar.jpg"
#define	SKYBOX_TEXTURE2			"res/images/stormydays.jpg"
#define	SKYBOX_TEXTURE3			"res/images/violentdays.jpg"
#define	SKYBOX_TEXTURE4			"res/images/miramar.jpg"


struct skybox
{
	struct renderable r_skybox;
	struct texture diffuse;
};


void skybox_init(struct skybox* sb, struct renderer* r);
void skybox_delete(struct skybox* sb);


#endif
