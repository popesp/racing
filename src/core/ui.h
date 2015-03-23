#ifndef UI
#define	UI


#include	<ft2build.h>
#include	FT_FREETYPE_H
#include	"../math/vec2f.h"
#include	"../render/render.h"
#include	"../render/texture.h"


#define	UI_FONT_FILENAME_DEFAULT	"res/fonts/labtsec.ttf"

#define	UI_DEFAULT_FONTSIZE			48

#define	UI_FONT_GLYPH_COUNT			128
#define	UI_FONT_TEXTURE_WIDTH		16
#define	UI_FONT_TEXTURE_ROWS		8


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

	struct font* activefont;

	struct font font_default;
};


void uimanager_startup(struct uimanager* um, struct window* window);

void uimanager_shutdown(struct uimanager* um);


void uimanager_render(struct uimanager* um, struct game* game);


void font_generate(struct font* font, struct uimanager* um, const char* filename, unsigned size);

void font_delete(struct font* font);


#endif
