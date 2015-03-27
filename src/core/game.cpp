#include	"game.h"

#include	<GL/glew.h>				// GL
#include	<GLFW/glfw3.h>			// GL
#include	<stdio.h>				// printf
#include	<stdlib.h>				// calloc, free TEMPORARY
#include	"../debug.h"			// printvec3f
#include	"../error.h"			// PRINT_ERROR
#include	"../math/mat4f.h"		// identity TEMPORARY
#include	"../math/vec3f.h"		// set TEMPORARY
#include	"../random.h"
#include	"../render/objloader.h"

#include	"../objects/entities/entity.h"

#include	"../physics/physics.h"		// startup, shutdown


static void resetplayers(struct game*);
static void resize(GLFWwindow*, int, int);
static void keyboard(GLFWwindow*, int, int, int, int);
static void cursor(GLFWwindow*, double, double);
static void mouse(GLFWwindow*, int, int, int);
static void scroll(GLFWwindow*, double, double);
static void update(struct game*);
static void render(struct game*);


static void resetplayers(struct game* game)
{
	int i;

	vehiclemanager_resetvehicle(&game->vehiclemanager, game->player.vehicle);

	for (i = 0; i < game->num_aiplayers; i++)
		vehiclemanager_resetvehicle(&game->vehiclemanager, game->aiplayers[i].vehicle);
}

static void resize(GLFWwindow* window, int width, int height)
{
	struct game* game;

	game = (struct game*)glfwGetWindowUserPointer(window);

	window_resize(&game->window, width, height);
	glViewport(0, 0, width, height);
}

static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	struct game* game;
	int i;

	(void)scancode;
	(void)mods;


	game = (struct game*)glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case GLFW_KEY_ESCAPE:
			game->flags |= GAME_FLAG_TERMINATED;
			break;

		case GLFW_KEY_M:
			// go to next track
			soundchannel_stop(game->currentchannel);
			game->index_currentsong = (game->index_currentsong + 1) % GAME_MUSIC_COUNT;
			game->currentchannel = audiomanager_playmusic(&game->audiomanager, game->index_currentsong, -1);
			break;

		case GLFW_KEY_P:
			// pause the music
			soundchannel_toggle(game->currentchannel);
			break;

		case GLFW_KEY_Q:
			if (game->flags & GAME_FLAG_WIREFRAME)
			{
				game->flags &= ~GAME_FLAG_WIREFRAME;
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			else
			{
				game->flags |= GAME_FLAG_WIREFRAME;
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			break;

		case GLFW_KEY_C:
			if (game->flags & GAME_FLAG_DEBUGCAM)
				game->flags &= ~GAME_FLAG_DEBUGCAM;
			else
				game->flags |= GAME_FLAG_DEBUGCAM;
			break;

		case GLFW_KEY_R:
			// reset players' position and speed
			resetplayers(game);
			break;

		case GLFW_KEY_A:
			vec3f offs;

			// if max ai count has not been reached, add a new ai player
			game->num_aiplayers++;
			if (game->num_aiplayers > GAME_AIPLAYER_COUNT)
				game->num_aiplayers = GAME_AIPLAYER_COUNT;
			else
			{
				vec3f_set(offs, 0.f, 0.f, 0.f);
				aiplayer_init(game->aiplayers + (game->num_aiplayers - 1), &game->vehiclemanager, 5, offs);
				printf("Computer-%d has joined the game.\n", game->num_aiplayers);
			}
			break;

		case GLFW_KEY_SPACE:
			
			break;

		case GLFW_KEY_BACKSPACE:
			for (i = 0; i < game->num_aiplayers; i++)
				aiplayer_delete(game->aiplayers + i, &game->vehiclemanager);
			game->num_aiplayers = 0;
			break;

		default:
			break;
		}
	}
}

static void cursor(GLFWwindow* window, double x, double y)
{
	struct game* game;

	(void)x;
	(void)y;

	game = (struct game*)glfwGetWindowUserPointer(window);

	// cursor processing
}

static void mouse(GLFWwindow* window, int button, int action, int mods)
{
	struct game* game;

	(void)button;
	(void)action;
	(void)mods;

	game = (struct game*)glfwGetWindowUserPointer(window);

	// mouse button processing
}

static void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	struct game* game;

	(void)xoffset;
	(void)yoffset;

	game = (struct game*)glfwGetWindowUserPointer(window);

	// scroll processing
}

static void mainmenu(struct game* game){
	game->flags &= ~GAME_FLAG_YOULOSE;
	game->flags &= ~GAME_FLAG_YOUWIN;
	game->flags |= GAME_FLAG_MAINMENU;
}

static void restart(struct game* game){
	vec3f color;

	//turn off flags and turn back on wincondition
	game->flags &= ~GAME_FLAG_SWITCHON;
	game->flags &= ~GAME_FLAG_YOULOSE;
	game->flags &= ~GAME_FLAG_YOUWIN;
	game->flags |= GAME_FLAG_WINCONDITION;
	
	//reset laps
	game->player.vehicle->lap=1;
	for(int i=0; i<game->num_aiplayers;i++){
		game->aiplayers[i].vehicle->lap=1;
	}

	removealltext(&game->uimanager);

	//rewrite race text
	vec3f_set(color, 1.0f,1.0f,1.0f);
	addtext(&game->uimanager,"laps",100,700,color,&game->uimanager.font_playerlap,-1);
	vec3f_set(color, 1.0f,1.0f,.0f);
	addtext(&game->uimanager,"place",100,650,color,&game->uimanager.font_place,-2);
	vec3f_set(color, 0.0f,0.0f,1.0f);
	addtext(&game->uimanager,"Speed",1050,600,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"velocity",1060,700,color,&game->uimanager.font_velocity,9001);

	//delete player and ai
	player_delete(&game->player, &game->vehiclemanager);
	for (int i = 0; i < game->num_aiplayers; i++){
		aiplayer_delete(&game->aiplayers[i], &game->vehiclemanager);
	}

	// initialize player 
	vec3f offs,aioffs;
	vec3f_set(offs, 1.f, 0.f, 0.f);
			
	if (game->inputmanager.controllers[0].flags & INPUT_FLAG_ENABLED)
		player_init(&game->player, &game->vehiclemanager, &game->inputmanager.controllers[0], 0, offs);
	else
		player_init(&game->player, &game->vehiclemanager, &game->inputmanager.keyboard, 0, offs);

	float w;
	int track_indice[] = {1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51}; // test values
	
	// create ai for if even amount
	if(game->num_aiplayers%2==0){
		for (int i=0; i < game->num_aiplayers/2; i++)
		{
			w =game->track.pathpoints[track_indice[i]].width * .25f;
			
			vec3f_set(aioffs, -w, 0.f, 0.f);
			aiplayer_init(game->aiplayers+i*2+0, &game->vehiclemanager, i, aioffs);

			vec3f_set(aioffs, w, 0.f, 0.f);
			aiplayer_init(game->aiplayers+i*2+1, &game->vehiclemanager, i, aioffs);
		}
	}else{
		//CURRENTLY BROKEN FOR AN ODD AMOUNT OF AI
	}
}

static void winlose(struct game* game){

	vec3f color;

	//remove race text
	removetext(&game->uimanager, "laps");
	removetext(&game->uimanager, "place");
	removetext(&game->uimanager, "placer");
	removetext(&game->uimanager, "Speed");
	removetext(&game->uimanager, "velocity");

	//Case: Game is lost
	if(game->flags & GAME_FLAG_YOULOSE){
		//set camera on winning AI
		for(int i=0;i<=game->num_aiplayers-1;i++){
			if(game->aiplayers[i].vehicle->lap==GAME_WINCONDITION_LAPS){
				aiwin_camera(&game->aiplayers[i]);
			}
		}

		vec3f_set(color,1.0f,0.0f,0.0f);
		addtext(&game->uimanager,"YOU LOST",200,200,color,&game->uimanager.font_youwinlost,0);

		vec3f_set(color,1.0f,1.0f,1.0f);
		addtext(&game->uimanager,"computerwon",345,280,color,&game->uimanager.font_playerlap,666);

	//Case: Game is won
	}else{
		vec3f_set(color,1.0f,0.0f,0.0f);
		addtext(&game->uimanager,"YOU WON",200,200,color,&game->uimanager.font_youwinlost,0);

		vec3f_set(color,1.0f,1.0f,1.0f);
		addtext(&game->uimanager,"Nice job!",550,280,color,&game->uimanager.font_playerlap,0);
	}
		
	vec3f_set(color,1.0f,1.0f,1.0f);
	addtext(&game->uimanager,"Restart",550,600,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Main   Menu",530,700,color,&game->uimanager.font_playerlap,0);
	vec3f_set(color,.0f,.0f,1.0f);
	if(!(game->flags & GAME_FLAG_SWITCHON)){
		addtext(&game->uimanager,"[                                                    ]",520,600,color,&game->uimanager.font_playerlap,0);
	}
}

static void update(struct game* game)
{
	vec3f move, up;
	int i;

	// check for callback events
	glfwPollEvents();

	// update player and ai input
	inputmanager_update(&game->inputmanager);
	for(i = 0; i < game->num_aiplayers; i++)
		aiplayer_updateinput(&game->aiplayers[i]);
	
	if (game->inputmanager.controllers[GLFW_JOYSTICK_1].flags & INPUT_FLAG_ENABLED)
	{
		// temporary debug button
		if (game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_Y] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			if(!(game->flags & GAME_FLAG_YOULOSE)){
				if (game->flags & GAME_FLAG_DEBUGCAM)
					game->flags &= ~GAME_FLAG_DEBUGCAM;
				else
					game->flags |= GAME_FLAG_DEBUGCAM;
			}
		}

		// PAUSE BUTTON
		if (game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_START] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			vec3f color;
			vec3f_set(color,1.0f,1.0f,1.0f);
	
			if (game->flags & GAME_FLAG_PAUSED){
				removetext(&game->uimanager, "Game   Paused");
				game->flags &= ~GAME_FLAG_PAUSED;
			}else{
				if(!(game->flags & GAME_FLAG_YOULOSE || game->flags & GAME_FLAG_YOUWIN)){
					addtext(&game->uimanager, "Game   Paused", 440, 350, color, &game->uimanager.font_pause,0);
					game->flags |= GAME_FLAG_PAUSED;
				}
			}
		}

		//TEMP, TOGGLE BETWEEN MAINMENU/RESTART
		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_X] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			removebrackets(&game->uimanager);
			if(game->flags & GAME_FLAG_YOULOSE || game->flags & GAME_FLAG_YOUWIN){
				vec3f color;
				vec3f_set(color,.0f,.0f,1.0f);

				if(game->flags & GAME_FLAG_SWITCHON){
					game->flags &= ~GAME_FLAG_SWITCHON;
					addtext(&game->uimanager,"[                                                    ]",520,600,color,&game->uimanager.font_playerlap,0);
				}
				else{
					game->flags |= GAME_FLAG_SWITCHON;
					addtext(&game->uimanager,"[                                                              ]",500,700,color,&game->uimanager.font_playerlap,0);
				}			
			}
		}

		//SELECT MAINMENU/RESTART
		if(game->inputmanager.controllers[GLFW_JOYSTICK_1].buttons[INPUT_BUTTON_A] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN)){
			if(game->flags & GAME_FLAG_YOULOSE || game->flags & GAME_FLAG_YOUWIN){
				if(game->flags & GAME_FLAG_SWITCHON){
					mainmenu(game);
				}else{
					restart(game);
				}
			}
		}
	}

	vec3f_set(up, 0.f, 1.f, 0.f);

	// DEBUG CAM UPDATE
	if (game->flags & GAME_FLAG_DEBUGCAM)
	{
		vec3f_copy(move, game->cam_debug.dir);
		move[VY] = 0.f;
		vec3f_normalize(move);
		if (game->inputmanager.controllers[GLFW_JOYSTICK_1].flags & INPUT_FLAG_ENABLED){
			if(!(game->flags & GAME_FLAG_YOULOSE)){
				vec3f_scale(move, -0.2f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_LEFT_UD]);
				move[VY] = - 0.1f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_TRIGGERS];
				vec3f_add(game->cam_debug.pos, move);

				camera_strafe(&game->cam_debug, 0.3f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_LEFT_LR]);

				camera_rotate(&game->cam_debug, up, -0.03f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_RIGHT_LR]);
				camera_rotate(&game->cam_debug, game->cam_debug.right, -0.03f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_RIGHT_UD]);
			}
		}

		// disable cart controls if debug camera is enabled
		game->player.vehicle->controller = NULL;
		} else
			game->player.vehicle->controller = &game->inputmanager.controllers[GLFW_JOYSTICK_1];

	//BASIC UPDATES
	if(!(game->flags&GAME_FLAG_PAUSED)){

		// update the audio manager
		audiomanager_update(&game->audiomanager, game->player.camera.pos, game->player.camera.dir, game->player.camera.up);

		// update the vehicles
		vehiclemanager_update(&game->vehiclemanager);

		// update the game objects
		entitymanager_update(&game->entitymanager);

		// update the pickup manager
		pickupmanager_update(&game->pickupmanager);

		// update physics simulation
		physicsmanager_update(&game->physicsmanager, GAME_SPU);

		if(!(game->flags&GAME_FLAG_YOULOSE)){
			// update player camera
			player_updatecamera(&game->player);
		}
	}

	// WIN/LOSE FLAG UPDATE
	if(game->flags & GAME_FLAG_YOULOSE || game->flags & GAME_FLAG_YOUWIN){
		winlose(game);
	}

	//UPDATE PLACE AND LAP
	if(game->flags & GAME_FLAG_WINCONDITION){
		checkwin(game);
		checkplace(game);
	}

	//Sound On Off
	if (!game->soundon){
		audiomanager_setmusicvolume(&game->audiomanager,0);
		audiomanager_setsfxvolume(&game->audiomanager,0);
	}
	// check for window close messages
	if (glfwWindowShouldClose(game->window.w))
		game->flags |= GAME_FLAG_TERMINATED;
}

static void render(struct game* game)
{
	physx::PxMat44 player_mw, aiplayer_mw;
	mat4f global_wv, skybox_wv;
	mat4f track_mw, skybox_mw;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get camera transform
	if (game->flags & GAME_FLAG_DEBUGCAM)
		camera_gettransform(&game->cam_debug, global_wv);

	// camera for AI if you lose
	else if (game->flags & GAME_FLAG_YOULOSE){
		for(int i=0;i<=game->num_aiplayers-1;i++){
			if(game->aiplayers[i].vehicle->lap==GAME_WINCONDITION_LAPS){
				camera_gettransform(&game->aiplayers[i].camera, global_wv);
			}
		}	
	}

	else
		camera_gettransform(&game->player.camera, global_wv);

	// remove translation from camera transform for the skybox
	mat4f_copy(skybox_wv, global_wv);
	vec3f_set(skybox_wv + C3, 0.f, 0.f, 0.f);
	skybox_wv[R3 + C3] = 1.f;

	// render skybox
	mat4f_identity(skybox_mw);
	renderable_render(&game->renderer, &game->skybox.r_skybox, skybox_mw, skybox_wv, 0);

	glClear(GL_DEPTH_BUFFER_BIT);

	// render track
	mat4f_identity(track_mw);
	renderable_render(&game->renderer, &game->track.r_track, track_mw, global_wv, 0);

	// render players
	vehiclemanager_render(&game->vehiclemanager, &game->renderer, global_wv);

	// render game entities
	entitymanager_render(&game->entitymanager, &game->renderer, global_wv);

	// render pickups
	pickupmanager_render(&game->pickupmanager, &game->renderer, global_wv);

	// render text
	uimanager_render(&game->uimanager, game);

	glfwSwapBuffers(game->window.w);
}

int start_subsystems(struct game* game)
{
	int major, minor, rev;
	int i;

	// initialize GLFW
	printf("Initializing GLFW...");
	glfwInit();
	glfwGetVersion(&major, &minor, &rev);
	printf("...done. Using version %d.%d.%d.\n", major, minor, rev);

	// initialize window object
	window_init(&game->window, GAME_DEFAULT_WIDTH, GAME_DEFAULT_HEIGHT, WINDOW_FLAG_NONE);

	// ensure a compatible context
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_SAMPLES, 16);				// 16X FSAA

	// create window handle
	if ((game->window.w = glfwCreateWindow(game->window.width, game->window.height, GAME_TITLE, NULL, NULL)) == NULL)
	{
		PRINT_ERROR("Could not create a window.\n");
		return 0;
	}

	// move to default screen position
	glfwSetWindowPos(game->window.w, GAME_DEFAULT_X, GAME_DEFAULT_Y);
	glfwShowWindow(game->window.w);

	// make window's opengl context current
	glfwMakeContextCurrent(game->window.w);
	glfwSwapInterval(1); // vertical sync

	// print OpenGL version context
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("OpenGL context initialized, using version %d.%d.\n", major, minor);

	// initialize GLEW
	printf("Initializing GLEW...");
	glewExperimental = GL_TRUE;
	glewInit();
	printf("...done. Using version %s.\n", glewGetString(GLEW_VERSION));

	// register game object as user pointer for callback functions
	glfwSetWindowUserPointer(game->window.w, (void*)game);

	// configure opengl
	glClearColor(GAME_CLEARCOLOR, 1.f);
	glPointSize(GAME_POINTSIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// initialize gl viewport
	glViewport(0, 0, game->window.width, game->window.height);

	// set callback functions for the window
	glfwSetWindowSizeCallback(game->window.w, &resize);
	glfwSetKeyCallback(game->window.w, &keyboard);
	glfwSetCursorPosCallback(game->window.w, &cursor);
	glfwSetMouseButtonCallback(game->window.w, &mouse);
	glfwSetScrollCallback(game->window.w, &scroll);

	// initialize renderer
	if (!renderer_init(&game->renderer, &game->window))
	{
		PRINT_ERROR("Problem initializing the renderer.\n");
		return 0;
	}

	// initialize physics manager
	printf("Starting up physics manager...");
	physicsmanager_startup(&game->physicsmanager);
	printf("...done. Using PhysX version %d.%d.%d.\n", PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX);

	// initialize input manager
	printf("Starting up input manager...");
	inputmanager_startup(&game->inputmanager, &game->window);
	printf("...done.\n");

	// print connected joystick information
	for (i = 0; i <= GLFW_JOYSTICK_LAST; i++)
	{
		if (game->inputmanager.controllers[GLFW_JOYSTICK_1+i].flags & INPUT_FLAG_ENABLED)
			printf("Joystick %d enabled. Name: %s\n", i+1, inputmanager_joystickname(&game->inputmanager, i));
	}
	
	// initialize ui manager
	printf("Starting up user interface manager...");
	uimanager_startup(&game->uimanager, &game->window);
	printf("...done.\n");

	// initialize audio manager
	printf("Starting up audio manager...");
	audiomanager_startup(&game->audiomanager);
	printf("...done. Using FMOD version %d.\n", audiomanager_getlibversion(&game->audiomanager));

	return 1;
}

int game_startup(struct game* game)
{
	vec3f up, dir, pos, offs, aioffs;

	vec3f_set(up, 0.f, 1.f, 0.f);

	random_timeseed();
	
	// initialize skybox
	skybox_init(&game->skybox, &game->renderer);

	// initialize track object
	track_init(&game->track, &game->physicsmanager, up);
	track_loadpointsfile(&game->track, "res/tracks/bigturn.track", &game->renderer);
	
	// start up the entity manager for the track
	entitymanager_startup(&game->entitymanager, &game->physicsmanager, &game->audiomanager, &game->renderer);

	// start up the vehicle manager for the track
	vehiclemanager_startup(&game->vehiclemanager, &game->physicsmanager, &game->entitymanager, &game->audiomanager, &game->renderer, &game->track, &game->uimanager);

	// start up the pickup manager for the track
	int track_indices[] = {50, 100, 150}; // test values
	pickupmanager_startup(&game->pickupmanager, &game->audiomanager, &game->physicsmanager, &game->renderer, &game->track, 3, track_indices);

	// initialize player objects
	vec3f_set(offs, 1.f, 0.f, 0.f);
	
	if (game->inputmanager.controllers[0].flags & INPUT_FLAG_ENABLED)
		player_init(&game->player, &game->vehiclemanager, &game->inputmanager.controllers[0], 0, offs);
	else
		player_init(&game->player, &game->vehiclemanager, &game->inputmanager.keyboard, 0, offs);

	float w;
	int track_indice[] = {1, 2, 3, 4, 5, 6, 7, 8, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
	20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51}; // test values
	
	// create ai for if even amount
	if(game->num_aiplayers%2==0){
		for (int i=0; i < game->num_aiplayers/2; i++)
		{
			w =game->track.pathpoints[track_indice[i]].width * .25f;
			
			vec3f_set(aioffs, -w, 0.f, 0.f);
			aiplayer_init(game->aiplayers+i*2+0, &game->vehiclemanager, i, aioffs);

			vec3f_set(aioffs, w, 0.f, 0.f);
			aiplayer_init(game->aiplayers+i*2+1, &game->vehiclemanager, i, aioffs);
		}
	}else{
		//CURRENTLY BROKEN FOR AN ODD AMOUNT OF AI
	}
	

	// initialize debug camera
	vec3f_set(pos, 0.f, 0.f, -30.f);
	vec3f_set(dir, 0.f, 0.f, -1.f);
	camera_init(&game->cam_debug, pos, dir, up);

	// initialize lights
	vec3f_set(game->track_lights[0].pos, 0.f, 10.f, 0.f);
	vec3f_set(game->track_lights[0].dif, 1.f, 1.f, 1.f);
	vec3f_set(game->track_lights[0].spc, 1.f, 1.f, 1.f);

	vec3f_set(game->track_lights[1].pos, 0.f, 0.f, 0.f);
	vec3f_set(game->track_lights[1].dif, 1.f, 1.f, 1.f);
	vec3f_set(game->track_lights[1].spc, 1.f, 1.f, 1.f);

	// give renderable objects references to the light objects
	// TODO: need a better way to do this without manually setting it
	game->track.r_track.lights[0] = game->track_lights + 0;
	game->track.r_track.lights[1] = game->track_lights + 1;
	game->vehiclemanager.r_vehicle.lights[0] = game->track_lights + 0;
	game->vehiclemanager.r_vehicle.lights[1] = game->track_lights + 1;

	// add background music
	game->songs[GAME_MUSIC_1_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_3_FILENAME);
	game->songs[GAME_MUSIC_2_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_2_FILENAME);
	game->songs[GAME_MUSIC_3_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_1_FILENAME);
	game->songs[GAME_MUSIC_4_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_4_FILENAME);
	game->index_currentsong = 0;
	game->currentchannel = audiomanager_playmusic(&game->audiomanager, game->songs[game->index_currentsong], -1);
	audiomanager_setmusicvolume(&game->audiomanager, 0.2f);
	
	vec3f color;
	vec3f_set(color, 1.0f,1.0f,1.0f);
	//laps
	addtext(&game->uimanager,"laps",100,700,color,&game->uimanager.font_playerlap,-1);

	//place
	vec3f_set(color, 1.0f,1.0f,.0f);
	addtext(&game->uimanager,"place",100,650,color,&game->uimanager.font_place,-2);

	//speed
	vec3f_set(color, 0.0f,0.0f,1.0f);
	addtext(&game->uimanager,"Speed",1080,600,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"velocity",1060,700,color,&game->uimanager.font_velocity,9001);

	game->flags = GAME_FLAG_INIT;

	return 1;
}

void game_mainloop(struct game* game)
{
	double time, timer, elapsed;
	unsigned fps, ups;

	timer = time = 0.;
	fps = ups = 0u;

	// initialize timer
	glfwSetTime(0.);

	// loop until terminated
	while (!(game->flags&GAME_FLAG_TERMINATED))
	{
		// get elapsed time for current iteration
		elapsed = glfwGetTime();
		glfwSetTime(0.);

		time += elapsed;
		timer += elapsed;

		// update as much as necessary
		while (time >= GAME_SPU)
		{

			update(game);
			ups++;
			time -= GAME_SPU;
		}

		// every second, reset fps and ups counters
		while (timer >= 1.)
		{
			timer -= 1.;
			fps = ups = 0u;
		}

		// render as frequently as possible
		render(game);
		fps++;
	}
}

void game_shutdown(struct game* game){
	int i;

	if(!(game->flags & GAME_FLAG_MAINMENU)){
		// delete players
		player_delete(&game->player, &game->vehiclemanager);
		for (i = 0; i < game->num_aiplayers; i++){
			aiplayer_delete(&game->aiplayers[i], &game->vehiclemanager);}
	
		pickupmanager_shutdown(&game->pickupmanager);
		vehiclemanager_shutdown(&game->vehiclemanager);
		entitymanager_shutdown(&game->entitymanager);

		// delete world objects
		track_delete(&game->track);
		skybox_delete(&game->skybox);

		audiomanager_shutdown(&game->audiomanager);
		physicsmanager_shutdown(&game->physicsmanager);
	}
	
	// shut down all the game subsytems
	uimanager_shutdown(&game->uimanager);
	inputmanager_shutdown(&game->inputmanager);
	
	glfwDestroyWindow(game->window.w);
	glfwTerminate();
}

