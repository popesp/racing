#ifndef UI
#define	UI


#include	<ft2build.h>
#include	FT_FREETYPE_H
#include	"../audio/audio.h"
#include	"../math/vec2f.h"
#include	"../render/render.h"
#include	"../render/texture.h"


#define	UI_MENU_OPTION_COUNT		4
#define	UI_PAUSEMENU_OPTION_COUNT	4
#define	UI_RACEDONE_OPTION_COUNT	2
#define	UI_CREDITS_COUNT			5
#define	UI_SETTINGS_COUNT			5

#define	UI_LIST_STRIDE				60

#define	UI_SETTINGS_ADJUSTSCALE		0.01f

#define	UI_FONT_COUNT				9
#define	UI_FONT_LABTSEC_MEDIUM		0
#define	UI_FONT_BEBAS_MEDIUM		1
#define	UI_FONT_BEBAS_LARGE			2
#define	UI_FONT_BEBAS_HUGE			3
#define	UI_FONT_SEASRN_MEDIUM		4
#define	UI_FONT_SEASRN_HUGE			5
#define	UI_FONT_AERO_MEDIUM			6
#define	UI_FONT_AERO_LARGE			7
#define	UI_FONT_AERO_MASSIVE		8

#define	UI_FONT_FILENAME_LABTSEC	"res/fonts/labtsec.ttf"
#define	UI_FONT_FILENAME_BEBAS		"res/fonts/bebas.ttf"
#define	UI_FONT_FILENAME_SEASRN		"res/fonts/seasrn.ttf"
#define	UI_FONT_FILENAME_AERO		"res/fonts/Aero.ttf"

#define	UI_FONT_GLYPH_COUNT			128
#define	UI_FONT_TEXTURE_WIDTH		16
#define	UI_FONT_TEXTURE_ROWS		8

#define	UI_BACK_MESSAGE				"Press    B    to    return"

#define	UI_COLOR1					1.f, 1.f, 1.f
#define	UI_COLOR2					0.5f, 0.5f, 0.6f
#define	UI_COLOR3					0.8f, 0.5f, 0.5f
#define	UI_COLOR_RED				1.0f, 0.f, 0.f
#define	UI_COLOR_BLUE				0.0f, 0.f, 1.f
#define	UI_COLOR_BLACK				0.0f, 0.f, 0.f

#define	UI_HALIGN_LEFT				0x00
#define	UI_HALIGN_CENTER			0x01
#define	UI_HALIGN_RIGHT				0x02
#define	UI_HALIGN_CENTERRIGHT		0x03
#define	UI_HALIGN_CENTERLEFT		0x04
#define	UI_VALIGN_TOP				0x00
#define	UI_VALIGN_CENTER			0x01
#define	UI_VALIGN_BOTTOM			0x02

#define	UI_SFX_FILENAME_MOVE		"res/soundfx/ui_move.wav"
#define	UI_SFX_FILENAME_SELECT		"res/soundfx/ui_select.wav"
#define	UI_SFX_FILENAME_CANCEL		"res/soundfx/ui_cancel.wav"


struct glyph
{
	vec2f coords[2];
	int width, height;
	int left, top;
	int xadvance;
};

struct font
{
	struct texture texture;
	struct renderable renderable;

	struct glyph glyphs[UI_FONT_GLYPH_COUNT];
};

struct uimanager
{
	struct audiomanager* am;
	struct window* window;
	struct renderer* r;

	FT_Library freetype;

	unsigned index_menuselection;

	struct font fonts[UI_FONT_COUNT];
	struct font* activefont;

	struct renderable r_guage;

	FMOD_SOUND* sfx_move;
	FMOD_SOUND* sfx_select;
	FMOD_SOUND* sfx_cancel;
};


void uimanager_startup(struct uimanager* um, struct audiomanager* am, struct window* window, struct renderer* r);

void uimanager_shutdown(struct uimanager* um);


void uimanager_render(struct uimanager* um, struct game* game);


void uimanager_update(struct uimanager* um, struct game* game);


void font_generate(struct font* font, struct uimanager* um, const char* filename, unsigned size);

void font_delete(struct font* font);


#endif
