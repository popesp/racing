#include	"game.h"

#include	<GL/glew.h>				// GL
#include	<GLFW/glfw3.h>			// GL
#include	<stdio.h>				// printf
#include	<stdlib.h>				// calloc, free TEMPORARY
#include	"debug.h"				// printvec3f
#include	"error.h"				// PRINT_ERROR
#include	"input.h"
#include	"math/mat4f.h"			// identity TEMPORARY
#include	"math/vec3f.h"			// set TEMPORARY
#include	"objects/camera.h"
#include	"objects/cart.h"		// init, generatemesh
#include	"objects/track.h"		// init, generatemesh
#include	"physics/physics.h"		// startup, shutdown
#include	"render/render.h"		// renderer: init
#include	"render/texture.h"
#include	"render/window.h"		// init, resize



static void resize(GLFWwindow*, int, int);
static void keyboard(GLFWwindow*, int, int, int, int);
static void cursor(GLFWwindow*, double, double);
static void mouse(GLFWwindow*, int, int, int);
static void scroll(GLFWwindow*, double, double);
static void update(struct game*);
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

		case GLFW_KEY_W:
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

	/*
	if (game->flags & GAME_FLAG_ROTATING)
	{
		dx = x - game->mx;
		dy = y - game->my;

		game->rotx += (float)(dy * GAME_ROTATEXSPEED);
		game->roty += (float)(dx * GAME_ROTATEYSPEED);

		while (game->roty > 360.f)
			game->roty -= 360.f;

		while (game->roty < 0.f)
			game->roty += 360.f;

		if (game->rotx > GAME_ROTATEXMAX)
			game->rotx = GAME_ROTATEXMAX;
		else if (game->rotx < GAME_ROTATEXMIN)
			game->rotx = GAME_ROTATEXMIN;
	}

	game->mx = x;
	game->my = y;
	*/
}

static void mouse(GLFWwindow* window, int button, int action, int mods)
{
	struct game* game;

	(void)button;
	(void)action;
	(void)mods;

	game = (struct game*)glfwGetWindowUserPointer(window);

	/*
	if (action == GLFW_PRESS)
	{
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			game->flags |= GAME_FLAG_ROTATING;
	}
	else
	{
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			game->flags &= ~GAME_FLAG_ROTATING;
	}
	*/
}

static void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	struct game* game;

	(void)xoffset;
	(void)yoffset;

	game = (struct game*)glfwGetWindowUserPointer(window);

	/*
	game->zoom += (float)yoffset*GAME_ZOOMSPEED;

	if (game->zoom > GAME_ZOOMMAX)
		game->zoom = GAME_ZOOMMAX;
	else if (game->zoom < GAME_ZOOMMIN)
		game->zoom = GAME_ZOOMMIN;
	*/
}

static void update(struct game* game)
{
	vec3f up, move;

	// check for callback events
	glfwPollEvents();

	inputmanager_update(&game->inputmanager);

	// simulate
	physicsmanager_update(&game->physicsmanager, GAME_SPU);

	vec3f_set(up, 0.f, 1.f, 0.f);

	// update debug camera; NOTE: a lot of this is temporarily hard-coded
	if (game->flags & GAME_FLAG_DEBUGCAM)
	{
		vec3f_copy(move, game->cam_debug.dir);
		move[VY] = 0.f;
		vec3f_normalize(move);
		vec3f_scale(move, -0.2f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_LEFT_UD]);
		move[VY] = - 0.1f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_TRIGGERS];
		vec3f_add(game->cam_debug.pos, move);

		camera_strafe(&game->cam_debug, 0.2f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_LEFT_LR]);

		camera_rotate(&game->cam_debug, up, -0.03f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_RIGHT_LR]);
		camera_rotate(&game->cam_debug, game->cam_debug.right, -0.03f * game->inputmanager.controllers[GLFW_JOYSTICK_1].axes[INPUT_AXIS_RIGHT_UD]);
	}

	// update player camera
	physx::PxMat44 t_player(game->player.p_cart->getGlobalPose());
	physx::PxVec3 campos = t_player.transform(physx::PxVec3(0.f, 1.f, 5.f));
	vec3f_copy(game->cam_player.pos, (float*)&campos);
	camera_lookat(&game->cam_player, (float*)&t_player.getPosition(), up);

	// check for window close messages
	if (glfwWindowShouldClose(game->window.w))
		game->flags |= GAME_FLAG_TERMINATED;
}

static void render(struct game* game)
{
	mat4f world_view, model_world;
	physx::PxMat44 player_world(game->player.p_cart->getGlobalPose());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// get camera transform
	if (game->flags & GAME_FLAG_DEBUGCAM)
		camera_gettransform(&game->cam_debug, world_view);
	else
		camera_gettransform(&game->cam_player, world_view);

	// render track
	mat4f_identity(model_world);
	renderable_render(&game->renderer, &game->track.r_track, model_world, world_view, 0);

	// render player
	renderable_render(&game->renderer, &game->player.r_cart, (float*)&player_world.column0, world_view, 0);

	glfwSwapBuffers(game->window.w);
}

static void trackpoint(struct track_point* p, vec3f pos, vec3f tan, float angle, float weight, float width, unsigned subdivisions)
{
	vec3f_copy(p->pos, pos);

	vec3f_copy(p->tan, tan);
	vec3f_normalize(p->tan);

	p->angle = angle;
	p->weight = weight;
	p->width = width;
	p->subdivisions = subdivisions;
}

int game_startup(struct game* game)
{
	vec3f up, dir, pos, tan;
	int major, minor, rev;
	GLenum err;

	// initialize GLFW
	if (glfwInit() != GL_TRUE)
		return 0;
	glfwGetVersion(&major, &minor, &rev);
	printf("GLFW initialized, using version %d.%d.%d\n", major, minor, rev);

	// initialize window object
	window_init(&game->window, GAME_DEFAULT_WIDTH, GAME_DEFAULT_HEIGHT, WINDOW_FLAG_NONE);

	// ensure a compatible context
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_SAMPLES, 16);				// 4X FSAA

	// create window handle
	if ((game->window.w = glfwCreateWindow(game->window.width, game->window.height, GAME_TITLE, NULL, NULL)) == NULL)
		return 0;

	// move to default screen position
	glfwSetWindowPos(game->window.w, GAME_DEFAULT_X, GAME_DEFAULT_Y);
	glfwShowWindow(game->window.w);

	// make window's opengl context current
	glfwMakeContextCurrent(game->window.w);
	glfwSwapInterval(1); // vertical sync

	// print OpenGL version context
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("OpenGL context initialized, using version %d.%d\n", major, minor);

	// initialize GLEW
	glewExperimental = GL_TRUE;
	err = glewInit();
	if (err != GLEW_OK)
	{
		PRINT_ERROR("game.c", "%s\n", glewGetErrorString(err));
		return 0;
	}
	printf("GLEW initialized, using version %s\n", glewGetString(GLEW_VERSION));

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
	texturemanager_startup(&game->texturemanager);
	printf("FreeImage initialized, using version %s\n", texturemanager_getlibversion(&game->texturemanager));

	// initialize renderer
	if (!renderer_init(&game->renderer, &game->texturemanager, &game->window))
		return 0;

	// initialize physics manager
	physicsmanager_startup(&game->physicsmanager);
	printf("PhysX initialized, using version %d.%d.%d\n", PX_PHYSICS_VERSION_MAJOR, PX_PHYSICS_VERSION_MINOR, PX_PHYSICS_VERSION_BUGFIX);

	// initialize input manager
	inputmanager_startup(&game->inputmanager);

	// print connected joystick information
	for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++)
	{
		if (game->inputmanager.controllers[GLFW_JOYSTICK_1+i].flags & INPUT_FLAG_ENABLED)
			printf("Joystick %d enabled. Name: %s\n", i+1, inputmanager_joystickname(&game->inputmanager, i));
	}

	/* ----- BEGIN GAME STATE INITIALIZATION ----- */

	// initialize track object
	vec3f_set(up, 0.f, 1.f, 0.f);
	track_init(&game->track, up, &game->physicsmanager, TRACK_FLAG_LOOPED);
/*
	std::ifstream trackfile;
	trackfile.open("track1.txt");

	trackfile >> game->track.num_points;
	game->track.points = (struct track_point*)calloc(game->track.num_points, sizeof(struct track_point));

	{
	float px, py, pz, tx, ty, tz, angle, weight, width;
	unsigned subdiv, ind;

	for(int i = 0; i<game->track.num_points; i++){
		trackfile >> ind>> px >> py >> pz >> tx >>ty >> tz >>angle >> weight >> width >> subdiv;
		vec3f_set(pos, px, py, pz);
		vec3f_set(tan, tx, ty, tz);
		trackpoint(game->track.points + ind, pos, tan, angle, weight, width, subdiv);
	}
	}*/

	game->track.num_points = 13;
	game->track.points = (struct track_point*)calloc(game->track.num_points, sizeof(struct track_point));

	vec3f_set(pos, -5.f, 0.f, 25.f);
	vec3f_set(tan, -1.f, 0.f, 0.f);
	trackpoint(game->track.points + 0, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, -15.f, 0.f, 15.f);
	vec3f_set(tan, 0.f, 0.f, -1.f);
	trackpoint(game->track.points + 1, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, -15.f, 0.f, -30.f);
	vec3f_set(tan, 0.f, 0.f, -1.f);
	trackpoint(game->track.points + 2, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, -5.f, 0.f, -40.f);
	vec3f_set(tan, 1.f, 0.f, 0.f);
	trackpoint(game->track.points + 3, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, 10.f, 0.f, -40.f);
	vec3f_set(tan, 1.f, 0.f, 0.f);
	trackpoint(game->track.points + 4, pos, tan, 0.34907f, 20.f, 5.f, 10);

	vec3f_set(pos, 15.f, -2.f, -25.f);
	vec3f_set(tan, -4.f, -1.f, 4.f);
	trackpoint(game->track.points + 5, pos, tan, 0.34907f, 20.f, 5.f, 10);

	vec3f_set(pos, -5.f, -15.f, -5.f);
	vec3f_set(tan, -4.f, -1.f, 4.f);
	trackpoint(game->track.points + 6, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, -5.f, -16.f, 10.f);
	vec3f_set(tan, 1.f, 0.f, 1.f);
	trackpoint(game->track.points + 7, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, 10.f, -15.f, 10.f);
	vec3f_set(tan, 4.f, 1.f, -4.f);
	trackpoint(game->track.points + 8, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, 30.f, -2.f, -10.f);
	vec3f_set(tan, 4.f, 1.f, -4.f);
	trackpoint(game->track.points + 9, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, 45.f, 0.f, -5.f);
	vec3f_set(tan, 0.f, 0.f, 1.f);
	trackpoint(game->track.points + 10, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, 45.f, 0.f, 15.f);
	vec3f_set(tan, 0.f, 0.f, 1.f);
	trackpoint(game->track.points + 11, pos, tan, 0.f, 20.f, 5.f, 10);

	vec3f_set(pos, 35.f, 0.f, 25.f);
	vec3f_set(tan, -1.f, 0.f, 0.f);
	trackpoint(game->track.points + 12, pos, tan, 0.f, 20.f, 5.f, 10);
	
	track_generatemesh(&game->renderer, &game->track);

	physicsmanager_addstatic_trianglestrip(&game->physicsmanager, game->track.r_track.num_verts, sizeof(float)*RENDER_VERTSIZE_BUMPM, game->track.r_track.buf_verts);
	renderable_sendbuffer(&game->renderer, &game->track.r_track);

	// initialize cart object
	//vec3f_set(pos, 10.f, 10.f, -40.f);
	vec3f_set(pos, 35.f, 2.f, 25.f);
	cart_init(&game->player, &game->physicsmanager, pos);
	cart_generatemesh(&game->renderer, &game->player);
	renderable_sendbuffer(&game->renderer, &game->player.r_cart);

	// initialize camera objects
	vec3f_set(pos, 0.f, 0.f, -30.f);
	vec3f_set(dir, 0.f, 0.f, 0.f);
	camera_init(&game->cam_debug, pos, dir, up);
	camera_init(&game->cam_player, pos, dir, up);

	// initialize lights
	vec3f_set(game->track_lights[0].pos, 0.f, 10.f, 0.f);
	vec3f_set(game->track_lights[0].dif, 1.f, 1.f, 1.f);
	vec3f_set(game->track_lights[0].spc, 1.f, 1.f, 1.f);

	vec3f_set(game->track_lights[1].pos, 0.f, 0.f, 0.f);
	vec3f_set(game->track_lights[1].dif, 1.f, 1.f, 1.f);
	vec3f_set(game->track_lights[1].spc, 1.f, 1.f, 1.f);

	// give renderable objects references to the light objects
	game->track.r_track.lights[0] = game->track_lights + 0;
	//game->track.r_track.lights[1] = game->track_lights + 1;
	game->player.r_cart.lights[0] = game->track_lights + 0;
	//game->player.r_cart.lights[1] = game->track_lights + 1;

	game->tex_trackbump = texturemanager_newtexture(&game->texturemanager);
	texture_loadfile(&game->texturemanager, game->tex_trackbump, "res/rock.jpg");
	texture_upload(&game->texturemanager, game->tex_trackbump, RENDER_TEXTURE_NORMAL);
	game->track.r_track.texture_ids[RENDER_TEXTURE_NORMAL] = game->tex_trackbump;

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

void game_shutdown(struct game* game)
{
	track_delete(&game->track);

	// shut down all the game subsytems
	physicsmanager_shutdown(&game->physicsmanager);
	inputmanager_shutdown(&game->inputmanager);
	texturemanager_shutdown(&game->texturemanager);

	glfwDestroyWindow(game->window.w);
	glfwTerminate();
}