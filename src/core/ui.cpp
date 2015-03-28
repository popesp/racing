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

	font_generate(&um->font_default, um, UI_FONT_FILENAME_LABTSEC, UI_DEFAULT_FONTSIZE);
	font_generate(&um->font_pause, um, UI_FONT_FILENAME_BEBAS, UI_PAUSE_FONTSIZE);
	font_generate(&um->font_playerlap, um, UI_FONT_FILENAME_BEBAS, UI_PLAYERLAP_FONTSIZE);
	font_generate(&um->font_place, um, UI_FONT_FILENAME_AERO, UI_PLACE_FONTSIZE);
	font_generate(&um->font_placer, um, UI_FONT_FILENAME_AERO, UI_PLACER_FONTSIZE);
	font_generate(&um->font_youwinlost, um, UI_FONT_FILENAME_AERO, UI_YOULOSE_FONTSIZE);
	font_generate(&um->font_velocity, um, UI_FONT_FILENAME_BEBAS, UI_VELOCITY_FONTSIZE);
	font_generate(&um->font_warpedsteel, um, UI_FONT_FILENAME_AERO, UI_WARPEDSTEEL_FONTSIZE);

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
	font_delete(&um->font_youwinlost);
	font_delete(&um->font_velocity);
	font_delete(&um->font_warpedsteel);

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

/*	adds a text that will be rendered on screen
	param:	um			uimanager
	param:	inputtext		text to be added
	param:	x			x coordinate
	param:	y			y coordinate
	param:  color			color of the text
	param:  font			font type and size
	param:  numberadder		for specialized font that can be automodified
*/
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

/*	removes a text on screen
	param:	um			uimanager
	param:	inputtext		the text to be removed
*/
void removetext(struct uimanager* um, char* inputtext){
	int i;

	for (i = 0; i < UI_TEXT_COUNT; i++)
		if ((um->texts + i)->inputtext==inputtext)
		{
			um->texts[i].inputtext = "";
			um->texts[i].flags = UI_TEXT_FLAG_INIT;
		}
}

/*	removes all text on screen
	param:	um			uimanager
*/
void removealltext(struct uimanager* um){
	for (int i =0; i < UI_TEXT_COUNT; i++){
		if(!((um->texts + i)->inputtext == "countdown")){
			um->texts[i].inputtext = "";
			um->texts[i].flags = UI_TEXT_FLAG_INIT;
		}
	}
}

/*	a simpler call to remove brackets for menus
	param:	um			uimanager
*/
void removebrackets(struct uimanager* um){
	
	removetext(um,"[                                                              ]");
	removetext(um,"[                                                    ]");
	removetext(um,"[                                                         ]");
	removetext(um,"[                                          ]");
	removetext(um,"[                                                 ]");
	removetext(um,"[                                                                    ]");
	removetext(um,"[                                  ]");
	removetext(um,"[                                                                                                                                      ]");
	removetext(um,"[                                                                                  ]");
	removetext(um,"[                                                                                                    ]");

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

			//See: numberadder in addtext()
			if(um->texts[j].numberadder==-1){
				sprintf(rendertext, "Lap   %d /%d", game->player.vehicle->lap, game->num_laps);
			}
			else if(um->texts[j].numberadder==111){
				sprintf(rendertext, "%d", game->dis);
			}
			else if(um->texts[j].numberadder==8){
				sprintf(rendertext, "%d", game->num_aiplayers);
			}
			else if(um->texts[j].numberadder==999){
				sprintf(rendertext, "%d", game->num_laps);
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
				sprintf(rendertext, "%d", (int)(game->player.vehicle->speed*10.7));
			}
			else if(um->texts[j].numberadder==666){
				for(int i=0;i<=game->num_aiplayers-1;i++){
					if(game->aiplayers[i].vehicle->lap==game->num_laps){
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

void displaymenu(struct game* game){
	vec3f color;
	vec3f_set(color, 1.0f,.0f,.0f);
	
	addtext(&game->uimanager, "Warped Steel", (game->window.width/2)-590, 170, color, &game->uimanager.font_warpedsteel, 0);

	vec3f_set(color, .0f,.0f,.0f);
	addtext(&game->uimanager,"Start     Game",(game->window.width/2)-90,300,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Settings",(game->window.width/2)-60,400,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Credits",(game->window.width/2)-50,500,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Exit     Game",(game->window.width/2)-80,600,color,&game->uimanager.font_playerlap,0);

	vec3f_set(color, .0f,.0f,1.0f);
	if(game->menuflags & MENU_FLAG_STARTGAME){
		addtext(&game->uimanager,"[                                                                    ]",(game->window.width/2)-110,300,color,&game->uimanager.font_playerlap,0);
	}
	else if(game->menuflags & MENU_FLAG_SETTINGS){
		addtext(&game->uimanager,"[                                                 ]",(game->window.width/2)-80,400,color,&game->uimanager.font_playerlap,0);
	}
	else if(game->menuflags & MENU_FLAG_CREDITS){
		addtext(&game->uimanager,"[                                          ]",(game->window.width/2)-70,500,color,&game->uimanager.font_playerlap,0);
	}
	else if(game->menuflags & MENU_FLAG_EXITGAME){
		addtext(&game->uimanager,"[                                                         ]",(game->window.width/2)-100,600,color,&game->uimanager.font_playerlap,0);
	}
	
}

void displaycredits(struct game* game){
	vec3f color;
	vec3f_set(color, 1.0f,.0f,.0f);//RED
	addtext(&game->uimanager, "Warped Steel", (game->window.width/2)-590, 170, color, &game->uimanager.font_warpedsteel, 0);

	vec3f_set(color,.0f,.0f,1.0f); //BLUE
	addtext(&game->uimanager,"CPSC   585   Winter   2015",(game->window.width/2)-220,250,color,&game->uimanager.font_playerlap,0);
	vec3f_set(color,0.0f,0.0f,.0f); //BLACK
	addtext(&game->uimanager,"Kyle   Kajorinne",(game->window.width/2)-540,400,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Kurtis   Danyluk",(game->window.width/2)-160,400,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Shawn   Sutherland",(game->window.width/2)+210,400,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Samuel   Evans",(game->window.width/2)-340,500,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Kyle   Orton",(game->window.width/2)+40,500,color,&game->uimanager.font_playerlap,0);

	addtext(&game->uimanager,"Back",(game->window.width/2)-50,600,color,&game->uimanager.font_playerlap,0);
	vec3f_set(color,.0f,.0f,1.0f); //BLUE
	addtext(&game->uimanager,"[                                  ]",(game->window.width/2)-80,600,color,&game->uimanager.font_playerlap,0);
}

void displaysettings(struct game* game){
	vec3f color;
	vec3f_set(color, 1.0f,.0f,.0f);//RED
	addtext(&game->uimanager, "Warped Steel", (game->window.width/2)-590, 170, color, &game->uimanager.font_warpedsteel, 0);

	vec3f_set(color,.0f,.0f,1.0f); //BLUE
	addtext(&game->uimanager,"Settings",(game->window.width/2)-90,250,color,&game->uimanager.font_playerlap,0);

	vec3f_set(color,.0f,.0f,.0f); //BLack
	addtext(&game->uimanager,"Number of Computers: ",(game->window.width/2)-260,350,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Number of Laps: ",(game->window.width/2)-190,550,color,&game->uimanager.font_playerlap,0);

	addtext(&game->uimanager,"Sound: ",(game->window.width/2)-140,450,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"/",(game->window.width/2)+70,450,color,&game->uimanager.font_playerlap,0);

	addtext(&game->uimanager,"Back",(game->window.width/2)-50,650,color,&game->uimanager.font_playerlap,0);

	vec3f_set(color,.0f,.0f,1.0f); //BLUE
	addtext(&game->uimanager,"numcomps",(game->window.width/2)+220,350,color,&game->uimanager.font_playerlap,8);
	addtext(&game->uimanager,"numlaps",(game->window.width/2)+160,550,color,&game->uimanager.font_playerlap,999);

	if(game->soundon==false){
		vec3f_set(color,1.0f,1.0f,1.0f); //whitE
	}
	addtext(&game->uimanager,"On",(game->window.width/2)+20,450,color,&game->uimanager.font_playerlap,0);
	vec3f_set(color,.0f,.0f,1.0f); //BLUE
	if(game->soundon==true){
		vec3f_set(color,1.0f,1.0f,1.0f); //whitE
	}
	addtext(&game->uimanager,"Off",(game->window.width/2)+100,450,color,&game->uimanager.font_playerlap,0);

	vec3f_set(color,.0f,.0f,1.0f); //BLUE
	if(game->menuflags & MENU_FLAG_NUMAI){
		addtext(&game->uimanager,"[                                                                                                                                      ]",(game->window.width/2)-280,350,color,&game->uimanager.font_playerlap,0);//numai
	}
	else if(game->menuflags & MENU_FLAG_SOUND){
		addtext(&game->uimanager,"[                                                                                  ]",(game->window.width/2)-160,450,color,&game->uimanager.font_playerlap,0);//sound
	}
	else if(game->anothermenuflag & MENU_FLAG_NUMLAPS){
		addtext(&game->uimanager,"[                                                                                                    ]",(game->window.width/2)-217,550,color,&game->uimanager.font_playerlap,0);
	}
	else{
		addtext(&game->uimanager,"[                                  ]",(game->window.width/2)-80,650,color,&game->uimanager.font_playerlap,0);//back
	}
	
}

void winlose(struct game* game){

	vec3f color;

	//remove race text
	removetext(&game->uimanager, "laps");
	removetext(&game->uimanager, "place");
	removetext(&game->uimanager, "placer");
	removetext(&game->uimanager, "Speed");
	removetext(&game->uimanager, "velocity");

	//Case: Game is lost
	if(game->flags & GAME_FLAG_YOULOSE){
		//set camera on winning AI
		for(int i=0;i<=game->num_aiplayers-1;i++){
			if(game->aiplayers[i].vehicle->lap==game->num_laps){
				aiwin_camera(&game->aiplayers[i]);
			}
		}

		vec3f_set(color,1.0f,0.0f,0.0f);
		addtext(&game->uimanager,"YOU LOST",(game->window.width/2)-440,200,color,&game->uimanager.font_youwinlost,0);

		vec3f_set(color,1.0f,1.0f,1.0f);
		addtext(&game->uimanager,"computerwon",(game->window.width/2)-295,280,color,&game->uimanager.font_playerlap,666);

	//Case: Game is won
	}else{
		vec3f_set(color,1.0f,0.0f,0.0f);
		addtext(&game->uimanager,"YOU WON",(game->window.width/2)-440,200,color,&game->uimanager.font_youwinlost,0);

		vec3f_set(color,1.0f,1.0f,1.0f);
		addtext(&game->uimanager,"Nice job!",(game->window.width/2)-90,280,color,&game->uimanager.font_playerlap,0);
	}
		
	vec3f_set(color,1.0f,1.0f,1.0f);
	addtext(&game->uimanager,"Restart",(game->window.width/2)-90,600,color,&game->uimanager.font_playerlap,0);
	addtext(&game->uimanager,"Main   Menu",(game->window.width/2)-110,700,color,&game->uimanager.font_playerlap,0);
	vec3f_set(color,.0f,.0f,1.0f);
	if(!(game->flags & GAME_FLAG_SWITCHON)){
		addtext(&game->uimanager,"[                                                    ]",(game->window.width/2)-120,600,color,&game->uimanager.font_playerlap,0);
	}else{
		addtext(&game->uimanager,"[                                                              ]",(game->window.width/2)-140,700,color,&game->uimanager.font_playerlap,0);
	}
}
