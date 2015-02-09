#ifndef TEXTURE
#define	TEXTURE


#define	TB							0
#define	TG							1
#define	TR							2

#define	TEXTURE_MAX					10


struct texture
{
	unsigned width;
	unsigned height;

	unsigned char* data;

	unsigned gl_id;

	bool enabled;
};

struct texturemanager
{
	struct texture textures[TEXTURE_MAX];
};


/*	start up the texture manager
	param:	tm			texture manager (modified)
*/
void texturemanager_startup(struct texturemanager* tm);

/*	shut down the texture manager
	param:	tm			texture manager (modified)
*/
void texturemanager_shutdown(struct texturemanager* tm);

/*	get the version string of FreeImage
	param:	tm			texture manager (modified)
*/
const char* texturemanager_getlibversion(struct texturemanager* tm);


/*	create a new texture object
	param:	tm			texture manager
	return	int			id for the new texture object
*/
int texturemanager_newtexture(struct texturemanager* tm);

/*	remove a texture object
	param:	tm			texture manager
	param:	int			id for the texture object to remove
*/
void texturemanager_removetexture(struct texturemanager* tm, int id);


/*	set a texture to a solid diffuse color
	param:	tm			texture manager
	param:	id			id for the texture object to set
	param:	r			red component (range: 0-1)
	param:	g			green component (range: 0-1)
	param:	b			blue component (range: 0-1)
*/
void texture_soliddiffuse(struct texturemanager* tm, int id, unsigned char r, unsigned char g, unsigned char b);

/*	set a texture to a flat normal map
	param:	tm			texture manager
	param:	id			id for the texture object to set
*/
void texture_defaultnormal(struct texturemanager* tm, int id);

/*	set a texture to the contents of an image file
	param:	tm			texture manager
	param:	id			id for the texture object to set
	param:	filename	path to file to load from
*/
unsigned texture_loadfile(struct texturemanager* tm, int id, const char* filename);


/*	upload a texture to OpenGL and generate mipmaps
	param:	tm			texture manager
	param:	id			id for the texture object to upload
	param:	target		texture target to upload to
*/
void texture_upload(struct texturemanager* tm, int id, unsigned target);


#endif
