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

#include	"../objects/entities.h"

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

	vehicle_reset(&game->vehiclemanager, game->player.vehicle);

	for (i = 0; i < game->num_aiplayers; i++)
		vehicle_reset(&game->vehiclemanager, game->aiplayers[i].vehicle);
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
			audiomanager_stopsound(game->currentchannel);
			game->index_currentsong = (game->index_currentsong + 1) % GAME_MUSIC_COUNT;
			game->currentchannel = audiomanager_playmusic(&game->audiomanager, game->index_currentsong, -1);
			break;

		case GLFW_KEY_P:
			// pause the music
			audiomanager_togglesound(game->currentchannel);
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
			// toggle win condition
			if (game->flags & GAME_FLAG_WINCONDITION)
			{
				printf("Win condition deactivated.\n");
				game->flags &= ~GAME_FLAG_WINCONDITION;
			} else
			{
				printf("Win condition activated.\n");
				printf("Player is on lap %d\n", game->player.vehicle->lap);

				for (i = 0; i < game->num_aiplayers; i++)
				{
					game->aiplayers[i].vehicle->lap = 1;
					printf("Computer-%d is on lap %d\n", i+1, game->aiplayers[i].vehicle->lap);
				}

				game->flags |= GAME_FLAG_WINCONDITION;
			}
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
		game->player.vehicle->controller = &game->inputmanager.controllers[GLFW_JOYSTICK_1];

	audiomanager_update(&game->audiomanager, game->player.camera.pos, game->player.camera.dir, game->player.camera.up);

	// update the vehicles
	vehiclemanager_update(&game->vehiclemanager);



	// reset hit flags
	for (i = 0; i < VEHICLE_COUNT; i++) {
		game->vehiclemanager.vehicles[i].hit_flag = 0;
	}
	for (i = 0; i < ENTITY_MISSILE_COUNT; i++) {
		game->vehiclemanager.em->missiles[i].hit = 0;
	}
	for (i = 0; i < ENTITY_MINE_COUNT; i++) {
		game->vehiclemanager.em->mines[i].hit = 0;
	}
	physicsmanager_update(&game->physicsmanager, GAME_SPU);

	// update the game objects
	entitymanager_update(&game->entitymanager, &game->vehiclemanager);

	player_updatecamera(&game->player);

	

	//update pickup position if vehicle is holding
	for(i = 0; i < ENTITY_PICKUP_COUNT; i++){
		if(game->entitymanager.pickups[i].owner!=NULL){
			physx::PxMat44 owner_vehicle(game->entitymanager.pickups[i].owner->body->getGlobalPose());
			game->entitymanager.pickups[i].powerpos = owner_vehicle.transform(physx::PxVec3(-.3f, -.1f, 1.5f));
			game->entitymanager.pickups[i].body->setGlobalPose(physx::PxTransform(physx::PxVec3(game->entitymanager.pickups[i].powerpos)));

			//physx::PxMat44 owner_angle()
			game->entitymanager.pickups[i].body->setAngularDamping(game->entitymanager.pickups[i].owner->body->getAngularDamping());
		}
	}


	//check who has won the game
	if(game->flags == GAME_FLAG_WINCONDITION){
		checkwin(game);
		checkplace(game);
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
	int i;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	player_mw = physx::PxMat44(game->player.vehicle->body->getGlobalPose());
	renderable_render(&game->renderer, &game->vehiclemanager.r_vehicle, (float*)&player_mw, global_wv, 0);
	for(i = 0; i < game->num_aiplayers; i++)
	{
		aiplayer_mw = physx::PxMat44(game->aiplayers[i].vehicle->body->getGlobalPose());
		renderable_render(&game->renderer, &game->vehiclemanager.r_vehicle, (float*)&aiplayer_mw, global_wv, 0);	
	}

	// render all game objects
	entitymanager_render(&game->entitymanager, &game->renderer, global_wv);

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
	skybox_init(&game->skybox, &game->renderer);

	// initialize track object
	track_init(&game->track, &game->physicsmanager, up);
	track_loadpointsfile(&game->track, "res/tracks/turn1.track");
	track_generate(&game->renderer, &game->track);
	renderable_sendbuffer(&game->renderer, &game->track.r_track);

	// send track mesh to physX
	physicsmanager_addstatic_trianglestrip(&game->physicsmanager, game->track.r_track.num_verts, sizeof(float)*RENDER_VERTSIZE_BUMP_L, game->track.r_track.buf_verts);
	
	// start up the entity manager for the track
	entitymanager_startup(&game->entitymanager, &game->physicsmanager, &game->renderer,&game->audiomanager, &game->track);

	// start up the vehicle manager for the track
	vehiclemanager_startup(&game->vehiclemanager, &game->physicsmanager, &game->entitymanager, &game->audiomanager, &game->renderer, &game->track);

	// initialize player objects
	vec3f_set(offs, 0.f, 0.f, 0.f);
	if (game->inputmanager.controllers[0].flags & INPUT_FLAG_ENABLED)
		player_init(&game->player, &game->vehiclemanager, &game->inputmanager.controllers[0], 0, offs);
	else
		player_init(&game->player, &game->vehiclemanager, &game->inputmanager.keyboard, 0, offs);
	aiplayer_init(&game->aiplayers[0], &game->vehiclemanager, 5, offs);
	game->num_aiplayers = 1;

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
	game->songs[GAME_MUSIC_1_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_1_FILENAME);
	game->songs[GAME_MUSIC_2_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_2_FILENAME);
	game->songs[GAME_MUSIC_3_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_3_FILENAME);
	game->songs[GAME_MUSIC_4_ID] = audiomanager_newmusic(&game->audiomanager, GAME_MUSIC_4_FILENAME);
	game->index_currentsong = 0;
	game->currentchannel = audiomanager_playmusic(&game->audiomanager, game->songs[game->index_currentsong], -1);
	audiomanager_setmusicvolume(&game->audiomanager,0.5);
	game->flags = GAME_FLAG_INIT;


	//spawn the tracks pickups
	entitymanager_newpickup(&game->entitymanager,game->track.pathpoints[PICKUP_SPAWN_LOC1].pos);
	game->entitymanager.pickupatspawn1=true;
	
	entitymanager_newpickup(&game->entitymanager,game->track.pathpoints[PICKUP_SPAWN_LOC2].pos);
	game->entitymanager.pickupatspawn2=true;

	entitymanager_newpickup(&game->entitymanager,game->track.pathpoints[PICKUP_SPAWN_LOC3].pos);
	game->entitymanager.pickupatspawn3=true;


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
	int i;

	// delete players
	player_delete(&game->player, &game->vehiclemanager);
	for (i = 0; i < game->num_aiplayers; i++)
		aiplayer_delete(&game->aiplayers[i], &game->vehiclemanager);
	
	// delete world objects
	track_delete(&game->track);
	skybox_delete(&game->skybox);

	// shut down all the game subsytems
	audiomanager_shutdown(&game->audiomanager);
	inputmanager_shutdown(&game->inputmanager);
	physicsmanager_shutdown(&game->physicsmanager);
	
	glfwDestroyWindow(game->window.w);
	glfwTerminate();
}