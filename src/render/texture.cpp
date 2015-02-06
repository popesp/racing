#include	"texture.h"

#include	<FreeImage.h>
#include	"../error.h"
#include	"../mem.h"


void texture_startup(struct texturemanager* tm)
{

}

const char* texture_getversion(struct texturemanager* tm)
{
	(void)tm;
	return FreeImage_GetVersion();
}


void texture_init(struct texture* t)
{
	t->data = NULL;
}

void texture_delete(struct texture* t)
{
	mem_free(t->data);
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
	(void)t;
	(void)filename;

	return 1;
}
