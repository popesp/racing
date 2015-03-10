#include	"win.h"


void checkwin(struct game* game){
	setblimp(game);

	unsigned int cp1 = game->track.num_pathpoints / 3;
	unsigned int cp2 = game->track.num_pathpoints / 2;

	if(game->player.vehicle->lap==GAME_WINCONDITION_LAPS){
		printf("Player has won the game!\nGame's over\n\n");

		entitymanager_removeblimp(&game->entitymanager,game->player.vehicle->ownblimp,game->player.vehicle);

		//reset laps
		game->player.vehicle->lap=1;
		for(int i=0; i<game->num_aiplayers-1;i++){
			game->aiplayers[i].vehicle->lap=1;
		}
		
		game->flags &= ~GAME_FLAG_WINCONDITION;
	}

	for(int i=0;i<=game->num_aiplayers-1;i++){
		//printf("%d\n", game->aiplayers[i].vehicle->lap);
		if(game->aiplayers[i].vehicle->lap==GAME_WINCONDITION_LAPS){
			printf("Computer-%d has won the game!\nGame's over\n\n", i);

			entitymanager_removeblimp(&game->entitymanager,game->player.vehicle->ownblimp,game->player.vehicle);

			//reset laps
			game->player.vehicle->lap=1;
			for(int i=0; i<=game->num_aiplayers-1;i++){
				game->aiplayers[i].vehicle->lap=1;
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
		printf("Player is on lap %d\n", game->player.vehicle->lap);
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
			printf("AI %d is on lap %d\n", i, game->aiplayers[i].vehicle->lap);
			game->aiplayers[i].vehicle->checkpoint1=false;
			game->aiplayers[i].vehicle->checkpoint2=false;
		}
	}

}

void checkplace(struct game* game){
	
	setblimp(game);

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
			entitymanager_removeblimp(&game->entitymanager,game->player.vehicle->ownblimp,game->player.vehicle);
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
	setblimp(game);
}
void setblimp(struct game* game){
	if(game->player.vehicle->place==2){
		game->entitymanager.r_blimp.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_place2;
	}
	else if(game->player.vehicle->place==1){
		game->entitymanager.r_blimp.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_place1;
	}
	else{
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_blimp;
	}


	if(game->player.vehicle->lap==1 && game->flags != GAME_FLAG_WINCONDITION){
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_win;
	}
	else if(game->flags != GAME_FLAG_WINCONDITION){
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_lose;
	}

	else if(game->player.vehicle->lap==1){
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_lap1;
	}
	else if(game->player.vehicle->lap==2){
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_lap2;
	}
	else if(game->player.vehicle->lap==3){
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_lap3;
	}
	else if(game->player.vehicle->lap==4){
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_lap4;
	}
	else if(game->player.vehicle->lap==5){
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_lap5;
	}
	else{
		game->entitymanager.r_blimplap.textures[RENDER_TEXTURE_DIFFUSE] = &game->entitymanager.diffuse_blimp;
	}


	//if(game->player.vehicle->hasblimp==true){
	//	entitymanager_removeblimp(&game->entitymanager,game->player.vehicle->ownblimp,game->player.vehicle);
	//}
	//entitymanager_newblimp(game->player.vehicle,&game->entitymanager,game->track.pathpoints[0].pos);
}