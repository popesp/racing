#ifndef	DEBUG_H
#define	DEBUG_H


#include	<stdio.h>
#include	"math/mat4f.h"
#include	"math/vec3f.h"


/*	print a vector to console
	param:	v				vector to print
*/
static inline void debug_printvec3f(vec3f v)
{
	printf("X:%f, Y:%f, Z:%f\n", v[VX], v[VY], v[VZ]);
}

/*	print a matrix to console
	param:	m				matrix to print
*/
static inline void debug_printmat4f(mat4f m)
{
	int r, c;

	for (c = 0; c < MAT4_SIZE; c += MAT4_DIM)
	{
		for (r = 0; r < MAT4_DIM; r++)
			printf("%f, ", m[c + r]);
		printf("\n");
	}
}


#endif