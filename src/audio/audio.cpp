#include	"audio.h"


static void resetsound(struct sound* s)
{
	s->track = 0;
	s->enabled = false;
}


#include	<stdio.h>
void audiomanager_startup(struct audiomanager* am)
{
	int i;

	// create audio system
	FMOD_System_Create(&am->system);

	printf("test\n");

	// initialize sound system
	FMOD_System_Init(am->system, 100, FMOD_INIT_3D_RIGHTHANDED, 0);
    
	printf("test\n");

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

unsigned audiomanager_getlibversion(struct audiomanager* am)
{
	unsigned version;

	FMOD_System_GetVersion(am->system, &version);

	return version;
}


void audiomanager_update(struct audiomanager* am, vec3f pos, vec3f dir, vec3f up)
{
	// change listener position and orientation
	FMOD_System_Set3DListenerAttributes(am->system, 0, (FMOD_VECTOR*)pos, 0, (FMOD_VECTOR*)dir, (FMOD_VECTOR*)up);

	// update system
	FMOD_System_Update(am->system);
}


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


FMOD_CHANNEL* audiomanager_playmusic(struct audiomanager* am, int id, int loops, float volume)
{
	FMOD_CHANNEL* channel;

	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, am->music[id].track, true, &channel);
	FMOD_Channel_SetChannelGroup(channel, am->group_music);

	// set channel properties
	FMOD_Channel_SetLoopCount(channel, loops);
	FMOD_Channel_SetVolume(channel, volume);
	
	// play the sound
	FMOD_Channel_SetPaused(channel, false);

	return channel;
}

FMOD_CHANNEL* audiomanager_playsfx(struct audiomanager* am, int id, vec3f pos, int loops, float volume)
{
	FMOD_CHANNEL* channel;
	
	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, am->sfx[id].track, true, &channel);
	FMOD_Channel_SetChannelGroup(channel, am->group_sfx);

	// position the sound effect in space
	soundchannel_setposition(channel, pos);

	// set channel properties
	FMOD_Channel_SetLoopCount(channel, loops);
	FMOD_Channel_SetVolume(channel, volume);

	// play the sound
	FMOD_Channel_SetPaused(channel, false);

	return channel;
}


void audiomanager_setmusicvolume(struct audiomanager* am, float volume)
{
	FMOD_ChannelGroup_SetVolume(am->group_music, volume);
}

void audiomanager_setsfxvolume(struct audiomanager* am, float volume)
{
	FMOD_ChannelGroup_SetVolume(am->group_sfx, volume);
}

void audiomanager_setmastervolume(struct audiomanager* am, float volume)
{
	FMOD_CHANNELGROUP* master;

	FMOD_System_GetMasterChannelGroup(am->system, &master);
	FMOD_ChannelGroup_SetVolume(master, volume);
}


void soundchannel_setposition(FMOD_CHANNEL* channel, vec3f pos)
{
	FMOD_VECTOR position = {pos[VX], pos[VY], pos[VZ]};

    FMOD_Channel_Set3DAttributes(channel, &position, 0);
}

void soundchannel_setvolume(FMOD_CHANNEL* channel, float volume)
{
	FMOD_Channel_SetVolume(channel, volume);
}

void soundchannel_setfreq(FMOD_CHANNEL* channel, float freq)
{
	FMOD_Channel_SetFrequency(channel, 44100.f * freq);
}


void soundchannel_toggle(FMOD_CHANNEL* channel)
{
	FMOD_BOOL state;

	FMOD_Channel_GetPaused(channel, &state);

	if (state)
		FMOD_Channel_SetPaused(channel, false);
	else
		FMOD_Channel_SetPaused(channel, true);
}

void soundchannel_stop(FMOD_CHANNEL* channel)
{
	FMOD_Channel_Stop(channel);
}