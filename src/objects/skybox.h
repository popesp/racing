#ifndef SKYBOX
#define	SKYBOX

#include	"../render/render.h"

#define SKYBOX_NIGHT		"res/images/night.jpg"
#define	SKYBOX_INTERSTELLAR	"res/images/interstellar.jpg"
#define	SKYBOX_STORMYDAY	"res/images/stormydays.jpg"
#define SKYBOX_VIOLENTDAY	"res/images/violentdays.jpg"
#define SKYBOX_MIRAMAR		"res/images/miramar.jpg"

struct skybox
{
	struct renderable r_skybox;
	struct texture diffuse;
};


void skybox_init(struct skybox* sb, struct renderer* r);
void skybox_delete(struct skybox* sb);


#endif
