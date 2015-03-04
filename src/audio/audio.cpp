#include	"audio.h"
#include <windows.h>
#include	<fmod.h>



static void resetsound(struct sound* s)
{
	s->track = 0;
	s->channel = 0;
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
	FMOD_System_Init(am->system, 100, FMOD_INIT_NORMAL, 0);

	// Set the distance units. (meters/feet etc).
    FMOD_System_Set3DSettings(am->system, 1.0, DISTANCEFACTOR, 1.0f);
    
	// create channel groups
	FMOD_System_CreateChannelGroup(am->system, "music", &am->group_music);
	FMOD_System_CreateChannelGroup(am->system, "sfx", &am->group_sfx);

	
	// initialize music sounds
	for (i = 0; i < AUDIO_MAX_MUSIC; i++)
		resetsound(&am->music[i]);

	// initialize sfx sounds
	for (i = 0; i < AUDIO_MAX_SFX; i++)
		resetsound(&am->sfx[i]);
}

/*	shut down the audio manager
	param:	am				audio manager (modified)
*/
void audiomanager_shutdown(struct audiomanager* am)
{
	int i;

	// release music sounds
	for (i = 0; i < AUDIO_MAX_MUSIC; i++)
		if (am->music[i].enabled)
			FMOD_Sound_Release(am->music[i].track);

	// release sfx sounds
	for (i = 0; i < AUDIO_MAX_SFX; i++)
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


/*	create a new music sound object
	param:	am				audio manager
	param:	filename		path to the audio file
	return:	int				index to the sound object, or -1 if failure
*/
int audiomanager_newmusic(struct audiomanager* am, const char* filename)
{
	int i;

	for (i = 0; i < AUDIO_MAX_MUSIC; i++)
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
int audiomanager_newsfx(struct audiomanager* am, const char* filename, FMOD_MODE fmode)
{
	int i;

	for (i = 0; i < AUDIO_MAX_SFX; i++)
	{
		if (!am->sfx[i].enabled)
		{
			FMOD_System_CreateSound(am->system, filename, fmode, 0, &am->sfx[i].track);
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
*/
void audiomanager_playmusic(struct audiomanager* am, int id, int loops)
{
	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, am->music[id].track, true, &am->music[id].channel);
	FMOD_Channel_SetChannelGroup(am->music[id].channel, am->group_music);

	// set loop count
	FMOD_Channel_SetLoopCount(am->music[id].channel, loops);
	
	// play the sound
	FMOD_Channel_SetPaused(am->music[id].channel, false);
}

/*	play an sfx sound
	param:	am				audio manager
	param:	id				index to the sound object
*/
void audiomanager_playsfx(struct audiomanager* am, int id, vec3f playerpos)
{
	FMOD_System_PlaySound(am->system, FMOD_CHANNEL_FREE, am->sfx[id].track, true, &am->sfx[id].channel);
	FMOD_Channel_SetChannelGroup(am->sfx[id].channel, am->group_sfx);
	audiomanager_setsoundposition(am,id,playerpos);
	// play the sound
	FMOD_Channel_SetPaused(am->sfx[id].channel, false);
}


/*	stop a music sound
	param:	am				audio manager
	param:	id				index to the sound object
*/
void audiomanager_stopmusic(struct audiomanager* am, int id)
{
	FMOD_Channel_Stop(am->music[id].channel);
}


/*	set the music volume
	param:	am				audio manager
	param:	volume			new music volume
*/
void audiomanager_setmusicvolume(struct audiomanager* am, float volume)
{
	FMOD_ChannelGroup_SetVolume(am->group_music, volume);
}

/*	set the sfx volume
	param:	am				audio manager
	param:	volume			new sfx volume
*/
void audiomanager_setsfxvolume(struct audiomanager* am, float volume)
{
	FMOD_ChannelGroup_SetVolume(am->group_sfx, volume);
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
	param:	am				audio manager
	param:	id				index to the sound object
*/
void audiomanager_togglemusic(struct audiomanager* am, int id)
{
    FMOD_BOOL state;
   
    FMOD_Channel_GetPaused(am->music[id].channel, &state);

	if (state)
		FMOD_Channel_SetPaused(am->music[id].channel, false);
	else
		FMOD_Channel_SetPaused(am->music[id].channel, true);
}


  /*
        Play sounds at certain positions
    */
void audiomanager_setsoundposition(struct audiomanager* am, int id,vec3f playerpos)
{
    FMOD_VECTOR pos = { playerpos[VX] * DISTANCEFACTOR, playerpos[VX] * DISTANCEFACTOR, playerpos[VX] * DISTANCEFACTOR };
    FMOD_VECTOR vel = {  0.0f, 0.0f, 0.0f };

    FMOD_Channel_Set3DAttributes(am->sfx[id].channel, &pos, &vel);
    
}

/*	for changing sound locations
	param:	am				audio manager
	param:	id				index to the sound object
*/
void audiomanager_updatelistener(struct audiomanager* am, vec3f playerpos){
	
        // ==========================================================================================
        // UPDATE THE LISTENER
        // ==========================================================================================
        
            static float t = 0;
            static FMOD_VECTOR lastpos = { 0.0f, 0.0f, 0.0f };
			static FMOD_VECTOR listenerpos = { playerpos[VX], playerpos[VY], playerpos[VZ] };
            FMOD_VECTOR forward        = { 0.0f, 0.0f, 1.0f };
            FMOD_VECTOR up             = { 0.0f, 1.0f, 0.0f };
            FMOD_VECTOR vel;

       

            // ********* NOTE ******* READ NEXT COMMENT!!!!!
            // vel = how far we moved last FRAME (m/f), then time compensate it to SECONDS (m/s).
            vel.x = (listenerpos.x - lastpos.x) * (1000 / INTERFACE_UPDATETIME);
            vel.y = (listenerpos.y - lastpos.y) * (1000 / INTERFACE_UPDATETIME);
            vel.z = (listenerpos.z - lastpos.z) * (1000 / INTERFACE_UPDATETIME);

            // store pos for next time
            lastpos = listenerpos;

            FMOD_System_Set3DListenerAttributes(am->system,0, &listenerpos, &vel, &forward, &up);
            

            t += (30 * (1.0f / (float)INTERFACE_UPDATETIME));    // t is just a time value .. it increments in 30m/s steps in this example

           
        
		FMOD_System_Update(am->system);
        

        //Sleep(INTERFACE_UPDATETIME - 1);
}