
#ifndef OBJLOADER
#define OBJLOADER


#include	"../math/vec3f.h"


bool loadOBJ(
    const char * path,
    vec3f*  out_vertices,
    vec3f*  out_uvs,
    vec3f*  out_normals
);

#endif