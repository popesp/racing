#include	"audio.h"

#include	<stdio.h>	// temp


static void resetsound(struct sound* s)
{
	s->track = 0;
	s->enabled = false;
}


/*	start up the audio manager
	param:	am				audio manager (modified)
*/
void audiomanager_startup(struct audiomanager* am)
{
	int i;

	// create audio system
	FMOD_System_Create(&am->system);

	// initialize sound system
	FMOD_System_Init(am->system, 100, FMOD_INIT_3D_RIGHTHANDED, 0);
    
	// create channel groups
	FMOD_System_CreateChannelGroup(am->system, "music", &am->group_music);
	FMOD_System_CreateChannelGroup(am->system, "sfx", &am->group_sfx);
	
	// initialize music sounds
	for (i = 0; i < AUDIO_MUSIC_COUNT; i++)
		resetsound(&am->music[i]);

	// initialize sfx sounds
	for (i = 0; i < AUDIO_SFX_COUNT; i++)
		resetsound(&am->sfx[i]);
}

/*	shut down the audio manager
	param:	am				audio manager (modified)
*/
void audiomanager_shutdown(struct audiomanager* am)
{
	int i;

	// release music sounds
	for (i = 0; i < AUDIO_MUSIC_COUNT; i++)
		if (am->music[i].enabled)
			FMOD_Sound_Release(am->music[i].track);

	// release sfx sounds
	for (i = 0; i < AUDIO_SFX_COUNT; i++)
		if (am->sfx[i].enabled)
			FMOD_Sound_Release(am->sfx[i].track);

	FMOD_ChannelGroup_Release(am->group_music);
	FMOD_ChannelGroup_Release(am->group_sfx);

	FMOD_System_Release(am->system);
}

/*	get the FMOD version number
	param:	am				audio manager
	return:	unsigned		version number
*/
unsigned audiomanager_getlibversion(struct audiomanager* am)
{
	unsigned version;

	FMOD_System_GetVersion(am->system, &version);

	return version;
}


/*	update the audio manager
	param:	am				audio manager
	param:	pos				listener position
	param:	dir				listener direction
	param:	up				listener 'up' vector
*/
void audiomanager_update(struct audiomanager* am, vec3f pos, vec3f dir, vec3f up)
{
	// change listener position and orientation
	FMOD_System_Set3DListenerAttributes(am->system, 0, (FMOD_VECTOR*)pos, 0, (FMOD_VECTOR*)dir, (FMOD_VECTOR*)up);

	// update system
	FMOD_System_Update(am->system);
}


/*	create a new music sound object
	param:	am				audio manager
	param:	filename		path to the audio file
	return:	int				index to the sound object, or -1 if failure
*/
int audiomanager_newmusic(struct audiomanager* am, const char* filename)
{
	int i;

	for (i = 0; i < AUDIO_MUSIC_COUNT; i++)
	{
		if (!am->music[i].enabled)
		{
			FMOD_System_CreateStream(am->system, filename, FMOD_LOOP_NORMAL, 0, &am->music[i].track);
			am->music[i].enabled = true;

			return i;
		}
	}

	return -1;
}

/*	create a new sfx sound object
	param:	am				audio manager
	param:	filename		path to the audio file
	return:	int				index to the sound object, or -1 if failure
*/
int audiomanager_newsfx(struct audiomanager* am, const char* filename)
{
	int i;

	for (i = 0; i < AUDIO_SFX_COUNT; i++)
	{
		if (!am->sfx[i].enabled)
		{
			FMOD_System_CreateSound(am->system, filename, (FMOD_3D | FMOD_LOOP_NORMAL), 0, &am->sfx[i].track);
			am->sfx[i].enabled = true;

			return i;
		}
	}

	return -1;
}


/*	play a music sound
	param:	am				audio manager
	param:	id				index to the sound object
	param:	loops			number of times to loop the song
	return:	FMOD_CHANNEL*	pointer to sund channel
*/
FMOD_CHANNEL* audiomanager_playmusic(struct audiomanager* am, int id, int loops)
{
	FMOD_CHANNEL* channel;

	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, am->music[id].track, true, &channel);
	FMOD_Channel_SetChannelGroup(channel, am->group_music);

	// set loop count
	FMOD_Channel_SetLoopCount(channel, loops);
	
	// play the sound
	FMOD_Channel_SetPaused(channel, false);

	return channel;
}

/*	play an sfx sound
	param:	am				audio manager
	param:	id				index to the sound object
	param:	pos				position to play the sound effect
	param:	loops			number of times to loop the sound effect
	return:	FMOD_CHANNEL*	pointer to sound channel
*/
FMOD_CHANNEL* audiomanager_playsfx(struct audiomanager* am, int id, vec3f pos, int loops, float vol)
{
	FMOD_CHANNEL* channel;
	
	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, am->sfx[id].track, true, &channel);
	
	audiomanager_setsfx_channelvolume(channel,vol);
	FMOD_Channel_SetChannelGroup(channel, am->group_sfx);

	// position the sound effect in space
	audiomanager_setsoundposition(channel, pos);

	// set loop count
	FMOD_Channel_SetLoopCount(channel, loops);

	

	// play the sound
	FMOD_Channel_SetPaused(channel, false);


	return channel;
}

void audiomanager_displayvol(FMOD_CHANNEL* channel){
	
	float volume;
	FMOD_Channel_GetVolume(channel, &volume);
	printf("missle vol %f\n", volume);
}


/*	stop a music sound
	param:	channel			sound channel
*/
void audiomanager_stopsound(FMOD_CHANNEL* channel)
{
	FMOD_Channel_Stop(channel);
}


/*	set the position in 3d space of a sound effect
	param:	channel			sound channel
	param:	pos				new position
*/
void audiomanager_setsoundposition(FMOD_CHANNEL* channel, vec3f pos)
{
    FMOD_VECTOR position = {pos[VX], pos[VY], pos[VZ]};

    FMOD_Channel_Set3DAttributes(channel, &position, 0);
}


/*	set the music volume
	param:	am				audio manager
	param:	volume			new music volume
*/
void audiomanager_setmusicvolume(struct audiomanager* am, float volume)
{
	
	FMOD_ChannelGroup_SetPaused(am->group_music, true);
	FMOD_ChannelGroup_SetVolume(am->group_music, volume);
	FMOD_ChannelGroup_SetPaused(am->group_music, false);
}

/*	set the overall sfx volume
	param:	am				audio manager
	param:	volume			new sfx volume
*/
void audiomanager_setsfxvolume(struct audiomanager* am, float volume)
{
	
	FMOD_ChannelGroup_SetPaused(am->group_sfx, true);
	FMOD_ChannelGroup_SetVolume(am->group_sfx, volume);
	
	FMOD_ChannelGroup_SetPaused(am->group_sfx, false);
}

/*	set the individual sfx volume
	param:	channel			specific sfx channel
	param:	volume			new sfx volume
*/
void audiomanager_setsfx_channelvolume(FMOD_CHANNEL* channel, float volume)
{
	FMOD_Channel_SetVolume(channel,volume);
}

/*	set the master volume
	param:	am				audio manager
	param:	volume			new master volume
*/
void audiomanager_setmastervolume(struct audiomanager* am, float volume)
{
	FMOD_CHANNELGROUP* master;

	FMOD_System_GetMasterChannelGroup(am->system, &master);
	FMOD_ChannelGroup_SetVolume(master, volume);
}


/*	toggle a music sound's paused state
	param:	channel			sound channel
*/
void audiomanager_togglesound(FMOD_CHANNEL* channel)
{
    FMOD_BOOL state;
   
    FMOD_Channel_GetPaused(channel, &state);

	if (state)
		FMOD_Channel_SetPaused(channel, false);
	else
		FMOD_Channel_SetPaused(channel, true);
}

void audiomanager_setchannelfreq(FMOD_CHANNEL* channel,float rate)
{
	FMOD_Channel_SetFrequency(channel, 44100.0*rate);

}
