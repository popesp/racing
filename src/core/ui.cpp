#include	"ui.h"

#include	"game.h"
#include	"../mem.h"


void uimanager_startup(struct uimanager* um, struct window* window)
{
	FT_Error error;

	error = FT_Init_FreeType(&um->freetype);
	if (error)
		return;

	um->window = window;

	font_generate(&um->font_default, um, UI_FONT_FILENAME_DEFAULT, UI_DEFAULT_FONTSIZE);
	um->activefont = &um->font_default;
}

void uimanager_shutdown(struct uimanager* um)
{
	font_delete(&um->font_default);
	FT_Done_FreeType(um->freetype);
}


static void convertcoords(struct uimanager* um, int x, int y, float* fx, float* fy)
{
	*fx = (float)(x - (um->window->width / 2)) / (float)(um->window->width / 2);
	*fy = (float)((um->window->height / 2) - y) / (float)(um->window->height / 2);
}

static float* renderchar(struct uimanager* um, int x, int y, char c, vec3f color, float* ptr)
{
	float lx, rx;
	float ty, by;

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

void uimanager_render(struct uimanager* um, struct game* game)
{
	char velocity[256];
	mat4f dummy;
	vec3f color;
	unsigned i;
	float* ptr;
	int x, y;
	char c;

	sprintf(velocity, "Velocity: %d\n", (int)game->player.vehicle->speed);

	renderable_allocate(&game->renderer, &um->activefont->renderable, strlen(velocity) * 6);
	
	ptr = um->activefont->renderable.buf_verts;

	vec3f_set(color, 1.f, 0.f, 0.f);

	x = 100;
	y = 100;
	for (i = 0; i < strlen(velocity); i++)
	{
		c = velocity[i];

		ptr = renderchar(um, x + um->activefont->glyphs[c].left, y - um->activefont->glyphs[c].top, velocity[i], color, ptr);

		x += um->activefont->glyphs[c].xadvance;
	}

	renderable_sendbuffer(&game->renderer, &um->activefont->renderable);
	renderable_render(&game->renderer, &um->activefont->renderable, dummy, dummy, 0);
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