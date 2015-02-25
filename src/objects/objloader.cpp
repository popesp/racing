#include "objloader.h"

bool loadOBJ(
    const char * path,
    std::vector < vec3f > & out_vertices,
    std::vector < vec3f > & out_uvs,
    std::vector < vec3f > & out_normals
){

	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< vec3f > temp_vertices;
	std::vector< vec3f > temp_uvs;
	std::vector< vec3f > temp_normals;

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
			vec3f vertex;
			float x,y,z;
			fscanf(file, "%f %f %f\n", &x, &y, &z);
			vec3f_set(vertex, x, y, z);
			temp_vertices.push_back(vertex);
		}
		else if(strcmp(lineHeader, "vt") == 0){
			vec3f uvc;
			float u,v;
			fscanf(file, "%f %f\n", &u, &v);
			vec3f_set(uvc, u, v, 0);
			temp_uvs.push_back(uvc);
		}
		else if ( strcmp( lineHeader, "vn" ) == 0 ){
			vec3f normal;
			float x,y,z;
			fscanf(file, "%f %f %f\n", x, y, z );
			vec3f_set(normal, x,y,z);
			temp_normals.push_back(normal);
		}
		else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}

	}
	for(unsigned int i = 0; i< vertexIndices.size(); i++){
		unsigned int vertexIndex = vertexIndices[i];
		vec3f vertex;
		vec3f_copy(vertex, temp_vertices[vertexIndex -1]);
		out_vertices.push_back(vertex);
	}
	for(unsigned int i = 0; i< uvIndices.size(); i++){
		unsigned int uvIndex = uvIndices[i];
		vec3f uv;
		vec3f_copy(uv, temp_vertices[uvIndex -1]);
		out_vertices.push_back(uv);
	}
	for(unsigned int i = 0; i< normalIndices.size(); i++){
		unsigned int normalIndex = normalIndices[i];
		vec3f normal;
		vec3f_copy(normal, temp_vertices[normalIndex -1]);
		out_vertices.push_back(normal);
	}

}
