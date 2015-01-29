#ifndef	VEC3F
#define	VEC3F


#include	<math.h>						// sqrtf


#define	X	0
#define	Y	1
#define	Z	2


typedef float vec3f[3];


/*	set vector values
	param:	v			vector to set (modified)
	param:	x			x value
	param:	y			y value
	param:	z			z value
*/
static void vec3f_set(vec3f v, float x, float y, float z)
{
	v[X] = x;
	v[Y] = y;
	v[Z] = z;
}

/*	copy vector
	param:	dst			destination vector (modified)
	param:	src			source vector
*/
static void vec3f_copy(vec3f dst, vec3f src)
{
	dst[X] = src[X];
	dst[Y] = src[Y];
	dst[Z] = src[Z];
}


/*	add vector to other vector (new)
	param:	res			resultant vector (modified)
	param:	v0			vector augend
	param:	v1			vector addend
*/
static void vec3f_addn(vec3f res, vec3f v0, vec3f v1)
{
	res[X] = v0[X] + v1[X];
	res[Y] = v0[Y] + v1[Y];
	res[Z] = v0[Z] + v1[Z];
}

/*	add vector to other vector
	param:	v0			vector augend (modified)
	param:	v1			vector addend
*/
static void vec3f_add(vec3f v0, vec3f v1)
{
	vec3f_addn(v0, v0, v1);
}

/*	subtract vector from other vector (new)
	param:	res			resultant vector (modified)
	param:	v0			vector minuend
	param:	v1			vector subtrahend
*/
static void vec3f_subtractn(vec3f res, vec3f v0, vec3f v1)
{
	res[X] = v0[X] - v1[X];
	res[Y] = v0[Y] - v1[Y];
	res[Z] = v0[Z] - v1[Z];
}

/*	subtract vector from other vector
	param:	v0			vector minuend (modified)
	param:	v1			vector subtrahend
*/
static void vec3f_subtract(vec3f v0, vec3f v1)
{
	vec3f_subtractn(v0, v0, v1);
}


/*	vector dot product
	param:	v0			first vector
	param:	v1			second vector
	return:	float		dot product of the two input vectors
*/
static float vec3f_dot(vec3f v0, vec3f v1)
{
	return v0[X] * v1[X] + v0[Y] * v1[Y] + v0[Z] * v1[Z];
}

/*	find squared length of vector
	param:	v			vector to find squared length of
	return:	float		squared length of input vector
*/
static float vec3f_length2(vec3f v)
{
	return vec3f_dot(v, v);
}

/*	find length of vector
	param:	v			vector to find length of
	return:	float		length of input vector
*/
static float vec3f_length(vec3f v)
{
	return sqrtf(vec3f_length2(v));
}


/*	scale vector by a scalar (new)
	param:	res			resultant vector (modified)
	param:	v			vector to scale
	param:	s			scalar
*/
static void vec3f_scalen(vec3f res, vec3f v, float s)
{
	res[X] = v[X] * s;
	res[Y] = v[Y] * s;
	res[Z] = v[Z] * s;
}

/*	scale vector by a scalar
	param:	v			vector to scale (modified)
	param:	s			scalar
*/
static void vec3f_scale(vec3f v, float s)
{
	vec3f_scalen(v, v, s);
}


/*	normalize vector (new)
	param:	res			resultant vector (modified)
	param:	v			vector to normalize (assumed length > 0)
*/
static void vec3f_normalizen(vec3f res, vec3f v)
{
	vec3f_scalen(res, v, 1.f / vec3f_length(v));
}

/*	normalize vector
	param:	v			vector to normalize (modified) (assumed length > 0)
*/
static void vec3f_normalize(vec3f v)
{
	vec3f_normalizen(v, v);
}


/*	negate vector (new)
	param:	res			resultant vector (modified)
	param:	v			vector to negate
*/
static void vec3f_negaten(vec3f res, vec3f v)
{
	res[X] = -v[X];
	res[Y] = -v[Y];
	res[Z] = -v[Z];
}

/*	negate vector
	param:	v			vector to negate (modified)
*/
static void vec3f_negate(vec3f v)
{
	vec3f_negaten(v, v);
}


/*	vector cross product
	param:	res			resultant vector (modified)
	param:	v0			first vector
	param:	v1			second vector
*/
static void vec3f_cross(vec3f res, vec3f v0, vec3f v1)
{
	res[X] = v0[Y] * v1[Z] - v0[Z] * v1[Y];
	res[Y] = v0[Z] * v1[X] - v0[X] * v1[Z];
	res[Z] = v0[X] * v1[Y] - v0[Y] * v1[X];
}


/*	project vector onto unit vector (new)
	param:	res			resultant vector (modified)
	param:	v0			vector to project
	param:	v1			axis of projection (assumed to be unit length)
*/
static void vec3f_projectn(vec3f res, vec3f v0, vec3f v1)
{
	vec3f_scalen(res, v1, vec3f_dot(v0, v1));
}

/*	project vector onto unit vector
	param:	v0			vector to project (modified)
	param:	v1			axis of projection (assumed to be unit length)
*/
static void vec3f_project(vec3f v0, vec3f v1)
{
	vec3f_projectn(v0, v0, v1);
}


/*	reflect vector around unit vector (new)
	param:	res			resultant vector (modified)
	param:	v0			vector to reflect
	param:	v1			axis of reflection (assumed to be unit length)
*/
static void vec3f_reflectn(vec3f res, vec3f v0, vec3f v1)
{
	float a = 2.f*vec3f_dot(v0, v1);

	res[X] = v1[X] * a - v0[X];
	res[Y] = v1[Y] * a - v0[Y];
	res[Z] = v1[Z] * a - v0[Z];
}

/*	reflect vector around unit vector
	param:	v0			vector to reflect (modified)
	param:	v1			axis of reflection (assumed to be unit length)
*/
static void vec3f_reflect(vec3f v0, vec3f v1)
{
	vec3f_reflectn(v0, v0, v1);
}


#endif