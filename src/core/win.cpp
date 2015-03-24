#include	"win.h"
#include	"ui.h"

void checkwin(struct game* game)
{
	

	unsigned int cp1 = game->track.num_pathpoints / 3;
	unsigned int cp2 = game->track.num_pathpoints / 2;

	if(game->player.vehicle->lap==GAME_WINCONDITION_LAPS){
		printf("Player has won the game!\nGame's over\n\n");
		//reset laps
		//game->player.vehicle->lap=1;
		for(int i=0; i<game->num_aiplayers-1;i++){
			game->aiplayers[i].vehicle->lap=1;
		}
		
		game->flags &= ~GAME_FLAG_WINCONDITION;
	}


	for(int i=0;i<=game->num_aiplayers-1;i++){
		//printf("%d\n", game->aiplayers[i].vehicle->lap);
		if(game->aiplayers[i].vehicle->lap==GAME_WINCONDITION_LAPS){
			printf("Computer-%d has won the game!\nGame's over\n\n", i);
			
			//reset laps
			game->player.vehicle->lap=1;
			for(int j=0; j<=game->num_aiplayers-1;j++){
				if (i!=j)
					game->aiplayers[j].vehicle->lap=1;
			}
			
			game->flags &= ~GAME_FLAG_WINCONDITION;
		}
	}

	//player win logic
	if(game->player.vehicle->checkpoint1==false && (unsigned)game->player.vehicle->index_track==cp1){
		game->player.vehicle->checkpoint1=true;
	}

	if(game->player.vehicle->checkpoint2==false && game->player.vehicle->checkpoint1==true && (unsigned)game->player.vehicle->index_track==cp2){
		game->player.vehicle->checkpoint2=true;
	}

	if(game->player.vehicle->checkpoint2==true && (unsigned)game->player.vehicle->index_track==cp1){
		game->player.vehicle->checkpoint2=false;
	}

	if(game->player.vehicle->checkpoint2==true&&game->player.vehicle->checkpoint1==true&&(unsigned)game->player.vehicle->index_track==game->track.num_pathpoints-1){
		game->player.vehicle->lap++;
		//printf("Player is on lap %d\n", game->player.vehicle->lap);
		game->player.vehicle->checkpoint1=false;
		game->player.vehicle->checkpoint2=false;
	}

	//AI win logic
	for(int i=0;i<=game->num_aiplayers-1;i++){
		if(game->aiplayers[i].vehicle->checkpoint1==false && (unsigned)game->aiplayers[i].vehicle->index_track==cp1){
			game->aiplayers[i].vehicle->checkpoint1=true;
		}

		if(game->aiplayers[i].vehicle->checkpoint2==false && game->aiplayers[i].vehicle->checkpoint1==true && (unsigned)game->aiplayers[i].vehicle->index_track==cp2){
			game->aiplayers[i].vehicle->checkpoint2=true;
		}

		if(game->aiplayers[i].vehicle->checkpoint2==true && (unsigned)game->aiplayers[i].vehicle->index_track==cp1){
			game->aiplayers[i].vehicle->checkpoint2=false;
		}

		if(game->aiplayers[i].vehicle->checkpoint2==true&&game->aiplayers[i].vehicle->checkpoint1==true&&(unsigned)game->aiplayers[i].vehicle->index_track==game->track.num_pathpoints-1){
			game->aiplayers[i].vehicle->lap++;
			//printf("AI %d is on lap %d\n", i, game->aiplayers[i].vehicle->lap);
			game->aiplayers[i].vehicle->checkpoint1=false;
			game->aiplayers[i].vehicle->checkpoint2=false;
		}
	}

}

void checkplace(struct game* game){

	
	//init everyone to last place
	for(int i=0;i<=game->num_aiplayers-1;i++){
		game->aiplayers[i].vehicle->place=game->num_aiplayers+1;
		game->player.vehicle->place=game->num_aiplayers+1;
	}

	
	for(int i=0; i<=game->num_aiplayers-1;i++){

		////////////////////////////////////
		//if only one AI
		if(game->num_aiplayers-1==0){

			//first check for laps
			//AI has larger lap he's ahead
			if(game->aiplayers[i].vehicle->lap > game->player.vehicle->lap){
				game->aiplayers[i].vehicle->place = game->player.vehicle->place-1;
				game->player.vehicle->place = game->aiplayers[i].vehicle->place+1;

			}

			//player has larger lap
			else if(game->aiplayers[i].vehicle->lap < game->player.vehicle->lap){
				game->player.vehicle->place = game->aiplayers[i].vehicle->place-1;
				game->aiplayers[i].vehicle->place = game->player.vehicle->place+1;
			}

			//if laps are the same
			else if(game->aiplayers[i].vehicle->lap == game->player.vehicle->lap){

				//now check for index
				//AI has larger index
				if(game->aiplayers[i].vehicle->index_track > game->player.vehicle->index_track){
					game->aiplayers[i].vehicle->place = game->player.vehicle->place-1;
					game->player.vehicle->place = game->aiplayers[i].vehicle->place+1;
				}

				//player has larger index
				else if(game->aiplayers[i].vehicle->index_track < game->player.vehicle->index_track){
					game->player.vehicle->place = game->aiplayers[i].vehicle->place-1;
					game->aiplayers[i].vehicle->place = game->player.vehicle->place+1;
				}

				//same index player ahead
				else if(game->aiplayers[i].vehicle->index_track == game->player.vehicle->index_track){
					game->player.vehicle->place = game->aiplayers[i].vehicle->place-1;
					game->aiplayers[i].vehicle->place = game->player.vehicle->place+1;
				}

			}

			//printf("Players place %d     AI[0] %d\n ", game->player.vehicle->place, game->aiplayers[0].vehicle->place);
		}
		

		// IF MORE THAN ONE AI
		if(game->num_aiplayers-1 > 0){

			for(int j=0; j<=game->num_aiplayers-1;j++){


				//FIXING INDEX BEFORE LAPS

				//check index first
				//AI has larger index than player
				if(game->aiplayers[i].vehicle->index_track > game->player.vehicle->index_track){
					game->aiplayers[i].vehicle->place = game->player.vehicle->place-1;
					game->player.vehicle->place = game->aiplayers[i].vehicle->place+1;
				}

				//player has larger index
				else if(game->aiplayers[i].vehicle->index_track < game->player.vehicle->index_track){
					game->player.vehicle->place = game->aiplayers[i].vehicle->place-1;
					game->aiplayers[i].vehicle->place = game->player.vehicle->place+1;
				}

				//same index player ahead
				else if(game->aiplayers[i].vehicle->index_track == game->player.vehicle->index_track){
					game->player.vehicle->place = game->aiplayers[i].vehicle->place-1;
					game->aiplayers[i].vehicle->place = game->player.vehicle->place+1;
				}

								if(game->aiplayers[i].vehicle->index_track > game->aiplayers[j].vehicle->index_track){

									game->aiplayers[i].vehicle->place = game->aiplayers[j].vehicle->place-1;
									game->aiplayers[j].vehicle->place = game->aiplayers[i].vehicle->place+1;

								}
								else if(game->aiplayers[i].vehicle->index_track < game->aiplayers[j].vehicle->index_track){
									game->aiplayers[j].vehicle->place = game->aiplayers[i].vehicle->place-1;
									game->aiplayers[i].vehicle->place = game->aiplayers[j].vehicle->place+1;

								}
															
			}

			if(game->num_aiplayers-1>1){
				//printf("Player   %d    AI[0] %d     AI[1] %d    AI[2] %d\n", game->player.vehicle->place, game->aiplayers[0].vehicle->place, game->aiplayers[1].vehicle->place, game->aiplayers[2].vehicle->place);
			}
		}
	}

	removetext(&game->uimanager, "placer");
	vec3f color;
	vec3f_set(color, 1.0f,1.0f,.0f);
	if(game->player.vehicle->place==21||game->player.vehicle->place==31||game->player.vehicle->place==41||game->player.vehicle->place==51||game->player.vehicle->place==61||game->player.vehicle->place==71||game->player.vehicle->place==81||game->player.vehicle->place==91){
		addtext(&game->uimanager, "placer", 225, 580,color,&game->uimanager.font_placer,-3);
	}
	else if(game->player.vehicle->place==1){
		addtext(&game->uimanager, "placer", 200, 580,color,&game->uimanager.font_placer,-3);
	}
	else if(game->player.vehicle->place==2){
		addtext(&game->uimanager, "placer", 200, 580,color,&game->uimanager.font_placer,-4);
	}
	else if(game->player.vehicle->place==22||game->player.vehicle->place==32||game->player.vehicle->place==42||game->player.vehicle->place==52||game->player.vehicle->place==62||game->player.vehicle->place==72||game->player.vehicle->place==82||game->player.vehicle->place==92){
		addtext(&game->uimanager, "placer", 270, 580,color,&game->uimanager.font_placer,-4);
	}
	else if(game->player.vehicle->place==3){
		addtext(&game->uimanager, "placer", 200, 580,color,&game->uimanager.font_placer,-5);
	}
	else if(game->player.vehicle->place==23||game->player.vehicle->place==33||game->player.vehicle->place==43||game->player.vehicle->place==53||game->player.vehicle->place==63||game->player.vehicle->place==73||game->player.vehicle->place==83||game->player.vehicle->place==93){
		addtext(&game->uimanager, "placer", 270, 580,color,&game->uimanager.font_placer,-5);
	}
	else if(game->player.vehicle->place<20&&game->player.vehicle->place!=10){
		addtext(&game->uimanager, "placer", 225, 580,color,&game->uimanager.font_placer,-6);
	}
	else if(game->player.vehicle->place==20){
		addtext(&game->uimanager, "placer", 300, 580,color,&game->uimanager.font_placer,-6);
	}
	else if(game->player.vehicle->place<30){
		addtext(&game->uimanager, "placer", 280, 580,color,&game->uimanager.font_placer,-6);
	}
	else if(game->player.vehicle->place%10==0){
		addtext(&game->uimanager, "placer", 315, 580,color,&game->uimanager.font_placer,-6);
	}
	else{
		addtext(&game->uimanager, "placer", 290, 580,color,&game->uimanager.font_placer,-6);
	}
	
}

