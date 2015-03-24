#ifndef OBJLOADER
#define	OBJLOADER


#include	"../render/render.h"


/*	load a mesh from an obj file into a renderable object
	param:	filename		path to obj file
	param:	r				renderer
	param:	obj				renderable object
	param:	dim				dimensions of the loaded object
	param:	center			centerpoint of the loaded model
*/
void objloader_load(const char* filename, struct renderer* r, struct renderable* obj, vec3f dim, vec3f center);


#endif