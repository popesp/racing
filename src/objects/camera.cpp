#include	"camera.h"

#define	_USE_MATH_DEFINES
#include	<math.h>
#include	"../math/mat4f.h"
#include	"../math/vec3f.h"


/*	initialize a "look" camera object
	param:	cam			"look" camera to initialize (modified)
	param:	pos			initial camera position
	param:	up			up vector (assumed unit length)
	param:	lookat		vector to 
*/
void lookcamera_init(struct lookcamera* cam, vec3f pos, vec3f lookat, vec3f up)
{
	vec3f_copy(cam->pos, pos);

	vec3f_copy(cam->lookat, lookat);
	vec3f_copy(cam->up, up);

	// TODO
}

/*	initialize a "free" camera object
	param:	cam			"free" camera to initialize (modified)
	param:	pos			initial camera position
	param:	dir			direction vector (assumed unit-length and orthogonal to up)
	param:	up			up vector (assumed unit length and orthogonal to dir)
*/
void freecamera_init(struct freecamera* cam, vec3f pos, vec3f dir, vec3f up)
{
	vec3f_copy(cam->pos, pos);

	vec3f_copy(cam->dir, dir);
	vec3f_copy(cam->up, up);
	vec3f_cross(cam->right, dir, up);
}


/*	move a "free" camera along its dir vector
	param:	cam			"free" camera to move (modified)
	param:	d			distance to move
*/
void freecamera_forward(struct freecamera* cam, float d)
{
	vec3f dv;

	vec3f_scalen(dv, cam->dir, d);
	vec3f_add(cam->pos, dv);
}

/*	move a "free" camera along its up vector
	param:	cam			"free" camera to move (modified)
	param:	d			distance to move
*/
void freecamera_vertical(struct freecamera* cam, float d)
{
	vec3f dv;

	vec3f_scalen(dv, cam->up, d);
	vec3f_add(cam->pos, dv);
}

/*	move a "free" camera along its right vector
	param:	cam			"free" camera to move (modified)
	param:	d			distance to move
*/
void freecamera_strafe(struct freecamera* cam, float d)
{
	vec3f dv;

	vec3f_scalen(dv, cam->right, d);
	vec3f_add(cam->pos, dv);
}


/*	rotate the orientation of a "free" camera around an axis
	param:	cam			"free" camera to rotate (modified)
	param:	axis		axis of rotation (assumed to be unequal to all camera basis vectors, unless it is in fact one of the basis vectors eg. freecamera_rotate(cam, cam->dir, 0.2f))
	param:	a			angle of rotation
*/
void freecamera_rotate(struct freecamera* cam, vec3f axis, float a)
{
	if (axis != cam->dir)
		vec3f_rotate(cam->dir, axis, a);
	if (axis != cam->up)
		vec3f_rotate(cam->up, axis, a);
	if (axis != cam->right)
		vec3f_rotate(cam->right, axis, a);
}


/*	get the world->camera space transformation matrix for a "look" camera
	param:	cam			"look" camera to get transformation for
	param:	transform	matrix to fill (modified)
*/
void lookcamera_gettransform(struct lookcamera* cam, mat4f transform)
{
	// TODO
	(void)cam;
	(void)transform;
}

/*	get the world->camera space transformation matrix for a "free" camera
	param:	cam			"free" camera to get transformation for
	param:	transform	matrix to fill (modified)
*/
void freecamera_gettransform(struct freecamera* cam, mat4f transform)
{
	// construct change of basis for world space into camera space
	transform[R0 + C0] = cam->right[VX];	transform[R0 + C1] = cam->right[VY];	transform[R0 + C2] = cam->right[VZ];	transform[R0 + C3] = 0.f;
	transform[R1 + C0] = cam->up[VX];		transform[R1 + C1] = cam->up[VY];		transform[R1 + C2] = cam->up[VZ];		transform[R1 + C3] = 0.f;
	transform[R2 + C0] = -cam->dir[VX];		transform[R2 + C1] = -cam->dir[VY];		transform[R2 + C2] = -cam->dir[VZ];		transform[R2 + C3] = 0.f;
	transform[R3 + C0] = 0.f;				transform[R3 + C1] = 0.f;				transform[R3 + C2] = 0.f;				transform[R3 + C3] = 1.f;

	// translate to camera position first
	mat4f_translatemul(transform, -cam->pos[VX], -cam->pos[VY], -cam->pos[VZ]);
}