#ifndef VEHICLE
#define	VEHICLE


#include	"../audio/audio.h"
#include	"../core/input.h"
#include	"../core/ui.h"
#include	"entities/entity.h"
#include	"../physics/physics.h"
#include	"../render/render.h"
#include	"../render/texture.h"
#include	"track.h"


#define	VEHICLE_COUNT								99
#define	VEHICLE_COUNT_RAYCASTS						4

#define	VEHICLE_RAYCAST_FRONTLEFT					0
#define	VEHICLE_RAYCAST_FRONTRIGHT					1
#define	VEHICLE_RAYCAST_BACKLEFT					2
#define	VEHICLE_RAYCAST_BACKRIGHT					3

#define	VEHICLE_RAYCAST_WIDTHOFFSET					0.2f
#define	VEHICLE_RAYCAST_FRONTOFFSET					0.2f
#define	VEHICLE_RAYCAST_BACKOFFSET					0.2f
#define	VEHICLE_RAYCAST_MAXDIST						1.f
#define	VEHICLE_RAYCAST_MAXFORCE					30.f

#define	VEHICLE_MESH_FILENAME						"res/models/car/car.obj"
#define	VEHICLE_MESH_SCALE							0.2f
#define	VEHICLE_MESH_YROTATE						-1.57080f

#define	VEHICLE_SFX_FILENAME_ENGINESTART			"res/soundfx/engine_start.wav"
#define	VEHICLE_SFX_FILENAME_ENGINEIDLE				"res/soundfx/engine_idle.wav"
#define	VEHICLE_SFX_FILENAME_COLLISION				"res/soundfx/collision.wav"

#define	VEHICLE_TEXTURE_DIFFUSE_COUNT				7
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME0			"res/models/car/carUVy.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME1			"res/models/car/carUVb.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME2			"res/models/car/carUVg.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME3			"res/models/car/carUVo.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME4			"res/models/car/carUVp.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME5			"res/models/car/carUVr.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME6			"res/models/car/carUVw.png"

#define	VEHICLE_POWERUP_COUNT						11
#define	VEHICLE_POWERUP_MISSILE						0
#define	VEHICLE_POWERUP_MINE						1
#define	VEHICLE_POWERUP_BOOST						2
#define	VEHICLE_POWERUP_MISSILEX2					3
#define	VEHICLE_POWERUP_MISSILEX3					4
#define	VEHICLE_POWERUP_TURRET						5
#define	VEHICLE_POWERUP_LONGBOOST					6
#define VEHICLE_POWERUP_MINEX2						7
#define VEHICLE_POWERUP_MINEX3						8
#define VEHICLE_POWERUP_UBERMODE					9
#define VEHICLE_POWERUP_SLOWMINE					10

#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILE	"res/models/powerup/attached/missile.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MINE		"res/models/powerup/attached/mine.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_BOOST		"res/models/powerup/attached/boost.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILEX2	"res/models/powerup/attached/missile_x2.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILEX3	"res/models/powerup/attached/missile_x3.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_TURRET		"res/models/powerup/attached/turret.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_LONGBOOST	"res/models/powerup/attached/boost_long.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MINEX2		"res/models/powerup/attached/mine_x2.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MINEX3		"res/models/powerup/attached/mine_x3.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_UBER		"res/models/powerup/attached/rocket_boost.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_SLOWMINE	"res/models/powerup/attached/mine_speed.png"


#define	VEHICLE_POWERUP_MESH_FILENAME				"res/models/powerup/attached/attached.obj"
#define	VEHICLE_POWERUP_MESH_SCALE					0.4f
#define	VEHICLE_POWERUP_MESH_YROTATE				-1.57080f

#define	VEHICLE_UBERRING_MESH_FILENAME				"res/models/Slowzone/stormring.obj"
#define VEHICLE_UBERRING_MESH_SCALE					1.5f
#define VEHICLE_UBERRING_MESH_YROTATE				-1.f

#define VEHICLE_UBERRING_TEXTURE					"res/models/Slowzone/red70%.png"

#define	VEHICLE_POWERUP_ATTACHLOCATION				-0.5f, 0.f, 1.5f
#define VEHICLE_UBERRING_LOCATION					0.f, -0.5f, 0.f

#define	VEHICLE_POWERUP_MISSILE_SPAWNDIST			1.f

#define	VEHICLE_POWERUP_MINE_SPAWNDIST				1.f

#define	VEHICLE_POWERUP_BOOST_DURATION				180
#define	VEHICLE_POWERUP_BOOST_STRENGTH				20.f
#define	VEHICLE_POWERUP_SLOW_STRENGTH				-20.0f

#define	VEHICLE_POWERUP_LONGBOOST_DURATION			540

#define VEHICLE_POWERUP_UBER_DURATION				360

#define	VEHICLE_DIMENSIONS							0.7254f, 0.3056f, 1.3884f
#define	VEHICLE_DENSITY								0.8f
#define	VEHICLE_ACCELERATION						40.f
#define	VEHICLE_TURNFORCE							12.f
#define	VEHICLE_DOWNFORCE							0.2f
#define	VEHICLE_SPAWNHEIGHT							1.f
#define	VEHICLE_FREQSCALE							0.0001f
#define	VEHICLE_LATERALDAMPFORCE					10.f

#define	VEHICLE_RESETSPEEDTHRESHHOLD				0.5f
#define	VEHICLE_RESETTILTTHRESHHOLD					0.2f

#define	VEHICLE_DAMP_LINEAR							0.6f
#define	VEHICLE_DAMP_ANGULAR						5.f

#define	VEHICLE_UP									0.f, 1.f, 0.f
#define	VEHICLE_DOWN								0.f, -1.f, 0.f
#define	VEHICLE_FORWARD								0.f, 0.f, -1.f
#define	VEHICLE_RIGHT								1.f, 0.f, 0.f

#define	VEHICLE_FLAG_ENABLED						0x01
#define	VEHICLE_FLAG_BOOSTING						0x02
#define	VEHICLE_FLAG_HASPOWERUP						0x04
#define	VEHICLE_FLAG_MISSILEHIT						0x08
#define	VEHICLE_FLAG_MINEHIT						0x10
#define VEHICLE_FLAG_UBER							0x20
#define VEHICLE_FLAG_SLOWED							0x40
#define VEHICLE_FLAG_SLOWEDHIT						0x80
#define	VEHICLE_FLAG_INIT							0x00
#define VEHICLE_COLLISION_SOUND						0x100

struct vehicle
{
	physx::PxRigidDynamic* body;
	struct vehiclemanager* vm;
	struct controller* controller;
	FMOD_CHANNEL* channel;
	FMOD_CHANNEL* collision_channel;

	vec3f ray_origins[VEHICLE_COUNT_RAYCASTS];
	vec3f ray_dirs[VEHICLE_COUNT_RAYCASTS];
	bool ray_touch[VEHICLE_COUNT_RAYCASTS];

	float speed;

	vec3f pos;

	int index_track;
	int index_diffuse;

	unsigned timer_boost;
	unsigned timer_uber;
	unsigned timer_slow;

	unsigned powerup;

	unsigned short flags;

	int lap, place;
	
	bool checkpoint1, checkpoint2;
};

struct vehiclemanager
{
	struct physicsmanager* pm;
	struct entitymanager* em;
	struct audiomanager* am;
	struct uimanager* um;

	struct track* track;

	struct renderable r_vehicle;
	struct renderable r_powerup;
	struct renderable r_uberring;

	struct texture diffuse_vehicle[VEHICLE_TEXTURE_DIFFUSE_COUNT];
	struct texture diffuse_powerup[VEHICLE_POWERUP_COUNT];
	struct texture diffuse_uberring;

	int sfx_engine_start;
	int sfx_engine_idle;
	int sfx_collision;

	struct vehicle vehicles[VEHICLE_COUNT];
};


/*	start up the vehicle manager
	param:	vm				vehicle manager
	param:	pm				physics manager
	param:	em				entity manager
	param:	am				audio manager
	param:	r				renderer
	param:	track			track object
*/
void vehiclemanager_startup(struct vehiclemanager* vm, struct physicsmanager* pm, struct entitymanager* em, struct audiomanager* am, struct renderer* r, struct track* track, struct uimanager* um);

/*	shut down the vehicle manager
	param:	vm				vehicle manager
*/
void vehiclemanager_shutdown(struct vehiclemanager* vm);

/*	update vehicles
	param:	vm				vehicle manager
*/
void vehiclemanager_update(struct vehiclemanager* vm);


/*	create a new vehicle
	param:	vm				vehicle manager
	param:	cntrl			controller
	param:	index_track		index into the track path on which to spawn the vehicle
	param:	offs			offset from the spawn point
	return:	struct vehicle*	pointer to the new vehicle object
*/
struct vehicle* vehiclemanager_newvehicle(struct vehiclemanager* vm, struct controller* cntrl, int index_track, vec3f offs);

/*	remove a vehicle
	param:	vm				vehicle manager
	param:	v				vehicle pointer
*/
void vehiclemanager_removevehicle(struct vehiclemanager* vm, struct vehicle* v);


/*	render all vehicles
	param:	vm				vehicle manager
	param:	r				renderer
	param:	worldview		world-view transformation matrix
*/
void vehiclemanager_render(struct vehiclemanager* vm, struct renderer* r, mat4f worldview);


/*	reset a vehicle onto the track
	param:	vm				vehicle manager
	param:	v				vehicle to reset
*/
void vehiclemanager_resetvehicle(struct vehiclemanager* vm, struct vehicle* v);


#endif