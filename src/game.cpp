#include	"game.h"

#include	<GL/glew.h>				// GL
#include	<GLFW/glfw3.h>			// GL
#include	<stdlib.h>				// calloc, free TEMPORARY
#include	"debug.h"				// printvec3f
#include	"error.h"				// PRINT_ERROR
#include	"math/mat4f.h"			// identity TEMPORARY
#include	"math/vec3f.h"			// set TEMPORARY
#include	"objects/cart.h"		// init, generatemesh
#include	"objects/track.h"		// init, generatemesh
#include	"physics/physics.h"		// startup, shutdown
#include	"render/render.h"		// renderer: init
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

		default:
			break;
		}
	}
}

static void cursor(GLFWwindow* window, double x, double y)
{
	struct game* game;
	double dx, dy;

	game = (struct game*)glfwGetWindowUserPointer(window);

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
}

static void mouse(GLFWwindow* window, int button, int action, int mods)
{
	struct game* game;

	(void)mods;

	game = (struct game*)glfwGetWindowUserPointer(window);

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
}

static void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
	struct game* game;

	(void)xoffset;

	game = (struct game*)glfwGetWindowUserPointer(window);

	game->zoom += (float)yoffset*GAME_ZOOMSPEED;

	if (game->zoom > GAME_ZOOMMAX)
		game->zoom = GAME_ZOOMMAX;
	else if (game->zoom < GAME_ZOOMMIN)
		game->zoom = GAME_ZOOMMIN;
}

static void update(struct game* game)
{
	// check for callback events
	glfwPollEvents();

	physics_update(&game->physics, 1.f/600.f);

	physx::PxTransform playerT = game->player.p_cart->getGlobalPose();

	// check for window close messages
	if (glfwWindowShouldClose(game->window.w))
		game->flags |= GAME_FLAG_TERMINATED;
}

static void render(struct game* game)
{
	mat4f modelview, player;
	physx::PxMat44 player_world(game->player.p_cart->getGlobalPose());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4f_identity(modelview);
	mat4f_translatemul(modelview, 0.f, 0.f, -GAME_ZOOMSCALE / game->zoom);
	mat4f_rotatexmul(modelview, game->rotx);
	mat4f_rotateymul(modelview, game->roty);

	// render track
	renderable_render(&game->renderer, &game->track.r_track, modelview, 0);

	// render player
	mat4f_multiplyn(player, modelview, (float*)&player_world.column0);
	renderable_render(&game->renderer, &game->player.r_cart, player, 0);

	// render control points
	glClear(GL_DEPTH_BUFFER_BIT);
	renderable_render(&game->renderer, &game->track.r_curve, modelview, 0);
	renderable_render(&game->renderer, &game->track.r_controlpoints, modelview, 0);

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
	GLenum err;

	// initialize GLFW
	if (glfwInit() != GL_TRUE)
		return 0;

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

	// initialize GLEW
	glewExperimental = GL_TRUE;
	err = glewInit();
	if (err != GLEW_OK)
	{
		PRINT_ERROR("game.c", "%s\n", glewGetErrorString(err));
		return 0;
	}

	// vertical sync
	glfwSwapInterval(1);

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
		return 0;

	physics_startup(&game->physics);

	// initialize track object
	vec3f up;
	vec3f_set(up, 0.f, 1.f, 0.f);
	track_init(&game->track, up, &game->physics, TRACK_FLAG_LOOPED);

	game->track.num_points = 13;
	game->track.points = (struct track_point*)calloc(game->track.num_points, sizeof(struct track_point));

	vec3f pos, tan;

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
	trackpoint(game->track.points + 4, pos, tan, 20.f, 20.f, 5.f, 10);

	vec3f_set(pos, 15.f, -2.f, -25.f);
	vec3f_set(tan, -4.f, -1.f, 4.f);
	trackpoint(game->track.points + 5, pos, tan, 20.f, 20.f, 5.f, 10);

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
	physics_addstatic_trianglestrip(&game->physics, game->track.r_track.num_verts, sizeof(float)*RENDER_VERTSIZE_SOLID, game->track.r_track.buf_verts);
	renderable_sendbuffer(&game->renderer, &game->track.r_track);
	renderable_sendbuffer(&game->renderer, &game->track.r_controlpoints);
	renderable_sendbuffer(&game->renderer, &game->track.r_curve);

	vec3f_set(pos, 10.f, 10.f, -40.f);
	cart_init(&game->player, &game->physics, pos);

	cart_generatemesh(&game->renderer, &game->player);
	renderable_sendbuffer(&game->renderer, &game->player.r_cart);

	// light position in model space
	vec3f_set(game->track_lights[0].pos, 0.f, 100.f, 0.f);
	vec3f_set(game->track_lights[0].dif, 1.f, 1.f, 1.f);
	vec3f_set(game->track_lights[0].spc, 1.f, 1.f, 1.f);

	vec3f_set(game->track_lights[1].pos, 0.f, 0.f, 0.f);
	vec3f_set(game->track_lights[1].dif, 1.f, 1.f, 1.f);
	vec3f_set(game->track_lights[1].spc, 1.f, 1.f, 1.f);

	game->track.r_track.lights[0] = game->track_lights + 0;
	game->track.r_track.lights[1] = game->track_lights + 1;

	game->player.r_cart.lights[0] = game->track_lights + 0;
	game->player.r_cart.lights[1] = game->track_lights + 1;

	game->rotx = 20.f;
	game->roty = 0.f;
	game->zoom = 1.;

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
	physics_shutdown(&game->physics);

	track_delete(&game->track);

	glfwDestroyWindow(game->window.w);
	glfwTerminate();
}