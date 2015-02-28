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


void player_init(struct player* p, struct vehiclemanager* vm, controller* controller, int index_track, vec3f offs)
{
	vec3f zero, up;

	// initialize vehicle
	p->vehicle = vehiclemanager_newvehicle(vm, index_track, offs);

	p->vehicle->controller = controller;

	vec3f_set(zero, 0.f, 0.f, 0.f);
	vec3f_set(up, 0.f, 1.f, 0.f);

	camera_init(&p->camera, zero, zero, up);

	//initialize lap
	p->vehicle->lap = 1;
	p->vehicle->checkpoint1 = false;
	p->vehicle->checkpoint2 = false;
	p->vehicle->checkpoint3 = false;

}

void aiplayer_init(struct aiplayer* p, struct vehiclemanager* vm, int index_track, vec3f offs)
{
	// initialize vehicle
	p->vehicle = vehiclemanager_newvehicle(vm, index_track, offs);

	p->track = vm->track;

	p->controller.flags = INPUT_FLAG_ENABLED;

	p->controller.num_buttons = INPUT_CONTROLLER_BUTTONS;
	p->controller.num_axes = INPUT_CONTROLLER_AXES;

	p->controller.buttons = (unsigned char*)mem_alloc(sizeof(unsigned char) * INPUT_CONTROLLER_BUTTONS);
	p->controller.axes = (float*)mem_alloc(sizeof(float) * INPUT_CONTROLLER_AXES);

	// connect controller to the cart
	p->vehicle->controller = &p->controller;

	resetcontroller(p);

	//initialize lap
	p->vehicle->lap = 1;
	p->vehicle->checkpoint1 = false;
	p->vehicle->checkpoint2 = false;
	p->vehicle->checkpoint3 = false;

}


void player_delete(struct player* p, struct vehiclemanager* vm)
{
	vehiclemanager_removevehicle(vm, p->vehicle);
}

void aiplayer_delete(struct aiplayer* p, struct vehiclemanager* vm)
{
	vehiclemanager_removevehicle(vm, p->vehicle);

	mem_free(p->controller.buttons);
	mem_free(p->controller.axes);
}


void aiplayer_updateinput(struct aiplayer* p)
{
	vec3f next_point, right, diff;
	int next_index;

	physx::PxMat44 pose(p->vehicle->body->getGlobalPose());

	resetcontroller(p);

	next_index = (p->vehicle->index_track + 3) % p->track->num_pathpoints;
	vec3f_copy(next_point, p->track->pathpoints[next_index]);

	// future point based on current speed

	vec3f_subtractn(diff, next_point, p->vehicle->pos);

	vec3f_set(right, VEHICLE_RIGHT);
	mat4f_transformvec3f(right, (float*)&pose);

	p->controller.axes[INPUT_AXIS_LEFT_LR] = vec3f_dot(right, diff) * 4.f / vec3f_length(diff);

	p->controller.axes[INPUT_AXIS_TRIGGERS] = -0.8f;
}


void player_updatecamera(struct player* p)
{
	vec3f diff, up;

	physx::PxMat44 t_player(p->vehicle->body->getGlobalPose());
	physx::PxVec3 targetpos = t_player.transform(physx::PxVec3(PLAYER_CAMERA_TARGETPOS));

	vec3f_subtractn(diff, (float*)&targetpos, p->camera.pos);
	vec3f_scale(diff, PLAYER_CAMERA_EASING);
	vec3f_add(p->camera.pos, diff);

	vec3f_set(up, 0.f, 1.f, 0.f);
	camera_lookat(&p->camera, (float*)&t_player.getPosition(), up);
}