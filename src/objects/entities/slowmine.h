#ifndef SLOWMINE
#define	SLOWMINE


#include	"entity.h"


#define	SLOWMINE_MESH_FILENAME				"res/models/mine/mine.obj"
#define	SLOWMINE_MESH_SCALE					0.5f
#define	SLOWMINE_MESH_YROTATE				0.f
#define	SLOWMINE_MESH_OFFSET				0.f, 0.f, 0.f

#define	SLOWMINE_TEXTURE_FILENAME_DIFFUSE	"res/models/mine/uv_slow.png"

#define	SLOWMINE_RADIUS						0.5f
#define	SLOWMINE_DENSITY					1.f
#define	SLOWMINE_DESPAWNTIME				3600


/*	initialize a slow mine entity
	param:	e			slow mine entity to initialize
	param:	em			entity manager
	param:	v			vehicle (owner of the mine)
	param:	pose		global pose of the mine
*/
void slowmine_init(struct entity* e, struct entitymanager* em, struct vehicle* v, physx::PxTransform pose);

/*	delete a slow mine entity
	param:	e			slow mine entity to delete
*/
void slowmine_delete(struct entity* e);

/*	update a slow mine entity
	param:	e			slow mine entity to update
	param:	em			entity manager
*/
void slowmine_update(struct entity* e, struct entitymanager* em);


#endif
