#include	"player.h"

#include	"../mem.h"
#include	"../objects/entities/missile.h"
#include	"../physics/physics.h"
#include	"../random.h"


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


void player_init(struct player* p, struct vehicle* v)
{
	vec3f zero, up;

	// initialize vehicle
	p->vehicle = v;

	vec3f_set(zero, 0.f, 0.f, 0.f);
	vec3f_set(up, 0.f, 1.f, 0.f);

	camera_init(&p->camera, zero, zero, up);

	p->anglecamera = 0.f;
}

void aiplayer_init(struct aiplayer* p, struct vehicle* v, struct track* track)
{
	// initialize vehicle
	p->vehicle = v;

	p->track = track;

	p->controller.flags = INPUT_FLAG_ENABLED;

	p->controller.num_buttons = INPUT_CONTROLLER_BUTTONS;
	p->controller.num_axes = INPUT_CONTROLLER_AXES;

	p->controller.buttons = (unsigned char*)mem_alloc(sizeof(unsigned char) * INPUT_CONTROLLER_BUTTONS);
	p->controller.axes = (float*)mem_alloc(sizeof(float) * INPUT_CONTROLLER_AXES);

	resetcontroller(p);

	p->timer_missile = 0;

	p->turn = 50.f/((float)random_int(15) + 7.5f);
	p->speed = (1.f / ((float)random_int(15) + 2.5f)) - 1.f;
}


void player_delete(struct player* p)
{
	// does nothing
	(void)p;
}

void aiplayer_delete(struct aiplayer* p)
{
	mem_free(p->controller.buttons);
	mem_free(p->controller.axes);
}


bool missiledetection(struct aiplayer* p, struct vehiclemanager* vm)
{
	vec3f dir, diff;
	physx::PxMat44 mat_pose;
	struct vehicle* v;
	unsigned i;
	float dist;

	mat_pose = physx::PxMat44(p->vehicle->body->getGlobalPose());
	vec3f_set(dir, VEHICLE_FORWARD);
	mat4f_transformvec3f(dir, (float*)&mat_pose);

	for (i = 0; i < VEHICLE_COUNT; i++)
	{
		v = vm->vehicles + i;

		// skip self
		if (v == p->vehicle)
			continue;

		// find normalized direction vector to each vehicle
		vec3f_subtractn(diff, v->pos, p->vehicle->pos);
		dist = vec3f_length(diff);
		vec3f_scale(diff, 1.f/dist);

		// if pointing sort of close to an opponent, and they are within range, shoot
		if (vec3f_dot(diff, dir) > 0.995f && dist < 100.f)
			return true;
	}

	return false;
}

void aiplayer_updateinput(struct aiplayer* p, struct vehiclemanager* vm, float difficulty)
{
	vec3f next_point, right, diff;
	int next_index;

	physx::PxMat44 pose(p->vehicle->body->getGlobalPose());

	resetcontroller(p);

	next_index = (p->vehicle->index_track + 5) % (int)p->track->num_pathpoints;
	vec3f_copy(next_point, p->track->pathpoints[next_index].pos);

	// IDEA: future point based on current speed

	vec3f_subtractn(diff, next_point, p->vehicle->pos);

	vec3f_set(right, VEHICLE_RIGHT);
	mat4f_transformvec3f(right, (float*)&pose);

	p->controller.axes[INPUT_AXIS_LEFT_LR] = vec3f_dot(right, diff) * p->turn / vec3f_length(diff);

	float aidiff = difficulty+0.5f;

	p->controller.axes[INPUT_AXIS_TRIGGERS] = p->speed*aidiff;

	if (p->timer_missile > 0)
		p->timer_missile--;

	if (p->vehicle->flags & VEHICLE_FLAG_HASPOWERUP)
	{
		switch (p->vehicle->powerup)
		{
		case VEHICLE_POWERUP_BOOST:
		case VEHICLE_POWERUP_LONGBOOST:
			if(p->vehicle->index_track==140||p->vehicle->index_track==160||p->vehicle->index_track==729)
				p->controller.buttons[INPUT_BUTTON_A] = (INPUT_STATE_CHANGED | INPUT_STATE_DOWN);
			break;

		case VEHICLE_POWERUP_MINE:
		case VEHICLE_POWERUP_MINEX2:
		case VEHICLE_POWERUP_MINEX3:
		case VEHICLE_POWERUP_TURRET:
		case VEHICLE_POWERUP_SLOWMINE:
			if (random_int(1500) == 0)
				p->controller.buttons[INPUT_BUTTON_A] = (INPUT_STATE_CHANGED | INPUT_STATE_DOWN);
			break;

		case VEHICLE_POWERUP_MISSILE:
		case VEHICLE_POWERUP_MISSILEX2:
		case VEHICLE_POWERUP_MISSILEX3:
			if (missiledetection(p, vm) && p->timer_missile == 0)
			{
				p->controller.buttons[INPUT_BUTTON_A] = (INPUT_STATE_CHANGED | INPUT_STATE_DOWN);
				p->timer_missile = AIPLAYER_MISSILE_COOLDOWN;
			}
			break;
		}
	}
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

void player_updatewincamera(struct player* p, struct vehicle* v)
{
	vec3f offs, up;
	mat4f rotate;

	vec3f_set(p->camera.pos, PLAYER_WINCAMERA_POS);
	mat4f_rotatey(rotate, p->anglecamera);
	mat4f_transformvec3f(p->camera.pos, rotate);

	physx::PxVec3 pos = v->body->getGlobalPose().p;
	vec3f_set(offs, pos.x, pos.y, pos.z);
	vec3f_add(p->camera.pos, offs);

	vec3f_set(up, 0.f, 1.f, 0.f);
	camera_lookat(&p->camera, offs, up);

	p->anglecamera += PLAYER_WINCAMERA_ROTATE;
}