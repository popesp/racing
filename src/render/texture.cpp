#include	"texture.h"

#include	<FreeImage.h>
#include	<GL/glew.h>
#include	<GLFW/glfw3.h>
#include	"../error.h"
#include	"../mem.h"



void texture_init(struct texture* t)
{
	t->data = NULL;

	glGenTextures(1, &t->gl_id);
	glBindTexture(GL_TEXTURE_2D, t->gl_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void texture_delete(struct texture* t)
{
	mem_free(t->data);

	glDeleteTextures(1, &t->gl_id);
}


void texture_soliddiffuse(struct texture* t, unsigned char r, unsigned char g, unsigned char b)
{
	t->width = 1;
	t->height = 1;

	t->data = (unsigned char*)mem_realloc(t->data, 4);
	t->data[TB] = b;
	t->data[TG] = g;
	t->data[TR] = r;
	t->data[TA] = 255u;
}

void texture_defaultnormal(struct texture* t)
{
	t->width = 1;
	t->height = 1;

	t->data = (unsigned char*)mem_realloc(t->data, 4);
	t->data[TB] = 255u;
	t->data[TG] = 127u;
	t->data[TR] = 127u;
	t->data[TA] = 255u;
}

void texture_loadfile(struct texture* t, const char* filename)
{
	FREE_IMAGE_FORMAT format;
	FIBITMAP* bmp, * bmp32;

	format = FreeImage_GetFileType(filename, 0);
	if (format == FIF_UNKNOWN)
		format = FreeImage_GetFIFFromFilename(filename);
	if (format == FIF_UNKNOWN)
		PRINT_ERROR("Unable to open texture file: %s\n", filename);

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
}


void texture_upload(struct texture* t, unsigned target)
{
	glActiveTexture(GL_TEXTURE0 + target);
	glBindTexture(GL_TEXTURE_2D, t->gl_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)t->width, (int)t->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, t->data);
	glGenerateMipmap(GL_TEXTURE_2D);
}