#include	"track.h"

#include	<math.h>				// cosf, sinf
#include	"../error.h"
#include	"../math/mat4f.h"		// identity
#include	"../math/vec3f.h"		// set, normalize
#include	"../mem.h"				// free
#include	"../physics/physics.h"
#include	"../render/render.h"	// renderable: init, allocate, deallocate


static vec3f segment_pos[TRACK_SEGMENT_VERTCOUNT] = {
	{ 0.f, 0.f, 0.f },
	{ 0.5f, 1.f, 0.f },
	{ 1.f, 1.f, 0.f },
	{ 1.f, -2.f, 0.f },
	{ -2.5f, -2.f, 0.f } };

static vec3f segment_nor[TRACK_SEGMENT_VERTCOUNT] = {
	{ 0.f, 1.f, 0.f },
	{ -0.894427f, 0.447214f, 0.f },
	{ 0.f, 1.f, 0.f },
	{ 1.f, 0.f, 0.f },
	{0.f, -1.f, 0.f} };


void track_init(struct track* t, vec3f up, struct physicsmanager* pm)
{
	(void)pm;

	t->num_points = 0;
	t->points = NULL;

	t->p_track = NULL;

	renderable_init(&t->r_track, RENDER_MODE_TRIANGLESTRIP, RENDER_TYPE_BUMP_L, RENDER_FLAG_NONE);

	// initialize material properties
	vec3f_set(t->r_track.material.amb, 0.4f, 0.4f, 0.4f);
	vec3f_set(t->r_track.material.dif, 0.4f, 0.4f, 0.4f);
	vec3f_set(t->r_track.material.spc, 0.8f, 0.8f, 0.8f);
	t->r_track.material.shn = 100.f;

	vec3f_copy(t->up, up);

	t->flags = TRACK_FLAG_INIT;
}

void track_delete(struct track* t)
{
	mem_free(t->points);

	renderable_deallocate(&t->r_track);
}


int track_closestindex(struct track* t, vec3f pos, int last)
{
	return 0;
}


void track_loadpointsfile(struct track* t, const char* filename)
{
	FILE* file;
	char s[64];
	int p;

	if (fopen_s(&file, filename, "rb"))
	{
		PRINT_ERROR("Could not open track file %s\n", filename);
		return;
	}

	while (fscanf(file, "%s = ", s) != EOF)
	{
		if (!strncmp(s, "npoints", 7))
		{
			if (fscanf(file, "%d;", &t->num_points) != 1)
			{
				PRINT_ERROR("Track file is malformed.\n");
				fclose(file);
				return;
			}

			// allocate space for points
			t->points = (struct track_point*)mem_realloc(t->points, sizeof(struct track_point) * t->num_points);
		} else if (!strncmp(s, "looped", 6))
		{
			if (fscanf(file, "%s;", s) != 1)
			{
				PRINT_ERROR("Track file is malformed.\n");
				fclose(file);
				return;
			}

			if (!strncmp(s, "true", 4))
				t->flags |= TRACK_FLAG_LOOPED;
			else if (!strncmp(s, "false", 5))
				t->flags &= ~TRACK_FLAG_LOOPED;
			else
			{
				PRINT_ERROR("Expected true or false for looped variable.\n");
				fclose(file);
				return;
			}
		} else if (!strncmp(s, "point", 5))
		{
			sscanf(s, "point%d", &p);

			fscanf(file, "pos(%f, %f, %f), ", t->points[p].pos + VX, t->points[p].pos + VY, t->points[p].pos + VZ);
			fscanf(file, "tan(%f, %f, %f), ", t->points[p].tan + VX, t->points[p].tan + VY, t->points[p].tan + VZ);
			vec3f_normalize(t->points[p].tan);

			fscanf(file, "angle(%f), ", &t->points[p].angle);
			fscanf(file, "weight(%f), ", &t->points[p].weight);
			fscanf(file, "width(%f), ", &t->points[p].width);
			fscanf(file, "subdivisions(%d), ", &t->points[p].subdivisions);
		}
	}

	fclose(file);
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

static inline float* fillbuffer(float* vptr, mat4f basis, vec3f p0, vec3f p1, vec3f n0, vec3f n1)
{
	vec3f bitan; // temp
	vec3f pos;

	vec3f_set(bitan, 1.f, 0.f, 0.f); // temp

	mat4f_fulltransformvec3fn(pos, p0, basis);
	vec3f_copy(vptr, pos);
	vptr += RENDER_ATTRIBSIZE_POS;

	mat4f_transformvec3fn(vptr, n0, basis);
	vptr += RENDER_ATTRIBSIZE_NOR;

	//vec3f_copy(vptr, basis + C2);
	vec3f_cross(vptr, vptr - RENDER_ATTRIBSIZE_NOR, bitan); // temp
	vec3f_normalize(vptr); // temp
	vptr += RENDER_ATTRIBSIZE_TAN;

	vptr[0] = (pos[VX] + pos[VY])/3.f;
	vptr[1] = (pos[VZ] + pos[VY])/3.f;
	vptr += RENDER_ATTRIBSIZE_TEX;

	mat4f_fulltransformvec3fn(pos, p1, basis);
	vec3f_copy(vptr, pos);
	vptr += RENDER_ATTRIBSIZE_POS;

	mat4f_transformvec3fn(vptr, n1, basis);
	vptr += RENDER_ATTRIBSIZE_NOR;

	//vec3f_copy(vptr, basis + C2);
	vec3f_cross(vptr, vptr - RENDER_ATTRIBSIZE_NOR, bitan); // temp
	vec3f_normalize(vptr); // temp
	vptr += RENDER_ATTRIBSIZE_TAN;

	vptr[0] = (pos[VX] + pos[VY])/3.f;
	vptr[1] = (pos[VZ] + pos[VY])/3.f;
	vptr += RENDER_ATTRIBSIZE_TEX;

	return vptr;
}

static float* addverts(struct track* t, struct track_point* p, float* vptr)
{
	vec3f bin, nor, p0, p1, n0, n1;
	mat4f basis;
	int i;

	// find binormal vector for the track (before rotation)
	vec3f_cross(bin, t->up, p->tan);
	vec3f_normalize(bin);

	// find normal vector (before rotation)
	vec3f_cross(nor, p->tan, bin);

	// construct change of basis and translation transformation matrix
	mat4f_identity(basis);
	vec3f_copy(basis + C0, bin);
	vec3f_copy(basis + C1, nor);
	vec3f_copy(basis + C2, p->tan);
	vec3f_copy(basis + C3, p->pos); // translation

	// rotate the track
	mat4f_rotatezmul(basis, p->angle);

	// TODO: redo this to copy all vertex attributes per segment

	// left side of track
	for (i = TRACK_SEGMENT_VERTCOUNT - 1; i > 0; i--)
	{
		// positions
		vec3f_copy(p0, segment_pos[i]);
		p0[VX] = p0[VX] + p->width*0.5f;
		vec3f_copy(p1, segment_pos[i - 1]);
		p1[VX] = p1[VX] + p->width*0.5f;

		// fill buffer
		vptr = fillbuffer(vptr, basis, p0, p1, segment_nor[i], segment_nor[i]);
	}

	// center of track

	// positions
	vec3f_copy(p0, segment_pos[0]);
	p0[VX] = p0[VX] + p->width*0.5f;
	vec3f_copy(p1, segment_pos[0]);
	p1[VX] = -1.f*(p1[VX] + p->width*0.5f);

	// normals
	vec3f_copy(n0, segment_nor[0]);
	vec3f_copy(n1, segment_nor[0]);
	n1[VX] *= -1.f;

	vptr = fillbuffer(vptr, basis, p0, p1, n0, n1);

	// right side of track
	for (i = 0; i < TRACK_SEGMENT_VERTCOUNT - 1; i++)
	{
		// positions
		vec3f_copy(p0, segment_pos[i]);
		p0[VX] = -1.f*(p0[VX] + p->width*0.5f);
		vec3f_copy(p1, segment_pos[i + 1]);
		p1[VX] = -1.f*(p1[VX] + p->width*0.5f);

		// normals
		vec3f_copy(n0, segment_nor[i + 1]);
		n0[VX] *= -1.f;

		// fill buffer
		vptr = fillbuffer(vptr, basis, p0, p1, n0, n0);
	}

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
	vec3f_copy(vptr, srcptr + RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR + RENDER_ATTRIBSIZE_TAN);
	vptr += RENDER_ATTRIBSIZE_TEX;

	return vptr;
}

void track_generatemesh(struct renderer* r, struct track* t)
{
	struct track_point p;
	unsigned i, j, n, s, ps;
	float* verts;
	float* ptr;
	int offs;
	float d;

	if (t->flags & TRACK_FLAG_LOOPED)
		n = t->num_points;
	else
		n = t->num_points - 1;

	s = 1;
	for (i = 0; i < n; i++)
		s += t->points[i].subdivisions + 1;

	// re-allocate mesh renderable vertex buffer
	// the second parameter here looks complicated but is just calculating the required number of vertices to triangle strip the track
	renderable_allocate(r, &t->r_track, 2 * ((2 * TRACK_SEGMENT_VERTCOUNT - 1)*s + 2 * (TRACK_SEGMENT_VERTCOUNT - 1)));

	// temporary vertex buffer
	verts = (float*)mem_calloc(2 * (2 * TRACK_SEGMENT_VERTCOUNT - 1)*s, RENDER_VERTSIZE_BUMP_L * sizeof(float));

	// place vertex attributes into buffer
	ptr = verts;
	for (i = 0; i < n; i++)
	{
		d = 0.f;

		ps = t->points[i].subdivisions + 1;
		for (j = 0; j < ps; j++)
		{
			// find point on track curve and add vertices to generate track
			curvepoint(t, i, d, &p);
			ptr = addverts(t, &p, ptr);

			d += 1.f / (float)ps;
		}
	}

	// generate the last point on the track
	curvepoint(t, n - 1, 1.f, &p);
	ptr = addverts(t, &p, ptr);

	// copy vector attributes into renderable buffer
	ptr = t->r_track.buf_verts;
	for (i = 0; i < 2 * TRACK_SEGMENT_VERTCOUNT - 1; i++)
	{
		offs = (int)i * 2;

		if (i)
		{
			// repeat points at start and end of track
			ptr = copyvert(ptr, ptr - RENDER_VERTSIZE_BUMP_L);
			ptr = copyvert(ptr, verts + offs*RENDER_VERTSIZE_BUMP_L);
		}

		// loop through each segment
		for (j = 0; j < s; j++)
		{
			// copy points
			ptr = copyvert(ptr, verts + offs*RENDER_VERTSIZE_BUMP_L);
			ptr = copyvert(ptr, verts + (offs + 1)*RENDER_VERTSIZE_BUMP_L);

			offs += 2 * (2 * TRACK_SEGMENT_VERTCOUNT - 1);
		}
	}

	mem_free(verts);
}