#include	"game.h"

#include	<GL/glew.h>				// GL
#include	<GLFW/glfw3.h>			// GL
#include	<stdio.h>				// printf
#include	<stdlib.h>				// calloc, free TEMPORARY
#include	"../debug.h"			// printvec3f
#include	"../error.h"			// PRINT_ERROR
#include	"../math/mat4f.h"		// identity TEMPORARY
#include	"../math/vec3f.h"		// set TEMPORARY
#include	"../render/objloader.h"


static void resize(GLFWwindow*, int, int);
static void keyboard(GLFWwindow*, int, int, int, int);
static void cursor(GLFWwindow*, double, double);
static void mouse(GLFWwindow*, int, int, int);
static void scroll(GLFWwindow*, double, double);
static void update(struct game*);
static void render(struct game*);

static void resetplayers(struct game* game)
{
	vehicle_reset(&game->vehiclemanager, game->player.vehicle);

	for(int i=0;i<=game->amountAI;i++){
		vehicle_reset(&game->vehiclemanager, game->aiplayer[i].vehicle);
	}
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

			//pause music
		case GLFW_KEY_P:
			//audiomanager_togglemusic(&game->audiomanager, game->song_1);
			game->current_song = audiomanager_playnexttrack(&game->audiomanager, game->current_song, -1);

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
			game->amountAI++;
			
			//glitches at max so decremented
			if(game->amountAI>AI_MAX_COUNT-1){
				game->amountAI=AI_MAX_COUNT-1;
			}
			else{
				vec3f offs;
				vec3f_set(offs, 0.f, 0.f, 0.f);
				aiplayer_init(&game->aiplayer[game->amountAI], &game->vehiclemanager, 5, offs);
				printf("AI %d has been added to the game.\n", (game->amountAI));
			}
			break;

		case GLFW_KEY_SPACE:
			if(game->winstate == GAME_WINSTATE_OFF){
				printf("Game's win state is activated.\n\n\n");
				printf("Player is on lap %d\n", game->player.vehicle->lap);
				for(int i=0;i<=game->amountAI;i++){
					printf("AI %d is on lap %d\n", i, game->aiplayer[i].vehicle->lap);
				}
				game->winstate = GAME_WINSTATE_ON;
			}
			else{
				//reset laps
				printf("Game's win state is turned off.\n\n\n");
				game->player.vehicle->lap=1;
				game->player.vehicle->checkpoint1 = game->player.vehicle->checkpoint2 = false;
				for(int i=0; i<=game->amountAI;i++){
					game->aiplayer[i].vehicle->lap=1;
					game->aiplayer[i].vehicle->checkpoint1 = game->aiplayer[i].vehicle->checkpoint2 = false;
				}
				game->winstate = GAME_WINSTATE_OFF;
			}
			break;

		case GLFW_KEY_BACKSPACE:
				//removes all added AI
				for (int i=0;i<=game->amountAI;i++){
					aiplayer_delete(&game->aiplayer[i], &game->vehiclemanager);
				}
				game->amountAI=-1;
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

static void update(struct game* game)
{
	vec3f move, up;

	// check for callback events
	glfwPollEvents();

	// update player and ai input
	inputmanager_update(&game->inputmanager);

	for(int i=0;i<=game->amountAI;i++){
		aiplayer_updateinput(&game->aiplayer[i]);
	}
	
	// temporary debug button
	if (game->inputmanager.controllers[GLFW_JOYSTICK_1].flags & INPUT_FLAG_ENABLED){
		if (game->inputmanager.controllers[0].buttons[INPUT_BUTTON_Y] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			if (game->flags & GAME_FLAG_DEBUGCAM)
				game->flags &= ~GAME_FLAG_DEBUGCAM;
			else
				game->flags |= GAME_FLAG_DEBUGCAM;
		}
	}

	vec3f_set(up, 0.f, 1.f, 0.f);

	// update debug camera; NOTE: a lot of this is temporarily hard-coded
	if (game->flags & GAME_FLAG_DEBUGCAM)
	{
		vec3f_copy(move, game->cam_debug.dir);
		move[VY] = 0.f;
		vec3f_normalize(move);
		if (game->inputmanager.controllers[GLFW_JOYSTICK_1].flags & INPUT_FLAG_ENABLED){
			vec3f_scale(move, -0.2f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_LEFT_UD]);
			move[VY] = - 0.1f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_TRIGGERS];
			vec3f_add(game->cam_debug.pos, move);

			camera_strafe(&game->cam_debug, 0.2f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_LEFT_LR]);

			camera_rotate(&game->cam_debug, up, -0.03f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_RIGHT_LR]);
			camera_rotate(&game->cam_debug, game->cam_debug.right, -0.03f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_RIGHT_UD]);
		}

		// disable cart controls if debug camera is enabled
		game->player.vehicle->controller = NULL;
	} else
		game->player.vehicle->controller = &game->inputmanager.controllers[0];

	// update the vehicles
	vehiclemanager_update(&game->vehiclemanager);

	// set closest point
	vec3f_copy(game->closestpoint.buf_verts, game->track.pathpoints[game->player.vehicle->index_track]);
	vec3f_set(game->closestpoint.buf_verts + 3, 1.f, 0.f, 0.f);
	vec3f_copy(game->closestpoint.buf_verts + 6, game->player.vehicle->pos);
	vec3f_set(game->closestpoint.buf_verts + 9, 1.f, 0.f, 0.f);
	renderable_sendbuffer(&game->renderer, &game->closestpoint);

	player_updatecamera(&game->player);

	// simulate
	physicsmanager_update(&game->physicsmanager, GAME_SPU);

	// check for window close messages
	if (glfwWindowShouldClose(game->window.w))
		game->flags |= GAME_FLAG_TERMINATED;
}

static void render(struct game* game)
{
	mat4f global_wv, skybox_wv, track_mw, skybox_mw;
	physx::PxMat44 player_world(game->player.vehicle->body->getGlobalPose());
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get camera transform
	if (game->flags & GAME_FLAG_DEBUGCAM)
		camera_gettransform(&game->cam_debug, global_wv);
	else
		camera_gettransform(&game->player.camera, global_wv);

	// remove translation from camera transform
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

	// render multiple AI's
	for(int i=0;i<=game->amountAI;i++){
		physx::PxMat44 newworld(game->aiplayer[i].vehicle->body->getGlobalPose());
		renderable_render(&game->renderer, &game->vehiclemanager.r_vehicle, (float*)&newworld, global_wv, 0);	
	}

	// render carts
	renderable_render(&game->renderer, &game->vehiclemanager.r_vehicle, (float*)&player_world, global_wv, 0);

	glClear(GL_DEPTH_BUFFER_BIT);
	renderable_render(&game->renderer, &game->closestpoint, track_mw, global_wv, 0);

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

	// initialize gl viewport
	glViewport(0, 0, game->window.width, game->window.height);

	// set callback functions for the window
	glfwSetWindowSizeCallback(game->window.w, &resize);
	glfwSetKeyCallback(game->window.w, &keyboard);
	glfwSetCursorPosCallback(game->window.w, &cursor);
	glfwSetMouseButtonCallback(game->window.w, &mouse);
	glfwSetScrollCallback(game->window.w, &scroll);

	// initialize texture manager
	printf("Starting up texture manager...");
	texturemanager_startup(&game->texturemanager);
	printf("...done. Using FreeImage version %s\n", texturemanager_getlibversion(&game->texturemanager));

	// initialize renderer
	if (!renderer_init(&game->renderer, &game->texturemanager, &game->window))
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

	
	return 1;
}

int game_startup(struct game* game)
{
	vec3f up, dir, pos, offs;

	vec3f_set(up, 0.f, 1.f, 0.f);

	if (!start_subsystems(game))
	{
		PRINT_ERROR("Problem starting game subsystems.\n");
		return 0;
	}

	// initialize skybox
	skybox_init(&game->skybox, &game->texturemanager, "res/images/night.jpg");
	skybox_generatemesh(&game->renderer, &game->skybox);
	renderable_sendbuffer(&game->renderer, &game->skybox.r_skybox);

	// initialize track object
	track_init(&game->track, up, &game->physicsmanager);
	track_loadpointsfile(&game->track, "res/tracks/wipeout.track");
	track_generate(&game->renderer, &game->track);
	renderable_sendbuffer(&game->renderer, &game->track.r_track);

	// send track mesh to physX
	physicsmanager_addstatic_trianglestrip(&game->physicsmanager, game->track.r_track.num_verts, sizeof(float)*RENDER_VERTSIZE_BUMP_L, game->track.r_track.buf_verts);
	
	// start up the vehicle manager for the track
	vehiclemanager_startup(&game->vehiclemanager, &game->renderer, &game->texturemanager, &game->physicsmanager, &game->track, "res/models/car/car.obj", "res/models/car/outUV.jpg");

	// initialize player objects
	vec3f_set(offs, 0.f, 0.f, 0.f);
	if (game->inputmanager.controllers[0].flags & INPUT_FLAG_ENABLED)
		player_init(&game->player, &game->vehiclemanager, &game->inputmanager.controllers[0], 0, offs);
	else
		player_init(&game->player, &game->vehiclemanager, &game->inputmanager.keyboard, 0, offs);
	aiplayer_init(&game->aiplayer[0], &game->vehiclemanager, 5, offs);

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
	
	// track normal map
	game->tex_trackbump = texturemanager_newtexture(&game->texturemanager);
	texture_loadfile(&game->texturemanager, game->tex_trackbump, "res/images/slate.jpg");
	texture_upload(&game->texturemanager, game->tex_trackbump, RENDER_TEXTURE_NORMAL);
	game->track.r_track.texture_ids[RENDER_TEXTURE_NORMAL] = game->tex_trackbump;
	
	// add background music
	game->song_1 = audiomanager_newmusic(&game->audiomanager, "res/music/Erasure Always.mp3");
	game->song_2 = audiomanager_newmusic(&game->audiomanager, "res/music/Daft Punk & The Glitch Mob - Derezzed.mp3");
	game->song_3 = audiomanager_newmusic(&game->audiomanager, "res/music/Full Force Forward.mp3");
	game->song_4 = audiomanager_newmusic(&game->audiomanager, "res/music/Daft Punk & Boys Noize - End Of Line.mp3");
	
	
	audiomanager_playmusic(&game->audiomanager, game->song_1, -1);
	game->current_song = game->song_1;

	/* temp */
	renderable_init(&game->closestpoint,  RENDER_MODE_LINESTRIP, RENDER_TYPE_WIRE_S, RENDER_FLAG_DYNAMIC);
	renderable_allocate(&game->renderer, &game->closestpoint, 2);
	/* end temp */
	
	game->amountAI = 0;
	game->winstate = GAME_WINSTATE_OFF;
	game->flags = GAME_FLAG_INIT;

	return 1;
}

static void checkwin(struct game* game){

	int cp1 = game->track.num_pathpoints / 3;
	int cp2 = game->track.num_pathpoints / 2;

	if(game->player.vehicle->lap==GAME_WIN_LAP){
		printf("Player has won the game!\nGame's over\n\n");

		//reset laps
		game->player.vehicle->lap=1;
		game->player.vehicle->checkpoint1 = game->player.vehicle->checkpoint2 = false;
		for(int i=0; i<=game->amountAI;i++){
			game->aiplayer[i].vehicle->lap=1;
			game->aiplayer[i].vehicle->checkpoint1 = game->aiplayer[i].vehicle->checkpoint2 = false;
		}

		game->winstate = GAME_WINSTATE_OFF;
	}

	for(int i=0;i<=game->amountAI;i++){
		if(game->aiplayer[i].vehicle->lap==GAME_WIN_LAP){
			printf("AI %d has won the game!\nGame's over\n\n", i);

			//reset laps
			game->player.vehicle->lap=1;
			game->player.vehicle->checkpoint1 = game->player.vehicle->checkpoint2 = false;
			for(int i=0; i<=game->amountAI;i++){
				game->aiplayer[i].vehicle->lap=1;
				game->aiplayer[i].vehicle->checkpoint1 = game->aiplayer[i].vehicle->checkpoint2 = false;
			}
			game->winstate = GAME_WINSTATE_OFF;
		}
	}

	//player win logic
	if(game->player.vehicle->checkpoint1==false && game->player.vehicle->index_track==cp1){
		game->player.vehicle->checkpoint1=true;
	}

	if(game->player.vehicle->checkpoint2==false && game->player.vehicle->checkpoint1==true && game->player.vehicle->index_track==cp2){
		game->player.vehicle->checkpoint2=true;
	}

	if(game->player.vehicle->checkpoint2==true && game->player.vehicle->index_track==cp1){
		game->player.vehicle->checkpoint2=false;
	}

	if(game->player.vehicle->checkpoint2==true&&game->player.vehicle->checkpoint1==true&&game->player.vehicle->index_track==game->track.num_pathpoints-1){
		game->player.vehicle->lap++;
		printf("Player is on lap %d\n", game->player.vehicle->lap);
		game->player.vehicle->checkpoint1=false;
		game->player.vehicle->checkpoint2=false;
	}

	//AI win logic
	for(int i=0;i<=game->amountAI;i++){
		if(game->aiplayer[i].vehicle->checkpoint1==false && game->aiplayer[i].vehicle->index_track==cp1){
			game->aiplayer[i].vehicle->checkpoint1=true;
		}

		if(game->aiplayer[i].vehicle->checkpoint2==false && game->aiplayer[i].vehicle->checkpoint1==true && game->aiplayer[i].vehicle->index_track==cp2){
			game->aiplayer[i].vehicle->checkpoint2=true;
		}

		if(game->aiplayer[i].vehicle->checkpoint2==true && game->aiplayer[i].vehicle->index_track==cp1){
			game->aiplayer[i].vehicle->checkpoint2=false;
		}

		if(game->aiplayer[i].vehicle->checkpoint2==true&&game->aiplayer[i].vehicle->checkpoint1==true&&game->aiplayer[i].vehicle->index_track==game->track.num_pathpoints-1){
			game->aiplayer[i].vehicle->lap++;
			printf("AI %d is on lap %d\n", i, game->aiplayer[i].vehicle->lap);
			game->aiplayer[i].vehicle->checkpoint1=false;
			game->aiplayer[i].vehicle->checkpoint2=false;
		}
	}
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

		// check if someone has won game
		if(game->winstate == GAME_WINSTATE_ON){
			checkwin(game);
		}
	}
}

void game_shutdown(struct game* game)
{
	// delete players
	player_delete(&game->player, &game->vehiclemanager);
	for (int i=0;i<=game->amountAI;i++){
		aiplayer_delete(&game->aiplayer[i], &game->vehiclemanager);
	}
	
	// delete world objects
	track_delete(&game->track);
	skybox_delete(&game->skybox);

	// shut down all the game subsytems
	audiomanager_shutdown(&game->audiomanager);
	inputmanager_shutdown(&game->inputmanager);
	physicsmanager_shutdown(&game->physicsmanager);
	texturemanager_shutdown(&game->texturemanager);
	
	glfwDestroyWindow(game->window.w);
	glfwTerminate();
}