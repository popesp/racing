#include	"ui.h"

#include	"../mem.h"


void uimanager_startup(struct uimanager* um)
{
	FT_GlyphSlot slot;
	FT_Error error;
	int i;

	error = FT_Init_FreeType(&um->freetype);
	if (error)
		return;

	font_create(&um->font_default, um, UI_FONT_FILENAME_DEFAULT);
}

void uimanager_shutdown(struct uimanager* um)
{
	font_delete(&um->font_default);
	FT_Done_FreeType(um->freetype);
}


void font_create(struct font* font, struct uimanager* um, const char* filename)
{
	unsigned max_w, max_h, i, j, k;
	FT_GlyphSlot slot;
	FT_Error error;

	error = FT_New_Face(um->freetype, filename, 0, &font->face);
	if (error)
		printf("error\n");

	slot = font->face->glyph;

	font_setsize(font, UI_DEFAULT_FONTSIZE);

	// find the maximum glyph dimensions
	max_w = 0;
	max_h = 0;
	for (i = 0; i < UI_FONT_GLYPH_COUNT; i++)
	{
		FT_Load_Char(font->face, i, FT_LOAD_RENDER);

		if (slot->bitmap.width > max_w)
			max_w = slot->bitmap.width;
		if (slot->bitmap.rows > max_h)
			max_h = slot->bitmap.rows;
	}

	// create font texture
	texture_init(&font->texture);
	texture_allocate(&font->texture, max_w * UI_FONT_TEXTURE_WIDTH, max_h * UI_FONT_TEXTURE_ROWS);

	// populate font texture data
	for (i = 0; i < UI_FONT_GLYPH_COUNT; i++)
	{
		FT_Load_Char(font->face, i, FT_LOAD_RENDER);

		// blit the glyph to the font atlas
		texture_blitalpha(&font->texture, slot->bitmap.buffer, slot->bitmap.width, slot->bitmap.rows, (i % UI_FONT_TEXTURE_WIDTH) * max_w, (i / UI_FONT_TEXTURE_WIDTH) * max_h);
	}

	renderable_init(&font->renderable, RENDER_MODE_TRIANGLES, RENDER_TYPE_TEXT, RENDER_FLAG_NONE);
	font->renderable.textures[RENDER_TEXTURE_DIFFUSE] = &font->texture;
}

void font_delete(struct font* font)
{
	FT_Done_Face(font->face);
	texture_delete(&font->texture);
	renderable_deallocate(&font->renderable);
}


void font_setsize(struct font* font,  unsigned size)
{
	FT_Set_Pixel_Sizes(font->face, size, size);
}