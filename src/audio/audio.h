#ifndef AUDIO
#define	AUDIO


#include	<fmod.h>
#include	"../math/vec3f.h"


#define	AUDIO_MUSIC_VOLUMESCALE	0.1f
#define	AUDIO_SFX_VOLUMESCALE	0.1f


struct sound
{
	FMOD_SOUND* track;

	bool enabled;
};

struct audiomanager
{
	FMOD_SYSTEM* system;

	FMOD_CHANNELGROUP* group_ingame;
	FMOD_SOUNDGROUP* group_music;
	FMOD_SOUNDGROUP* group_sfx;
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
	return:	FMOD_SOUND*		sound object pointer
*/
FMOD_SOUND* audiomanager_newmusic(struct audiomanager* am, const char* filename);

/*	create a new sfx sound object
	param:	am				audio manager
	param:	filename		path to the audio file
	param:	is3D			whether the sound is 3 dimensional
	return:	FMOD_SOUND*		sound object pointer
*/
FMOD_SOUND* audiomanager_newsfx(struct audiomanager* am, const char* filename, bool is3D);


/*	play a music sound
	param:	am				audio manager
	param:	music			sound object pointer
	param:	loops			number of times to loop the song
	param:	ingame			whether the sound is in-game
	param:	volume			volume for music channel
	return:	FMOD_CHANNEL*	pointer to sound channel
*/
FMOD_CHANNEL* audiomanager_playmusic(struct audiomanager* am, FMOD_SOUND* music, int loops, bool ingame, float volume = 1.f);

/*	play an sfx sound
	param:	am				audio manager
	param:	sfx				sound object pointer
	param:	pos				position to play the sound effect
	param:	loops			number of times to loop the sound effect
	param:	ingame			whether the sound is in-game
	param:	volume			volume for sfx channel
	return:	FMOD_CHANNEL*	pointer to sound channel
*/
FMOD_CHANNEL* audiomanager_playsfx(struct audiomanager* am, FMOD_SOUND* sfx, vec3f pos, int loops, bool ingame, float volume = 1.f);


/*	get the music volume
	param:	am				audio manager
	return:	float			music channel group volume
*/
float audiomanager_getmusicvolume(struct audiomanager* am);

/*	set the music volume
	param:	am				audio manager
	param:	volume			new music volume
*/
void audiomanager_setmusicvolume(struct audiomanager* am, float volume);

/*	get the sfx volume
	param:	am				audio manager
	return:	float			sfx channel group volume
*/
float audiomanager_getsfxvolume(struct audiomanager* am);

/*	set the sfx volume
	param:	am				audio manager
	param:	volume			new sfx volume
*/
void audiomanager_setsfxvolume(struct audiomanager* am, float volume);

/*	get the master volume
	param:	am				audio manager
	return:	float			master channel group volume
*/
float audiomanager_getmastervolume(struct audiomanager* am);

/*	set the master volume
	param:	am				audio manager
	param:	volume			new master volume
*/
void audiomanager_setmastervolume(struct audiomanager* am, float volume);


/*	set the paused state of all in-game sounds
	param:	am				audio manager
	param:	paused			paused state
*/
void audiomanager_ingamepausedstate(struct audiomanager* am, bool paused);


/*	delete a sound object
	param:	sound			sound object to delete
*/
void sound_delete(FMOD_SOUND* sound);


/*	set a sound's position in space
	param:	channel			sound channel
	param:	pos				new position
*/
void soundchannel_setposition(FMOD_CHANNEL* channel, vec3f pos);

/*	set a sound channel's volume
	param:	channel			sound channel
	param:	volume			new volume
*/
void soundchannel_setvolume(FMOD_CHANNEL* channel, float volume);

/*	set a sound channel's frequency
	param:	channel			sound channel
	param:	freq			new frequency
*/
void soundchannel_setfreq(FMOD_CHANNEL* channel, float freq);


/*	set a channel's paused state
	param:	channel			sound channel
	param:	paused			paused state
*/
void soundchannel_pausedstate(FMOD_CHANNEL* channel, bool paused);

/*	toggle whether a sound channel is paused
	param:	channel			sound channel
*/
void soundchannel_toggle(FMOD_CHANNEL* channel);

/*	stop a sound channel
	param:	channel			sound channel
*/
void soundchannel_stop(FMOD_CHANNEL* channel);


#endif
