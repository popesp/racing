#include	"skybox.h"

#include	"../render/texture.h"


static vec3f skybox_pos[8] =
{
	{-1.f, -1.f, -1.f},
	{-1.f, 1.f, -1.f},
	{1.f, 1.f, -1.f},
	{1.f, -1.f, -1.f},
	{1.f, 1.f, 1.f},
	{-1.f, 1.f, 1.f},
	{-1.f, -1.f, 1.f},
	{1.f, -1.f, 1.f}
};

static int skybox_posindex[36] =
{
	0, 5, 6, 0, 1, 5,
	0, 7, 3, 0, 6, 7,
	0, 2, 1, 0, 3, 2,
	4, 3, 7, 4, 2, 3,
	4, 1, 2, 4, 5, 1,
	4, 6, 5, 4, 7, 6
};

static float skybox_uv[36][2] =
{
	{0.25f, 0.66667f}, {0.f, 0.33333f}, {0.f, 0.66667f}, {0.25f, 0.66667f}, {0.25f, 0.33333f}, {0.f, 0.33333f},
	{0.25f, 0.66667f}, {0.5f, 1.f}, {0.5f, 0.66667f}, {0.25f, 0.66667f}, {0.25f, 1.f}, {0.5f, 1.f},
	{0.25f, 0.66667f}, {0.5f, 0.33333f}, {0.25f, 0.33333f}, {0.25f, 0.66667f}, {0.5f, 0.66667f}, {0.5f, 0.33333f},
	{0.75f, 0.33333f}, {0.5f, 0.66667f}, {0.75f, 0.66667f}, {0.75f, 0.33333f}, {0.5f, 0.33333f}, {0.5f, 0.66667f},
	{0.5f, 0.f}, {0.25f, 0.33333f}, {0.5f, 0.33333f}, {0.5f, 0.f}, {0.25f, 0.f}, {0.25f, 0.33333f},
	{0.75f, 0.33333f}, {1.f, 0.66667f}, {1.f, 0.33333f}, {0.75f, 0.33333f}, {0.75f, 0.66667f}, {1.f, 0.66667f}
};


void skybox_init(struct skybox* sb, struct texturemanager* tm, const char* filename)
{
	renderable_init(&sb->r_skybox, RENDER_MODE_TRIANGLES, RENDER_TYPE_TXTR_S, RENDER_FLAG_NONE);

	sb->tm = tm;

	sb->texture = texturemanager_newtexture(tm);
	texture_loadfile(tm, sb->texture, filename);
	texture_upload(tm, sb->texture, RENDER_TEXTURE_DIFFUSE);
	sb->r_skybox.texture_ids[RENDER_TEXTURE_DIFFUSE] = sb->texture;
}

void skybox_delete(struct skybox* sb)
{
	renderable_deallocate(&sb->r_skybox);
	texturemanager_removetexture(sb->tm, sb->texture);
}


void skybox_generatemesh(struct renderer* r, struct skybox* sb)
{
	float* ptr;
	int i;

	renderable_allocate(r, &sb->r_skybox, 36);

	ptr = sb->r_skybox.buf_verts;

	for (i = 0; i < 36; i++)
	{
		vec3f_copy(ptr, skybox_pos[skybox_posindex[i]]);
		ptr += RENDER_ATTRIBSIZE_POS;
		
		ptr[0] = skybox_uv[i][0];
		ptr[1] = skybox_uv[i][1];
		ptr += RENDER_ATTRIBSIZE_TEX;
	}
}