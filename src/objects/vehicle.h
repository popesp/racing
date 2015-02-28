#ifndef VEHICLE
#define	VEHICLE


#include	"../core/input.h"
#include	"../math/vec3f.h"
#include	"../objects/track.h"
#include	"../physics/physics.h"
#include	"../render/render.h"


#define	VEHICLE_COUNT				10
#define	VEHICLE_COUNT_RAYCASTS		4

#define	VEHICLE_RAYCAST_FRONTLEFT	0
#define	VEHICLE_RAYCAST_FRONTRIGHT	1
#define	VEHICLE_RAYCAST_BACKLEFT	2
#define	VEHICLE_RAYCAST_BACKRIGHT	3

#define	VEHICLE_RAYCAST_WIDTHOFFSET	0.2f
#define	VEHICLE_RAYCAST_FRONTOFFSET	0.2f
#define	VEHICLE_RAYCAST_BACKOFFSET	0.2f
#define	VEHICLE_RAYCAST_MAXDIST		1.f
#define	VEHICLE_RAYCAST_MAXFORCE	20.f

#define	VEHICLE_MESHSCALE			0.2f

#define	VEHICLE_DENSITY				0.8f

#define	VEHICLE_SPAWNHEIGHT			1.5f

#define	VEHICLE_UP					0.f, 1.f, 0.f
#define	VEHICLE_DOWN				0.f, -1.f, 0.f
#define	VEHICLE_FORWARD				0.f, 0.f, -1.f
#define	VEHICLE_RIGHT				1.f, 0.f, 0.f

#define	VEHICLE_ACCELERATION		40.f
#define	VEHICLE_TURNFORCE			10.f
#define	VEHICLE_DOWNFORCE			0.2f

#define	VEHICLE_DAMP_LINEAR			0.6f
#define	VEHICLE_DAMP_ANGULAR		5.f

#define	VEHICLE_FLAG_INIT			0x00
#define	VEHICLE_FLAG_ENABLED		0x01
#define	VEHICLE_FLAG_MISSILE		0x02


struct vehicle
{
	physx::PxRigidDynamic* body;

	vec3f ray_origins[VEHICLE_COUNT_RAYCASTS];
	vec3f ray_dirs[VEHICLE_COUNT_RAYCASTS];
	bool ray_touch[VEHICLE_COUNT_RAYCASTS];

	struct controller* controller;

	vec3f pos;

	int index_track;

	int lap;

	//these are used to see if player vehicle is completing the race
	bool checkpoint1;
	bool checkpoint2;
	bool checkpoint3;

	unsigned char flags;
};

struct vehiclemanager
{
	struct physicsmanager* pm;
	struct texturemanager* tm;
	struct track* track;

	struct renderable r_vehicle;

	vec3f dim;

	int tex_diffuse;

	struct vehicle vehicles[VEHICLE_COUNT];
};


/*	start up the vehicle manager
	param:	vm				vehicle manager
	param:	r				renderer
	param:	tm				texture manager
	param:	pm				physics manager
	param:	t				track object
	param:	mesh_filename	filename for the vehicle mesh
	param:	tex_filename	filename for the vehicle texture
*/
void vehiclemanager_startup(struct vehiclemanager* vm, struct renderer* r, struct texturemanager* tm, struct physicsmanager* pm, struct track* t, const char* mesh_filename, const char* tex_filename);

/*	shut down the vehicle manager
	param:	vm				vehicle manager
*/
void vehiclemanager_shutdown(struct vehiclemanager* vm);


/*	create a new vehicle
	param:	vm				vehicle manager
	param:	index_track		index into the track path on which to spawn the vehicle
	param:	offs			offset from the spawn point
	return:	struct vehicle*	pointer to the new vehicle object
*/
struct vehicle* vehiclemanager_newvehicle(struct vehiclemanager* vm, int index_track, vec3f offs);

/*	remove a vehicle
	param:	vm				vehicle manager
	param:	v				vehicle to remove
*/
void vehiclemanager_removevehicle(struct vehiclemanager* vm, struct vehicle* v);

/*	update vehicles
	param:	vm				vehicle manager
*/
void vehiclemanager_update(struct vehiclemanager* vm);


/*	reset a vehicle onto the track
	param:	vm				vehicle manager
	param:	v				vehicle to reset
*/
void vehicle_reset(struct vehiclemanager* vm, struct vehicle* v);


#endif