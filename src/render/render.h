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

#define	RENDER_TYPE_COUNT			5
#define	RENDER_TYPE_WIRE_S			0
#define	RENDER_TYPE_TXTR_S			1
#define	RENDER_TYPE_MATS_L			2
#define	RENDER_TYPE_TXTR_L			3
#define	RENDER_TYPE_BUMP_L			4

#define	RENDER_VERTSIZE_WIRE_S		(RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_COL)
#define	RENDER_VERTSIZE_TXTR_S		(RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_TEX)
#define	RENDER_VERTSIZE_MATS_L		(RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR)
#define	RENDER_VERTSIZE_TXTR_L		(RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR + RENDER_ATTRIBSIZE_TEX)
#define	RENDER_VERTSIZE_BUMP_L		(RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR + RENDER_ATTRIBSIZE_TAN + RENDER_ATTRIBSIZE_TEX)

#define	RENDER_LIGHT_UNIFORMS		3
#define	RENDER_LIGHT_POS			0
#define	RENDER_LIGHT_DIF			1
#define	RENDER_LIGHT_SPC			2

#define	RENDER_MATERIAL_UNIFORMS	4
#define	RENDER_MATERIAL_AMB			0
#define	RENDER_MATERIAL_DIF			1
#define	RENDER_MATERIAL_SPC			2
#define	RENDER_MATERIAL_SHN			3

#define	RENDER_TEXTURE_TYPES		2
#define	RENDER_TEXTURE_DIFFUSE		0
#define	RENDER_TEXTURE_NORMAL		1

#define	RENDER_MAX_LIGHTS			2

#define	RENDER_DEFAULT_MATERIAL_AMB	0.1f, 0.1f, 0.1f
#define	RENDER_DEFAULT_MATERIAL_DIF	0.5f, 0.5f, 0.5f
#define	RENDER_DEFAULT_MATERIAL_SPC	0.4f, 0.4f, 0.4f
#define	RENDER_DEFAULT_MATERIAL_SHN	100.f

#define	RENDER_DEFAULT_LIGHT_POS	0.f, 0.f, 0.f
#define	RENDER_DEFAULT_LIGHT_DIF	0.f, 0.f, 0.f
#define	RENDER_DEFAULT_LIGHT_SPC	0.f, 0.f, 0.f

#define	RENDER_DEFAULT_AMBIENT		0.2f, 0.2f, 0.2f

#define	RENDER_SHADER_VERT_WIRE_S	"shaders/wires.vert"
#define	RENDER_SHADER_FRAG_WIRE_S	"shaders/wires.frag"
#define	RENDER_SHADER_VERT_TXTR_S	"shaders/txtrs.vert"
#define	RENDER_SHADER_FRAG_TXTR_S	"shaders/txtrs.frag"
#define	RENDER_SHADER_VERT_MATS_L	"shaders/matsl.vert"
#define	RENDER_SHADER_FRAG_MATS_L	"shaders/matsl.frag"
#define	RENDER_SHADER_VERT_TXTR_L	"shaders/txtrl.vert"
#define	RENDER_SHADER_FRAG_TXTR_L	"shaders/txtrl.frag"
#define	RENDER_SHADER_VERT_BUMP_L	"shaders/bumpl.vert"
#define	RENDER_SHADER_FRAG_BUMP_L	"shaders/bumpl.frag"

#define	RENDER_FLAG_NONE			0x00
#define	RENDER_FLAG_DYNAMIC			0x01


struct texturemanager;
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

	int texture_ids[RENDER_TEXTURE_TYPES];

	unsigned char flags;
	unsigned char type;
};


struct renderer
{
	unsigned id_gl_wire_s;
	unsigned id_gl_txtr_s;
	unsigned id_gl_mats_l;
	unsigned id_gl_txtr_l;
	unsigned id_gl_bump_l;

	unsigned vertsize[RENDER_TYPE_COUNT];
	unsigned shader[RENDER_TYPE_COUNT];

	struct window* window;
	struct texturemanager* tm;

	struct
	{
		int transform;
	} uniforms_wire_s;

	struct
	{
		int transform;
		int tex_diffuse;
	} uniforms_txtr_s;

	struct
	{
		int transform;
		int eyepos;
		int lights[RENDER_MAX_LIGHTS][RENDER_LIGHT_UNIFORMS];
		int ambient;
		int material[RENDER_MATERIAL_UNIFORMS];
	} uniforms_mats_l;

	struct
	{
		int transform;
		int eyepos;
		int lights[RENDER_MAX_LIGHTS][RENDER_LIGHT_UNIFORMS];
		int ambient;
		int tex_diffuse;
		int material[RENDER_MATERIAL_UNIFORMS];
	} uniforms_txtr_l;

	struct
	{
		int transform;
		int eyepos;
		int lights[RENDER_MAX_LIGHTS][RENDER_LIGHT_UNIFORMS];
		int ambient;
		int tex_normal;
		int material[RENDER_MATERIAL_UNIFORMS];
	} uniforms_bump_l;
};


/*	initialize a renderable object
	param:	obj				renderable object to initialize (modified)
	param:	mode			draw mode to use when rendering
	param:	type			type of renderable object
	param:	flags			flags for the renderable object
*/
void renderable_init(struct renderable* obj, unsigned char mode, unsigned char type, unsigned char flags);

/*	allocate client and server side buffers for a renderable object
	param:	r				renderer object used to render
	param:	obj				renderable object to allocate buffers for (modified)
	param:	num_verts		number of vertices for the object
*/
void renderable_allocate(struct renderer* r, struct renderable* obj, unsigned num_verts);

/*	deallocate client side buffer for a renderable object
	param:	obj				renderable object to deallocate buffer for (modified)
*/
void renderable_deallocate(struct renderable* obj);


/*	send buffer data from client side to OpenGL
	param:	r				renderer object used to render
	param:	obj				renderable object for which to upload buffer
*/
void renderable_sendbuffer(struct renderer* r, struct renderable* obj);

/*	render an object with OpenGL (assuming the object's buffer has been uploaded)
	param:	r				renderer object used to render
	param:	obj				renderable object to draw
	param:	modelworld		transformation matrix for the object from model-space to world-space
	param:	worldview		transformation matrix for world-space to view-space
	param:	num_draw		number of vertices to draw from the buffer; if 0, draw them all
*/
void renderable_render(struct renderer* r, struct renderable* obj, mat4f modelworld, mat4f worldview, unsigned num_draw);

/*	initialize renderer, including shader objects
	param:	r				renderer struct to initialize (modified)
	param:	tm				texture manager
	param:	window			window to render to
*/
unsigned renderer_init(struct renderer* r, struct texturemanager* tm, struct window* window);


#endif