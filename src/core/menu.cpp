#include    "menu.h"

#include	"ui.h"
#include	"game.h"
#include	"../mem.h"

static void rendermenu(struct game* game)
{
	//Clear screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//render text
	uimanager_render(&game->uimanager, game);

	//update window
	glfwSwapBuffers(game->window.w);
}

static void updatemenu(struct game* game)
{
	// check for callback events
	glfwPollEvents();

	//update controller
	inputmanager_update(&game->inputmanager);

	printf("window width %d window height %d\r", game->window.width, game->window.height);

	if (game->inputmanager.controllers[GLFW_JOYSTICK_1].flags & INPUT_FLAG_ENABLED)
	{
		vec3f color;
		vec3f_set(color,0.0f,0.0f,1.0f); //BLUE

		//Dpad down
		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_DDOWN] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			
			//check to make sure not in credits screen
			if(!(game->menuflags & MENU_FLAG_INCREDITS)){
				//check to make sure not in settings screen
				if(!(game->menuflags & MENU_FLAG_INSETTINGS)){
					
					//Dpad down "Start Game"
					if(game->menuflags & MENU_FLAG_STARTGAME){
						game->menuflags &= ~MENU_FLAG_STARTGAME;
						game->menuflags |= MENU_FLAG_SETTINGS;
					}
					//Dpad down "Settings"
					else if (game->menuflags & MENU_FLAG_SETTINGS){
						game->menuflags &= ~MENU_FLAG_SETTINGS;
						game->menuflags |= MENU_FLAG_CREDITS;
					}
					//Dpad down "Credits"
					else if (game->menuflags & MENU_FLAG_CREDITS){
						game->menuflags &= ~MENU_FLAG_CREDITS;
						game->menuflags |= MENU_FLAG_EXITGAME;
					}
					//Dpad down "Exit Game"
					else if (game->menuflags & MENU_FLAG_EXITGAME){
						game->menuflags &= ~MENU_FLAG_EXITGAME;
						game->menuflags |= MENU_FLAG_STARTGAME;
					}		
				}else{
					//In Settings screen
					
					//move down to sound
					if(game->menuflags & MENU_FLAG_NUMAI){
						game->menuflags &= ~MENU_FLAG_NUMAI;
						game->menuflags |= MENU_FLAG_SOUND;
					}
					//move down to num laps
					else if(game->menuflags & MENU_FLAG_SOUND){
						game->menuflags &= ~MENU_FLAG_SOUND;
						game->anothermenuflag |= MENU_FLAG_NUMLAPS;
					}
					//move down to back
					else if(game->anothermenuflag & MENU_FLAG_NUMLAPS){
						game->anothermenuflag &= ~MENU_FLAG_NUMLAPS;
					}else{
						//move to numai
						game->menuflags |= MENU_FLAG_NUMAI;
					}
				}
			}
		}

		//DPAD UP
		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_DUP] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			if(!(game->menuflags & MENU_FLAG_INCREDITS)){
				if(!(game->menuflags & MENU_FLAG_INSETTINGS)){
					if(game->menuflags & MENU_FLAG_STARTGAME){
						game->menuflags &= ~MENU_FLAG_STARTGAME;
						game->menuflags |= MENU_FLAG_EXITGAME;
					}
					else if (game->menuflags & MENU_FLAG_EXITGAME){
						game->menuflags &= ~MENU_FLAG_EXITGAME;
						game->menuflags |= MENU_FLAG_CREDITS;
					}	
					else if (game->menuflags & MENU_FLAG_SETTINGS){
						game->menuflags &= ~MENU_FLAG_SETTINGS;
						game->menuflags |= MENU_FLAG_STARTGAME;
					}
					else if (game->menuflags & MENU_FLAG_CREDITS){
						game->menuflags &= ~MENU_FLAG_CREDITS;
						game->menuflags |= MENU_FLAG_SETTINGS;
					}
				}else{
					if(game->menuflags & MENU_FLAG_NUMAI){
						game->menuflags &= ~MENU_FLAG_NUMAI;
					}
					else if(game->menuflags & MENU_FLAG_SOUND){
						game->menuflags &= ~MENU_FLAG_SOUND;
						game->menuflags |= MENU_FLAG_NUMAI;
					}
					else if(game->anothermenuflag & MENU_FLAG_NUMLAPS){
						game->anothermenuflag &= ~MENU_FLAG_NUMLAPS;
						game->menuflags |= MENU_FLAG_SOUND;
					}
					
					else{
						game->anothermenuflag |= MENU_FLAG_NUMLAPS;
					}
				}
			}
		}

		//A BUTTON
		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_A] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			
			if(!(game->menuflags & MENU_FLAG_INSETTINGS)){
				if(game->menuflags & MENU_FLAG_STARTGAME){
					game->flags &= ~GAME_FLAG_MAINMENU;
					removealltext(&game->uimanager);
				}
				else if (game->menuflags & MENU_FLAG_SETTINGS){
					game->menuflags &= ~MENU_FLAG_NUMAI;
					game->menuflags &= ~MENU_FLAG_SOUND;
					game->menuflags &= ~MENU_FLAG_SETTINGS;

					game->menuflags |= MENU_FLAG_INSETTINGS;
				}
				else if (game->menuflags & MENU_FLAG_CREDITS){
					game->menuflags &= ~MENU_FLAG_CREDITS;
					game->menuflags |= MENU_FLAG_INCREDITS;
				}	
				//SPECIAL: IN CREDITS
				else if (game->menuflags & MENU_FLAG_INCREDITS){
					game->menuflags &= ~MENU_FLAG_INCREDITS;
					game->menuflags |= MENU_FLAG_STARTGAME;
				}
				else if (game->menuflags & MENU_FLAG_EXITGAME){
					game_shutdown(game);
					game->flags &= ~GAME_FLAG_MAINMENU;
					game->menuflags |= MENU_FLAG_SHUTDOWN;
				}
			}else{
				if(game->menuflags & MENU_FLAG_NUMAI){
					if(game->num_aiplayers < GAME_AIPLAYER_COUNT){
						game->num_aiplayers+=2;
					}
				}
				else if(game->menuflags & MENU_FLAG_SOUND){
					if(game->soundon==false){
						game->soundon=true;
					}else{
						game->soundon=false;
					}
				}
				else if(game->anothermenuflag & MENU_FLAG_NUMLAPS){
					if(game->num_laps < 19){
						game->num_laps++;
					}
				}else{
					game->menuflags &= ~MENU_FLAG_INSETTINGS;
					game->menuflags |= MENU_FLAG_STARTGAME;
				}
			}

		}
		//B Button
		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_B] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			if(game->menuflags & MENU_FLAG_INSETTINGS){
				//decrease AI if on numAItoggle
				if(game->menuflags & MENU_FLAG_NUMAI){
					if(game->num_aiplayers > 2){
						game->num_aiplayers-=2;
					}
				}
				else if(game->anothermenuflag & MENU_FLAG_NUMLAPS){
					if(game->num_laps > 2){
						game->num_laps--;
					}
				}
				else if(game->menuflags & MENU_FLAG_SOUND){
					printf("%d\n", (game->num_aiplayers/2)); //test
				}
			}
		}
	}
}

int menu_startup(struct game* game){

	
	//set flags for menu
	game->flags = GAME_FLAG_MAINMENU;
	game->menuflags &= ~MENU_FLAG_INSETTINGS;
	game->menuflags |= MENU_FLAG_STARTGAME;

	//loop until not in menu
	while (game->flags & GAME_FLAG_MAINMENU){

		//render menu text
		if(!(game->menuflags & MENU_FLAG_INCREDITS)){
			//check to make sure not in settings screen
			if(!(game->menuflags & MENU_FLAG_INSETTINGS)){
				removealltext(&game->uimanager);
				displaymenu(game);
			}else{
				removealltext(&game->uimanager);
				displaysettings(game);
			}
		}else{
			removealltext(&game->uimanager);
			displaycredits(game);
		}

		updatemenu(game);

		if(!(game->flags & GAME_FLAG_MAINMENU))
			break;//for exit game and shutdown

		rendermenu(game);
	}

	return 1;
}
