#ifndef CAMERA
#define	CAMERA

#include	"../math/mat4f.h"
#include	"../math/vec3f.h"


#define	CAMERA_XROT_MAX	(M_PI * 0.9f)
#define	CAMERA_XROT_MIN	-(M_PI * 0.9f)


struct lookcamera
{
	vec3f pos;

	vec3f lookat;
	vec3f up;
};

struct freecamera
{
	vec3f pos;

	vec3f dir;
	vec3f up;
	vec3f right;
};


void lookcamera_init(struct lookcamera* cam, vec3f pos, vec3f look, vec3f up);
void freecamera_init(struct freecamera* cam, vec3f pos, vec3f dir, vec3f up);

void freecamera_forward(struct freecamera* cam, float d);
void freecamera_vertical(struct freecamera* cam, float d);
void freecamera_strafe(struct freecamera* cam, float d);

void freecamera_rotate(struct freecamera* cam, vec3f axis, float a);

void lookcamera_gettransform(struct lookcamera* cam, mat4f transform);
void freecamera_gettransform(struct freecamera* cam, mat4f transform);


#endif
