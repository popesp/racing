#include	"ui.h"

#include	"game.h"
#include	"../mem.h"


static char* font_filenames[UI_FONT_COUNT] =
{
	UI_FONT_FILENAME_LABTSEC,
	UI_FONT_FILENAME_BEBAS,
	UI_FONT_FILENAME_BEBAS,
	UI_FONT_FILENAME_BEBAS,
	UI_FONT_FILENAME_SEASRN,
	UI_FONT_FILENAME_SEASRN,
	UI_FONT_FILENAME_AERO,
	UI_FONT_FILENAME_AERO,
	UI_FONT_FILENAME_AERO
};

static unsigned font_sizes[UI_FONT_COUNT] =
{
	48,
	48,
	78,
	100,
	48,
	100,
	48,
	78,
	140
};


static void resetrace(struct game* game)
{
	// reset skybox
	skybox_reset(&game->skybox);

	// reset game objects
	entitymanager_reset(&game->entitymanager);

	// reset vehicles
	vehiclemanager_reset(&game->vehiclemanager);

	// reset pickups
	pickupmanager_reset(&game->pickupmanager);

	// reset game music
	soundchannel_stop(game->currentchannel);
	game->index_currentsong = 0;
	game->currentchannel = audiomanager_playmusic(&game->audiomanager, game->songs[game->index_currentsong], -1, true);

	// unpause in-game music
	audiomanager_ingamepausedstate(&game->audiomanager, false);

	game->state = GAME_STATE_RACE;
	game->timer_racestart = GAME_TIMER_RACESTART;

	// reset clock
	glfwSetTime(0.);
}

static void endrace(struct game* game)
{
	unsigned i;

	// delete in-game music
	soundchannel_pausedstate(game->currentchannel, true);
	sound_delete(game->songs[GAME_MUSIC_1_ID]);
	sound_delete(game->songs[GAME_MUSIC_2_ID]);
	sound_delete(game->songs[GAME_MUSIC_3_ID]);
	sound_delete(game->songs[GAME_MUSIC_4_ID]);

	// unpause in-game music
	audiomanager_ingamepausedstate(&game->audiomanager, false);

	// shut down pickup manager
	pickupmanager_shutdown(&game->pickupmanager);

	// delete player objects
	player_delete(&game->player);
	for (i = 0; i < GAME_AIPLAYER_COUNT; i++)
		aiplayer_delete(game->aiplayers + i);

	// shut down remaining track systems
	vehiclemanager_shutdown(&game->vehiclemanager);
	entitymanager_shutdown(&game->entitymanager);

	// delete start sign object
	texture_delete(&game->diffuse_startsign);
	renderable_deallocate(&game->startsign);

	// delete the track
	track_delete(&game->track);

	// delete the skybox
	skybox_delete(&game->skybox);
}


static char* menu_option_names[UI_MENU_OPTION_COUNT] =
{
	"Start    Game",
	"Settings",
	"Credits",
	"Exit"
};

static void menu_option_play(struct game* game)
{
	game->state = GAME_STATE_LOADRACE;
}

static void menu_option_settings(struct game* game)
{
	game->state = GAME_STATE_SETTINGS;
	game->uimanager.index_menuselection = 0;
}

static void menu_option_credits(struct game* game)
{
	game->state = GAME_STATE_CREDITS;
}

static void menu_option_exit(struct game* game)
{
	game->flags |= GAME_FLAG_TERMINATED;
}

static void (* menu_option_function[UI_MENU_OPTION_COUNT])(struct game*) =
{
	menu_option_play,
	menu_option_settings,
	menu_option_credits,
	menu_option_exit
};

static char* pausemenu_option_names[UI_PAUSEMENU_OPTION_COUNT] =
{
	"Resume",
	"Settings",
	"Restart",
	"Main      Menu"
};

static void pausemenu_option_resume(struct game* game)
{
	// unpause in-game music
	audiomanager_ingamepausedstate(&game->audiomanager, false);

	game->state = GAME_STATE_RACE;
}

static void pausemenu_option_settings(struct game* game)
{
	game->state = GAME_STATE_PAUSESETTINGS;
	game->uimanager.index_menuselection = 0;
}

static void pausemenu_option_restart(struct game* game)
{
	resetrace(game);
}

static void pausemenu_option_mainmenu(struct game* game)
{
	endrace(game);

	game->menupauseswitch = true;
	game->state = GAME_STATE_MAINMENU;
	game->uimanager.index_menuselection = 0;

	menuscreen(game);
}

static void (* pausemenu_option_function[UI_PAUSEMENU_OPTION_COUNT])(struct game*) =
{
	pausemenu_option_resume,
	pausemenu_option_settings,
	pausemenu_option_restart,
	pausemenu_option_mainmenu
};

static char* racedone_option_names[UI_RACEDONE_OPTION_COUNT] =
{
	"Restart",
	"Main      Menu"
};

static void racedone_option_restart(struct game* game)
{
	resetrace(game);
}

static void racedone_option_mainmenu(struct game* game)
{
	endrace(game);

	game->menupauseswitch = true;
	game->state = GAME_STATE_MAINMENU;
	game->uimanager.index_menuselection = 0;

	menuscreen(game);
}

static void (* racedone_option_function[UI_RACEDONE_OPTION_COUNT])(struct game*) =
{
	racedone_option_restart,
	racedone_option_mainmenu
};

static char* credits_names[UI_CREDITS_COUNT] =
{
	"Kurtis      Danyluk",
	"Kyle      Kajorinne",
	"John      Orton",
	"Samuel      Evans",
	"Shawn      Sutherland"
};


void uimanager_startup(struct uimanager* um, struct audiomanager* am, struct window* window, struct renderer* r)
{
	FT_Error error;
	unsigned i;

	um->am = am;
	um->window = window;
	um->r = r;

	error = FT_Init_FreeType(&um->freetype);
	if (error)
		return;

	um->index_menuselection = 0;

	// generate fonts
	for (i = 0; i < UI_FONT_COUNT; i++)
		font_generate(um->fonts + i, um, font_filenames[i], font_sizes[i]);

	// create sounds
	um->sfx_move = audiomanager_newsfx(am, UI_SFX_FILENAME_MOVE, false);
	um->sfx_select = audiomanager_newsfx(am, UI_SFX_FILENAME_SELECT, false);
	um->sfx_cancel = audiomanager_newsfx(am, UI_SFX_FILENAME_CANCEL, false);
}

void uimanager_shutdown(struct uimanager* um)
{
	unsigned i;

	// delete fonts
	for (i = 0; i < UI_FONT_COUNT; i++)
		font_delete(um->fonts + i);

	FT_Done_FreeType(um->freetype);

	// delete sounds
	sound_delete(um->sfx_move);
	sound_delete(um->sfx_select);
	sound_delete(um->sfx_cancel);
}


static void convertcoords(struct uimanager* um, int x, int y, float* fx, float* fy)
{
	*fx = (float)(x - (um->window->width / 2)) / (float)(um->window->width / 2);
	*fy = (float)((um->window->height / 2) - y) / (float)(um->window->height / 2);
}

static float* renderchar(struct uimanager* um, int x, int y, char c, vec3f color, float* ptr)
{
	struct glyph* g;
	float lx, rx;
	float ty, by;

	g = um->activefont->glyphs + c;

	x += g->left;
	y -= g->top;

	convertcoords(um, x, y, &lx, &ty);
	convertcoords(um, x + um->activefont->glyphs[c].width, y + um->activefont->glyphs[c].height, &rx, &by);

	vec3f_set(ptr, lx, ty, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->activefont->glyphs[c].coords[0][VU], um->activefont->glyphs[c].coords[0][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, lx, by, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->activefont->glyphs[c].coords[0][VU], um->activefont->glyphs[c].coords[1][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, rx, by, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->activefont->glyphs[c].coords[1][VU], um->activefont->glyphs[c].coords[1][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, lx, ty, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->activefont->glyphs[c].coords[0][VU], um->activefont->glyphs[c].coords[0][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, rx, by, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->activefont->glyphs[c].coords[1][VU], um->activefont->glyphs[c].coords[1][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, rx, ty, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->activefont->glyphs[c].coords[1][VU], um->activefont->glyphs[c].coords[0][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	return ptr;
}

unsigned stringwidth(struct font* font, const char* string)
{
	unsigned width, i;

	width = 0;
	for (i = 0; i < strlen(string); i++)
		width += font->glyphs[string[i]].xadvance;

	return width;
}

int renderstring(struct uimanager* um, unsigned char halign, unsigned char valign, int xoff, int yoff, char* string, vec3f color, bool selected)
{
	unsigned i, width, chars;
	vec3f selected_color;
	mat4f dummy;
	float* ptr;

	chars = strlen(string);
	if (selected)
		chars += 2;

	renderable_allocate(um->r, &um->activefont->renderable, chars * 6);
	ptr = um->activefont->renderable.buf_verts;

	// find the string width
	width = stringwidth(um->activefont, string);

	if (halign == UI_HALIGN_CENTER)
		xoff += (um->window->width / 2) - ((int)width / 2);
	else if (halign == UI_HALIGN_RIGHT)
		xoff += um->window->width - width;
	else if (halign == UI_HALIGN_CENTERRIGHT)
		xoff += (um->window->width / 2) - (int)width;
	else if (halign == UI_HALIGN_CENTERLEFT)
		xoff += (um->window->width / 2);

	if (valign == UI_VALIGN_CENTER)
		yoff += (um->window->height / 2);
	else if (valign == UI_VALIGN_BOTTOM)
		yoff += um->window->height;

	// render selected brackets
	if (selected)
	{
		vec3f_set(selected_color, UI_COLOR2);
		ptr = renderchar(um, xoff - um->activefont->glyphs['['].xadvance, yoff, '[', selected_color, ptr);
		ptr = renderchar(um, xoff + (int)width, yoff, ']', selected_color, ptr);
	}

	// render each character
	for (i = 0; i < strlen(string); i++)
	{
		ptr = renderchar(um, xoff, yoff, string[i], color, ptr);
		xoff += um->activefont->glyphs[string[i]].xadvance;
	}

	// send text buffer to opengl
	renderable_sendbuffer(um->r, &um->activefont->renderable);
	renderable_render(um->r, &um->activefont->renderable, dummy, dummy, 0);

	return xoff;
}

void renderguage(struct uimanager* um, unsigned char halign, unsigned char valign, int xoff, int yoff, float value, bool selected)
{
	unsigned width;
	vec3f color;

	um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;

	vec3f_set(color, UI_COLOR1);
	renderstring(um, halign, valign, xoff, yoff, "I--------I", color, selected);

	// find the guage width
	width = stringwidth(um->activefont, "I--------");

	vec3f_set(color, UI_COLOR3);
	renderstring(um, halign, valign, xoff + (int)(value * width), yoff, "I", color, false);
}

void uimanager_render(struct uimanager* um, struct game* game)
{
	unsigned i, lap;
	char text[256];
	physx::PxMat44 transform;
	float angle;
	vec3f color;
	int x;

	switch (game->state)
	{
	case GAME_STATE_MAINMENU:
		/* --- main menu options --- */
		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
		vec3f_set(color, UI_COLOR1);
		for (i = 0; i < UI_MENU_OPTION_COUNT; i++)
		{
			if (i == um->index_menuselection)
				renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, (int)i * UI_LIST_STRIDE, menu_option_names[i], color, true);
			else
				renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, (int)i * UI_LIST_STRIDE, menu_option_names[i], color, false);
		}
		/* ------ */

		/* --- main menu title --- */
		um->activefont = um->fonts + UI_FONT_AERO_MASSIVE;
		vec3f_set(color, UI_COLOR_RED);
		renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_TOP, 0, 170, GAME_TITLE, color, false);
		/* ------ */
		break;

	case GAME_STATE_LOADRACE:
		/* --- Loading message --- */
		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
		vec3f_set(color, UI_COLOR1);
		x = renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, (int)font_sizes[UI_FONT_BEBAS_MEDIUM] / 2, "Loading", color, false);

		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
		vec3f_set(color, UI_COLOR2);
		renderstring(um, UI_HALIGN_LEFT, UI_VALIGN_CENTER, x, (int)font_sizes[UI_FONT_BEBAS_MEDIUM] / 2, "...", color, false);
		/* ------ */
		break;

	case GAME_STATE_RACE:
		/* --- Start of game countdown --- */
		if (game->timer_racestart > 0)
		{
			if (game->timer_racestart > 60)
				sprintf(text, "%d", (game->timer_racestart-1) / 60);
			else
				sprintf(text, "Go!");

			um->activefont = um->fonts + UI_FONT_AERO_MASSIVE;
			vec3f_set(color, UI_COLOR1);
			renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, -100, text, color, false);

			renderable_allocate(&game->renderer, &um->activefont->renderable, strlen(text) * 6);
		}
		/* ------ */

		/* --- Wrong way --- */
		// transform local vectors
		transform.operator=(game->player.vehicle->body->getGlobalPose());
		vec3f forward;
		vec3f_set(forward, VEHICLE_FORWARD);
		mat4f_transformvec3f(forward, (float*)&transform);
			
		vec3f tan;
		vec3f_copy(tan, game->track.pathpoints[game->player.vehicle->index_track].tan);
		angle = vec3f_dot(forward, tan);
		if(angle < 0){
			um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
			vec3f_set(color, UI_COLOR1);
			renderstring(um, UI_HALIGN_CENTER,UI_VALIGN_BOTTOM, 0,-50, "Wrong Way", color, false);
		}
		/* ------- */

		/* --- Lap number --- */
		lap = game->player.vehicle->lap;
		sprintf(text, "Lap    %d/%d", (lap > 0) ? lap : lap + 1, game->laps);
		
		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
		vec3f_set(color, UI_COLOR1);
		renderstring(um, UI_HALIGN_LEFT, UI_VALIGN_BOTTOM, 50, -50, text, color, false);
		/* ------ */

		/* --- Place --- */
		sprintf(text, "%d", game->player.vehicle->place);

		um->activefont = um->fonts + UI_FONT_AERO_MASSIVE;
		vec3f_set(color, 1.f, 1.f, 0.f);
		x = renderstring(um, UI_HALIGN_LEFT, UI_VALIGN_BOTTOM, 50, -(50 + (int)font_sizes[UI_FONT_BEBAS_MEDIUM]), text, color, false);

		// place suffix
		if (game->player.vehicle->place%10 == 1)
			sprintf(text, "st");
		else if (game->player.vehicle->place%10 == 2)
			sprintf(text, "nd");
		else if (game->player.vehicle->place%10 == 3)
			sprintf(text, "rd");
		else
			sprintf(text, "th");

		um->activefont = um->fonts + UI_FONT_AERO_MEDIUM;
		renderstring(um, UI_HALIGN_LEFT, UI_VALIGN_BOTTOM, x, -(100 + (int)font_sizes[UI_FONT_BEBAS_MEDIUM]), text, color, false);
		/* ------ */

		/* --- Speed --- */
		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
		vec3f_set(color, 0.f, 0.f, 1.f);
		renderstring(um, UI_HALIGN_RIGHT, UI_VALIGN_BOTTOM, -50, -(50 + (int)font_sizes[UI_FONT_BEBAS_HUGE]), "Speed", color, false);

		// actual speed value
		sprintf(text, "%d", (int)game->player.vehicle->speed);

		um->activefont = um->fonts + UI_FONT_BEBAS_HUGE;
		renderstring(um, UI_HALIGN_RIGHT, UI_VALIGN_BOTTOM, -50, -50, text, color, false);
		/* ------ */
		break;

	case GAME_STATE_RACEDONE:
		/* --- post-race options --- */
		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
		vec3f_set(color, UI_COLOR1);
		for (i = 0; i < UI_RACEDONE_OPTION_COUNT; i++)
		{
			if (i == um->index_menuselection)
				renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, (int)i * UI_LIST_STRIDE, racedone_option_names[i], color, true);
			else
				renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, (int)i * UI_LIST_STRIDE, racedone_option_names[i], color, false);
		}
		/* ------ */

		/* --- You won/lost --- */
		if (game->winningvehicle == game->player.vehicle)
			sprintf(text, "You won!");
		else{
			char * names [] = {"Spock", "Shepard", "Darth Vader", "Han Solo",
								"The Doctor", "Morpheus", "Cthulhu", "Agent Smith",
								"Bender", "RoboCop", "Mulder","Buck Rogers",
								"HAL-9000","James T. Kirk", "River Tam","Marvin"
								};
			int j= 0;
			for(int i = 0; i < 7; i++){
				if( game->aiplayers[i].vehicle == game->winningvehicle)
					j = i;
			}
			sprintf(text, "%s Won!", names[j]);
		}
		um->activefont = um->fonts + UI_FONT_AERO_MASSIVE;
		vec3f_set(color, UI_COLOR_RED);
		renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_TOP, 0, 170, text, color, false);
		/* ------ */
		break;

	case GAME_STATE_PAUSEMENU:
		/* --- pause menu options --- */
		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
		vec3f_set(color, UI_COLOR1);
		for (i = 0; i < UI_PAUSEMENU_OPTION_COUNT; i++)
		{
			if (i == um->index_menuselection)
				renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, (int)i * UI_LIST_STRIDE, pausemenu_option_names[i], color, true);
			else
				renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, (int)i * UI_LIST_STRIDE, pausemenu_option_names[i], color, false);
		}
		/* ------ */

		/* --- Game Paused --- */
		um->activefont = um->fonts + UI_FONT_AERO_LARGE;
		vec3f_set(color, UI_COLOR_RED);
		renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_TOP, 0, 170, "Game Paused", color, false);
		/* ------ */
		break;

	case GAME_STATE_SETTINGS:
	case GAME_STATE_PAUSESETTINGS:
		/* --- Settings Title --- */
		um->activefont = um->fonts + UI_FONT_AERO_LARGE;
		vec3f_set(color, UI_COLOR_RED);
		renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_TOP, 0, 170, "Settings", color, false);
		/* ------ */

		/* --- Settings text --- */
		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;

		vec3f_set(color, UI_COLOR1);
		renderstring(um, UI_HALIGN_CENTERRIGHT, UI_VALIGN_CENTER, -20, (0 * UI_LIST_STRIDE) - 80, "Master    Volume:", color, false);
		renderstring(um, UI_HALIGN_CENTERRIGHT, UI_VALIGN_CENTER, -20, (1 * UI_LIST_STRIDE) - 80, "Music    Volume:", color, false);
		renderstring(um, UI_HALIGN_CENTERRIGHT, UI_VALIGN_CENTER, -20, (2 * UI_LIST_STRIDE) - 80, "Sound    Effect    Volume:", color, false);
		renderstring(um, UI_HALIGN_CENTERRIGHT, UI_VALIGN_CENTER, -20, (3 * UI_LIST_STRIDE) - 80, "Game    Difficulty:", color, false);

		if(game->menupauseswitch==true)
			renderstring(um, UI_HALIGN_CENTERRIGHT, UI_VALIGN_CENTER, -20, (4 * UI_LIST_STRIDE) - 80, "Number    of    laps:", color, false);

		renderguage(um, UI_HALIGN_CENTERLEFT, UI_VALIGN_CENTER, 20, (0 * UI_LIST_STRIDE) - 80, audiomanager_getmastervolume(um->am), 0 == um->index_menuselection);
		renderguage(um, UI_HALIGN_CENTERLEFT, UI_VALIGN_CENTER, 20, (1 * UI_LIST_STRIDE) - 80, audiomanager_getmusicvolume(um->am), 1 == um->index_menuselection);
		renderguage(um, UI_HALIGN_CENTERLEFT, UI_VALIGN_CENTER, 20, (2 * UI_LIST_STRIDE) - 80, audiomanager_getsfxvolume(um->am), 2 == um->index_menuselection);
		renderguage(um, UI_HALIGN_CENTERLEFT, UI_VALIGN_CENTER, 20, (3 * UI_LIST_STRIDE) - 80, game->difficulty, 3 == um->index_menuselection);

		if(game->menupauseswitch==true){
			sprintf(text, "%d", (int)game->laps);
			renderstring(um, UI_HALIGN_CENTERLEFT, UI_VALIGN_CENTER, 20, (4 * UI_LIST_STRIDE) - 80, text, color, 4 == um->index_menuselection);
		}

		// Press B to return
		vec3f_set(color, UI_COLOR2);
		renderstring(um, UI_HALIGN_RIGHT, UI_VALIGN_BOTTOM, -50, -50, UI_BACK_MESSAGE, color, false);
		/* ------ */
		break;

	case GAME_STATE_CREDITS:
		/* --- Credits Title --- */
		um->activefont = um->fonts + UI_FONT_AERO_LARGE;
		vec3f_set(color, UI_COLOR_RED);
		renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_TOP, 0, 170, "Credits", color, false);
		/* ------ */

		/* --- Credits Text --- */
		um->activefont = um->fonts + UI_FONT_BEBAS_MEDIUM;
		vec3f_set(color, UI_COLOR1);
		for (i = 0; i < UI_CREDITS_COUNT; i++)
			renderstring(um, UI_HALIGN_CENTER, UI_VALIGN_CENTER, 0, ((int)i * UI_LIST_STRIDE) - 80, credits_names[i], color, false);

		// Press B to return
		vec3f_set(color, UI_COLOR2);
		renderstring(um, UI_HALIGN_RIGHT, UI_VALIGN_BOTTOM, -50, -50, UI_BACK_MESSAGE, color, false);
		/* ------ */
		break;
	}
}


void uimanager_update(struct uimanager* um, struct game* game)
{
	float adjust, setting;

	switch (game->state)
	{
	case GAME_STATE_MAINMENU:
		// move up and down in the menu
		if (game->controller_main->buttons[INPUT_BUTTON_DDOWN] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			um->index_menuselection = (um->index_menuselection + 1) % UI_MENU_OPTION_COUNT;
			audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
		}
		if (game->controller_main->buttons[INPUT_BUTTON_DUP] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			um->index_menuselection = (um->index_menuselection - 1) % UI_MENU_OPTION_COUNT;
			audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
		}

		// make a selection
		if (game->controller_main->buttons[INPUT_BUTTON_A] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			audiomanager_playsfx(um->am, um->sfx_select, NULL, 0, false);
			menu_option_function[um->index_menuselection](game);
		}
		break;

	case GAME_STATE_SETTINGS:
	case GAME_STATE_PAUSESETTINGS:
		// move up and down in the menu
		if (game->controller_main->buttons[INPUT_BUTTON_DDOWN] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			if(game->menupauseswitch==true){
				um->index_menuselection = (um->index_menuselection + 1) % UI_SETTINGS_COUNT;
				audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
			}else{
				um->index_menuselection = (um->index_menuselection + 1) % (UI_SETTINGS_COUNT-1);
				audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
			}
		}
		if (game->controller_main->buttons[INPUT_BUTTON_DUP] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			if(game->menupauseswitch==true){
				um->index_menuselection = (um->index_menuselection - 1) % UI_SETTINGS_COUNT;
				audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
			}else{
				um->index_menuselection = (um->index_menuselection - 1) % (UI_SETTINGS_COUNT-1);
				audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
			}
		}

		if (game->controller_main->buttons[INPUT_BUTTON_DLEFT] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			switch (um->index_menuselection){
				case 4:
					if(game->laps>1)
						game->laps--;
					break;
			}
		}
		if (game->controller_main->buttons[INPUT_BUTTON_DRIGHT] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			switch (um->index_menuselection){
				case 4:
					if(game->laps<10)
						game->laps++;
					break;
			}
		}

		// change settings
		adjust = ((game->controller_main->buttons[INPUT_BUTTON_DLEFT] & INPUT_STATE_DOWN) * -1.f) + ((game->controller_main->buttons[INPUT_BUTTON_DRIGHT] & INPUT_STATE_DOWN) * 1.f);
		adjust *= UI_SETTINGS_ADJUSTSCALE;
		switch (um->index_menuselection)
		{
		case 0:
			setting = audiomanager_getmastervolume(um->am) + adjust;

			if (setting > 1.f)
				setting = 1.f;
			else if (setting < 0.f)
				setting = 0.f;

			audiomanager_setmastervolume(um->am, setting);
			break;

		case 1:
			setting = audiomanager_getmusicvolume(um->am) + adjust;

			if (setting > 1.f)
				setting = 1.f;
			else if (setting < 0.f)
				setting = 0.f;

			audiomanager_setmusicvolume(um->am, setting);
			break;

		case 2:
			setting = audiomanager_getsfxvolume(um->am) + adjust;

			if (setting > 1.f)
				setting = 1.f;
			else if (setting < 0.f)
				setting = 0.f;

			audiomanager_setsfxvolume(um->am, setting);
			break;

		case 3:
			setting = game->difficulty + adjust;

			if (setting > 1.f)
				setting = 1.f;
			else if (setting < 0.f)
				setting = 0.f;

			game->difficulty = setting;
			break;
	
		}
		

		if (game->controller_main->buttons[INPUT_BUTTON_B] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			audiomanager_playsfx(um->am, um->sfx_cancel, NULL, 0, false);
			game->uimanager.index_menuselection = 1;

			if (game->state == GAME_STATE_PAUSESETTINGS)
				game->state = GAME_STATE_PAUSEMENU;
			else
				game->state = GAME_STATE_MAINMENU;
		}
		break;

	case GAME_STATE_CREDITS:
		if (game->controller_main->buttons[INPUT_BUTTON_B] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			audiomanager_playsfx(um->am, um->sfx_cancel, NULL, 0, false);
			game->state = GAME_STATE_MAINMENU;
		}
		break;

	case GAME_STATE_PAUSEMENU:
		// move up and down in the menu
		if (game->controller_main->buttons[INPUT_BUTTON_DDOWN] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			um->index_menuselection = (um->index_menuselection + 1) % UI_PAUSEMENU_OPTION_COUNT;
			audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
		}
		if (game->controller_main->buttons[INPUT_BUTTON_DUP] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			um->index_menuselection = (um->index_menuselection - 1) % UI_PAUSEMENU_OPTION_COUNT;
			audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
		}

		// make a selection
		if (game->controller_main->buttons[INPUT_BUTTON_A] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			audiomanager_playsfx(um->am, um->sfx_select, NULL, 0, false);
			pausemenu_option_function[um->index_menuselection](game);
		}
		break;

	case GAME_STATE_RACEDONE:
		// move up and down in the menu
		if (game->controller_main->buttons[INPUT_BUTTON_DDOWN] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			um->index_menuselection = (um->index_menuselection + 1) % UI_RACEDONE_OPTION_COUNT;
			audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
		}
		if (game->controller_main->buttons[INPUT_BUTTON_DUP] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			um->index_menuselection = (um->index_menuselection - 1) % UI_RACEDONE_OPTION_COUNT;
			audiomanager_playsfx(um->am, um->sfx_move, NULL, 0, false);
		}

		// make a selection
		if (game->controller_main->buttons[INPUT_BUTTON_A] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			audiomanager_playsfx(um->am, um->sfx_select, NULL, 0, false);
			racedone_option_function[um->index_menuselection](game);
		}
	
	case GAME_STATE_RACE:
		if (game->controller_main->buttons[INPUT_BUTTON_LB] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			soundchannel_stop(game->currentchannel);
			game->index_currentsong = (game->index_currentsong - 1) % GAME_MUSIC_COUNT;
			game->currentchannel = audiomanager_playmusic(&game->audiomanager, game->songs[game->index_currentsong], -1, true);
		}

		if (game->controller_main->buttons[INPUT_BUTTON_RB] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			soundchannel_stop(game->currentchannel);
			game->index_currentsong = (game->index_currentsong + 1) % GAME_MUSIC_COUNT;
			game->currentchannel = audiomanager_playmusic(&game->audiomanager, game->songs[game->index_currentsong], -1, true);
		}

		// debug button TEMP
		if (game->controller_main->buttons[INPUT_BUTTON_Y] == (INPUT_STATE_CHANGED | INPUT_STATE_DOWN))
		{
			if (game->flags & GAME_FLAG_DEBUGCAM)
				game->flags &= ~GAME_FLAG_DEBUGCAM;
			else
				game->flags |= GAME_FLAG_DEBUGCAM;
		}
		break;
	}
}


void font_generate(struct font* font, struct uimanager* um, const char* filename, unsigned size)
{
	FT_GlyphSlot slot;
	FT_Error error;
	FT_Face face;
	int max_w, max_h;
	unsigned i;

	error = FT_New_Face(um->freetype, filename, 0, &face);
	if (error)
		printf("error\n");

	slot = face->glyph;

	FT_Set_Pixel_Sizes(face, size, size);

	// find the maximum glyph dimensions and store glyph data
	max_w = 0;
	max_h = 0;
	for (i = 0; i < UI_FONT_GLYPH_COUNT; i++)
	{
		FT_Load_Char(face, i, FT_LOAD_RENDER);

		font->glyphs[i].width = slot->bitmap.width;
		font->glyphs[i].height = slot->bitmap.rows;
		font->glyphs[i].left = slot->bitmap_left;
		font->glyphs[i].top = slot->bitmap_top;
		font->glyphs[i].xadvance = slot->advance.x >> 6;

		if (slot->bitmap.width > max_w)
			max_w = slot->bitmap.width;
		if (slot->bitmap.rows > max_h)
			max_h = slot->bitmap.rows;
	}

	// create font texture
	texture_init(&font->texture);
	texture_allocate(&font->texture, (unsigned)max_w * UI_FONT_TEXTURE_WIDTH, (unsigned)max_h * UI_FONT_TEXTURE_ROWS);
	texture_fill(&font->texture, 0, 0, 0, 0);

	// populate font texture data and generate uv coordinates
	for (i = 0; i < UI_FONT_GLYPH_COUNT; i++)
	{
		unsigned u, v;

		FT_Load_Char(face, i, FT_LOAD_RENDER);

		u = i % UI_FONT_TEXTURE_WIDTH;
		v = i / UI_FONT_TEXTURE_WIDTH;
		
		font->glyphs[i].coords[0][VU] = (float)u / (float)UI_FONT_TEXTURE_WIDTH;
		font->glyphs[i].coords[0][VV] = (float)v / (float)UI_FONT_TEXTURE_ROWS;
		font->glyphs[i].coords[1][VU] = ((float)u + ((float)font->glyphs[i].width / (float)max_w)) / (float)UI_FONT_TEXTURE_WIDTH;
		font->glyphs[i].coords[1][VV] = ((float)v + ((float)font->glyphs[i].height / (float)max_h)) / (float)UI_FONT_TEXTURE_ROWS;

		// blit the glyph to the font atlas
		texture_blitalpha(&font->texture, slot->bitmap.buffer, (unsigned)slot->bitmap.width, (unsigned)slot->bitmap.rows, u * max_w, v * max_h);
	}

	FT_Done_Face(face);

	texture_upload(&font->texture, RENDER_TEXTURE_DIFFUSE);

	renderable_init(&font->renderable, RENDER_MODE_TRIANGLES, RENDER_TYPE_TEXT, RENDER_FLAG_DYNAMIC);
	font->renderable.textures[RENDER_TEXTURE_DIFFUSE] = &font->texture;
}

void font_delete(struct font* font)
{
	texture_delete(&font->texture);
	renderable_deallocate(&font->renderable);
}