#ifndef CAMERA
#define	CAMERA

#include	"../math/mat4f.h"
#include	"../math/vec3f.h"


#define	CAMERA_XROT_MAX	(M_PI * 0.9f)
#define	CAMERA_XROT_MIN	-(M_PI * 0.9f)


struct lookcamera
{
	vec3f pos;
	vec3f look;
	vec3f up;
};

struct freecamera
{
	vec3f pos;
	vec3f dir;
	vec3f up;

	float xrot;
	float yrot;
};


void lookcamera_init(struct lookcamera* cam, vec3f pos, vec3f look, vec3f up);
void freecamera_init(struct freecamera* cam, vec3f pos, float xrot, float yrot, vec3f up);
void freecamera_initdir(struct freecamera* cam, vec3f pos, vec3f dir, vec3f up);
void freecamera_initlook(struct freecamera* cam, vec3f pos, vec3f look, vec3f up);

void lookcamera_update(struct lookcamera* cam);
void freecamera_update(struct freecamera* cam);

void lookcamera_gettransform(struct lookcamera* cam, mat4f transform);
void freecamera_gettransform(struct freecamera* cam, mat4f transform);

void freecamera_lookat(struct freecamera* cam, vec3f look);


#endif
