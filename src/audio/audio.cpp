#include	"audio.h"

#include	<stdio.h>


void audiomanager_startup(struct audiomanager* am)
{
	FMOD_CHANNELGROUP* master;

	// create audio system
	FMOD_System_Create(&am->system);

	// initialize sound system
	FMOD_System_Init(am->system, 100, FMOD_INIT_3D_RIGHTHANDED, 0);
	FMOD_System_Set3DSettings(am->system, 1.f, 5.f, 1.f);

	// create channel and sound groups
	FMOD_System_CreateChannelGroup(am->system, NULL, &am->group_ingame);
	FMOD_System_CreateSoundGroup(am->system, NULL, &am->group_music);
	FMOD_System_CreateSoundGroup(am->system, NULL, &am->group_sfx);

	// add in-game channel group to master
	FMOD_System_GetMasterChannelGroup(am->system, &master);
	FMOD_ChannelGroup_AddGroup(master, am->group_ingame);

	FMOD_SoundGroup_SetVolume(am->group_music, AUDIO_MUSIC_VOLUMESCALE);
	FMOD_SoundGroup_SetVolume(am->group_sfx, AUDIO_SFX_VOLUMESCALE);
}

void audiomanager_shutdown(struct audiomanager* am)
{
	FMOD_ChannelGroup_Release(am->group_ingame);
	FMOD_SoundGroup_Release(am->group_music);
	FMOD_SoundGroup_Release(am->group_sfx);

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


FMOD_SOUND* audiomanager_newmusic(struct audiomanager* am, const char* filename)
{
	FMOD_SOUND* sound;

	FMOD_System_CreateStream(am->system, filename, FMOD_LOOP_NORMAL, 0, &sound);
	FMOD_Sound_SetSoundGroup(sound, am->group_music);

	return sound;
}

FMOD_SOUND* audiomanager_newsfx(struct audiomanager* am, const char* filename, bool is3D)
{
	FMOD_SOUND* sound;

	if (is3D)
		FMOD_System_CreateSound(am->system, filename, FMOD_LOOP_NORMAL | FMOD_3D | FMOD_3D_LINEARROLLOFF, 0, &sound);
	else
		FMOD_System_CreateSound(am->system, filename, FMOD_LOOP_NORMAL, 0, &sound);

	FMOD_Sound_SetSoundGroup(sound, am->group_sfx);

	FMOD_Sound_Set3DMinMaxDistance(sound, 1.f, 50.f);

	return sound;
}


FMOD_CHANNEL* audiomanager_playmusic(struct audiomanager* am, FMOD_SOUND* music, int loops, bool ingame, float volume)
{
	FMOD_CHANNEL* channel;

	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, music, true, &channel);

	if (ingame)
		FMOD_Channel_SetChannelGroup(channel, am->group_ingame);

	// set channel properties
	FMOD_Channel_SetLoopCount(channel, loops);
	FMOD_Channel_SetVolume(channel, volume);
	
	// play the sound
	FMOD_Channel_SetPaused(channel, false);

	return channel;
}

FMOD_CHANNEL* audiomanager_playsfx(struct audiomanager* am, FMOD_SOUND* sfx, vec3f pos, int loops, bool ingame, float volume)
{
	FMOD_CHANNEL* channel;
	
	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, sfx, true, &channel);
	
	if (ingame)
		FMOD_Channel_SetChannelGroup(channel, am->group_ingame);

	// position the sound effect in space
	if (pos)
		soundchannel_setposition(channel, pos);

	// set channel properties
	FMOD_Channel_SetLoopCount(channel, loops);
	FMOD_Channel_SetVolume(channel, volume);

	// play the sound
	FMOD_Channel_SetPaused(channel, false);

	return channel;
}


float audiomanager_getmusicvolume(struct audiomanager* am)
{
	float volume;

	FMOD_SoundGroup_GetVolume(am->group_music, &volume);

	return volume / AUDIO_MUSIC_VOLUMESCALE;
}

void audiomanager_setmusicvolume(struct audiomanager* am, float volume)
{
	FMOD_SoundGroup_SetVolume(am->group_music, AUDIO_MUSIC_VOLUMESCALE * volume);
}

float audiomanager_getsfxvolume(struct audiomanager* am)
{
	float volume;

	FMOD_SoundGroup_GetVolume(am->group_sfx, &volume);

	return volume / AUDIO_SFX_VOLUMESCALE;
}

void audiomanager_setsfxvolume(struct audiomanager* am, float volume)
{
	FMOD_SoundGroup_SetVolume(am->group_sfx, AUDIO_SFX_VOLUMESCALE * volume);
}

float audiomanager_getmastervolume(struct audiomanager* am)
{
	FMOD_CHANNELGROUP* master;
	float volume;

	FMOD_System_GetMasterChannelGroup(am->system, &master);
	FMOD_ChannelGroup_GetVolume(master, &volume);

	return volume;
}

void audiomanager_setmastervolume(struct audiomanager* am, float volume)
{
	FMOD_CHANNELGROUP* master;

	FMOD_System_GetMasterChannelGroup(am->system, &master);
	FMOD_ChannelGroup_SetVolume(master, volume);
}


void audiomanager_ingamepausedstate(struct audiomanager* am, bool paused)
{
	FMOD_ChannelGroup_SetPaused(am->group_ingame, paused);
}


void sound_delete(FMOD_SOUND* sound)
{
	FMOD_Sound_Release(sound);
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


void soundchannel_pausedstate(FMOD_CHANNEL* channel, bool paused)
{
	FMOD_Channel_SetPaused(channel, paused);
}

void soundchannel_toggle(FMOD_CHANNEL* channel)
{
	FMOD_BOOL paused;

	FMOD_Channel_GetPaused(channel, &paused);

	if (paused)
		FMOD_Channel_SetPaused(channel, false);
	else
		FMOD_Channel_SetPaused(channel, true);
}

void soundchannel_stop(FMOD_CHANNEL* channel)
{
	FMOD_Channel_Stop(channel);
}