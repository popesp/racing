#ifndef	RENDER
#define	RENDER


#include	"../math/mat4f.h"
#include	"../math/vec3f.h"


#define	RENDER_ATTRIB_POS			0
#define	RENDER_ATTRIB_NOR			1
#define	RENDER_ATTRIB_TAN			2
#define	RENDER_ATTRIB_TEX			3
#define	RENDER_ATTRIB_COL			4

#define	RENDER_ATTRIBSIZE_POS		3
#define	RENDER_ATTRIBSIZE_NOR		3
#define	RENDER_ATTRIBSIZE_TAN		3
#define	RENDER_ATTRIBSIZE_TEX		2
#define	RENDER_ATTRIBSIZE_COL		3

#define	RENDER_MODE_POINTS			0
#define	RENDER_MODE_LINESTRIP		1
#define	RENDER_MODE_TRIANGLESTRIP	2
#define	RENDER_MODE_TRIANGLES		3

#define	RENDER_TYPE_COUNT			3
#define	RENDER_TYPE_WIREF			0
#define	RENDER_TYPE_SOLID			1
#define	RENDER_TYPE_BUMPM			2

#define	RENDER_VERTSIZE_WIREF		(RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_COL)
#define	RENDER_VERTSIZE_SOLID		(RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR)
#define	RENDER_VERTSIZE_BUMPM		(RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR + RENDER_ATTRIBSIZE_TAN + RENDER_ATTRIBSIZE_TEX)

#define	RENDER_LIGHT_UNIFORMS		3
#define	RENDER_LIGHT_POS			0
#define	RENDER_LIGHT_DIF			1
#define	RENDER_LIGHT_SPC			2

#define	RENDER_MATERIAL_UNIFORMS	4
#define	RENDER_MATERIAL_AMB			0
#define	RENDER_MATERIAL_DIF			1
#define	RENDER_MATERIAL_SPC			2
#define	RENDER_MATERIAL_SHN			3

#define	RENDER_TEXTURE_TYPES		1
#define	RENDER_TEXTURE_NORMAL		0

#define	RENDER_MAX_LIGHTS			2

#define	RENDER_DEFAULT_MATERIAL_AMB	0.1f, 0.1f, 0.1f
#define	RENDER_DEFAULT_MATERIAL_DIF	0.5f, 0.5f, 0.5f
#define	RENDER_DEFAULT_MATERIAL_SPC	0.4f, 0.4f, 0.4f
#define	RENDER_DEFAULT_MATERIAL_SHN	100.f

#define	RENDER_DEFAULT_LIGHT_POS	0.f, 0.f, 0.f
#define	RENDER_DEFAULT_LIGHT_DIF	0.f, 0.f, 0.f
#define	RENDER_DEFAULT_LIGHT_SPC	0.f, 0.f, 0.f

#define	RENDER_DEFAULT_AMBIENT		0.2f, 0.2f, 0.2f

#define	RENDER_SHADER_WIREFVERT		"shaders\\wiref.vert"
#define	RENDER_SHADER_WIREFFRAG		"shaders\\wiref.frag"
#define	RENDER_SHADER_SOLIDVERT		"shaders\\solid.vert"
#define	RENDER_SHADER_SOLIDFRAG		"shaders\\solid.frag"
#define	RENDER_SHADER_BUMPMVERT		"shaders\\bump.vert"
#define	RENDER_SHADER_BUMPMFRAG		"shaders\\bump.frag"

#define	RENDER_FLAG_NONE			0x00
#define	RENDER_FLAG_DYNAMIC			0x01


struct window;


struct light
{
	vec3f pos;
	vec3f dif;
	vec3f spc;
};

struct material
{
	vec3f amb;
	vec3f dif;
	vec3f spc;
	float shn;
};


struct renderable
{
	unsigned id_gl_mode;
	unsigned id_gl_vao;
	unsigned id_gl_buf;

	unsigned num_verts;
	float* buf_verts;

	struct light* lights[RENDER_MAX_LIGHTS];
	struct material material;
	float* ambient;

	unsigned id_gl_textures[RENDER_TEXTURE_TYPES];

	unsigned char flags;
	unsigned char type;

	char PADDING[2];
};


struct renderer
{
	unsigned id_gl_wiref;
	unsigned id_gl_solid;
	unsigned id_gl_bumpm;

	unsigned vertsize[RENDER_TYPE_COUNT];
	unsigned shader[RENDER_TYPE_COUNT];

	struct window* window;

	struct
	{
		int transform;
	} uniforms_wiref;

	struct
	{
		int transform;
		int eyepos;
		int lights[RENDER_MAX_LIGHTS][RENDER_LIGHT_UNIFORMS];
		int ambient;
		int material[RENDER_MATERIAL_UNIFORMS];
	} uniforms_solid;

	struct
	{
		int transform;
		int eyepos;
		int lights[RENDER_MAX_LIGHTS][RENDER_LIGHT_UNIFORMS];
		int ambient;
		int tex_normal;
		int material[RENDER_MATERIAL_UNIFORMS];
	} uniforms_bumpm;
};


void renderable_init(struct renderable*, unsigned char, unsigned char, unsigned char);
void renderable_allocate(struct renderer*, struct renderable*, unsigned);
void renderable_deallocate(struct renderable*);

void renderable_settexture(struct renderable*, unsigned char);

void renderable_sendbuffer(struct renderer*, struct renderable*);
void renderable_render(struct renderer*, struct renderable*, mat4f, unsigned);

unsigned renderer_init(struct renderer*, struct window*);


#endif