#include	"objloader.h"

#include	<float.h>
#include	<stdio.h>
#include	<string.h>
#include	"../error.h"
#include	"../math/vec2f.h"
#include	"../math/vec3f.h"
#include	"../mem.h"


void objloader_load(const char* filename, struct renderer* r, struct renderable* obj, vec3f dim, vec3f center)
{
	unsigned num_verts, num_norms, num_uvs, num_faces;
	unsigned* ptr_ind_verts, * ptr_ind_norms, * ptr_ind_uvs;
	unsigned* ind_verts, * ind_norms, * ind_uvs;
	float* ptr_verts, * ptr_norms, * ptr_uvs;
	vec3f* verts, * norms;
	vec3f min, max;
	vec2f* uvs;
	char line[256];
	FILE* file;
	unsigned i, j;

	// open file
	if (fopen_s(&file, filename, "r"))
	{
		PRINT_ERROR("Could not open obj file %s\n", filename);
		return;
	}

	// find the number of vertices, uv coordinates, normals and faces
	num_verts = num_uvs = num_norms = num_faces = 0;
	while (fscanf(file, "%64s", line) != EOF)
	{
		if (!strcmp(line, "v"))
			num_verts++;
		else if (!strcmp(line, "vt"))
			num_uvs++;
		else if (!strcmp(line, "vn"))
			num_norms++;
		else if (!strcmp(line, "f"))
			num_faces++;
	}

	// reset file pointer
	fseek(file, 0, SEEK_SET);

	// allocate temporary vertex buffers
	ptr_verts	= (float*)(verts	= (vec3f*)mem_alloc(num_verts * sizeof(vec3f)));
	ptr_norms	= (float*)(norms	= (vec3f*)mem_alloc(num_norms * sizeof(vec3f)));
	ptr_uvs		= (float*)(uvs		= (vec2f*)mem_alloc(num_uvs * sizeof(vec2f)));

	// allocate temporary index buffers
	ptr_ind_verts	= (unsigned*)(ind_verts	= (unsigned*)mem_alloc(num_faces * 3 * sizeof(unsigned)));
	ptr_ind_norms	= (unsigned*)(ind_norms	= (unsigned*)mem_alloc(num_faces * 3 * sizeof(unsigned)));
	ptr_ind_uvs		= (unsigned*)(ind_uvs	= (unsigned*)mem_alloc(num_faces * 3 * sizeof(unsigned)));

	// allocate the renderable component
	renderable_allocate(r, obj, num_faces * 3);

	// read in vertex attributes and faces
	while (fscanf(file, "%64s", line) != EOF)
	{
		if (!strcmp(line, "v"))
		{
			// vertex position
			fscanf(file, "%f %f %f", ptr_verts + VX, ptr_verts + VY, ptr_verts + VZ);
			ptr_verts += 3;
		} else if (!strcmp(line, "vn"))
		{
			// vertex normal
			fscanf(file, "%f %f %f", ptr_norms + VX, ptr_norms + VY, ptr_norms + VZ);
			ptr_norms += 3;
		} else if(!strcmp(line, "vt"))
		{
			// vertex texture coords
			fscanf(file, "%f %f", ptr_uvs + U, ptr_uvs + V);
			ptr_uvs[V] = -ptr_uvs[V];
			ptr_uvs += 2;
		} else if (!strcmp(line, "f"))
		{
			// face
			if (fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d",
					ptr_ind_verts + 0, ptr_ind_uvs + 0, ptr_ind_norms + 0, 
					ptr_ind_verts + 1, ptr_ind_uvs + 1, ptr_ind_norms + 1,
					ptr_ind_verts + 2, ptr_ind_uvs + 2, ptr_ind_norms + 2) != 9)
			{
				PRINT_ERROR("File can't be read by parser; maybe missing texture coordinates.\n");
				break;
			}

			ptr_ind_verts += 3;
			ptr_ind_norms += 3;
			ptr_ind_uvs += 3;
		}
	}

	// close file
	fclose(file);

	// copy temporary vertex data into outgoing vertex buffer
	ptr_verts = obj->buf_verts;
	for (i = 0; i < num_faces; i++)
	{
		for (j = 0; j < 3; j++)
		{
			vec3f_copy(ptr_verts, verts[ind_verts[i*3 + j] - 1]);
			ptr_verts += RENDER_ATTRIBSIZE_POS;

			vec3f_copy(ptr_verts, norms[ind_norms[i*3 + j] - 1]);
			ptr_verts += RENDER_ATTRIBSIZE_NOR;

			vec2f_copy(ptr_verts, uvs[ind_uvs[i*3 + j] - 1]);
			ptr_verts += RENDER_ATTRIBSIZE_TEX;
		}
	}

	// find dimensions and center point
	vec3f_set(min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3f_set(max, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (i = 0; i < num_verts; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (verts[i][j] < min[j])
				min[j] = verts[i][j];
			if (verts[i][j] > max[j])
				max[j] = verts[i][j];
		}
	}

	// center point
	vec3f_addn(center, min, max);
	vec3f_scale(center, 0.5f);

	// dimensions
	vec3f_subtractn(dim, max, min);

	// deallocate temporary vertex buffers
	mem_free(verts);
	mem_free(norms);
	mem_free(uvs);

	// deallocate temporary index buffers
	mem_free(ind_verts);
	mem_free(ind_norms);
	mem_free(ind_uvs);
}
