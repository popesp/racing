#include	"camera.h"

#define	_USE_MATH_DEFINES
#include	<math.h>
#include	"../math/mat4f.h"
#include	"../math/vec3f.h"


void lookcamera_init(struct lookcamera* cam, vec3f pos, vec3f look, vec3f up)
{
	vec3f_copy(cam->pos, pos);
	vec3f_copy(cam->look, look);
	vec3f_copy(cam->up, up);
}

/*	Initialize a free moving camera (dir and up assumed to be orthogonal and unit length)
*/
void freecamera_init(struct freecamera* cam, vec3f pos, vec3f dir, float xrot, float yrot, vec3f up)
{
	vec3f_copy(cam->pos, pos);
	vec3f_copy(cam->dir, dir);

	vec3f_copy(cam->up, up);

	cam->xrot = xrot;
	cam->yrot = yrot;
}

void freecamera_initdir(struct freecamera* cam, vec3f pos, vec3f dir, vec3f up)
{
	vec3f_copy(cam->pos, pos);
	vec3f_copy(cam->dir, dir);
	vec3f_copy(cam->up, up);
}

void freecamera_initlook(struct freecamera* cam, vec3f pos, vec3f look, vec3f up)
{
	vec3f_copy(cam->pos, pos);

	// find direction from look position
	vec3f_subtractn(cam->dir, look, pos);
	vec3f_normalize(cam->dir);

	vec3f_copy(cam->up, up);
}


void lookcamera_update(struct lookcamera* cam)
{

}

void freecamera_update(struct freecamera* cam)
{
	if (cam->xrot > CAMERA_XROT_MAX)
		cam->xrot = CAMERA_XROT_MAX;
	else if (cam->xrot < CAMERA_XROT_MIN)
		cam->xrot = CAMERA_XROT_MIN;

	while (cam->yrot > M_PI)
		cam->yrot -= M_PI;
	while (cam->yrot < 0.f)
		cam->yrot += M_PI;
}


void lookcamera_gettransform(struct lookcamera* cam, mat4f transform)
{
	float len, xrot, yrot;
	vec3f dir;

	mat4f_identity(transform);

	// find negated direction vector
	vec3f_subtractn(dir, cam->pos, cam->look);
	vec3f_normalize(dir);

	vec3f_cross(


	// calculate rotation angles for camera direction vector
	xrot = asinf(dir[VY]);
	yrot = atan2f(dir[VX], -dir[VZ]);

	// z-axis translation (distance from look-at position)
	mat4f_translate(transform, 0.f, 0.f, -len);
	
	// camera rotation
	mat4f_rotatexmul(transform, xrot);
	mat4f_rotateymul(transform, yrot);

	// initial translation (to look-at position)
	mat4f_translatemul(transform, -cam->look[VX], -cam->look[VY], -cam->look[VZ]);
}

void freecamera_gettransform(struct freecamera* cam, mat4f transform)
{
	// calculate rotation angles for camera direction vector
	xrot = asinf(cam->dir[VY]);
	yrot = atan2f(cam->dir[VX], -cam->dir[VZ]);

	// camera rotation
	mat4f_rotatexmul(transform, xrot);
	mat4f_rotateymul(transform, yrot);

	// translate to camera pos
	mat4f_translatemul(transform, -cam->pos[VX], -cam->pos[VY], -cam->pos[VZ]);
}


void freecamera_lookat(struct freecamera* cam, vec3f look)
{

}