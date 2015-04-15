#ifndef UI
#define	UI


#include	<ft2build.h>
#include	FT_FREETYPE_H
#include	"../math/vec2f.h"
#include	"../render/render.h"
#include	"../render/texture.h"

#define	MENU_FLAG_STARTGAME		0x01
#define	MENU_FLAG_SETTINGS		0x02
#define	MENU_FLAG_CREDITS		0x04
#define	MENU_FLAG_EXITGAME		0x08
#define	MENU_FLAG_SHUTDOWN		0x10
#define MENU_FLAG_INCREDITS		0x20
#define MENU_FLAG_INSETTINGS	0x40
#define MENU_FLAG_SOUND			0x80
#define MENU_FLAG_NUMAI			0x1b
#define MENU_FLAG_NUMLAPS		0x10

#define	UI_FONT_FILENAME_LABTSEC	"res/fonts/labtsec.ttf"
#define	UI_FONT_FILENAME_BEBAS		"res/fonts/bebas.ttf"
#define	UI_FONT_FILENAME_SEASRN		"res/fonts/seasrn.ttf"
#define	UI_FONT_FILENAME_AERO		"res/fonts/Aero.ttf"

#define	UI_DEFAULT_FONTSIZE			48
#define	UI_PAUSE_FONTSIZE			78
#define UI_PLAYERLAP_FONTSIZE		48
#define UI_PLACE_FONTSIZE			120
#define UI_PLACER_FONTSIZE			40
#define UI_YOULOSE_FONTSIZE			180
#define UI_VELOCITY_FONTSIZE		100
#define UI_WARPEDSTEEL_FONTSIZE		175

#define	UI_FONT_GLYPH_COUNT			128
#define	UI_FONT_TEXTURE_WIDTH		16
#define	UI_FONT_TEXTURE_ROWS		8

#define UI_TEXT_COUNT				30
#define	UI_TEXT_FLAG_INIT			0x00
#define	UI_TEXT_FLAG_ENABLED		0x01

#define num_names					16



struct text
{
	int x, y;
	const char* inputtext;
	vec3f color;

	int numberadder;

	unsigned char flags;

	struct font* activefont;
};

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
	struct window* window;

	FT_Library freetype;

	struct font font_default;
	struct font font_pause;
	struct font font_playerlap;
	struct font font_place;
	struct font font_placer;
	struct font font_youwinlost;
	struct font font_velocity;
	struct font font_warpedsteel;

	struct text texts[UI_TEXT_COUNT];
};


void uimanager_startup(struct uimanager* um, struct window* window);

void uimanager_shutdown(struct uimanager* um);

void uimanager_render(struct uimanager* um, struct game* game);

void font_generate(struct font* font, struct uimanager* um, const char* filename, unsigned size);

void font_delete(struct font* font);

void addtext(struct uimanager* um, char* inputtext, int x, int y, vec3f color, struct font* font, int numberadder);

void removetext(struct uimanager* um, char* inputtext);
void removealltext(struct uimanager* um);
void removebrackets(struct uimanager* um);

void displaymenu(struct game* game);
void displaycredits(struct game* game);
void displaysettings(struct game* game);
void winlose(struct game* game);

#endif
