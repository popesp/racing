#include	"texture.h"

#include	<FreeImage.h>
#include	<GL/glew.h>
#include	<GLFW/glfw3.h>
#include	"../error.h"
#include	"../mem.h"


/*	start up the texture manager
	param:	tm			texture manager (modified)
*/
void texturemanager_startup(struct texturemanager* tm)
{
	int i;

	for (i = 0; i < TEXTURE_MAX; i++)
	{
		tm->textures[i].data = NULL;
		tm->textures[i].enabled = false;
	}
}

/*	shut down the texture manager
	param:	tm			texture manager (modified)
*/
void texturemanager_shutdown(struct texturemanager* tm)
{
	int i;

	for (i = 0; i < TEXTURE_MAX; i++)
		texturemanager_removetexture(tm, i);
}

/*	get the version string of FreeImage
	param:	tm			texture manager (modified)
*/
const char* texturemanager_getlibversion(struct texturemanager* tm)
{
	(void)tm;
	return FreeImage_GetVersion();
}


/*	create a new texture object
	param:	pm			texture manager
	return	int			id for the new texture object
*/
int texturemanager_newtexture(struct texturemanager* tm)
{
	int i;

	for (i = 0; i < TEXTURE_MAX; i++)
	{
		if (!tm->textures[i].enabled)
		{
			tm->textures[i].enabled = true;

			glGenTextures(1, &tm->textures[i].gl_id);
			glBindTexture(GL_TEXTURE_2D, tm->textures[i].gl_id);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			return i;
		}
	}

	return -1;
}

/*	remove a texture object
	param:	tm			texture manager
	param:	int			id for the texture object to remove
*/
void texturemanager_removetexture(struct texturemanager* tm, int id)
{
	mem_free(tm->textures[id].data);
	glDeleteTextures(1, &tm->textures[id].gl_id);
	tm->textures[id].data = NULL;
	tm->textures[id].enabled = false;
}


/*	set a texture to a solid diffuse color
	param:	tm			texture manager
	param:	id			id for the texture object to set
	param:	r			red component
	param:	g			green component
	param:	b			blue component
*/
void texture_soliddiffuse(struct texturemanager* tm, int id, unsigned char r, unsigned char g, unsigned char b)
{
	struct texture* t;

	t = tm->textures + id;

	t->width = 1;
	t->height = 1;

	t->data = (unsigned char*)mem_realloc(t->data, 3);
	t->data[TR] = r;
	t->data[TG] = g;
	t->data[TB] = b;
}

/*	set a texture to a flat normal map
	param:	tm			texture manager
	param:	id			id for the texture object to set
*/
void texture_defaultnormal(struct texturemanager* tm, int id)
{
	struct texture* t;

	t = tm->textures + id;

	t->width = 1;
	t->height = 1;

	t->data = (unsigned char*)mem_realloc(t->data, 3);
	t->data[TR] = 127;
	t->data[TG] = 127;
	t->data[TB] = 255;
}

/*	set a texture to the contents of an image file
	param:	tm			texture manager
	param:	id			id for the texture object to set
	param:	filename	path to file to load from
*/
unsigned texture_loadfile(struct texturemanager* tm, int id, const char* filename)
{
	FREE_IMAGE_FORMAT format;
	FIBITMAP* bmp, * bmp32;
	struct texture* t;

	t = tm->textures + id;

	format = FreeImage_GetFileType(filename, 0);
	if (format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(filename);
	if (format == FIF_UNKNOWN)
	{
		PRINT_ERROR("Unable to open texture file: %s\n", filename);
		return 0;
	}

	// load and convert to 32-bit
	bmp = FreeImage_Load(format, filename);
	bmp32 = FreeImage_ConvertTo32Bits(bmp);
	FreeImage_Unload(bmp);

	t->width = FreeImage_GetWidth(bmp32);
	t->height = FreeImage_GetHeight(bmp32);

	// allocate and fill texture buffer
	t->data = (unsigned char*)mem_realloc(t->data, sizeof(unsigned char) * t->width * t->height * 32);
	FreeImage_ConvertToRawBits(t->data, bmp32, (int)t->width * 4, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, true);
	FreeImage_Unload(bmp32);

	return 1;
}


/*	upload a texture to OpenGL and generate mipmaps
	param:	tm			texture manager
	param:	id			id for the texture object to upload
	param:	target		texture target to upload to
*/
void texture_upload(struct texturemanager* tm, int id, unsigned target)
{
	struct texture* t;

	t = tm->textures + id;

	glActiveTexture(GL_TEXTURE0 + target);
	glBindTexture(GL_TEXTURE_2D, t->gl_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)t->width, (int)t->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, t->data);
	glGenerateMipmap(GL_TEXTURE_2D);
}