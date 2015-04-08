#include	"win.h"
#include	"ui.h"

void checkwin(struct game* game)
{
	

	unsigned int cp1 = game->track.num_pathpoints / 3;
	unsigned int cp2 = game->track.num_pathpoints / 2;

	if(game->player.vehicle->lap==game->num_laps+1){
		game->flags &= ~GAME_FLAG_WINCONDITION;
		game->flags |= GAME_FLAG_YOUWIN;
	}


	for(int i=0;i<game->num_aiplayers;i++){
		if(game->aiplayers[i].vehicle->lap==game->num_laps+1){
			game->flags &= ~GAME_FLAG_WINCONDITION;
			game->flags |= GAME_FLAG_YOULOSE;
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
	for(int i=0;i<game->num_aiplayers;i++){
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
			//printf("AI %d is on lap %d and place %d\n", i, game->aiplayers[i].vehicle->lap,game->aiplayers[i].vehicle->place );
			game->aiplayers[i].vehicle->checkpoint1=false;
			game->aiplayers[i].vehicle->checkpoint2=false;
		}
	}

}

void checkplace(struct game* game){

	
	//init to last place
	game->player.vehicle->place=game->num_aiplayers+1;
	
	int count_behind_player = 0;
	
	for(int i=0; i<game->num_aiplayers;i++){
		if(game->aiplayers[i].vehicle->lap == game->player.vehicle->lap){
			if(game->aiplayers[i].vehicle->checkpoint1 == game->player.vehicle->checkpoint1){
				if(game->aiplayers[i].vehicle->checkpoint2 == game->player.vehicle->checkpoint2){
					if(game->aiplayers[i].vehicle->index_track > game->player.vehicle->index_track){
					
					}
					else if(game->aiplayers[i].vehicle->index_track < game->player.vehicle->index_track){
						count_behind_player++;
						
					}else if(game->aiplayers[i].vehicle->index_track == game->player.vehicle->index_track){
					
					}
				
					//game->aiplayers[j].vehicle->place = game->player.vehicle->place-1;
				
				}
				else if(game->aiplayers[i].vehicle->checkpoint2 == false && game->player.vehicle->checkpoint2 == true){
					count_behind_player++;
				}
				else if(game->aiplayers[i].vehicle->checkpoint2 == true && game->player.vehicle->checkpoint2 == false){
					//vh2->place--;
				}
			}
			else if(game->aiplayers[i].vehicle->checkpoint1 == false && game->player.vehicle->checkpoint1 == true){
				count_behind_player++;
			}
			else if(game->aiplayers[i].vehicle->checkpoint1 == true && game->player.vehicle->checkpoint1 == false){
				//vh2->place--;
			}

						
		}else if(game->aiplayers[i].vehicle->lap < game->player.vehicle->lap){
			count_behind_player++;
		}else if(game->aiplayers[i].vehicle->lap > game->player.vehicle->lap){
			//vh2->place--;
		}
	}
	game->player.vehicle->place = game->num_aiplayers +1 - count_behind_player;

	removetext(&game->uimanager, "placer");
	vec3f color;
	vec3f_set(color, 1.0f,1.0f,.0f);
	if(game->player.vehicle->place==21||game->player.vehicle->place==31||game->player.vehicle->place==41||game->player.vehicle->place==51||game->player.vehicle->place==61||game->player.vehicle->place==71||game->player.vehicle->place==81||game->player.vehicle->place==91){
		addtext(&game->uimanager, "placer", 240, (game->window.height-220),color,&game->uimanager.font_placer,-3);
	}
	else if(game->player.vehicle->place==1){
		addtext(&game->uimanager, "placer", 170, (game->window.height-220),color,&game->uimanager.font_placer,-3);
	}
	else if(game->player.vehicle->place==2){
		addtext(&game->uimanager, "placer", 200, (game->window.height-220),color,&game->uimanager.font_placer,-4);
	}
	else if(game->player.vehicle->place==22||game->player.vehicle->place==32||game->player.vehicle->place==42||game->player.vehicle->place==52||game->player.vehicle->place==62||game->player.vehicle->place==72||game->player.vehicle->place==82||game->player.vehicle->place==92){
		addtext(&game->uimanager, "placer", 270, (game->window.height-220),color,&game->uimanager.font_placer,-4);
	}
	else if(game->player.vehicle->place==3){
		addtext(&game->uimanager, "placer", 200, (game->window.height-220),color,&game->uimanager.font_placer,-5);
	}
	else if(game->player.vehicle->place==23||game->player.vehicle->place==33||game->player.vehicle->place==43||game->player.vehicle->place==53||game->player.vehicle->place==63||game->player.vehicle->place==73||game->player.vehicle->place==83||game->player.vehicle->place==93){
		addtext(&game->uimanager, "placer", 270, (game->window.height-220),color,&game->uimanager.font_placer,-5);
	}
	else if(game->player.vehicle->place<10){
		addtext(&game->uimanager, "placer", 215, (game->window.height-220),color,&game->uimanager.font_placer,-6);
	}
	else if(game->player.vehicle->place<20){
		addtext(&game->uimanager, "placer", 235, (game->window.height-220),color,&game->uimanager.font_placer,-6);
	}
	else if(game->player.vehicle->place==20){
		addtext(&game->uimanager, "placer", 280, (game->window.height-220),color,&game->uimanager.font_placer,-6);
	}
	else if(game->player.vehicle->place<30){
		addtext(&game->uimanager, "placer", 280, (game->window.height-220),color,&game->uimanager.font_placer,-6);
	}
	else if(game->player.vehicle->place%10==0){
		addtext(&game->uimanager, "placer", 280, (game->window.height-220),color,&game->uimanager.font_placer,-6);
	}
	else{
		addtext(&game->uimanager, "placer", 280, (game->window.height-220),color,&game->uimanager.font_placer,-6);
	}
	
}
