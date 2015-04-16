#include	"game.h"

#include	<GL/glew.h>				// GL
#include	<GLFW/glfw3.h>			// GL
#include	<stdio.h>				// printf
#include	<stdlib.h>				// calloc, free TEMPORARY
#include	"../debug.h"			// printvec3f
#include	"../error.h"			// PRINT_ERROR
#include	"../math/mat4f.h"		// identity TEMPORARY
#include	"../math/vec3f.h"		// set TEMPORARY
#include	"../objects/entities/entity.h"
#include	"../physics/physics.h"		// startup, shutdown
#include	"../random.h"
#include	"../render/objloader.h"


static void render(struct game*);


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

		default:
			break;
		}
	}
}


static void loadrace(struct game* game)
{
	vec3f up, pos, dir;
	unsigned i;

	glClearColor(0.f, 0.f, 0.f, 1.f);
	render(game);
	glClearColor(GAME_CLEARCOLOR, 1.f);

	// set up vector
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

	// set up player objects
	player_init(&game->player, game->vehiclemanager.vehicles + (VEHICLE_COUNT-1));
	for (i = 0; i < GAME_AIPLAYER_COUNT; i++)
		aiplayer_init(game->aiplayers + i, game->vehiclemanager.vehicles + i, &game->track);

	// start up the pickup manager for the track
	int track_indices[] = {50, 100, 150}; // test values
	pickupmanager_startup(&game->pickupmanager, &game->audiomanager, &game->physicsmanager, &game->renderer, &game->track, 3, track_indices);

	// initialize debug camera TEMP
	vec3f_set(pos, 0.f, 0.f, 0.f);
	vec3f_set(dir, 0.f, 0.f, -1.f);
	camera_init(&game->cam_debug, pos, dir, up);

	// initialize lights TODO: rethink track lighting
	vec3f_set(game->track_lights[0].pos, 0.f, 10.f, 0.f);
	vec3f_set(game->track_lights[0].dif, 1.f, 1.f, 1.f);
	vec3f_set(game->track_lights[0].spc, 1.f, 1.f, 1.f);

	vec3f_set(game->track_lights[1].pos, 0.f, 0.f, 0.f);
	vec3f_set(game->track_lights[1].dif, 1.f, 1.f, 1.f);
	vec3f_set(game->track_lights[1].spc, 1.f, 1.f, 1.f);

	// give renderable objects references to the light objects
	game->track.r_track.num_lights = VEHICLE_COUNT;
	for (i = 0; i < VEHICLE_COUNT; i++)
		game->track.r_track.lights[i] = &game->vehiclemanager.vehicles[i].light;
	game->vehiclemanager.r_vehicle.lights[0] = game->track_lights + 0;
	//game->vehiclemanager.r_vehicle.lights[1] = game->track_lights + 1;

	// add background music
	game->songs[GAME_MUSIC_1_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_3_FILENAME);
	game->songs[GAME_MUSIC_2_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_2_FILENAME);
	game->songs[GAME_MUSIC_3_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_1_FILENAME);
	game->songs[GAME_MUSIC_4_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_4_FILENAME);
	game->index_currentsong = 0;
	game->currentchannel = audiomanager_playmusic(&game->audiomanager, game->songs[game->index_currentsong], -1, true);

	game->state = GAME_STATE_RACE;
	game->timer_racestart = GAME_TIMER_RACESTART;

	// reset the clock so no update cycles are lost when the race begins
	glfwSetTime(0.);
}

static void update(struct game* game)
{
	vec3f move;
	unsigned i;
	float s;

	// check for callback events
	glfwPollEvents();

	// update player input
	inputmanager_update(&game->inputmanager);
	if (!(game->controller_main->flags & INPUT_FLAG_ENABLED))
	{
		// controller was disconnected
		game->controller_main = NULL;
		for (i = 0; i < INPUT_MAX_JOYSTICKS; i++)
			if (game->inputmanager.controllers[i].flags & INPUT_FLAG_ENABLED)
			{
				game->controller_main = game->inputmanager.controllers + i;
				break;
			}

		// if no gamepad is available, switch to keyboard controls
		if (!game->controller_main)
			game->controller_main = &game->inputmanager.keyboard;
	}

	// update the user interface
	uimanager_update(&game->uimanager, game);

	// update the audio manager
	audiomanager_update(&game->audiomanager, game->player.camera.pos, game->player.camera.dir, game->player.camera.up);

	switch (game->state)
	{
	case GAME_STATE_LOADRACE:
		loadrace(game);
		break;

	case GAME_STATE_RACE:
		game->menupauseswitch = false;

		if (game->timer_racestart <= 59)
		{
			if(game->timer_racestart>0)
				game->timer_racestart--;

			// set vehicle controllers
			game->player.vehicle->controller = game->controller_main;
			for (i = 0; i < GAME_AIPLAYER_COUNT; i++)
				game->aiplayers[i].vehicle->controller = &game->aiplayers[i].controller;
		}  else
			game->timer_racestart--;

		// update the debug camera TEMP
		if (game->flags & GAME_FLAG_DEBUGCAM)
		{
			vec3f_copy(move, game->cam_debug.dir);
			move[VY] = 0.f;
			vec3f_normalize(move);

			vec3f_scale(move, -0.2f * game->controller_main->axes[INPUT_AXIS_LEFT_UD]);
			move[VY] = - 0.1f * game->controller_main->axes[INPUT_AXIS_TRIGGERS];
			vec3f_add(game->cam_debug.pos, move);

			camera_strafe(&game->cam_debug, 0.3f * game->controller_main->axes[INPUT_AXIS_LEFT_LR]);

			camera_rotate(&game->cam_debug, game->track.up, -0.03f * game->controller_main->axes[INPUT_AXIS_RIGHT_LR]);
			camera_rotate(&game->cam_debug, game->cam_debug.right, -0.03f * game->controller_main->axes[INPUT_AXIS_RIGHT_UD]);

			// disable cart controls if debug camera is enabled
			game->player.vehicle->controller = NULL;
		} else if (game->timer_racestart == 0)
			game->player.vehicle->controller = game->controller_main;

		// pause button
		if (game->controller_main->buttons[INPUT_BUTTON_START] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			// pause in-game audio
			audiomanager_ingamepausedstate(&game->audiomanager, true);

			game->uimanager.index_menuselection = 0;

			game->state = GAME_STATE_PAUSEMENU;
		}

		// update computer input processing
		for(i = 0; i < GAME_AIPLAYER_COUNT; i++)
			aiplayer_updateinput(&game->aiplayers[i], &game->vehiclemanager, game->difficulty);

		// update the vehicles
		vehiclemanager_update(&game->vehiclemanager);

		// update the game objects
		entitymanager_update(&game->entitymanager);

		// update the pickup manager
		pickupmanager_update(&game->pickupmanager);

		// update physics simulation
		physicsmanager_update(&game->physicsmanager, GAME_SPU);

		// update player camera
		player_updatecamera(&game->player);

		// update the window view angle with player speed
		s = game->player.vehicle->speed;
		window_viewangle(&game->window, WINDOW_DEFAULT_VIEWANGLE + (s*s/120.f - s/12.f));
		window_updateprojection(&game->window);

		// check for the win condition
		for (i = 0; i < VEHICLE_COUNT; i++)
			if (game->vehiclemanager.vehicles[i].lap == game->laps + 1)
			{
				game->uimanager.index_menuselection = 0;
				game->winningvehicle = game->vehiclemanager.vehicles + i;
				game->state = GAME_STATE_RACEDONE;
			}

		break;

	case GAME_STATE_RACEDONE:
		// update computer input processing
		for(i = 0; i < GAME_AIPLAYER_COUNT; i++)
			aiplayer_updateinput(&game->aiplayers[i], &game->vehiclemanager, game->difficulty);

		// update the vehicles
		vehiclemanager_update(&game->vehiclemanager);

		// update the game objects
		entitymanager_update(&game->entitymanager);

		// update the pickup manager
		pickupmanager_update(&game->pickupmanager);

		// update physics simulation
		physicsmanager_update(&game->physicsmanager, GAME_SPU);

		// update player camera
		player_updatewincamera(&game->player, game->winningvehicle);
		break;

	case GAME_STATE_PAUSEMENU:
		// unpause button
		if (game->controller_main->buttons[INPUT_BUTTON_START] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			// play in-game audio
			audiomanager_ingamepausedstate(&game->audiomanager, false);

			game->state = GAME_STATE_RACE;
		}
		break;
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

	switch (game->state)
	{
	case GAME_STATE_RACE:
	case GAME_STATE_RACEDONE:
	case GAME_STATE_PAUSEMENU:
	case GAME_STATE_PAUSESETTINGS:
		// get camera transform
		if (game->flags & GAME_FLAG_DEBUGCAM)
			camera_gettransform(&game->cam_debug, global_wv);
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
	}

	// render user interface
	glDisable(GL_DEPTH_TEST);
	uimanager_render(&game->uimanager, game);
	glEnable(GL_DEPTH_TEST);

	glfwSwapBuffers(game->window.w);
}

static int start_subsystems(struct game* game)
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

	// initialize audio manager
	printf("Starting up audio manager...");
	audiomanager_startup(&game->audiomanager);
	printf("...done. Using FMOD version %d.\n", audiomanager_getlibversion(&game->audiomanager));
	
	// initialize ui manager
	printf("Starting up user interface manager...");
	uimanager_startup(&game->uimanager, &game->audiomanager, &game->window, &game->renderer);
	printf("...done.\n");

	return 1;
}

int game_startup(struct game* game)
{
	if (!start_subsystems(game))
	{
		PRINT_ERROR("Problem starting game subsystems.\n");
		return 0;
	}

	// setup default controller
	if (game->inputmanager.controllers[0].flags & INPUT_FLAG_ENABLED)
		game->controller_main = game->inputmanager.controllers;
	else
		game->controller_main = &game->inputmanager.keyboard;

	game->flags = GAME_FLAG_INIT;
	game->state = GAME_STATE_MAINMENU;
	game->laps = GAME_DEFAULT_LAPS;
	game->difficulty = GAME_DEFAULT_DIFF;
	game->menupauseswitch = true;

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

void game_shutdown(struct game* game)
{
	// shut down all the game subsytems
	uimanager_shutdown(&game->uimanager);
	audiomanager_shutdown(&game->audiomanager);
	inputmanager_shutdown(&game->inputmanager);
	physicsmanager_shutdown(&game->physicsmanager);
	
	glfwDestroyWindow(game->window.w);
	glfwTerminate();
}