#include	"player.h"

#include	"../mem.h"
#include	"../random.h"
#include	"../objects/entities/missile.h"

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
	p->vehicle = vehiclemanager_newvehicle(vm, controller, index_track, offs);

	vec3f_set(zero, 0.f, 0.f, 0.f);
	vec3f_set(up, 0.f, 1.f, 0.f);

	camera_init(&p->camera, zero, zero, up);

	//initialize lap
	p->vehicle->lap = 1;
	p->vehicle->checkpoint1 = false;
	p->vehicle->checkpoint2 = false;	
}

void aiplayer_init(struct aiplayer* p, struct vehiclemanager* vm, int index_track, vec3f offs)
{
	// initialize vehicle
	p->vehicle = vehiclemanager_newvehicle(vm, &p->controller, index_track, offs);

	p->track = vm->track;

	p->controller.flags = INPUT_FLAG_ENABLED;

	p->controller.num_buttons = INPUT_CONTROLLER_BUTTONS;
	p->controller.num_axes = INPUT_CONTROLLER_AXES;

	p->controller.buttons = (unsigned char*)mem_alloc(sizeof(unsigned char) * INPUT_CONTROLLER_BUTTONS);
	p->controller.axes = (float*)mem_alloc(sizeof(float) * INPUT_CONTROLLER_AXES);

	resetcontroller(p);

	p->turn = 50/(random_int(15)+7.5);
	p->speed = -(1-(1/(random_int(15)+5.0)));
	p->next = (random_int(4)+3);
	printf("turn=%f speed=%f next=%d\n",p->turn,p->speed,p->next);

	vec3f zero, up;
	vec3f_set(zero, 0.f, 0.f, 0.f);
	vec3f_set(up, 0.f, 1.f, 0.f);
	camera_init(&p->camera, zero, zero, up);
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

//Tries to predict if shooting a missile will result in a hit on another, quite basic and doesnt work
bool aiplayer_missilehit(struct aiplayer* p, struct vehiclemanager* vm) {

	struct vehicle* v;

	vec3f missile;
	physx::PxMat44 transform(p->vehicle->body->getGlobalPose());
	float x_accuracy_max = 10;
	float x_accuracy_min = -10;
	float y_accuracy_max = 10;
	float y_accuracy_min = -10;
	float z_accuracy_max = 50;
	float z_accuracy_min = -50;

	//The next 5 seconds of frames
	for (int i = 1; i <= 5; i++) {

		//Where a missile should be after i seconds, the 60 is for frames since we calculate this 60 times a second
		vec3f_set(missile, ENTITY_FORWARD);
		mat4f_transformvec3f(missile, (float*)&transform);
		vec3f_scale(missile, (MISSILE_SPEED*(float)i));

		for (int j = 0; j < VEHICLE_COUNT; j++) {
			v = vm->vehicles + j;

			if (!(v->flags & VEHICLE_FLAG_ENABLED)) {
				continue;
			}
			//Ignore this vehicle as it is the one shooting, no point calculating how to shoot yourself!
			if (v == p->vehicle) {
				continue;
			}
			//advance advance this vehicle i seconds into the future
			vec3f vehicle_position;
			physx::PxMat44 vehicle_transform(v->body->getGlobalPose());
			vec3f_set(vehicle_position, VEHICLE_FORWARD);
			mat4f_transformvec3f(vehicle_position, (float*)&vehicle_transform);
			vec3f_scale(vehicle_position, (v->speed*(float)i));

			//These check if the missile will be close to another vehicle after i seconds, if so shoot the missile
			if ((missile[0] - vehicle_position[0] < x_accuracy_max) && (missile[0] - vehicle_position[0] > x_accuracy_min)) {
				if ((missile[1] - vehicle_position[1] < y_accuracy_max) && (missile[1] - vehicle_position[1] > y_accuracy_min)) {
					if ((missile[2] - vehicle_position[2] < z_accuracy_max) && (missile[2] - vehicle_position[2] > z_accuracy_min)){
						//printf("Accuracy = %f %f %f\n", missile[0] - vehicle_position[0], missile[1] - vehicle_position[1], missile[2] - vehicle_position[2]);
						return true;
					}
				}
			}
			/*if ((missile[1] - vehicle_position[1] < accuracy_max) && (missile[1] - vehicle_position[1] > accuracy_min)) {
				printf("Firing witn an accuracy of: %f in the y\n", missile[1] - vehicle_position[1]);
				return true;
			}
			if ((missile[2] - vehicle_position[2] < accuracy_max) && (missile[2] - vehicle_position[2] > accuracy_min)) {
				printf("Firing witn an accuracy of: %f in the z\n", missile[2] - vehicle_position[2]);
				return true;
			}*/
		}
	}

	return false;
}

void aiplayer_updateinput(struct aiplayer* p, struct vehiclemanager* vm, int aidifficulty)
{
	vec3f next_point, right, diff;
	int next_index;

	float difficultyspeed;

	if(aidifficulty == GAME_DIFFICULTY_EASY)
		difficultyspeed = 0.9f;
	else if(aidifficulty == GAME_DIFFICULTY_NORMAL)
		difficultyspeed = 1.0f;
	else
		difficultyspeed = 1.1f;//Hard

	physx::PxMat44 pose(p->vehicle->body->getGlobalPose());
	resetcontroller(p);

	next_index = (p->vehicle->index_track + p->next) % (int)p->track->num_pathpoints;
	vec3f_copy(next_point, p->track->pathpoints[next_index].pos);

	// IDEA: future point based on current speed
	vec3f_subtractn(diff, next_point, p->vehicle->pos);

	vec3f_set(right, VEHICLE_RIGHT);
	mat4f_transformvec3f(right, (float*)&pose);

	p->controller.axes[INPUT_AXIS_LEFT_LR] = vec3f_dot(right, diff) * p->turn / vec3f_length(diff);

	p->controller.axes[INPUT_AXIS_TRIGGERS] = (p->speed)*difficultyspeed;

	if((p->vehicle->powerup==VEHICLE_POWERUP_BOOST||p->vehicle->powerup==VEHICLE_POWERUP_LONGBOOST)&&(p->vehicle->flags & VEHICLE_FLAG_HASPOWERUP)){
		if(p->vehicle->index_track==140||p->vehicle->index_track==160||p->vehicle->index_track==729){
			p->controller.buttons[INPUT_BUTTON_A] = (INPUT_STATE_CHANGED | INPUT_STATE_DOWN);
		}
	}
	else if((p->vehicle->powerup==VEHICLE_POWERUP_SLOWMINE||p->vehicle->powerup==VEHICLE_POWERUP_MINE||p->vehicle->powerup==VEHICLE_POWERUP_TURRET||p->vehicle->powerup==VEHICLE_POWERUP_MINEX2||p->vehicle->powerup==VEHICLE_POWERUP_MINEX3)&&(p->vehicle->flags & VEHICLE_FLAG_HASPOWERUP)){
		int usemine = random_int(1500);
		if(usemine%1499==0){
			p->controller.buttons[INPUT_BUTTON_A] = (INPUT_STATE_CHANGED | INPUT_STATE_DOWN);
		}
	}
	else if((p->vehicle->powerup==VEHICLE_POWERUP_MISSILE||p->vehicle->powerup==VEHICLE_POWERUP_MISSILEX2||p->vehicle->powerup==VEHICLE_POWERUP_MISSILEX3)&&(p->vehicle->flags & VEHICLE_FLAG_HASPOWERUP)){
		if(aiplayer_missilehit(p, vm)){
			p->controller.buttons[INPUT_BUTTON_A] = (INPUT_STATE_CHANGED | INPUT_STATE_DOWN);
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

void aiwin_camera(struct aiplayer* aip)
{
	vec3f diff, up;

	physx::PxMat44 t_player(aip->vehicle->body->getGlobalPose());
	physx::PxVec3 targetpos = t_player.transform(physx::PxVec3(PLAYER_CAMERA_TARGETPOS));

	vec3f_subtractn(diff, (float*)&targetpos, aip->camera.pos);
	vec3f_scale(diff, AI_CAMERA_EASING);
	vec3f_add(aip->camera.pos, diff);

	vec3f_set(up, 0.f, 1.f, 0.f);
	camera_lookat(&aip->camera, (float*)&t_player.getPosition(), up);
}