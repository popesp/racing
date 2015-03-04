#ifndef SKYBOX
#define	SKYBOX


#include	"../render/render.h"


struct skybox
{
	struct renderable r_skybox;
	struct texture diffuse;
};


void skybox_init(struct skybox* sb, struct renderer* r, const char* file_diffuse);
void skybox_delete(struct skybox* sb);


#endif
