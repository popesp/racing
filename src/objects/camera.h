#ifndef CAMERA
#define	CAMERA

#include	"../math/mat4f.h"
#include	"../math/vec3f.h"


struct camera
{
	vec3f pos;

	vec3f dir;
	vec3f up;
	vec3f right;
};


void camera_init(struct camera* cam, vec3f pos, vec3f look, vec3f up);

void camera_forward(struct camera* cam, float d);
void camera_vertical(struct camera* cam, float d);
void camera_strafe(struct camera* cam, float d);

void camera_rotate(struct camera* cam, vec3f axis, float a);
void camera_lookat(struct camera* cam, vec3f lookat, vec3f up);

void camera_gettransform(struct camera* cam, mat4f transform);


#endif
