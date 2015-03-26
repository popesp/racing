#ifndef MISSILE
#define	MISSILE


#include	"entity.h"


#define	MISSILE_MESH_FILENAME				"res/models/missile/missile.obj"
#define	MISSILE_MESH_SCALE					0.35f
#define	MISSILE_MESH_YROTATE				3.14159f
#define	MISSILE_MESH_OFFSET					0.f, 0.f, 0.f

#define	MISSILE_TEXTURE_FILENAME_DIFFUSE	"res/models/missile/missile.png"

#define	MISSILE_SFX_FILENAME_LAUNCH			"res/soundfx/missile_launch.wav"
#define	MISSILE_SFX_FILENAME_IDLE			"res/soundfx/missile_idle.wav"
#define	MISSILE_SFX_FILENAME_EXPLODE		"res/soundfx/missile_explode.wav"

#define	MISSILE_RADIUS						0.5f
#define	MISSILE_LOCALFORCE					0.f, 1000.f, 0.f
#define	MISSILE_DENSITY						1.f
#define	MISSILE_SPEED						80.f
#define	MISSILE_DESPAWNTIME					300


/*	initialize a missile entity
	param:	e			missile entity to initialize
	param:	em			entity manager
	param:	v			vehicle (owner of the missile)
	param:	pose		global pose of the missile
*/
void missile_init(struct entity* e, struct entitymanager* em, struct vehicle* v, physx::PxTransform pose);

/*	delete a missile entity
	param:	e			missile entity to delete
*/
void missile_delete(struct entity* e);

/*	update a missile entity
	param:	e			missile entity to update
	param:	em			entity manager
*/
void missile_update(struct entity* e, struct entitymanager* em);


#endif
