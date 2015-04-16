#ifndef RACE
#define	RACE

#include "game.h"


void checkplace(struct game* game);

void checkwin(struct game* game);


void check_ai_vs_other(struct vehicle* vh1,struct vehicle* vh2,int num_aiplayers);


#endif