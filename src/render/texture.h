#ifndef TEXTURE
#define	TEXTURE


#define	TB	0
#define	TG	1
#define	TR	2
#define	TA	3


struct texture
{
	unsigned width;
	unsigned height;

	unsigned char* data;

	unsigned gl_id;
};


/*	initialize a texture object
	param:	t			texture object to initialize
*/
void texture_init(struct texture* t);

/*	delete a texture object
	param:	t			texture object to delete
	*/
void texture_delete(struct texture* t);


/*	set a texture to a solid diffuse color
	param:	t			texture object
	param:	r			red component (range: 0-1)
	param:	g			green component (range: 0-1)
	param:	b			blue component (range: 0-1)
*/
void texture_soliddiffuse(struct texture* t, unsigned char r, unsigned char g, unsigned char b);

/*	set a texture to a flat normal map
	param:	t			texture object
*/
void texture_defaultnormal(struct texture* t);

/*	set a texture to the contents of an image file
	param:	t			texture object
	param:	filename	path to file to load from
*/
void texture_loadfile(struct texture* t, const char* filename);


/*	upload a texture to OpenGL and generate mipmaps
	param:	t			texture object
	param:	target		texture target to upload to
*/
void texture_upload(struct texture* t, unsigned target);


#endif
