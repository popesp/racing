#ifndef UI
#define	UI


#include	<ft2build.h>
#include	FT_FREETYPE_H
#include	"../math/vec2f.h"
#include	"../render/render.h"
#include	"../render/texture.h"
#include	"../render/window.h"


#define	UI_FONT_FILENAME_DEFAULT	"res/fonts/labtsec.ttf"

#define	UI_DEFAULT_FONTSIZE			16

#define	UI_FONT_GLYPH_COUNT			128
#define	UI_FONT_TEXTURE_WIDTH		16
#define	UI_FONT_TEXTURE_ROWS		8


struct font
{
	FT_Face face;
	struct texture texture;
	struct renderable renderable;
	vec2f coords[128][2];
};

struct uimanager
{
	struct window* window;

	FT_Library freetype;

	struct font* activefont;

	struct font font_default;
};


void uimanager_startup(struct uimanager* um);

void uimanager_shutdown(struct uimanager* um);


void font_create(struct font* font, struct uimanager* um, const char* filename);

void font_delete(struct font* font);


void font_setsize(struct font* font,  unsigned size);


#endif
