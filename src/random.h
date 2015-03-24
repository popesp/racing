#ifndef RANDOM
#define	RANDOM


#include	<stdlib.h>
#include	<time.h>


static void random_timeseed()
{
	srand((unsigned)time(NULL));
}

static int random_int(int range)
{
	return rand() % range;
}


#endif
