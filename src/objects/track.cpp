#include	"track.h"

#include	<float.h>
#include	<math.h>				// cosf, sinf
#include	"../error.h"
#include	"../math/vec2f.h"
#include	"../math/mat4f.h"		// identity
#include	"../mem.h"				// free
#include	"../physics/collision.h"


static vec3f segment_pos[TRACK_SEGMENT_VERTCOUNT] =
{
	{ 0.f, 0.f, 0.f },
	{ 0.5f, 1.f, 0.f },
	{ 1.f, 1.f, 0.f },
	{ 1.f, -2.f, 0.f }
};

static vec3f segment_nor[TRACK_SEGMENT_VERTCOUNT] =
{
	{0.f, 1.f, 0.f},
	{ -0.894427f, 0.447214f, 0.f},
	{ 0.f, 1.f, 0.f },
	{ 1.f, 0.f, 0.f }
};

static float segment_tex[TRACK_SEGMENT_VERTCOUNT] =
{
	0.f,
	1.11803f,
	0.5f,
	3.f
};


void track_init(struct track* t, struct physicsmanager* pm, vec3f up)
{
	t->pm = pm;

	t->num_points = 0;
	t->points = NULL;

	t->num_pathpoints = 0;
	t->pathpoints = NULL;

	t->p_track = NULL;

	renderable_init(&t->r_track, RENDER_MODE_TRIANGLESTRIP, RENDER_TYPE_BUMP_L, RENDER_FLAG_NONE);

	// initialize material properties
	vec3f_set(t->r_track.material.amb, 0.3f, 0.3f, 0.3f);
	vec3f_set(t->r_track.material.dif, 0.1f, 0.1f, 0.1f);
	vec3f_set(t->r_track.material.spc, 0.5f, 0.5f, 0.5f);
	t->r_track.material.shn = 100.f;

	texture_init(&t->diffuse);
	texture_init(&t->normal);
	texture_loadfile(&t->diffuse, TRACK_TEXTURE_FILENAME_DIFFUSE);
	texture_loadfile(&t->normal, TRACK_TEXTURE_FILENAME_NORMAL);
	texture_upload(&t->diffuse, RENDER_TEXTURE_DIFFUSE);
	texture_upload(&t->normal, RENDER_TEXTURE_NORMAL);
	t->r_track.textures[RENDER_TEXTURE_DIFFUSE] = &t->diffuse;
	t->r_track.textures[RENDER_TEXTURE_NORMAL] = &t->normal;

	t->dist_boundary = TRACK_DEFAULT_DISTBOUND;

	vec3f_copy(t->up, up);

	t->flags = TRACK_FLAG_INIT;
}

void track_delete(struct track* t)
{
	mem_free(t->points);
	mem_free(t->pathpoints);
	
	texture_delete(&t->diffuse);
	texture_delete(&t->normal);
	renderable_deallocate(&t->r_track);
}


int track_closestindex(struct track* t, vec3f pos, int last)
{
	float d, least;
	int i, l, e;
	vec3f diff;

	least = FLT_MAX;
	l = last;

	// find starting index
	i = last - TRACK_SEARCHSIZE;
	if (i < 0)
	{
		if (t->flags & TRACK_FLAG_LOOPED)
			i += t->num_pathpoints;
		else
			i = 0;
	}

	// find ending index
	e = last + TRACK_SEARCHSIZE;
	if (e >= (int)t->num_pathpoints)
	{
		if (t->flags & TRACK_FLAG_LOOPED)
			e -= t->num_pathpoints;
		else
			e = (int)t->num_pathpoints - 1;
	}

	while (i != e)
	{
		vec3f_subtractn(diff, pos, t->pathpoints[i].pos);
		d = vec3f_length(diff);

		if (d < least)
		{
			least = d;
			l = i;
		}

		i = (i+1) % (int)t->num_pathpoints;
	}

	return l;
}


void track_transformindex(struct track* t, mat4f res, int index)
{
	vec3f tan, bin, nor;

	// find negated tangent
	vec3f_copy(tan, t->pathpoints[index].tan);
	vec3f_negate(tan);

	vec3f_cross(bin, t->up, tan);
	vec3f_normalize(bin);

	vec3f_cross(nor, tan, bin);

	// find the change of basis matrix
	mat4f_identity(res);
	vec3f_copy(res + C0, bin);
	vec3f_copy(res + C1, nor);
	vec3f_copy(res + C2, tan);
	vec3f_copy(res + C3, t->pathpoints[index].pos);

	// rotate to the track angle
	mat4f_rotatezmul(res, -t->pathpoints[index].angle);
}


void track_loadpointsfile(struct track* t, const char* filename, struct renderer* r)
{
	FILE* file;
	char s[64];
	int p=0;

	if (fopen_s(&file, filename, "rb"))
	{
		PRINT_ERROR("Could not open track file %s\n", filename);
		return;
	}

	while (fscanf(file, "%s = ", s) != EOF)
	{
		if (!strcmp(s, "npoints"))
		{
			if (fscanf(file, "%d", &t->num_points) != 1)
			{
				PRINT_ERROR("Track file is malformed.\n");
				fclose(file);
				return;
			}

			// allocate space for points
			t->points = (struct track_point*)mem_realloc(t->points, sizeof(struct track_point) * t->num_points);
		} else if (!strcmp(s, "looped"))
		{
			if (fscanf(file, "%s", s) != 1)
			{
				PRINT_ERROR("Track file is malformed.\n");
				fclose(file);
				return;
			}

			if (!strcmp(s, "true"))
				t->flags |= TRACK_FLAG_LOOPED;
			else if (!strcmp(s, "false"))
				t->flags &= ~TRACK_FLAG_LOOPED;
			else
			{
				PRINT_ERROR("Expected true or false for looped variable.\n");
				fclose(file);
				return;
			}
		} else if (!strcmp(s, "point"))
		{
			fscanf(file, "pos(%f, %f, %f), ", t->points[p].pos + VX, t->points[p].pos + VY, t->points[p].pos + VZ);
			fscanf(file, "tan(%f, %f, %f), ", t->points[p].tan + VX, t->points[p].tan + VY, t->points[p].tan + VZ);
			vec3f_normalize(t->points[p].tan);

			fscanf(file, "angle(%f), ", &t->points[p].angle);
			fscanf(file, "weight(%f), ", &t->points[p].weight);
			fscanf(file, "width(%f), ", &t->points[p].width);
			fscanf(file, "subdivisions(%d)", &t->points[p].subdivisions);

			p++;
		}
	}

	fclose(file);

	track_generate(t, r);
}


static void curvepoint(struct track* t, unsigned index, float d, struct track_point* res)
{
	struct track_point* p0, * p1;
	float dp0, dp1, dt0, dt1;
	float d2, d3;

	d2 = d*d;
	d3 = d2*d;

	p0 = t->points + index;
	if ((t->flags & TRACK_FLAG_LOOPED) && (index >= t->num_points - 1))
		p1 = t->points;
	else
		p1 = t->points + index + 1;

	// position coefficients
	dp0 = 2.f*d3 - 3.f*d2 + 1.f;
	dp1 = -2.f*d3 + 3.f*d2;
	dt0 = (d3 - 2.f*d2 + d)*p0->weight;
	dt1 = (d3 - d2)*p1->weight;

	// solve curve for position
	res->pos[VX] = dp0*p0->pos[VX] + dp1*p1->pos[VX] + dt0*p0->tan[VX] + dt1*p1->tan[VX];
	res->pos[VY] = dp0*p0->pos[VY] + dp1*p1->pos[VY] + dt0*p0->tan[VY] + dt1*p1->tan[VY];
	res->pos[VZ] = dp0*p0->pos[VZ] + dp1*p1->pos[VZ] + dt0*p0->tan[VZ] + dt1*p1->tan[VZ];

	// cubic interpolation for angle and width attributes
	res->angle = dp0*p0->angle + dp1*p1->angle;
	res->width = dp0*p0->width + dp1*p1->width;

	// tangent coefficients
	dp0 = 6.f*(d2 - d);
	dp1 = 6.f*(d - d2);
	dt0 = (3.f*d2 - 4.f*d + 1.f)*p0->weight;
	dt1 = (3.f*d2 - 2.f*d)*p1->weight;

	// solve curve for tangent
	res->tan[VX] = dp0*p0->pos[VX] + dp1*p1->pos[VX] + dt0*p0->tan[VX] + dt1*p1->tan[VX];
	res->tan[VY] = dp0*p0->pos[VY] + dp1*p1->pos[VY] + dt0*p0->tan[VY] + dt1*p1->tan[VY];
	res->tan[VZ] = dp0*p0->pos[VZ] + dp1*p1->pos[VZ] + dt0*p0->tan[VZ] + dt1*p1->tan[VZ];

	// normalize tangent vector
	res->weight = vec3f_length(res->tan);
	vec3f_normalize(res->tan);
}

static inline float* fillbuffer(float* vptr, mat4f basis, vec3f p0, vec3f p1, vec3f n0, vec3f n1, float v0, float v1, float u)
{
	vec3f pos;

	mat4f_fulltransformvec3fn(pos, p0, basis);
	vec3f_copy(vptr, pos);
	vptr += RENDER_ATTRIBSIZE_POS;

	mat4f_transformvec3fn(vptr, n0, basis);
	vptr += RENDER_ATTRIBSIZE_NOR;

	vec3f_copy(vptr, basis + C2);
	vptr += RENDER_ATTRIBSIZE_TAN;

	vptr[0] = u;
	vptr[1] = v0;
	vptr += RENDER_ATTRIBSIZE_TEX;

	mat4f_fulltransformvec3fn(pos, p1, basis);
	vec3f_copy(vptr, pos);
	vptr += RENDER_ATTRIBSIZE_POS;

	mat4f_transformvec3fn(vptr, n1, basis);
	vptr += RENDER_ATTRIBSIZE_NOR;

	vec3f_copy(vptr, basis + C2);
	vptr += RENDER_ATTRIBSIZE_TAN;

	vptr[0] = u;
	vptr[1] = v1;
	vptr += RENDER_ATTRIBSIZE_TEX;

	return vptr;
}

static float* addverts(struct track* t, struct track_point* p, float* vptr, float u)
{
	vec3f nor, bin, p0, p1, n;
	mat4f basis;
	float v;
	int i;

	// find binormal vector for the track
	vec3f_cross(bin, t->up, p->tan);
	vec3f_normalize(bin);

	// find normal
	vec3f_cross(nor, p->tan, bin);

	// construct change of basis and translation transformation matrix
	mat4f_identity(basis);
	vec3f_copy(basis + C0, bin);
	vec3f_copy(basis + C1, nor);
	vec3f_copy(basis + C2, p->tan);
	vec3f_copy(basis + C3, p->pos); // translation

	mat4f_rotatezmul(basis, p->angle);

	/* --- left side --- */
	// far left
	// positions
	vec3f_set(p0, 0.f, segment_pos[TRACK_SEGMENT_VERTCOUNT-1][VY], 0.f);
	vec3f_copy(p1, segment_pos[TRACK_SEGMENT_VERTCOUNT-1]);
	p1[VX] = p1[VX] + p->width*0.5f;

	// normal
	vec3f_set(n, 0.f, -1.f, 0.f);

	// fill buffer
	v = 0.f;
	vptr = fillbuffer(vptr, basis, p0, p1, n, n, v, v + p->width*0.5f + segment_pos[TRACK_SEGMENT_VERTCOUNT-1][VX], u);
	v += p->width + segment_pos[TRACK_SEGMENT_VERTCOUNT-1][VX];

	// left segment
	for (i = TRACK_SEGMENT_VERTCOUNT - 1; i > 0; i--)
	{
		// positions
		vec3f_copy(p0, segment_pos[i]);
		p0[VX] = p0[VX] + p->width*0.5f;
		vec3f_copy(p1, segment_pos[i-1]);
		p1[VX] = p1[VX] + p->width*0.5f;

		// fill buffer
		vptr = fillbuffer(vptr, basis, p0, p1, segment_nor[i], segment_nor[i], v, v + segment_tex[i], u);
		v += segment_tex[i];
	}

	// left center
	// positions
	vec3f_copy(p0, segment_pos[0]);
	p0[VX] = p0[VX] + p->width*0.5f;
	vec3f_set(p1, 0.f, 0.f, 0.f);

	// fill buffer
	vptr = fillbuffer(vptr, basis, p0, p1, segment_nor[0], segment_nor[0], v, v + p->width*0.5f, u);
	v += p->width*0.5f;
	/* --- end left side --- */

	/* --- right side --- */
	// right center
	// positions
	vec3f_set(p0, 0.f, 0.f, 0.f);
	vec3f_copy(p1, segment_pos[0]);
	p1[VX] = -1.f*(p1[VX] + p->width*0.5f);

	// normal
	vec3f_copy(n, segment_nor[0]);
	n[VX] *= -1.f;

	// fill buffer
	vptr = fillbuffer(vptr, basis, p0, p1, n, n, v, v + p->width*0.5f, u);
	v += p->width*0.5f;

	// right segment
	for (i = 0; i < TRACK_SEGMENT_VERTCOUNT - 1; i++)
	{
		// positions
		vec3f_copy(p0, segment_pos[i]);
		p0[VX] = -1.f*(p0[VX] + p->width*0.5f);
		vec3f_copy(p1, segment_pos[i + 1]);
		p1[VX] = -1.f*(p1[VX] + p->width*0.5f);

		// normal
		vec3f_copy(n, segment_nor[i + 1]);
		n[VX] *= -1.f;

		// fill buffer
		vptr = fillbuffer(vptr, basis, p0, p1, n, n, v, v + segment_tex[i + 1], u);
		v += segment_tex[i];
	}

	// far right
	// positions
	vec3f_copy(p0, segment_pos[TRACK_SEGMENT_VERTCOUNT-1]);
	p0[VX] = -1.f*(p0[VX] + p->width*0.5f);
	vec3f_set(p1, 0.f, segment_pos[TRACK_SEGMENT_VERTCOUNT-1][VY], 0.f);
	
	// normal
	vec3f_set(n, 0.f, -1.f, 0.f);

	// fill buffer
	vptr = fillbuffer(vptr, basis, p0, p1, n, n, v, v + p->width*0.5f + segment_pos[TRACK_SEGMENT_VERTCOUNT-1][VX], u);

	return vptr;
}

static float* copyvert(float* vptr, float* srcptr)
{
	vec3f_copy(vptr, srcptr);
	vptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(vptr, srcptr + RENDER_ATTRIBSIZE_POS);
	vptr += RENDER_ATTRIBSIZE_NOR;
	vec3f_copy(vptr, srcptr + RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR);
	vptr += RENDER_ATTRIBSIZE_TAN;
	vec2f_copy(vptr, srcptr + RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR + RENDER_ATTRIBSIZE_TAN);
	vptr += RENDER_ATTRIBSIZE_TEX;

	return vptr;
}

void track_generate(struct track* t, struct renderer* r)
{
	struct track_point p;
	unsigned i, j, n, s, ps;
	float* verts;
	float* ptr;
	float d, u;
	int offs;

	if (t->flags & TRACK_FLAG_LOOPED)
		n = t->num_points;
	else
		n = t->num_points - 1;

	s = 1;
	for (i = 0; i < n; i++)
		s += t->points[i].subdivisions + 1;

	// allocate mesh renderable vertex buffer
	// the second parameter here looks complicated but is just calculating the required number of vertices to triangle strip the track
	renderable_allocate(r, &t->r_track, 4 * (TRACK_SEGMENT_VERTCOUNT + 1) * (s + 1));

	// allocate space for the search points (underlying track spline, independent from rendered/physical mesh)
	t->num_pathpoints = n * (TRACK_SEARCHDIVIDE + 1);
	t->pathpoints = (struct path_point*)mem_alloc(t->num_pathpoints * sizeof(struct path_point));

	// temporary vertex buffer
	verts = (float*)mem_alloc(s * 4 * (TRACK_SEGMENT_VERTCOUNT + 1) * r->vertsize[t->r_track.type] * sizeof(float));

	// place vertex attributes into buffer
	ptr = verts;
	u = 0.f;
	for (i = 0; i < n; i++)
	{
		d = 0.f;

		// loop for rendered/physical mesh
		ps = t->points[i].subdivisions + 1;
		for (j = 0, d = 0.f; j < ps; j++, d += 1.f / (float)ps)
		{
			// find point on track curve and add vertices to generate track
			curvepoint(t, i, d, &p);
			ptr = addverts(t, &p, ptr, u);
			u += 1.f;
		}

		// loop for search points
		for (j = 0, d = 0.f; j < TRACK_SEARCHDIVIDE + 1; j++, d += 1.f / (float)(TRACK_SEARCHDIVIDE + 1))
		{
			curvepoint(t, i, d, &p);
			vec3f_copy(t->pathpoints[i*(TRACK_SEARCHDIVIDE+1) + j].pos, p.pos);
			vec3f_copy(t->pathpoints[i*(TRACK_SEARCHDIVIDE+1) + j].tan, p.tan);
			t->pathpoints[i*(TRACK_SEARCHDIVIDE+1) + j].angle = p.angle;
			t->pathpoints[i*(TRACK_SEARCHDIVIDE+1) + j].width = p.width;
		}
	}

	// generate the last point on the track
	curvepoint(t, n - 1, 1.f, &p);
	ptr = addverts(t, &p, ptr, u);

	// copy vector attributes into renderable buffer
	int temp = 0;
	ptr = t->r_track.buf_verts;
	for (i = 0; i < 2 * (TRACK_SEGMENT_VERTCOUNT + 1); i++)
	{
		offs = (int)i * 2;

		// loop through each segment
		for (j = 0; j < s; j++)
		{
			// copy points
			ptr = copyvert(ptr, verts + offs*r->vertsize[t->r_track.type]);
			ptr = copyvert(ptr, verts + (offs + 1)*r->vertsize[t->r_track.type]);
			temp += 2;

			offs += 4 * (TRACK_SEGMENT_VERTCOUNT + 1);
		}

		// repeat points at start and end of track
		ptr = copyvert(ptr, ptr - r->vertsize[t->r_track.type]);
		ptr = copyvert(ptr, verts + 2*(i + 1)*r->vertsize[t->r_track.type]);
		temp += 2;
	}

	mem_free(verts);

	// send renderable buffer to OpenGL
	renderable_sendbuffer(r, &t->r_track);

	// send mesh to physics manager
	t->p_track = physicsmanager_addstatic_trianglestrip(t->pm, t->r_track.num_verts, sizeof(float)*r->vertsize[t->r_track.type], t->r_track.buf_verts);
	collision_setupactor(t->p_track, COLLISION_FILTER_STATIC, 0);
}