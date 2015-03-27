#include    "menu.h"

#include	"ui.h"
#include	"game.h"
#include	"../mem.h"

static void rendermenu(struct game* game)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	uimanager_render(&game->uimanager, game);//render text

	glfwSwapBuffers(game->window.w);
}

static void updatemenu(struct game* game)
{
	inputmanager_update(&game->inputmanager);

	if (game->inputmanager.controllers[GLFW_JOYSTICK_1].flags & INPUT_FLAG_ENABLED)
	{
		vec3f color;
		vec3f_set(color,0.0f,0.0f,1.0f); //BLUE

		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_DDOWN] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			if(!(game->menuflags & MENU_FLAG_INCREDITS)){
				if(!(game->menuflags & MENU_FLAG_INSETTINGS)){
					if(game->menuflags & MENU_FLAG_STARTGAME){
						game->menuflags &= ~MENU_FLAG_STARTGAME;
						game->menuflags |= MENU_FLAG_SETTINGS;

						removebrackets(&game->uimanager);
						addtext(&game->uimanager,"[                                                 ]",460,400,color,&game->uimanager.font_playerlap,0);
					}

					else if (game->menuflags & MENU_FLAG_SETTINGS){
						game->menuflags &= ~MENU_FLAG_SETTINGS;
						game->menuflags |= MENU_FLAG_CREDITS;

						removebrackets(&game->uimanager);
						addtext(&game->uimanager,"[                                          ]",470,500,color,&game->uimanager.font_playerlap,0);
					}
					else if (game->menuflags & MENU_FLAG_CREDITS){
						game->menuflags &= ~MENU_FLAG_CREDITS;
						game->menuflags |= MENU_FLAG_EXITGAME;

						removebrackets(&game->uimanager);
						addtext(&game->uimanager,"[                                                         ]",440,600,color,&game->uimanager.font_playerlap,0);
					}
					else if (game->menuflags & MENU_FLAG_EXITGAME){
						game->menuflags &= ~MENU_FLAG_EXITGAME;
						game->menuflags |= MENU_FLAG_STARTGAME;

						removebrackets(&game->uimanager);
						addtext(&game->uimanager,"[                                                                    ]",430,300,color,&game->uimanager.font_playerlap,0);
					}		
				}else{

					removebrackets(&game->uimanager);
					
					//move down to sound
					if(game->menuflags & MENU_FLAG_NUMAI){
						game->menuflags &= ~MENU_FLAG_NUMAI;
						game->menuflags |= MENU_FLAG_SOUND;
						addtext(&game->uimanager,"[                                                                                  ]",480,450,color,&game->uimanager.font_playerlap,0);//sound
					}

					//move down to back
					else if(game->menuflags & MENU_FLAG_SOUND){
						game->menuflags &= ~MENU_FLAG_SOUND;
						game->anothermenuflag |= MENU_FLAG_NUMLAPS;
						addtext(&game->uimanager,"[                                                                                                    ]",423,550,color,&game->uimanager.font_playerlap,0);
					}
					else if(game->anothermenuflag & MENU_FLAG_NUMLAPS){
						game->anothermenuflag &= ~MENU_FLAG_NUMLAPS;
						addtext(&game->uimanager,"[                                  ]",560,650,color,&game->uimanager.font_playerlap,0);//back
					}else{
						//move to numai
						game->menuflags |= MENU_FLAG_NUMAI;
						addtext(&game->uimanager,"[                                                                                                                                      ]",360,350,color,&game->uimanager.font_playerlap,0);//numai
					}
				}
			}
		}

		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_DUP] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			if(!(game->menuflags & MENU_FLAG_INCREDITS)){
				if(!(game->menuflags & MENU_FLAG_INSETTINGS)){
					if(game->menuflags & MENU_FLAG_STARTGAME){
						game->menuflags &= ~MENU_FLAG_STARTGAME;
						game->menuflags |= MENU_FLAG_EXITGAME;

						removebrackets(&game->uimanager);
						addtext(&game->uimanager,"[                                                         ]",440,600,color,&game->uimanager.font_playerlap,0);
					}
					else if (game->menuflags & MENU_FLAG_EXITGAME){
						game->menuflags &= ~MENU_FLAG_EXITGAME;
						game->menuflags |= MENU_FLAG_CREDITS;

						removebrackets(&game->uimanager);
						addtext(&game->uimanager,"[                                          ]",470,500,color,&game->uimanager.font_playerlap,0);
					}	
					else if (game->menuflags & MENU_FLAG_SETTINGS){
						game->menuflags &= ~MENU_FLAG_SETTINGS;
						game->menuflags |= MENU_FLAG_STARTGAME;

						removebrackets(&game->uimanager);
						addtext(&game->uimanager,"[                                                                    ]",430,300,color,&game->uimanager.font_playerlap,0);
					}
					else if (game->menuflags & MENU_FLAG_CREDITS){
						game->menuflags &= ~MENU_FLAG_CREDITS;
						game->menuflags |= MENU_FLAG_SETTINGS;

						removebrackets(&game->uimanager);
						addtext(&game->uimanager,"[                                                 ]",460,400,color,&game->uimanager.font_playerlap,0);
					}
				}else{
					removebrackets(&game->uimanager);
					if(game->menuflags & MENU_FLAG_NUMAI){
						game->menuflags &= ~MENU_FLAG_NUMAI;
						addtext(&game->uimanager,"[                                  ]",560,650,color,&game->uimanager.font_playerlap,0);//back
					}
					else if(game->menuflags & MENU_FLAG_SOUND){
						game->menuflags &= ~MENU_FLAG_SOUND;
						game->menuflags |= MENU_FLAG_NUMAI;
						addtext(&game->uimanager,"[                                                                                                                                      ]",360,350,color,&game->uimanager.font_playerlap,0);//numai
					}
					else if(game->anothermenuflag & MENU_FLAG_NUMLAPS){
						game->anothermenuflag &= ~MENU_FLAG_NUMLAPS;
						game->menuflags |= MENU_FLAG_SOUND;
						addtext(&game->uimanager,"[                                                                                  ]",480,450,color,&game->uimanager.font_playerlap,0);//sound
					}
					
					else{
						game->anothermenuflag |= MENU_FLAG_NUMLAPS;
						addtext(&game->uimanager,"[                                                                                                    ]",423,550,color,&game->uimanager.font_playerlap,0);
					}

					
				}
			}
		}

		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_A] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			if(!(game->menuflags & MENU_FLAG_INSETTINGS)){
				if(game->menuflags & MENU_FLAG_STARTGAME){
					game->flags &= ~GAME_FLAG_MAINMENU;
					removealltext(&game->uimanager);
				}
				else if (game->menuflags & MENU_FLAG_SETTINGS){
					removealltext(&game->uimanager);
					displaysettings(game);
				
					game->menuflags &= ~MENU_FLAG_NUMAI;
					game->menuflags &= ~MENU_FLAG_SOUND;

					game->menuflags &= ~MENU_FLAG_SETTINGS;
					game->menuflags |= MENU_FLAG_INSETTINGS;
				}
				else if (game->menuflags & MENU_FLAG_CREDITS){
					removealltext(&game->uimanager);
					displaycredits(game);
				
					game->menuflags &= ~MENU_FLAG_CREDITS;
					game->menuflags |= MENU_FLAG_INCREDITS;
				}	
				else if (game->menuflags & MENU_FLAG_INCREDITS){
					removealltext(&game->uimanager);
					displaymenu(game);
					game->menuflags &= ~MENU_FLAG_INCREDITS;
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
					removetext(&game->uimanager, "On");
					removetext(&game->uimanager, "Off");
					if(game->soundon==false){
						game->soundon=true;
					}else{
						game->soundon=false;
					}

					vec3f_set(color,.0f,.0f,1.0f); //BLUE
					if(game->soundon==false){
						vec3f_set(color,1.0f,1.0f,1.0f); //whitE
					}
					addtext(&game->uimanager,"On",660,450,color,&game->uimanager.font_playerlap,0);

					vec3f_set(color,.0f,.0f,1.0f); //BLUE
					if(game->soundon==true){
						vec3f_set(color,1.0f,1.0f,1.0f); //whitE
					}
					addtext(&game->uimanager,"Off",740,450,color,&game->uimanager.font_playerlap,0);
				}
				else if(game->anothermenuflag & MENU_FLAG_NUMLAPS){
					if(game->num_laps < 19){
						game->num_laps++;
					}
				}else{
					removealltext(&game->uimanager);
					displaymenu(game);
					game->menuflags &= ~MENU_FLAG_INSETTINGS;
					game->menuflags &= ~MENU_FLAG_CREDITS;
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
					if(game->num_laps > 1){
						game->num_laps--;
					}
				}
				else if(game->menuflags & MENU_FLAG_SOUND){
					printf("%d\n", (game->num_aiplayers/2));
				}
			}
		}
	}
}

int menu_startup(struct game* game){

	displaymenu(game);
	game->flags = GAME_FLAG_MAINMENU;
	game->menuflags &= ~MENU_FLAG_INSETTINGS;

	while (game->flags & GAME_FLAG_MAINMENU){
		updatemenu(game);

		if(!(game->flags & GAME_FLAG_MAINMENU))
			break;//for exit game and shutdown

		rendermenu(game);
	}

	return 1;
}