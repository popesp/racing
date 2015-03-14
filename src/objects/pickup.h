#ifndef PICKUP
#define	PICKUP


#include	"../objects/track.h"
#include	"../physics/physics.h"
#include	"../render/render.h"
#include	"../render/texture.h"


#define	PICKUP_MESH_FILENAME			"res/models/powerup/powerup.obj"
#define	PICKUP_MESH_SCALE				0.2f
#define	PICKUP_MESH_YROTATE				-1.57080f

#define	PICKUP_TEXTURE_FILENAME_MISSILE	"res/models/powerup/missile.png"
#define	PICKUP_TEXTURE_FILENAME_MINE	"res/models/powerup/mine.png"
#define	PICKUP_TEXTURE_FILENAME_BOOST	"res/models/powerup/boost.png"

#define	PICKUP_TYPE_COUNT				3
#define	PICKUP_TYPE_MISSILE				0
#define	PICKUP_TYPE_MINE				1
#define	PICKUP_TYPE_BOOST				2

#define	PICKUP_RADIUS					0.5f
#define	PICKUP_SPAWNHEIGHT				0.f
#define	PICKUP_DENSITY					1.f
#define	PICKUP_SPAWNTIME				350

#define	PICKUP_FLAG_COLLECTED			0x01
#define	PICKUP_FLAG_VEHICLEHIT			0x02
#define	PICKUP_FLAG_INIT				0x00


struct pickup
{
	physx::PxRigidDynamic* body;

	struct vehicle* collector;

	unsigned timer;

	unsigned char type;
	unsigned char flags;
};


struct pickupmanager
{
	unsigned num_pickups;
	struct pickup* pickups;

	struct renderable renderable;

	struct texture diffuse[PICKUP_TYPE_COUNT];
};


/*	start up the pickup manager
	param:	pum				pickup manager
	param:	pm				physics manager
	param:	r				renderer
	param:	t				track object
	param:	num_pickups		number of pickup objects
	param:	track_indices	array of track inddices for the powerups
*/
void pickupmanager_startup(struct pickupmanager* pum, struct physicsmanager* pm, struct renderer* r, struct track* t, unsigned num_pickups, int* track_indices);

/*	shut down the pickup manager
	param:	pum				pickup manager
*/
void pickupmanager_shutdown(struct pickupmanager* pum);

/*	update the pickup manager
	param:	pum				pickup manager
*/
void pickupmanager_update(struct pickupmanager* pum);


/*	render all pickups
	param:	pum				pickup manager
	param:	r				renderer
	param:	worldview		world-view transformation matrix
*/
void pickupmanager_render(struct pickupmanager* pum, struct renderer* r, mat4f worldview);


#endif