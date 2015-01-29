#ifndef	MEM
#define	MEM


#include	<stdlib.h>


#define	mem_alloc(size) malloc(size)
#define	mem_calloc(num, size) calloc(num, size)
#define	mem_realloc(ptr, size) realloc(ptr, size)
#define	mem_free(ptr) free(ptr)


#endif