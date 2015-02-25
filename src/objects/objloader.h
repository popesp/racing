#ifndef OBJLOADER
#define OBJLOADER


#include	<vector>
#include	"../math/mat4f.h"


bool loadOBJ(
    const char * path,
    std::vector < vec3f > & out_vertices,
    std::vector < vec3f > & out_uvs,
    std::vector < vec3f > & out_normals
);

#endif