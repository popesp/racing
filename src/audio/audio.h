#ifndef AUDIO
#define	AUDIO


#include	<stdio.h>				// printf
#include	<fmod.h>
#include <fmod_errors.h>

#define	AUDIO_MAX_MUSIC		5
#define	AUDIO_MAX_SFX		20

#define	AUDIO_FILEPATH		"res/music"



                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
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

	struct sound music[AUDIO_MAX_MUSIC];
	struct sound sfx[AUDIO_MAX_SFX];
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
*/
void audiomanager_playsfx(struct audiomanager* am, int id);


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
