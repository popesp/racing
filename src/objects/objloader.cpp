
#include "objloader.h"
#include "../mem.h"
#include <stdio.h>
#include <string.h>

bool loadOBJ(
    const char * path,
    vec3f**  out_vertices,
    vec3f**  out_uvs,
    vec3f**  out_normals)
{


	unsigned int numv, numuv, numnorm, numface= 0;
	unsigned int t_vert_p, t_norm_p, t_uv_p, t_i_v_p, t_i_uv_p, t_i_n_p = 0;
	numv= numuv= numnorm= numface= 0;
	t_vert_p= t_norm_p= t_uv_p= t_i_v_p = t_i_uv_p= t_i_n_p = 0;
	unsigned int * vertexIndices, *uvIndices, *normalIndices;
	vec3f* temp_vertices;
	vec3f* temp_uvs;
	vec3f* temp_normals;

	FILE * file = fopen(path, "r");
	if( file == NULL){
		printf("Failed to open file at %s", path);
		return false;
	}

	while(true){
	char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if(res == EOF) break;
		if(strcmp(lineHeader, "v") == 0){
			numv++;
		}
		else if(strcmp(lineHeader, "vt") == 0){
			numuv++;
		}
		else if ( strcmp( lineHeader, "vn" ) == 0 ){
			numnorm++;
		}
		else if ( strcmp( lineHeader, "f" ) == 0 ){
			numface++;
		}
	}
	fseek(file, 0, SEEK_SET);

	vertexIndices = (unsigned int*)mem_alloc(3 * numface * sizeof(unsigned int));
	uvIndices	  = (unsigned int*)mem_alloc(3 * numface * sizeof(unsigned int));
	normalIndices = (unsigned int*)mem_alloc(3 * numface * sizeof(unsigned int));
	temp_vertices = (vec3f*)mem_alloc(numv * sizeof(vec3f));
	temp_uvs = (vec3f*)mem_alloc(numuv * sizeof(vec3f));
	temp_normals = (vec3f*)mem_alloc(numnorm * sizeof(vec3f));
	*out_vertices = (vec3f*)mem_alloc(3 * numface * sizeof(vec3f));
	*out_uvs  = (vec3f*)mem_alloc(3 * numface * sizeof(vec3f));
	*out_normals  = (vec3f*)mem_alloc(3 * numface * sizeof(vec3f));
	while(true){
		
		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);
		if(res == EOF) break;
		if(strcmp(lineHeader, "v") == 0){
			vec3f vertex;
			float x,y,z;
			fscanf(file, "%f %f %f\n", &x, &y, &z);
			vec3f_set(vertex, x, y, z);
			vec3f_set(temp_vertices[t_vert_p++],x,y,z);
		}
		else if(strcmp(lineHeader, "vt") == 0){
			vec3f uvc;
			float u,v;
			fscanf(file, "%f %f\n", &u, &v);
			vec3f_set(uvc, u, v, 0);
			vec3f_set(temp_uvs[t_uv_p++], u,v,0);
		}
		else if ( strcmp( lineHeader, "vn" ) == 0 ){
		//	vec3f normal;
			float x,y,z;
			fscanf(file, "%f %f %f\n", &x, &y, &z );
			vec3f_set(temp_normals[t_norm_p++], x,y,z);
		}
		else if ( strcmp( lineHeader, "f" ) == 0 ){
		//	std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			
			vertexIndices[t_i_v_p++] = vertexIndex[0];
			vertexIndices[t_i_v_p++] = vertexIndex[1];
			vertexIndices[t_i_v_p++] = vertexIndex[2];

			uvIndices[t_i_uv_p++] =(uvIndex[0]);
			uvIndices[t_i_uv_p++] =(uvIndex[1]);
			uvIndices[t_i_uv_p++] =(uvIndex[2]);
			
			normalIndices[t_i_n_p++] =(normalIndex[0]);
			normalIndices[t_i_n_p++] =(normalIndex[1]);
			normalIndices[t_i_n_p++] =(normalIndex[2]);
		}

	}
	for(unsigned int i = 0; i< t_i_v_p - 1; i++){
		unsigned int vertexIndex = vertexIndices[i];
		vec3f vertex;
		vec3f_copy(vertex, temp_vertices[vertexIndex -1]);
		vec3f_copy((*out_vertices)[i],(vertex));
	}
	for(unsigned int i = 0; i< t_i_uv_p - 1; i++){
		unsigned int uvIndex = uvIndices[i];
		vec3f uv;
		vec3f_copy(uv, temp_uvs[uvIndex -1]);
		vec3f_copy((*out_uvs)[i],(uv));
	}
	for(unsigned int i = 0; i< t_i_n_p - 1; i++){
		unsigned int normalIndex = normalIndices[i];
		vec3f normal;
		vec3f_copy(normal, temp_normals[normalIndex -1]);
		vec3f_copy((*out_normals)[i],(normal));
	}
	return true;
}
