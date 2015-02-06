#include	"camera.h"

#define	_USE_MATH_DEFINES
#include	<math.h>
#include	"../math/mat4f.h"
#include	"../math/vec3f.h"


/*	initialize a camera object
	param:	cam			camera to initialize (modified)
	param:	pos			initial camera position
	param:	lookat		position to look at
	param:	up			up vector (assumed to be unit length and non-parallel to the look direction)
*/
void camera_init(struct camera* cam, vec3f pos, vec3f lookat, vec3f up)
{
	vec3f_copy(cam->pos, pos);

	camera_lookat(cam, lookat, up);
}


/*	move a camera along its dir vector
	param:	cam			camera to move (modified)
	param:	d			distance to move
*/
void camera_forward(struct camera* cam, float d)
{
	vec3f dv;

	vec3f_scalen(dv, cam->dir, d);
	vec3f_add(cam->pos, dv);
}

/*	move a camera along its up vector
	param:	cam			camera to move (modified)
	param:	d			distance to move
*/
void camera_vertical(struct camera* cam, float d)
{
	vec3f dv;

	vec3f_scalen(dv, cam->up, d);
	vec3f_add(cam->pos, dv);
}

/*	move a camera along its right vector
	param:	cam			camera to move (modified)
	param:	d			distance to move
*/
void camera_strafe(struct camera* cam, float d)
{
	vec3f dv;

	vec3f_scalen(dv, cam->right, d);
	vec3f_add(cam->pos, dv);
}


/*	rotate the orientation of a camera around an axis
	param:	cam			camera to rotate (modified)
	param:	axis		axis of rotation
	param:	a			angle of rotation
*/
void camera_rotate(struct camera* cam, vec3f axis, float a)
{
	if (axis != cam->dir)
		vec3f_rotate(cam->dir, axis, a);
	if (axis != cam->up)
		vec3f_rotate(cam->up, axis, a);
	if (axis != cam->right)
		vec3f_rotate(cam->right, axis, a);
}

/*	change a camera's orientation to look at a specific position
	param:	cam			camera whose orientation to change (modified)
	param:	lookat		position to look at
	param:	up			up vector
*/
void camera_lookat(struct camera* cam, vec3f lookat, vec3f up)
{
	vec3f_subtractn(cam->dir, lookat, cam->pos);
	vec3f_normalize(cam->dir);

	vec3f_cross(cam->right, cam->dir, up);
	vec3f_normalize(cam->right);

	vec3f_cross(cam->up, cam->right, cam->dir);
}


/*	get the world->camera space transformation matrix for a camera
	param:	cam			camera to get transformation for
	param:	transform	matrix to fill (modified)
*/
void camera_gettransform(struct camera* cam, mat4f transform)
{
	// construct change of basis for world space into camera space
	transform[R0 + C0] = cam->right[VX];	transform[R0 + C1] = cam->right[VY];	transform[R0 + C2] = cam->right[VZ];	transform[R0 + C3] = 0.f;
	transform[R1 + C0] = cam->up[VX];		transform[R1 + C1] = cam->up[VY];		transform[R1 + C2] = cam->up[VZ];		transform[R1 + C3] = 0.f;
	transform[R2 + C0] = -cam->dir[VX];		transform[R2 + C1] = -cam->dir[VY];		transform[R2 + C2] = -cam->dir[VZ];		transform[R2 + C3] = 0.f;
	transform[R3 + C0] = 0.f;				transform[R3 + C1] = 0.f;				transform[R3 + C2] = 0.f;				transform[R3 + C3] = 1.f;

	// translate to camera position first
	mat4f_translatemul(transform, -cam->pos[VX], -cam->pos[VY], -cam->pos[VZ]);
}