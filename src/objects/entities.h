#ifndef ENTITIES
#define	ENTITIES


#include	"../objects/track.h"
#include	"../objects/vehicle.h"
#include	"../physics/physics.h"
#include	"../physics/collisions.h"
#include	"../audio/audio.h"

#define	ENTITY_MISSILE_COUNT		100
#define ENTITY_PICKUP_COUNT			170
#define ENTITY_MINE_COUNT			100

#define	ENTITY_MISSILE_DENSITY		1.f
#define	ENTITY_MISSILE_SPEED		80.f
#define	ENTITY_MISSILE_SIZE			1.f

#define	ENTITY_PICKUP_DENSITY		1.f
#define	ENTITY_PICKUP_WIDTH			.8f
#define ENTITY_PICKUP_HEIGHT		1.2f
#define ENTITY_PICKUP_LENGTH		0.1f

#define	ENTITY_MINE_DENSITY			1.f
#define	ENTITY_MINE_WIDTH			.8f
#define ENTITY_MINE_HEIGHT			.8f
#define ENTITY_MINE_LENGTH			.8f

#define	ENTITY_MISSILE_DESPAWNTIME	300

#define	ENTITY_MISSILE_SPAWNDIST	1.f
#define	ENTITY_PICKUP_SPAWNHEIGHT	1.2f
#define ENTITY_MINE_SPAWNDIST		5.f
#define BLIMP_SPAWNDIST				2.f

#define	ENTITY_MISSILE_FLAG_ENABLED	0x01
#define	ENTITY_MISSILE_FLAG_INIT	0x00

#define ENTITY_PICKUP_FLAG_INIT		0x00
#define ENTITY_PICKUP_FLAG_ENABLED	0x01

#define ENTITY_MINE_FLAG_INIT		0x00
#define ENTITY_MINE_FLAG_ENABLED	0x01

#define	PICKUP_MESHSCALE			0.2f
#define	PICKUP_ATTACHED_MESHSCALE	0.4f
#define MISSILE_MESHSCALE			0.35f
#define MINE_MESHSCALE				0.1f

#define PICKUP_ATTACHED_OBJ					"res/models/powerup/power_up_attached/powerup_attached.obj"
#define PICKUP_ATTACHED_MISSILE_TEXTURE		"res/models/powerup/power_up_attached/powerup_attached_rocket.png"
#define	PICKUP_ATTACHED_MINE_TEXTURE		"res/models/powerup/power_up_attached/powerup_attached_mine.png"
#define	PICKUP_ATTACHED_SPEED_TEXTURE		"res/models/powerup/power_up_attached/powerup_attached_speed.png"

#define PICKUP_OBJ					"res/models/powerup/powerup.obj"
#define PICKUP_MISSILE_TEXTURE		"res/models/powerup/powerup_rocketUV.png"
#define	PICKUP_MINE_TEXTURE			"res/models/powerup/powerup_mineUV.png"
#define	PICKUP_SPEED_TEXTURE		"res/models/powerup/powerup_speedUV.png"

#define	MINE_OBJ					"res/models/mine/mine.obj"
#define	MINE_TEXTURE				"res/models/mine/mineUV.png"

#define	MISSILE_OBJ					"res/models/Projectile/rocket.obj"
#define	MISSILE_TEXTURE				"res/models/Projectile/rocket_tex.png"

#define PICKUP_TIMERS				350

#define PICKUP_SPAWN_LOC1			70
#define PICKUP_SPAWN_LOC2			20
#define PICKUP_SPAWN_LOC3			100
#define PICKUP_SPAWN_LOC4			140

#define BLIMP_COUNT					152
#define	BLIMP_FLAG_ENABLED			0x01
#define	BLIMP_FLAG_INIT				0x00

#define BLIMP_TYPE_LAP				0x00
#define BLIMP_TYPE_PLACE			0x01

#define BLIMP_OBJ					"res/Models/Blimp/blimp.obj"
#define BLIMP_REGULAR_TEXTURE		"res/Models/Blimp/blimpUV.png"
#define BLIMP_WIN_TEXTURE			"res/Models/Blimp/blimp_win.png"
#define BLIMP_LOSE_TEXTURE			"res/Models/Blimp/blimp_lose.png"
#define BLIMP_WELCOME_TEXTURE		"res/Models/Blimp/blimp_welcome.png"

#define BLIMP_LAP1_TEXTURE			"res/Models/Blimp/lap_tex/blimp_1.png"
#define BLIMP_LAP2_TEXTURE			"res/Models/Blimp/lap_tex/blimp_2.png"
#define BLIMP_LAP3_TEXTURE			"res/Models/Blimp/lap_tex/blimp_3.png"
#define BLIMP_LAP4_TEXTURE			"res/Models/Blimp/lap_tex/blimp_4.png"
#define BLIMP_LAP5_TEXTURE			"res/Models/Blimp/lap_tex/blimp_5.png"

#define BLIMP_PLACE1_TEXTURE		"res/Models/Blimp/place_tex/blimp_place_1.png"
#define BLIMP_PLACE2_TEXTURE		"res/Models/Blimp/place_tex/blimp_place_2.png"
#define BLIMP_PLACE3_TEXTURE		"res/Models/Blimp/place_tex/blimp_place_3.png"
#define BLIMP_PLACE4_TEXTURE		"res/Models/Blimp/place_tex/blimp_place_4.png"
#define BLIMP_PLACE5_TEXTURE		"res/Models/Blimp/place_tex/blimp_place_5.png"
#define BLIMP_PLACE6_TEXTURE		"res/Models/Blimp/place_tex/blimp_place_6.png"
#define BLIMP_PLACE7_TEXTURE		"res/Models/Blimp/place_tex/blimp_place_7.png"
#define BLIMP_PLACE8_TEXTURE		"res/Models/Blimp/place_tex/blimp_place_8.png"

#define	BLIMP_MESHSCALE				0.3f
#define BLIMP_LAP_MESHSCALE			5.f
#define BLIMP_LAP_SPAWNHEIGHT		15.f
#define	BLIMP_DENSITY				1.f

struct pickup{
	physx::PxRigidDynamic* body;

	struct vehicle* owner;

	unsigned char flags;

	vec3f pos, avg;

	physx::PxVec3 powerpos;

	int hit, set;

	bool holdingpu1, holdingpu2, holdingpu3, holdingpu4;

	struct texture diffuse_pickupMINE;
	struct texture diffuse_pickupMISSILE;
	struct texture diffuse_pickupSPEED;
	struct renderable r_pickup;
	vec3f dim_pickup;

	//0=mine, 1=missile, 2=speed
	int typepickup;
};

struct missile
{
	physx::PxRigidDynamic* body;
	vec3f pos;
	struct vehicle* owner;
	unsigned timer;
	unsigned char flags;
	int hit;

	FMOD_CHANNEL* missle_channel;
};

struct mine{
	physx::PxRigidDynamic* body;
	struct vehicle* owner;
	unsigned char flags;
	int hit;
};

struct blimp{
	physx::PxRigidDynamic* body;
	struct vehicle* owner;
	unsigned char flags;
	
	physx::PxVec3 blimppos;

	unsigned char typeblimp;

	vec3f pos;
};

struct entitymanager
{
	struct physicsmanager* pm;
	struct audiomanager* am;
	struct track* track;
	struct renderer* r;

	struct renderable r_missile;
	struct renderable r_mine;
	struct renderable r_blimp;
	struct renderable r_blimplap;

	vec3f dim_missile;
	vec3f dim_mine;
	vec3f dim_blimp;

	struct texture diffuse_missile;
	struct texture diffuse_mine;
	struct texture diffuse_blimp;

	struct texture diffuse_place1;
	struct texture diffuse_place2;
	struct texture diffuse_place3;
	struct texture diffuse_place4;
	struct texture diffuse_place5;
	struct texture diffuse_place6;
	struct texture diffuse_place7;
	struct texture diffuse_place8;

	struct texture diffuse_lap1;
	struct texture diffuse_lap2;
	struct texture diffuse_lap3;
	struct texture diffuse_lap4;
	struct texture diffuse_lap5;

	struct texture diffuse_win;
	struct texture diffuse_lose;
	struct texture diffuse_welcome;

	int sfx_missile;

	bool pickupatspawn1;
	int timerspawn1;

	bool pickupatspawn2;
	int timerspawn2;

	bool pickupatspawn3;
	int timerspawn3;

	bool pickupatspawn4;
	int timerspawn4;

	int num_blimps, num_missiles, num_mines, num_pickups;

	struct missile missiles[ENTITY_MISSILE_COUNT];
	struct mine mines[ENTITY_MINE_COUNT];
	struct pickup pickups[ENTITY_PICKUP_COUNT];
	struct blimp blimps[BLIMP_COUNT];
};


void entitymanager_startup(struct entitymanager* em, struct physicsmanager* pm, struct renderer* r,struct audiomanager* am, struct track* t);
void entitymanager_shutdown(struct entitymanager* em);

void entitymanager_update(struct entitymanager* em, struct vehiclemanager* vm);

struct missile* entitymanager_newmissile(struct entitymanager* em, struct vehicle* v, vec3f dim);
void entitymanager_removemissile(struct entitymanager* em, struct missile* m);

void entitymanager_render(struct entitymanager* em, struct renderer* r, mat4f worldview);

void entitymanager_attachpickup(struct vehicle* v, struct pickup* pu, struct entitymanager* em);
struct pickup* entitymanager_newpickup(struct entitymanager* em, vec3f pos);
void entitymanager_removepickup(struct entitymanager* em, struct pickup* pu);

struct mine* entitymanager_newmine(struct entitymanager* em, vec3f dim, struct vehicle* v);
void entitymanager_removemine(struct entitymanager* em, struct mine* x);

struct blimp* entitymanager_placeblimp(struct vehicle* v, struct entitymanager* em, vec3f pos);
void entitymanager_removeblimp(struct entitymanager* em, struct blimp* b,struct vehicle* v);
struct blimp* entitymanager_lapblimp(struct entitymanager* em, vec3f pos);

void entitymanager_textures(struct entitymanager* em, struct renderer* r);
void entitymanager_blimpinit(struct entitymanager* em);
void entitymanager_missileinit(struct entitymanager* em);
void entitymanager_mineinit(struct entitymanager* em);
#endif
