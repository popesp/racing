#include	"primitives.h"

#include	<math.h>

/*
static void spherepos(vec3f pos, float theta, float phi)
{
	float r;

	r = sinf(theta);

	vec3f_set(pos, r*sinf(phi), cosf(theta), r*cosf(phi));
}

static void spherevertex(float** ptr, unsigned vertsize, float r_theta, float r_phi)
{
	spherepos(*ptr, r_theta*(float)M_PI, r_phi*(float)M_PI);
	*ptr += 
}

void primitives_generatesphere(struct renderable* obj, struct renderer* r, float radius, unsigned num_theta, unsigned num_phi)
{
	unsigned p;
	float r_theta;
	float* ptr;

	renderable_allocate(r, obj, num_theta*num_phi*6);

	ptr = obj->buf_verts;

	// bottom cap
	r_theta = (float)num_theta / (float)(num_theta + 1);
	for (p = 0; p < num_phi; p++)
	{
		spherevertex(&ptr, r->vertsize[obj->type], 1.f, ((float)p + 0.5f) / (float)num_phi);
		spherevertex(&ptr, r->vertsize[obj->type], r_theta, (float)(p+1) / (float)num_phi);
		spherevertex(&ptr, r->vertsize[obj->type], r_theta, (float)p / (float)num_phi);
	}
}
*/