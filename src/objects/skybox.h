#ifndef SKYBOX
#define	SKYBOX


#include	"../render/render.h"


struct skybox
{
	struct renderable r_skybox;

	struct texturemanager* tm;

	int texture;
};


void skybox_init(struct skybox* sb, struct texturemanager* tm, const char* filename);
void skybox_delete(struct skybox* sb);

void skybox_generatemesh(struct renderer* r, struct skybox* sb);


#endif
