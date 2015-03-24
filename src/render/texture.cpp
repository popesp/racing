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
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
}

void texture_delete(struct texture* t)
{
	mem_free(t->data);

	glDeleteTextures(1, &t->gl_id);
}


void texture_allocate(struct texture* t, unsigned width, unsigned height)
{
	t->width = width;
	t->height = height;

	t->data = (pixel*)mem_realloc(t->data, sizeof(pixel) * width * height);
}

void texture_soliddiffuse(struct texture* t, unsigned char r, unsigned char g, unsigned char b)
{
	t->width = 1;
	t->height = 1;

	t->data = (pixel*)mem_realloc(t->data, sizeof(pixel));
	t->data[0][TB] = b;
	t->data[0][TG] = g;
	t->data[0][TR] = r;
	t->data[0][TA] = 255;
}

void texture_defaultnormal(struct texture* t)
{
	t->width = 1;
	t->height = 1;

	t->data = (pixel*)mem_realloc(t->data, sizeof(pixel));
	t->data[0][TB] = 255;
	t->data[0][TG] = 127;
	t->data[0][TR] = 127;
	t->data[0][TA] = 255;
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
	t->data = (pixel*)mem_realloc(t->data, sizeof(pixel) * t->width * t->height);
	FreeImage_ConvertToRawBits((unsigned char*)t->data, bmp32, (int)t->width * sizeof(pixel), 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, true);
	FreeImage_Unload(bmp32);
}


void texture_fill(struct texture* t, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	unsigned i;

	for (i = 0; i < t->width*t->height; i++)
	{
		t->data[i][TR] = r;
		t->data[i][TG] = g;
		t->data[i][TB] = b;
		t->data[i][TA] = a;
	}
}


void texture_blitalpha(struct texture* t, unsigned char* data, unsigned width, unsigned height, unsigned x, unsigned y)
{
	unsigned i, j;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			t->data[(y+i)*t->width + (x+j)][TR] = 0;
			t->data[(y+i)*t->width + (x+j)][TG] = 0;
			t->data[(y+i)*t->width + (x+j)][TB] = 0;
			t->data[(y+i)*t->width + (x+j)][TA] = data[i*width + j];
		}
	}
}


void texture_upload(struct texture* t, unsigned target)
{
	glActiveTexture(GL_TEXTURE0 + target);
	glBindTexture(GL_TEXTURE_2D, t->gl_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)t->width, (int)t->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, t->data);
	glGenerateMipmap(GL_TEXTURE_2D);
}