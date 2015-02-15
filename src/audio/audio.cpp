#include	"audio.h"

#include	<fmod.h>
#include	"../error.h"


void audiomanager_startup(struct audiomanager* am)
{
	int i;

	// create audio system
	FMOD_System_Create(&am->system);

	// initialize sound system
	FMOD_System_Init(am->system, 100, FMOD_INIT_NORMAL, 0);

	for (i = 0; i < AUDIO_MAX_SOUNDS; i++)
	{
		am->sounds[i].track = NULL;
		am->sounds[i].enabled = false;
	}

	// default volume to 100%
	am->volume = 1.f;
}

void audiomanager_shutdown(struct audiomanager* am)
{
	int i;

	for (i = 0; i < AUDIO_MAX_SOUNDS; i++)
		if (am->sounds[i].enabled)
			FMOD_Sound_Release(am->sounds[i].track);

	FMOD_System_Release(am->system);
}

unsigned audiomanager_getlibversion(struct audiomanager* am)
{
	unsigned version;

	FMOD_System_GetVersion(am->system, &version);

	return version;
}


int audiomanager_newsound(struct audiomanager* am, const char* filename)
{
	int i;

	for (i = 0; i < AUDIO_MAX_SOUNDS; i++)
	{
		if (!am->sounds[i].enabled)
		{
			FMOD_System_CreateSound(am->system, filename, FMOD_SOFTWARE, 0, &am->sounds[i].track);
			am->sounds[i].enabled = true;

			return i;
		}
	}

	return -1;
}

void audiomanager_removesound(struct audiomanager* am, int id)
{
	FMOD_Sound_Release(am->sounds[id].track);
	am->sounds[id].track = NULL;
	am->sounds[id].enabled = false;
}


void audiomanager_playsound(struct audiomanager* am, int id, int loops)
{
	FMOD_CHANNEL* channel;

	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, am->sounds[id].track, true, &channel);

	// set volume and loop count
	FMOD_Channel_SetVolume(channel, am->volume);
	FMOD_Channel_SetLoopCount(channel, loops);
	
	// play the sound
	FMOD_Channel_SetPaused(channel, false);
}