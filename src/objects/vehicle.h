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


#define	VEHICLE_COUNT									8
#define	VEHICLE_COUNT_RAYCASTS							4

#define	VEHICLE_RAYCAST_FRONTLEFT						0
#define	VEHICLE_RAYCAST_FRONTRIGHT						1
#define	VEHICLE_RAYCAST_BACKLEFT						2
#define	VEHICLE_RAYCAST_BACKRIGHT						3

#define	VEHICLE_RAYCAST_WIDTHOFFSET						0.2f
#define	VEHICLE_RAYCAST_FRONTOFFSET						0.2f
#define	VEHICLE_RAYCAST_BACKOFFSET						0.2f
#define	VEHICLE_RAYCAST_MAXDIST							1.f
#define	VEHICLE_RAYCAST_MAXFORCE						30.f

#define	VEHICLE_MESH_FILENAME							"res/models/car/car.obj"
#define	VEHICLE_MESH_SCALE								0.2f
#define	VEHICLE_MESH_YROTATE							-1.57080f

#define	VEHICLE_SFX_FILENAME_ENGINESTART				"res/soundfx/engine_start.wav"
#define	VEHICLE_SFX_FILENAME_ENGINEIDLE					"res/soundfx/engine_idle.wav"
#define	VEHICLE_SFX_FILENAME_MISSILELAUNCH				"res/soundfx/missile_launch.wav"
#define	VEHICLE_SFX_FILENAME_COLLISION					"res/soundfx/collision.wav"

#define	VEHICLE_TEXTURE_DIFFUSE_COUNT					8
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME0				"res/models/car/uv_black.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME1				"res/models/car/uv_blue.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME2				"res/models/car/uv_green.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME3				"res/models/car/uv_orange.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME4				"res/models/car/uv_purple.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME5				"res/models/car/uv_red.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME6				"res/models/car/uv_white.png"
#define	VEHICLE_TEXTURE_DIFFUSE_FILENAME7				"res/models/car/uv_yellow.png"

#define	VEHICLE_POWERUP_COUNT							11
#define	VEHICLE_POWERUP_MISSILE							0
#define	VEHICLE_POWERUP_MINE							1
#define	VEHICLE_POWERUP_BOOST							2
#define	VEHICLE_POWERUP_MISSILEX2						3
#define	VEHICLE_POWERUP_MISSILEX3						4
#define	VEHICLE_POWERUP_MINEX2							5
#define	VEHICLE_POWERUP_MINEX3							6
#define	VEHICLE_POWERUP_LONGBOOST						7
#define	VEHICLE_POWERUP_TURRET							8
#define	VEHICLE_POWERUP_ROCKETBOOST						9
#define	VEHICLE_POWERUP_SLOWMINE						10

#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILE		"res/models/powerup/missile.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MINE			"res/models/powerup/mine.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_BOOST			"res/models/powerup/boost.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILEX2		"res/models/powerup/missile_x2.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MISSILEX3		"res/models/powerup/missile_x3.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MINEX2			"res/models/powerup/mine_x2.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_MINEX3			"res/models/powerup/mine_x3.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_LONGBOOST		"res/models/powerup/boost_long.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_TURRET			"res/models/powerup/turret.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_ROCKETBOOST	"res/models/powerup/rocketboost.png"
#define	VEHICLE_POWERUP_TEXTURE_FILENAME_SLOWMINE		"res/models/powerup/slowmine.png"

#define	VEHICLE_POWERUP_MESH_FILENAME					"res/models/powerup/powerup.obj"
#define	VEHICLE_POWERUP_MESH_SCALE						0.4f
#define	VEHICLE_POWERUP_MESH_YROTATE					-1.57080f

#define	VEHICLE_POWERUP_ATTACHLOCATION					-0.5f, 0.f, 1.5f

#define	VEHICLE_POWERUP_MISSILE_SPAWNDIST				1.f

#define	VEHICLE_POWERUP_MINE_SPAWNDIST					1.f

#define	VEHICLE_POWERUP_BOOST_DURATION					180
#define	VEHICLE_POWERUP_BOOST_STRENGTH					20.f

#define	VEHICLE_POWERUP_LONGBOOST_DURATION				540

#define	VEHICLE_POWERUP_ROCKETBOOST_DURATION			180

#define	VEHICLE_POWERUP_SLOWMINE_DURATION				300
#define	VEHICLE_POWERUP_SLOWMINE_STRENGTH				0.6f

#define	VEHICLE_INVINCIBILITY_MESH_FILENAME				"res/models/invincibility/invincibility.obj"
#define	VEHICLE_INVINCIBILITY_MESH_SCALE				1.5f
#define	VEHICLE_INVINCIBILITY_COLOR						255, 0, 0, 128
#define	VEHICLE_INVINCIBILITY_MESH_OFFSET				0.f, -0.5f, 0.f

#define	VEHICLE_DIMENSIONS								0.7254f, 0.3056f, 1.3884f
#define	VEHICLE_DENSITY									0.8f
#define	VEHICLE_ACCELERATION							40.f
#define	VEHICLE_TURNFORCE								12.f
#define	VEHICLE_DOWNFORCE								0.2f
#define	VEHICLE_SPAWNHEIGHT								1.f
#define	VEHICLE_FREQSCALE								0.0001f
#define	VEHICLE_LATERALDAMPFORCE						10.f

#define	VEHICLE_RESETSPEEDTHRESHHOLD					0.5f
#define	VEHICLE_RESETTILTTHRESHHOLD						0.2f

#define	VEHICLE_DAMP_LINEAR								0.6f
#define	VEHICLE_DAMP_ANGULAR							5.f

#define	VEHICLE_UP										0.f, 1.f, 0.f
#define	VEHICLE_DOWN									0.f, -1.f, 0.f
#define	VEHICLE_FORWARD									0.f, 0.f, -1.f
#define	VEHICLE_RIGHT									1.f, 0.f, 0.f

#define	VEHICLE_FLAG_BOOSTING							0x0001
#define	VEHICLE_FLAG_HASPOWERUP							0x0002
#define	VEHICLE_FLAG_MISSILEHIT							0x0004
#define	VEHICLE_FLAG_MINEHIT							0x0008
#define	VEHICLE_FLAG_VEHICLEHIT							0x0010
#define	VEHICLE_FLAG_SLOWMINEHIT						0x0020
#define	VEHICLE_FLAG_INVINCIBLE							0x0040
#define	VEHICLE_FLAG_SLOWED								0x0080
#define	VEHICLE_FLAG_CHECKPOINT1						0x0100
#define	VEHICLE_FLAG_CHECKPOINT2						0x0200
#define	VEHICLE_FLAG_INIT								(VEHICLE_FLAG_CHECKPOINT1 | VEHICLE_FLAG_CHECKPOINT2)


struct vehicle
{
	physx::PxRigidDynamic* body;
	struct vehiclemanager* vm;
	struct controller* controller;
	FMOD_CHANNEL* channel;

	vec3f ray_origins[VEHICLE_COUNT_RAYCASTS];
	vec3f ray_dirs[VEHICLE_COUNT_RAYCASTS];
	bool ray_touch[VEHICLE_COUNT_RAYCASTS];

	float speed;

	vec3f pos;

	int index_track;
	int index_tracklast;
	int index_diffuse;

	struct light light;

	unsigned timer_boost;
	unsigned timer_invincible;
	unsigned timer_slow;

	physx::PxVec3 starting_pos;

	unsigned powerup;

	unsigned lap;
	unsigned place;

	unsigned short flags;
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
	struct renderable r_invincibility;

	struct texture diffuse_vehicle[VEHICLE_TEXTURE_DIFFUSE_COUNT];
	struct texture diffuse_powerup[VEHICLE_POWERUP_COUNT];
	struct texture diffuse_invincibility;

	FMOD_SOUND* sfx_engine_start;
	FMOD_SOUND* sfx_engine_idle;
	FMOD_SOUND* sfx_missile_launch;
	FMOD_SOUND* sfx_collision;
	
	struct vehicle vehicles[VEHICLE_COUNT];
};


/*	start up the vehicle manager
	param:	vm			vehicle manager
	param:	pm			physics manager
	param:	em			entity manager
	param:	am			audio manager
	param:	r			renderer
	param:	track		track object
*/
void vehiclemanager_startup(struct vehiclemanager* vm, struct physicsmanager* pm, struct entitymanager* em, struct audiomanager* am, struct renderer* r, struct track* track, struct uimanager* um);

/*	reset the vehicle manager
	param:	vm			vehicle manager
*/
void vehiclemanager_reset(struct vehiclemanager* vm);

/*	shut down the vehicle manager
	param:	vm			vehicle manager
*/
void vehiclemanager_shutdown(struct vehiclemanager* vm);

/*	update vehicles
	param:	vm			vehicle manager
*/
void vehiclemanager_update(struct vehiclemanager* vm);


/*	render all vehicles
	param:	vm			vehicle manager
	param:	r			renderer
	param:	worldview	world-view transformation matrix
*/
void vehiclemanager_render(struct vehiclemanager* vm, struct renderer* r, mat4f worldview);


/*	place a vehicle somewhere on the track
	param:	vm			vehicle manager
	param:	v			vehicle to set
	param:	index_track	track point index
	param:	offs		position offset vector
*/
void vehiclemanager_setvehicletrackpos(struct vehiclemanager* vm, struct vehicle* v, int index_track, vec3f offs);


#endif