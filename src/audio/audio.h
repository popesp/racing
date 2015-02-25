#ifndef AUDIO
#define	AUDIO


#include	<stdio.h>				// printf
#include	<fmod.h>
#include <fmod_errors.h>

#define	AUDIO_MAX_SOUNDS	20
#define AUDIO_PATH "res/music"



                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
struct sound
{
	FMOD_SOUND* track;

	bool enabled;
	bool is_playing;
};

struct audiomanager
{
	FMOD_SYSTEM* system;

	struct sound sounds[AUDIO_MAX_SOUNDS];
	                                                                 
	float volume;
};


void audiomanager_startup(struct audiomanager* am);
void audiomanager_shutdown(struct audiomanager* am);
unsigned audiomanager_getlibversion(struct audiomanager* am);

int audiomanager_newsound(struct audiomanager* am, const char* filename);
void audiomanager_removesound(struct audiomanager* am, int id);

void audiomanager_playsound(struct audiomanager* am, int id, int loops);
void audio_menu(struct audiomanager* am);

void audiomanager_pausetoggle(struct audiomanager* am);


#endif
