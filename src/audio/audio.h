#ifndef AUDIO
#define	AUDIO


#include	<fmod.h>
#include	"../math/vec3f.h"


#define	AUDIO_MUSIC_COUNT		8
#define	AUDIO_SFX_COUNT			3


struct sound
{
	FMOD_SOUND* track;
	FMOD_CHANNEL* channel;

	bool enabled;
};

struct audiomanager
{
	FMOD_SYSTEM* system;

	FMOD_CHANNELGROUP* group_music;
	FMOD_CHANNELGROUP* group_sfx;

	struct sound music[AUDIO_MUSIC_COUNT];
	struct sound sfx[AUDIO_SFX_COUNT];
};


/*	start up the audio manager
	param:	am				audio manager (modified)
*/
void audiomanager_startup(struct audiomanager* am);

/*	shut down the audio manager
	param:	am				audio manager (modified)
*/
void audiomanager_shutdown(struct audiomanager* am);

/*	get the FMOD version number
	param:	am				audio manager
	return:	unsigned		version number
*/
unsigned audiomanager_getlibversion(struct audiomanager* am);


/*	update the audio manager
	param:	am				audio manager
	param:	pos				listener position
	param:	dir				listener direction
	param:	up				listener 'up' vector
*/
void audiomanager_update(struct audiomanager* am, vec3f pos, vec3f dir, vec3f up);


/*	create a new music sound object
	param:	am				audio manager
	param:	filename		path to the audio file
	return:	int				index to the sound object, or -1 if failure
*/
int audiomanager_newmusic(struct audiomanager* am, const char* filename);

/*	create a new sfx sound object
	param:	am				audio manager
	param:	filename		path to the audio file
	return:	int				index to the sound object, or -1 if failure
*/
int audiomanager_newsfx(struct audiomanager* am, const char* filename);


/*	play a music sound
	param:	am				audio manager
	param:	id				index to the sound object
	param:	loops			number of times to loop the song
*/
void audiomanager_playmusic(struct audiomanager* am, int id, int loops);

/*	play an sfx sound
	param:	am				audio manager
	param:	id				index to the sound object
	param:	pos				position to play the sound effect
*/
void audiomanager_playsfx(struct audiomanager* am, int id, vec3f pos);


/*	stop a music sound
	param:	am				audio manager
	param:	id				index to the sound object
*/
void audiomanager_stopmusic(struct audiomanager* am, int id);


/*	set the position in 3d space of a sound effect
	param:	am				audio manager
	param:	id				index to the sound object
	param:	pos				new position
*/
void audiomanager_setsfxposition(struct audiomanager* am, int id, vec3f pos);


/*	set the music volume
	param:	am				audio manager
	param:	volume			new music volume
*/
void audiomanager_setmusicvolume(struct audiomanager* am, float volume);

/*	set the sfx volume
	param:	am				audio manager
	param:	volume			new sfx volume
*/
void audiomanager_setsfxvolume(struct audiomanager* am, float volume);

/*	set the master volume
	param:	am				audio manager
	param:	volume			new master volume
*/
void audiomanager_setmastervolume(struct audiomanager* am, float volume);


/*	toggle a music sound's paused state
	param:	am				audio manager
	param:	id				index to the sound object
*/
void audiomanager_togglemusic(struct audiomanager* am, int id);


#endif
