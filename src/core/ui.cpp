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
	font_generate(&um->font_pause, um, UI_FONT_FILENAME_PAUSE, UI_PAUSE_FONTSIZE);
	font_generate(&um->font_playerlap, um, UI_FONT_FILENAME_PAUSE, UI_PLAYERLAP_FONTSIZE);
	font_generate(&um->font_place, um, UI_FONT_FILENAME_PLACE, UI_PLACE_FONTSIZE);
	font_generate(&um->font_placer, um, UI_FONT_FILENAME_PLACE, UI_PLACER_FONTSIZE);
	font_generate(&um->font_youlost, um, UI_FONT_FILENAME_YOULOSE, UI_YOULOSE_FONTSIZE);
	font_generate(&um->font_velocity, um, UI_FONT_FILENAME_PAUSE, UI_VELOCITY_FONTSIZE);

	for(int i=0;i<UI_TEXT_COUNT;i++){
		um->texts[i].flags=UI_TEXT_FLAG_INIT;
	}
}

void uimanager_shutdown(struct uimanager* um)
{
	font_delete(&um->font_default);
	font_delete(&um->font_pause);
	font_delete(&um->font_playerlap);
	font_delete(&um->font_place);
	font_delete(&um->font_placer);
	font_delete(&um->font_youlost);
	font_delete(&um->font_velocity);

	FT_Done_FreeType(um->freetype);
}


static void convertcoords(struct uimanager* um, int x, int y, float* fx, float* fy)
{
	*fx = (float)(x - (um->window->width / 2)) / (float)(um->window->width / 2);
	*fy = (float)((um->window->height / 2) - y) / (float)(um->window->height / 2);
}

static float* renderchar(struct uimanager* um, int x, int y, char c, vec3f color, float* ptr, int textnumber)
{
	float lx, rx;
	float ty, by;

	convertcoords(um, x, y, &lx, &ty);
	convertcoords(um, x + um->texts[textnumber].activefont->glyphs[c].width, y + um->texts[textnumber].activefont->glyphs[c].height, &rx, &by);

	vec3f_set(ptr, lx, ty, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->texts[textnumber].activefont->glyphs[c].coords[0][VU], um->texts[textnumber].activefont->glyphs[c].coords[0][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, lx, by, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->texts[textnumber].activefont->glyphs[c].coords[0][VU], um->texts[textnumber].activefont->glyphs[c].coords[1][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, rx, by, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->texts[textnumber].activefont->glyphs[c].coords[1][VU], um->texts[textnumber].activefont->glyphs[c].coords[1][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, lx, ty, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->texts[textnumber].activefont->glyphs[c].coords[0][VU], um->texts[textnumber].activefont->glyphs[c].coords[0][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, rx, by, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->texts[textnumber].activefont->glyphs[c].coords[1][VU], um->texts[textnumber].activefont->glyphs[c].coords[1][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	vec3f_set(ptr, rx, ty, 0.f);
	ptr += RENDER_ATTRIBSIZE_POS;
	vec3f_copy(ptr, color);
	ptr += RENDER_ATTRIBSIZE_COL;
	vec2f_set(ptr, um->texts[textnumber].activefont->glyphs[c].coords[1][VU], um->texts[textnumber].activefont->glyphs[c].coords[0][VV]);
	ptr += RENDER_ATTRIBSIZE_TEX;

	return ptr;
}

void addtext(struct uimanager* um, char* inputtext, int x, int y, vec3f color, struct font* font, int numberadder){
	int i;
	for (i = 0; i < UI_TEXT_COUNT; i++)
		if (!(um->texts[i].flags & UI_TEXT_FLAG_ENABLED))
			break;

	if (i == UI_TEXT_COUNT)
		return;

	struct text* t = um->texts + i;

	t->inputtext = inputtext;
	t->numberadder = numberadder;
	t->x = x;
	t->y = y;
	vec3f_copy(t->color, color);
	t->activefont= font;
	t->flags = UI_TEXT_FLAG_ENABLED;

}

void removetext(struct uimanager* um, char* inputtext){
	int i;

	for (i = 0; i < UI_TEXT_COUNT; i++)
		if ((um->texts + i)->inputtext==inputtext)
		{
			um->texts[i].inputtext = "";
			um->texts[i].flags = UI_TEXT_FLAG_INIT;
		}
}

void uimanager_render(struct uimanager* um, struct game* game)
{
	char rendertext[256];
	mat4f dummy;
	unsigned i,j;
	float* ptr;
	char c;

	for(j=0;j<UI_TEXT_COUNT;j++){
		if(um->texts[j].flags & UI_TEXT_FLAG_ENABLED){


			if(um->texts[j].numberadder==-1){
				sprintf(rendertext, "Lap   %d /%d", game->player.vehicle->lap, GAME_WINCONDITION_LAPS);
			}
			else if(um->texts[j].numberadder==-2){
				sprintf(rendertext, "%d", game->player.vehicle->place);
			}
			else if(um->texts[j].numberadder==-3){
				sprintf(rendertext, "st");
			}
			else if(um->texts[j].numberadder==-4){
				sprintf(rendertext, "nd");
			}
			else if(um->texts[j].numberadder==-5){
				sprintf(rendertext, "rd");
			}
			else if(um->texts[j].numberadder==-6){
				sprintf(rendertext, "th");
			}
			else if(um->texts[j].numberadder==9001){
				sprintf(rendertext, "%d", (int)game->player.vehicle->speed);
			}
			else if(um->texts[j].numberadder==666){
				for(int i=0;i<=game->num_aiplayers-1;i++){
					if(game->aiplayers[i].vehicle->lap==GAME_WINCONDITION_LAPS){
						sprintf(rendertext, "Computer   %d   won   the   race", i);
					}
				}
			}else{
				sprintf(rendertext, um->texts[j].inputtext);
			}

			renderable_allocate(&game->renderer, &um->texts[j].activefont->renderable, strlen(rendertext) * 6);

			ptr = um->texts[j].activefont->renderable.buf_verts;

			int x = um->texts[j].x;
			int y = um->texts[j].y;

			for (i = 0; i < strlen(rendertext); i++)
			{
				c = rendertext[i];

				ptr = renderchar(um, x + um->texts[j].activefont->glyphs[c].left, y - um->texts[j].activefont->glyphs[c].top, rendertext[i], um->texts[j].color, ptr, j);

				x += um->texts[j].activefont->glyphs[c].xadvance;
			}

			renderable_sendbuffer(&game->renderer, &um->texts[j].activefont->renderable);
			renderable_render(&game->renderer, &um->texts[j].activefont->renderable, dummy, dummy, 0);
		}
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