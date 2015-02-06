#include	"texture.h"

#include	<FreeImage.h>
#include	"../error.h"
#include	"../mem.h"


void texture_startup(struct texturemanager* tm)
{
	int i;

	tm->num_textures = 0;

	for (i = 0; i < TEXTURE_MAX; i++)
		tm->textures[i].data = NULL;
}

void texture_shutdown(struct texturemanager* tm)
{
	int i;

	for (i = 0; i < tm->num_textures; i++)
		mem_free(tm->textures[i].data);
}

const char* texture_getversion(struct texturemanager* tm)
{
	(void)tm;
	return FreeImage_GetVersion();
}


int texture_new(struct texturemanager* tm)
{
	int i;

	for (i = 0; i < TEXTURE_MAX; i++)
	{
		// TODO
		return 0;
	}

	return 0;
}



void texture_defaultdiffuse(struct texture* t)
{
	t->width = 1;
	t->height = 1;

	t->data = (unsigned char*)mem_realloc(t->data, 3);
	t->data[TR] = TEXTURE_DEFAULT_DIFFUSE_R;
	t->data[TG] = TEXTURE_DEFAULT_DIFFUSE_G;
	t->data[TB] = TEXTURE_DEFAULT_DIFFUSE_B;
}

void texture_defaultnormal(struct texture* t)
{
	t->width = 1;
	t->height = 1;

	t->data = (unsigned char*)mem_realloc(t->data, 3);
	t->data[TR] = 127;
	t->data[TG] = 127;
	t->data[TB] = 255;
}

unsigned texture_loadPNG(struct texture* t, char* filename)
{
	return 1;
}
