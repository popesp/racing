#include	"audio.h"

#include	<fmod.h>
#include	"../error.h"

#include	"../tinydir.h"
#include	"string.h"


void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        
    }
}

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
		am->sounds[i].channel = NULL;
		am->sounds[i].enabled = false;
		
		am->fx[i].track = NULL;
		am->fx[i].enabled = false;
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
	FMOD_RESULT result;

	for (i = 0; i < AUDIO_MAX_SOUNDS; i++)
	{
		if (!am->sounds[i].enabled)
		{
			result = FMOD_System_CreateSound(am->system, filename, FMOD_SOFTWARE, 0, &am->sounds[i].track);
			ERRCHECK(result);
			am->sounds[i].enabled = true;

			return i;
		}
	}

	return -1;
}

void audiomanager_playcrash(struct audiomanager* am){
	
}

void audiomanager_removesound(struct audiomanager* am, int id)
{
	FMOD_Sound_Release(am->sounds[id].track);
	am->sounds[id].track = NULL;
	am->sounds[id].channel = NULL;
	
	am->sounds[id].enabled = false;
}


void audiomanager_playsound(struct audiomanager* am, int id, int loops)
{
	
	
	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, am->sounds[id].track, true, &am->sounds[id].channel);

	// set volume and loop count
	FMOD_Channel_SetVolume(am->sounds[id].channel, am->volume);
	FMOD_Channel_SetLoopCount(am->sounds[id].channel, loops);
	
	// play the sound

	FMOD_Channel_SetPaused(am->sounds[id].channel, false);

}

void audiomanager_pausetoggle(struct audiomanager* am, int id)
{
	

    FMOD_BOOL state;
   
    FMOD_Channel_GetPaused(am->sounds[id].channel, &state);


	if (state)
		FMOD_Channel_SetPaused(am->sounds[id].channel,false);
	else
		FMOD_Channel_SetPaused(am->sounds[id].channel,true);
	
}


void audio_menu(struct audiomanager* am)
{
	
	tinydir_dir dir;
	tinydir_open(&dir, AUDIO_PATH);

	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);

		if ( file.extension[0] == 'm'){
			audiomanager_newsound(am, file.path);
			printf("%s added to game sounds\n", file.name);

		}

		tinydir_next(&dir);
	}

	tinydir_close(&dir);
}