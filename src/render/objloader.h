#ifndef OBJLOADER
#define	OBJLOADER


#include	"../render/render.h"


/*	load a mesh from an obj file into a renderable object
	param:	filename		path to obj file
	param:	r				renderer
	param:	obj				renderable object
*/
void objloader_load(const char* filename, struct renderer* r, struct renderable* obj);


#endif