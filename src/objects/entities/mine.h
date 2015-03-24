#ifndef MINE
#define	MINE


#include	"entity.h"


#define	MINE_MESH_FILENAME				"res/models/mine/mine.obj"
#define	MINE_MESH_SCALE					0.1f
#define	MINE_MESH_YROTATE				0.f
#define	MINE_MESH_OFFSET				0.f, 1.f, 0.f

#define	MINE_TEXTURE_FILENAME_DIFFUSE	"res/models/mine/mine.png"

#define	MINE_SFX_FILENAME_IDLE			"res/soundfx/mine_idle.wav"
#define	MINE_SFX_FILENAME_EXPLODE		"res/soundfx/mine_explode.wav"

#define	MINE_LOCALFORCE					0.f, 1000.f, 0.f
#define	MINE_RADIUS						0.5f
#define	MINE_DENSITY					1.f
#define	MINE_DESPAWNTIME				3600


/*	initialize a mine entity
	param:	e			mine entity to initialize
	param:	em			entity manager
	param:	v			vehicle (owner of the mine)
	param:	pose		global pose of the mine
*/
void mine_init(struct entity* e, struct entitymanager* em, struct vehicle* v, physx::PxTransform pose);

/*	delete a mine entity
	param:	e			mine entity to delete
*/
void mine_delete(struct entity* e);

/*	update a mine entity
	param:	e			mine entity to update
	param:	em			entity manager
*/
void mine_update(struct entity* e, struct entitymanager* em);


#endif
