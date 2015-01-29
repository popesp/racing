#ifndef	ERROR
#define	ERROR


#include	"stdio.h"


#define	PRINT_ERROR(file, ...)	printf("Error (%s): " file, __VA_ARGS__)


#endif