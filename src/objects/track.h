#ifndef	TRACK
#define	TRACK


#include	<PxRigidStatic.h>
#include	"../math/vec3f.h"
#include	"../render/render.h"


#define	TRACK_SEGMENT_VERTCOUNT	5

#define	TRACK_FLAG_NONE			0x00
#define	TRACK_FLAG_LOOPED		0x01


struct physicsmanager;


struct track_point
{
	vec3f pos;
	vec3f tan;

	unsigned subdivisions;

	float angle;
	float weight;
	float width;
};

struct track
{
	unsigned num_points;
	struct track_point* points;

	physx::PxRigidStatic* p_track;

	struct renderable r_track;

	vec3f up;

	unsigned char flags;
};


void track_init(struct track*, vec3f, struct physicsmanager*, unsigned char);
void track_delete(struct track*);

void track_generatemesh(struct renderer*, struct track*);


#endif