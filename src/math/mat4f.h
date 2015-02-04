#ifndef	MAT4F
#define	MAT4F


#include	<math.h>		//	cosf, sinf
#include	"vec3f.h"


#define	MAT4_SIZE	16
#define	MAT4_DIM	4

#define	R0			0
#define	R1			1
#define	R2			2
#define	R3			3
#define	C0			0
#define	C1			4
#define	C2			8
#define	C3			12


typedef float mat4f[16];


static void swap(float* a, float* b)
{
	float t;

	t = *a;
	*a = *b;
	*b = t;
}


/*	copy matrix
	param:	dst			destination matrix (modified)
	param:	src			source matrix
*/
static void mat4f_copy(mat4f dst, mat4f src)
{
	int i;

	for (i = 0; i < MAT4_SIZE; i++)
		dst[i] = src[i];
}


/*	set all matrix elements to 0
	param:	m			matrix to clear (modified)
*/
static void mat4f_clear(mat4f m)
{
	int i;

	for (i = 0; i < MAT4_SIZE; i++)
		m[i] = 0.f;
}

/*	set matrix to the identity
	param:	m			matrix to be set (modified)
*/
static void mat4f_identity(mat4f m)
{
	mat4f_clear(m);
	m[R0 + C0] = m[R1 + C1] = m[R2 + C2] = m[R3 + C3] = 1.f;
}


/*	find transpose of matrix
	param:	m			matrix to find transpose of (modified)
*/
static void mat4f_transpose(mat4f m)
{
	swap(m + R0 + C1, m + R1 + C0);
	swap(m + R0 + C2, m + R2 + C0);
	swap(m + R0 + C3, m + R3 + C0);
	swap(m + R1 + C2, m + R2 + C1);
	swap(m + R1 + C3, m + R3 + C1);
	swap(m + R2 + C3, m + R3 + C2);
}

/*	find transpose of matrix
	param:	res			resultant matrix (modified)
	param:	m			matrix to find transpose of
*/
static void mat4f_transposen(mat4f res, mat4f m)
{
	mat4f_copy(res, m);
	mat4f_transpose(res);
}


/*	multiply matrix by other matrix (new)
	param:	res			resultant matrix (modified)
	param:	m0			matrix multiplicand
	param:	m1			matrix multiplier
*/
static void mat4f_multiplyn(mat4f res, mat4f m0, mat4f m1)
{
	int r, c;

	for (c = 0; c < MAT4_SIZE; c += MAT4_DIM)
		for (r = 0; r < MAT4_DIM; r++)
			res[c + r] =
				m0[r + C0] * m1[c + R0] +
				m0[r + C1] * m1[c + R1] +
				m0[r + C2] * m1[c + R2] +
				m0[r + C3] * m1[c + R3];
}

/*	multiply matrix by other matrix
	param:	m0			matrix multiplicand (modified)
	param:	m1			matrix multiplier
*/
static void mat4f_multiply(mat4f m0, mat4f m1)
{
	mat4f n;

	mat4f_copy(n, m0);
	mat4f_multiplyn(m0, n, m1);
}


/*	set matrix to perspective projection
	param:	m			matrix to be set (modified)
	param:	l			left clipping plane
	param:	r			right clipping plane
	param:	b			bottom clipping plane
	param:	t			top clipping plane
	param:	n			near clipping plane
	param:	f			far clipping plane
*/
static void mat4f_frustum(mat4f m, float l, float r, float b, float t, float n, float f)
{
	m[R0 + C0] = 2.f*n / (r - l);
	m[R0 + C1] = 0.f;
	m[R0 + C2] = (r + l) / (r - l);
	m[R0 + C3] = 0.f;
	m[R1 + C0] = 0.f;
	m[R1 + C1] = 2.f*n / (t - b);
	m[R1 + C2] = (t + b) / (t - b);
	m[R1 + C3] = 0.f;
	m[R2 + C0] = 0.f;
	m[R2 + C1] = 0.f;
	m[R2 + C2] = -(f + n) / (f - n);
	m[R2 + C3] = -(2.f*f*n) / (f - n);
	m[R3 + C0] = 0.f;
	m[R3 + C1] = 0.f;
	m[R3 + C2] = -1.f;
	m[R3 + C3] = 0.f;
}

/*	set matrix to orthographic projection
	param:	m			matrix to be set (modified)
	param:	l			left clipping plane
	param:	r			right clipping plane
	param:	b			bottom clipping plane
	param:	t			top clipping plane
	param:	n			near clipping plane
	param:	f			far clipping plane
*/
static void mat4f_ortho(mat4f m, float l, float r, float b, float t, float n, float f)
{
	m[R0 + C0] = 2.f / (r - l);
	m[R0 + C1] = 0.f;
	m[R0 + C2] = 0.f;
	m[R0 + C3] = -(r + l) / (r - l);
	m[R1 + C0] = 0.f;
	m[R1 + C1] = 2.f / (t - b);
	m[R1 + C2] = 0.f;
	m[R1 + C3] = -(t + b) / (t - b);
	m[R2 + C0] = 0.f;
	m[R2 + C1] = 0.f;
	m[R2 + C2] = -2.f / (f - n);
	m[R2 + C3] = -(f + n) / (f - n);
	m[R3 + C0] = 0.f;
	m[R3 + C1] = 0.f;
	m[R3 + C2] = 0.f;
	m[R3 + C3] = 1.f;
}


/*	set matrix to translation transformation
	param:	m			matrix to be set (modified)
	param:	x			translation in x-axis
	param:	y			translation in y-axis
	param:	z			translation in z-axis
*/
static void mat4f_translate(mat4f m, float x, float y, float z)
{
	mat4f_identity(m);

	m[R0 + C3] = x;
	m[R1 + C3] = y;
	m[R2 + C3] = z;
}

/*	multiply matrix by translation transformation (new)
	param:	res			resultant matrix (modified)
	param:	m			original matrix
	param:	x			translation in x-axis
	param:	y			translation in y-axis
	param:	z			translation in z-axis
*/
static void mat4f_translatemuln(mat4f res, mat4f m, float x, float y, float z)
{
	mat4f n;

	mat4f_translate(n, x, y, z);
	mat4f_multiplyn(res, m, n);
}

/*	multiply matrix by translation transformation
	param:	m			original matrix (modified)
	param:	x			translation in x-axis
	param:	y			translation in y-axis
	param:	z			translation in z-axis
*/
static void mat4f_translatemul(mat4f m, float x, float y, float z)
{
	mat4f n;

	mat4f_translate(n, x, y, z);
	mat4f_multiply(m, n);
}


/*	set matrix to non-uniform scale transformation
	param:	m			matrix to be set (modified)
	param:	x			scale along x-axis
	param:	y			scale along y-axis
	param:	z			scale along z-axis
*/
static void mat4f_scale(mat4f m, float x, float y, float z)
{
	mat4f_clear(m);

	m[R0 + C0] = x;
	m[R1 + C1] = y;
	m[R2 + C2] = z;
	m[R3 + C3] = 1.f;
}

/*	multiply matrix by non-uniform scale transformation (new)
	param:	res			resultant matrix (modified)
	param:	m			original matrix
	param:	x			scale along x-axis
	param:	y			scale along y-axis
	param:	z			scale along z-axis
*/
static void mat4f_scalemuln(mat4f res, mat4f m, float x, float y, float z)
{
	mat4f n;

	mat4f_scale(n, x, y, z);
	mat4f_multiplyn(res, m, n);
}

/*	multiply matrix by non-uniform scale transformation
	param:	m			original matrix (modified)
	param:	x			scale along x-axis
	param:	y			scale along y-axis
	param:	z			scale along z-axis
*/
static void mat4f_scalemul(mat4f m, float x, float y, float z)
{
	mat4f n;

	mat4f_scale(n, x, y, z);
	mat4f_multiply(m, n);
}


/*	set matrix to rotation around x-axis
	param:	m			matrix to be set (modified)
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotatex(mat4f m, float angle)
{
	mat4f_identity(m);

	m[R1 + C1] = cosf(angle);
	m[R1 + C2] = -sinf(angle);
	m[R2 + C1] = sinf(angle);
	m[R2 + C2] = cosf(angle);
}

/*	multiply matrix by rotation around x-axis (new)
	param:	res			resultant matrix (modified)
	param:	m			original matrix
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotatexmuln(mat4f res, mat4f m, float angle)
{
	mat4f n;

	mat4f_rotatex(n, angle);
	mat4f_multiplyn(res, m, n);
}

/*	multiply matrix by rotation around x-axis
	param:	m			original matrix (modified)
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotatexmul(mat4f m, float angle)
{
	mat4f n;

	mat4f_rotatex(n, angle);
	mat4f_multiply(m, n);
}


/*	set matrix to rotation around y-axis
	param:	m			matrix to be set (modified)
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotatey(mat4f m, float angle)
{
	mat4f_identity(m);

	m[R0 + C0] = cosf(angle);
	m[R0 + C2] = sinf(angle);
	m[R2 + C0] = -sinf(angle);
	m[R2 + C2] = cosf(angle);
}

/*	multiply matrix by rotation around y-axis (new)
	param:	res			resultant matrix (modified)
	param:	m			original matrix
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotateymuln(mat4f res, mat4f m, float angle)
{
	mat4f n;

	mat4f_rotatey(n, angle);
	mat4f_multiplyn(res, m, n);
}

/*	multiply matrix by rotation around y-axis
	param:	m			original matrix (modified)
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotateymul(mat4f m, float angle)
{
	mat4f n;

	mat4f_rotatey(n, angle);
	mat4f_multiply(m, n);
}


/*	set matrix to rotation around z-axis
	param:	m			matrix to be set (modified)
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotatez(mat4f m, float angle)
{
	mat4f_identity(m);

	m[R0 + C0] = cosf(angle);
	m[R0 + C1] = -sinf(angle);
	m[R1 + C0] = sinf(angle);
	m[R1 + C1] = cosf(angle);
}

/*	multiply matrix by rotation around z-axis (new)
	param:	res			resultant matrix (modified)
	param:	m			original matrix
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotatezmuln(mat4f res, mat4f m, float angle)
{
	mat4f n;

	mat4f_rotatez(n, angle);
	mat4f_multiplyn(res, m, n);
}

/*	multiply matrix by rotation around z-axis
	param:	m			original matrix (modified)
	param:	angle		angle of rotation in radians
*/
static void mat4f_rotatezmul(mat4f m, float angle)
{
	mat4f n;

	mat4f_rotatez(n, angle);
	mat4f_multiply(m, n);
}


/*	find the inverse of a matrix (new)
	param:	res			resultant matrix (modified) (assumed invertible)
	param:	m			original matrix
*/
static void mat4f_invertn(mat4f res, mat4f m)
{
	float det;
	int i;

	// ugly math; taken from GLU's inverse implementation
	res[R0 + C0] =
		m[R1 + C1] * m[R2 + C2] * m[R3 + C3] -
		m[R1 + C1] * m[R3 + C2] * m[R2 + C3] -
		m[R1 + C2] * m[R2 + C1] * m[R3 + C3] +
		m[R1 + C2] * m[R3 + C1] * m[R2 + C3] +
		m[R1 + C3] * m[R2 + C1] * m[R3 + C2] -
		m[R1 + C3] * m[R3 + C1] * m[R2 + C2];

	res[R0 + C1] =
		-m[R0 + C1] * m[R2 + C2] * m[R3 + C3] +
		m[R0 + C1] * m[R3 + C2] * m[R2 + C3] +
		m[R0 + C2] * m[R2 + C1] * m[R3 + C3] -
		m[R0 + C2] * m[R3 + C1] * m[R2 + C3] -
		m[R0 + C3] * m[R2 + C1] * m[R3 + C2] +
		m[R0 + C3] * m[R3 + C1] * m[R2 + C2];

	res[R0 + C2] =
		m[R0 + C1] * m[R1 + C2] * m[R3 + C3] -
		m[R0 + C1] * m[R3 + C2] * m[R1 + C3] -
		m[R0 + C2] * m[R1 + C1] * m[R3 + C3] +
		m[R0 + C2] * m[R3 + C1] * m[R1 + C3] +
		m[R0 + C3] * m[R1 + C1] * m[R3 + C2] -
		m[R0 + C3] * m[R3 + C1] * m[R1 + C2];

	res[R0 + C3] =
		-m[R0 + C1] * m[R1 + C2] * m[R2 + C3] +
		m[R0 + C1] * m[R2 + C2] * m[R1 + C3] +
		m[R0 + C2] * m[R1 + C1] * m[R2 + C3] -
		m[R0 + C2] * m[R2 + C1] * m[R1 + C3] -
		m[R0 + C3] * m[R1 + C1] * m[R2 + C2] +
		m[R0 + C3] * m[R2 + C1] * m[R1 + C2];

	res[R1 + C0] =
		-m[R1 + C0] * m[R2 + C2] * m[R3 + C3] +
		m[R1 + C0] * m[R3 + C2] * m[R2 + C3] +
		m[R1 + C2] * m[R2 + C0] * m[R3 + C3] -
		m[R1 + C2] * m[R3 + C0] * m[R2 + C3] -
		m[R1 + C3] * m[R2 + C0] * m[R3 + C2] +
		m[R1 + C3] * m[R3 + C0] * m[R2 + C2];

	res[R1 + C1] =
		m[R0 + C0] * m[R2 + C2] * m[R3 + C3] -
		m[R0 + C0] * m[R3 + C2] * m[R2 + C3] -
		m[R0 + C2] * m[R2 + C0] * m[R3 + C3] +
		m[R0 + C2] * m[R3 + C0] * m[R2 + C3] +
		m[R0 + C3] * m[R2 + C0] * m[R3 + C2] -
		m[R0 + C3] * m[R3 + C0] * m[R2 + C2];

	res[R1 + C2] =
		-m[R0 + C0] * m[R1 + C2] * m[R3 + C3] +
		m[R0 + C0] * m[R3 + C2] * m[R1 + C3] +
		m[R0 + C2] * m[R1 + C0] * m[R3 + C3] -
		m[R0 + C2] * m[R3 + C0] * m[R1 + C3] -
		m[R0 + C3] * m[R1 + C0] * m[R3 + C2] +
		m[R0 + C3] * m[R3 + C0] * m[R1 + C2];

	res[R1 + C3] =
		m[R0 + C0] * m[R1 + C2] * m[R2 + C3] -
		m[R0 + C0] * m[R2 + C2] * m[R1 + C3] -
		m[R0 + C2] * m[R1 + C0] * m[R2 + C3] +
		m[R0 + C2] * m[R2 + C0] * m[R1 + C3] +
		m[R0 + C3] * m[R1 + C0] * m[R2 + C2] -
		m[R0 + C3] * m[R2 + C0] * m[R1 + C2];

	res[R2 + C0] =
		m[R1 + C0] * m[R2 + C1] * m[R3 + C3] -
		m[R1 + C0] * m[R3 + C1] * m[R2 + C3] -
		m[R1 + C1] * m[R2 + C0] * m[R3 + C3] +
		m[R1 + C1] * m[R3 + C0] * m[R2 + C3] +
		m[R1 + C3] * m[R2 + C0] * m[R3 + C1] -
		m[R1 + C3] * m[R3 + C0] * m[R2 + C1];

	res[R2 + C1] =
		-m[R0 + C0] * m[R2 + C1] * m[R3 + C3] +
		m[R0 + C0] * m[R3 + C1] * m[R2 + C3] +
		m[R0 + C1] * m[R2 + C0] * m[R3 + C3] -
		m[R0 + C1] * m[R3 + C0] * m[R2 + C3] -
		m[R0 + C3] * m[R2 + C0] * m[R3 + C1] +
		m[R0 + C3] * m[R3 + C0] * m[R2 + C1];

	res[R2 + C2] =
		m[R0 + C0] * m[R1 + C1] * m[R3 + C3] -
		m[R0 + C0] * m[R3 + C1] * m[R1 + C3] -
		m[R0 + C1] * m[R1 + C0] * m[R3 + C3] +
		m[R0 + C1] * m[R3 + C0] * m[R1 + C3] +
		m[R0 + C3] * m[R1 + C0] * m[R3 + C1] -
		m[R0 + C3] * m[R3 + C0] * m[R1 + C1];

	res[R2 + C3] =
		-m[R0 + C0] * m[R1 + C1] * m[R2 + C3] +
		m[R0 + C0] * m[R2 + C1] * m[R1 + C3] +
		m[R0 + C1] * m[R1 + C0] * m[R2 + C3] -
		m[R0 + C1] * m[R2 + C0] * m[R1 + C3] -
		m[R0 + C3] * m[R1 + C0] * m[R2 + C1] +
		m[R0 + C3] * m[R2 + C0] * m[R1 + C1];

	res[R3 + C0] =
		-m[R1 + C0] * m[R2 + C1] * m[R3 + C2] +
		m[R1 + C0] * m[R3 + C1] * m[R2 + C2] +
		m[R1 + C1] * m[R2 + C0] * m[R3 + C2] -
		m[R1 + C1] * m[R3 + C0] * m[R2 + C2] -
		m[R1 + C2] * m[R2 + C0] * m[R3 + C1] +
		m[R1 + C2] * m[R3 + C0] * m[R2 + C1];

	res[R3 + C1] =
		m[R0 + C0] * m[R2 + C1] * m[R3 + C2] -
		m[R0 + C0] * m[R3 + C1] * m[R2 + C2] -
		m[R0 + C1] * m[R2 + C0] * m[R3 + C2] +
		m[R0 + C1] * m[R3 + C0] * m[R2 + C2] +
		m[R0 + C2] * m[R2 + C0] * m[R3 + C1] -
		m[R0 + C2] * m[R3 + C0] * m[R2 + C1];

	res[R3 + C2] =
		-m[R0 + C0] * m[R1 + C1] * m[R3 + C2] +
		m[R0 + C0] * m[R3 + C1] * m[R1 + C2] +
		m[R0 + C1] * m[R1 + C0] * m[R3 + C2] -
		m[R0 + C1] * m[R3 + C0] * m[R1 + C2] -
		m[R0 + C2] * m[R1 + C0] * m[R3 + C1] +
		m[R0 + C2] * m[R3 + C0] * m[R1 + C1];

	res[R3 + C3] =
		m[R0 + C0] * m[R1 + C1] * m[R2 + C2] -
		m[R0 + C0] * m[R2 + C1] * m[R1 + C2] -
		m[R0 + C1] * m[R1 + C0] * m[R2 + C2] +
		m[R0 + C1] * m[R2 + C0] * m[R1 + C2] +
		m[R0 + C2] * m[R1 + C0] * m[R2 + C1] -
		m[R0 + C2] * m[R2 + C0] * m[R1 + C1];

	det = 1.f / (m[R0 + C0] * res[R0 + C0] + m[R1 + C0] * res[R0 + C1] + m[R2 + C0] * res[R0 + C2] + m[R3 + C0] * res[R0 + C3]);

	for (i = 0; i < MAT4_SIZE; i++)
		res[i] *= det;
}

/*	Find the inverse of a matrix
	param:	m			original matrix (modified) (assumed invertible)
*/
static void mat4f_invert(mat4f m)
{
	mat4f n;

	mat4f_invertn(n, m);
	mat4f_copy(m, n);
}


/*	transform a vector by a matrix with translation (new)
	param:	res			resultant vector (modified)
	param:	v			vector to be transformed
	param:	m			transformation matrix
*/
static void mat4f_fulltransformvec3fn(vec3f res, vec3f v, mat4f m)
{
	res[VX] = m[R0 + C0] * v[VX] + m[R0 + C1] * v[VY] + m[R0 + C2] * v[VZ] + m[R0 + C3];
	res[VY] = m[R1 + C0] * v[VX] + m[R1 + C1] * v[VY] + m[R1 + C2] * v[VZ] + m[R1 + C3];
	res[VZ] = m[R2 + C0] * v[VX] + m[R2 + C1] * v[VY] + m[R2 + C2] * v[VZ] + m[R2 + C3];
}

/*	transform a vector by a matrix with translation
	param:	v			vector to be transformed (modified)
	param:	m			transformation matrix
*/
static void mat4f_fulltransformvec3f(vec3f v, mat4f m)
{
	vec3f n;

	mat4f_fulltransformvec3fn(n, v, m);
	vec3f_copy(v, n);
}

/*	transform a vector by a matrix without translation (new)
	param:	res			resultant vector (modified)
	param:	v			vector to be transformed
	param:	m			transformation matrix
*/
static void mat4f_transformvec3fn(vec3f res, vec3f v, mat4f m)
{
	res[VX] = m[R0 + C0] * v[VX] + m[R0 + C1] * v[VY] + m[R0 + C2] * v[VZ];
	res[VY] = m[R1 + C0] * v[VX] + m[R1 + C1] * v[VY] + m[R1 + C2] * v[VZ];
	res[VZ] = m[R2 + C0] * v[VX] + m[R2 + C1] * v[VY] + m[R2 + C2] * v[VZ];
}

/*	transform a vector by a matrix without translation
	param:	v			vector to be transformed (modified)
	param:	m			transformation matrix
*/
static void mat4f_transformvec3f(vec3f v, mat4f m)
{
	vec3f n;

	mat4f_transformvec3fn(n, v, m);
	vec3f_copy(v, n);
}


#endif