#ifndef TEXTURE
#define	TEXTURE


#define	TR							0
#define	TG							1
#define	TB							2

#define	TEXTURE_DEFAULT_DIFFUSE_R	50
#define	TEXTURE_DEFAULT_DIFFUSE_G	100
#define	TEXTURE_DEFAULT_DIFFUSE_B	50


struct texture
{
	unsigned width;
	unsigned height;

	unsigned char* data;
};

struct texturemanager
{

};


void texture_startup(struct texturemanager* tm);
const char* texture_getversion(struct texturemanager* tm);

void texture_init(struct texture* t);
void texture_delete(struct texture* t);

void texture_defaultdiffuse(struct texture* t);
void texture_defaultnormal(struct texture* t);
unsigned texture_loadPNG(struct texture* t, char* filename);


#endif
