#ifndef TURRET
#define	TURRET


#include	"entity.h"


#define	TURRET_MESH_FILENAME				"res/models/turret/scifiturret.obj"
#define	TURRET_MESH_SCALE					0.5f
#define	TURRET_MESH_YROTATE					-1.57080f
#define	TURRET_MESH_OFFSET					0.f, 0.f, 0.f

#define	TURRET_TEXTURE_FILENAME_DIFFUSE		"res/models/turret/scifiturret.png"

#define	TURRET_RADIUS						1.f
#define	TURRET_DENSITY						1.f
#define	TURRET_DESPAWNTIME					600
#define	TURRET_COOLDOWN						10
#define	TURRET_MISSILE_SPAWNDIST			-2.f


/*	initialize a turret entity
	param:	e			turret entity to initialize
	param:	em			entity manager
	param:	v			vehicle (owner of the turret)
	param:	pose		global pose of the turret
*/
void turret_init(struct entity* e, struct entitymanager* em, struct vehicle* v, physx::PxTransform pose, int dummy);

/*	delete a  turret entity
	param:	e			turret entity to delete
*/
void  turret_delete(struct entity* e);

/*	update a turret entity
	param:	e			turret entity to update
	param:	em			entity manager
*/
void turret_update(struct entity* e, struct entitymanager* em);


#endif
