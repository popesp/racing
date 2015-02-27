#include	"player.h"

#include	"../mem.h"


static void resetcontroller(struct aiplayer* p)
{
	int i;

	// reset button states
	for (i = 0; i < INPUT_CONTROLLER_BUTTONS; i++)
		p->controller.buttons[i] = 0;

	// reset axis states
	for (i = 0; i < INPUT_CONTROLLER_AXES; i++)
		p->controller.axes[i] = 0.f;
}


void player_init(struct player* p, struct physicsmanager* pm, struct renderer* r, struct controller* controller, struct track* t, int index_track)
{
	vec3f zero, up;

	// generate cart and send mesh to OpengL
	cart_init(&p->cart, pm, t, index_track);
	cart_generatemesh(&p->cart, r);
	renderable_sendbuffer(r, &p->cart.r_cart);

	p->cart.controller = controller;

	vec3f_set(zero, 0.f, 0.f, 0.f);
	vec3f_set(up, 0.f, 1.f, 0.f);

	camera_init(&p->camera, zero, zero, up);
}

void aiplayer_init(struct aiplayer* p, struct physicsmanager* pm, struct renderer* r, struct track* t, int index_track)
{
	// generate cart and send mesh to OpengL
	cart_init(&p->cart, pm, t, index_track);
	cart_generatemesh(&p->cart, r);
	renderable_sendbuffer(r, &p->cart.r_cart);

	p->controller.flags = INPUT_FLAG_ENABLED;

	p->controller.num_buttons = INPUT_CONTROLLER_BUTTONS;
	p->controller.num_axes = INPUT_CONTROLLER_AXES;

	p->controller.buttons = (unsigned char*)mem_alloc(sizeof(unsigned char) * INPUT_CONTROLLER_BUTTONS);
	p->controller.axes = (float*)mem_alloc(sizeof(float) * INPUT_CONTROLLER_AXES);

	// connect controller to the cart
	p->cart.controller = &p->controller;

	resetcontroller(p);
}


void player_delete(struct player* p)
{
	cart_delete(&p->cart);
}

void aiplayer_delete(struct aiplayer* p)
{
	cart_delete(&p->cart);

	mem_free(p->controller.buttons);
	mem_free(p->controller.axes);
}


void aiplayer_updateinput(struct aiplayer* p)
{
	resetcontroller(p);

	p->controller.axes[INPUT_AXIS_TRIGGERS] = -1.f;
}


void player_updatecamera(struct player* p)
{
	vec3f diff, up;

	physx::PxMat44 t_player(p->cart.vehicle->body->getGlobalPose());
	physx::PxVec3 targetpos = t_player.transform(physx::PxVec3(PLAYER_CAMERA_TARGETPOS));

	vec3f_subtractn(diff, (float*)&targetpos, p->camera.pos);
	vec3f_scale(diff, PLAYER_CAMERA_EASING);
	vec3f_add(p->camera.pos, diff);

	vec3f_set(up, 0.f, 1.f, 0.f);
	camera_lookat(&p->camera, (float*)&t_player.getPosition(), up);
}


void player_update(struct player* p, struct track* t)
{
	cart_update(&p->cart, t);
}

void aiplayer_update(struct aiplayer* p, struct track* t)
{
	cart_update(&p->cart, t);
}