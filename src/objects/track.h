#ifndef	TRACK
#define	TRACK


#include	<PxRigidStatic.h>
#include	"../math/vec3f.h"
#include	"../render/render.h"


#define	TRACK_SEGMENT_VERTCOUNT			5

#define	TRACK_SEARCHDIVIDE				10
#define	TRACK_SEARCHSIZE				10

#define	TRACK_DEFAULT_DISTBOUND			30.f

#define	TRACK_FLAG_INIT					0x00
#define	TRACK_FLAG_LOOPED				0x01

#define TRACK_TEXTURE_FILENAME_NORMAL	"res/images/slate.jpg"

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

struct path_point
{
	vec3f pos;
	vec3f tan;
	float angle;
};

struct track
{
	unsigned num_points;
	struct track_point* points;

	unsigned num_pathpoints;
	struct path_point* pathpoints;

	physx::PxRigidStatic* p_track;

	struct renderable r_track;
	struct texture normal;

	float dist_boundary;

	vec3f up;

	unsigned char flags;
};


/*	initialize a track object
	param:	t			track object to initialize
	param:	pm			physics manager
	param:	up			up vector
*/
void track_init(struct track* t, struct physicsmanager* pm, vec3f up);

/*	delete a track object
	param:	t			track object to delete
*/
void track_delete(struct track* t);


/*	find the closest point on the track to a given position
	param:	t			track object
	param:	pos			position in space to search around
	param:	last		last known "closest" index (used so as to not search every point on the track)
	return:	int			index of the closest track point
*/
int track_closestindex(struct track* t, vec3f pos, int last);


/*	gets a matrix transform for the specified track path point
	param:	t			track object
	param:	res			resultant transformation matrix
	param:	index		path point index
	param:	pos_offs	local position offset to apply before rotation
*/
void track_transformindex(struct track* t, mat4f res, int index, vec3f pos_offs);


/*	load track points from a file
	param:	t			track object to load points into
	param:	filename	path to file holding track points
*/
void track_loadpointsfile(struct track* t, const char* filename);


/*	generate the track mesh (assuming points have already been specified)
	param:	r			renderer
	param:	t			track object to generate mesh for
*/
void track_generate(struct renderer* r, struct track* t);


#endif