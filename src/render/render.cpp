#include	"render.h"

#include	<GL/glew.h>			// GL
#include	<GLFW/glfw3.h>		// GL
#include	<stdio.h>			// sprintf
#include	<string.h>			// strlen, strcpy
#include	"../math/mat4f.h"	// invertn, multiplyn
#include	"../math/vec3f.h"	// set, scalen
#include	"../mem.h"			// calloc, free
#include	"shader.h"			// create, program, link, delete
#include	"window.h"			// window.projection


// enables and initializes vertex attribute
static inline void attrib(unsigned index, int size, int vsize, int offs)
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, (GLsizei)(sizeof(float)*vsize), (void*)(sizeof(float)*offs));
}


/*	initialize a renderable object
	param:	obj				renderable object to initialize (modified)
	param:	mode			draw mode to use when rendering
	param:	type			type of renderable object
	param:	flags			flags for the renderable object
*/
void renderable_init(struct renderable* obj, unsigned char mode, unsigned char type, unsigned char flags)
{
	int i;

	switch (mode)
	{
	case RENDER_MODE_POINTS:
		obj->id_gl_mode = GL_POINTS;
		break;

	case RENDER_MODE_LINESTRIP:
		obj->id_gl_mode = GL_LINE_STRIP;
		break;

	case RENDER_MODE_TRIANGLESTRIP:
		obj->id_gl_mode = GL_TRIANGLE_STRIP;
		break;

	case RENDER_MODE_TRIANGLES:
		obj->id_gl_mode = GL_TRIANGLES;
		break;

	default:
		break;
	}

	// create OpenGL VAO and VBO
	glGenVertexArrays(1, (GLuint*)&obj->id_gl_vao);
	glGenBuffers(1, (GLuint*)&obj->id_gl_buf);

	// bind the vertex objects
	glBindVertexArray(obj->id_gl_vao);
	glBindBuffer(GL_ARRAY_BUFFER, obj->id_gl_buf);

	// initialize attributes given the object type
	switch (type)
	{
	case RENDER_TYPE_WIREF:
		attrib(RENDER_ATTRIB_POS, RENDER_ATTRIBSIZE_POS, RENDER_VERTSIZE_WIREF, 0);
		attrib(RENDER_ATTRIB_COL, RENDER_ATTRIBSIZE_COL, RENDER_VERTSIZE_WIREF, RENDER_ATTRIBSIZE_POS);
		break;

	case RENDER_TYPE_SOLID:
		attrib(RENDER_ATTRIB_POS, RENDER_ATTRIBSIZE_POS, RENDER_VERTSIZE_SOLID, 0);
		attrib(RENDER_ATTRIB_NOR, RENDER_ATTRIBSIZE_NOR, RENDER_VERTSIZE_SOLID, RENDER_ATTRIBSIZE_POS);
		break;

	case RENDER_TYPE_BUMPM:
		attrib(RENDER_ATTRIB_POS, RENDER_ATTRIBSIZE_POS, RENDER_VERTSIZE_BUMPM, 0);
		attrib(RENDER_ATTRIB_NOR, RENDER_ATTRIBSIZE_NOR, RENDER_VERTSIZE_BUMPM, RENDER_ATTRIBSIZE_POS);
		attrib(RENDER_ATTRIB_TAN, RENDER_ATTRIBSIZE_TAN, RENDER_VERTSIZE_BUMPM, RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR);
		attrib(RENDER_ATTRIB_TEX, RENDER_ATTRIBSIZE_TEX, RENDER_VERTSIZE_BUMPM, RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR + RENDER_ATTRIBSIZE_TAN);
		break;

	default:
		break;
	}

	obj->num_verts = 0;
	obj->buf_verts = NULL;

	for (i = 0; i < RENDER_MAX_LIGHTS; i++)
		obj->lights[i] = NULL;

	vec3f_set(obj->material.amb, RENDER_DEFAULT_MATERIAL_AMB);
	vec3f_set(obj->material.dif, RENDER_DEFAULT_MATERIAL_DIF);
	vec3f_set(obj->material.spc, RENDER_DEFAULT_MATERIAL_SPC);
	obj->material.shn = RENDER_DEFAULT_MATERIAL_SHN;

	obj->ambient = NULL;

	glGenTextures(RENDER_TEXTURE_TYPES, obj->id_gl_textures);

	for ( i = 0; i < RENDER_TEXTURE_TYPES; i++)
	{
		glBindTexture(GL_TEXTURE_2D, obj->id_gl_textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	obj->flags = flags;
	obj->type = type;
}

/*	allocate client and server side buffers for a renderable object
	param:	r				renderer object used to render
	param:	obj				renderable object to allocate buffers for (modified)
	param:	num_verts		number of vertices for the object
*/
void renderable_allocate(struct renderer* r, struct renderable* obj, unsigned num_verts)
{
	glBindBuffer(GL_ARRAY_BUFFER, obj->id_gl_buf);

	// allocate client side buffers
	obj->buf_verts = (float*)mem_realloc(obj->buf_verts, sizeof(float) * r->vertsize[obj->type] * num_verts);

	// allocate server (OpenGL) side buffer
	if (obj->flags & RENDER_FLAG_DYNAMIC)
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(float) * r->vertsize[obj->type] * num_verts), NULL, GL_DYNAMIC_DRAW);
	else
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(sizeof(float) * r->vertsize[obj->type] * num_verts), NULL, GL_STATIC_DRAW);

	obj->num_verts = num_verts;
}

/*	deallocate client side buffer for a renderable object
	param:	obj				renderable object to deallocate buffer for (modified)
*/
void renderable_deallocate(struct renderable* obj)
{
	mem_free(obj->buf_verts);
}


void renderable_settexture(struct renderable* obj, unsigned char type)
{
	glBindTexture(GL_TEXTURE_2D, obj->id_gl_textures[type]);

	// set texture data
}


/*	send buffer data from client side to OpenGL
	param:	r				renderer object used to render
	param:	obj				renderable object for which to upload buffer
*/
void renderable_sendbuffer(struct renderer* r, struct renderable* obj)
{
	glBindBuffer(GL_ARRAY_BUFFER, obj->id_gl_buf);
	glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(sizeof(float) * r->vertsize[obj->type] * obj->num_verts), obj->buf_verts);
}

/*	render an object with OpenGL (assuming the object's buffer has been uploaded)
	param:	r				renderer object used to render
	param:	obj				renderable object to draw
	param:	modelview		transformation matrix for the object from modelspace to viewspace
	param:	num_draw		number of vertices to draw from the buffer; if 0, draw them all
*/
void renderable_render(struct renderer* r, struct renderable* obj, mat4f modelview, unsigned num_draw)
{
	mat4f transform, inverse;
	vec3f eyepos, temp;
	int i;

	glUseProgram(r->shader[obj->type]);
	glBindVertexArray(obj->id_gl_vao);

	// calculate inverse modelview
	mat4f_invertn(inverse, modelview);

	// hack to extract eye position from inverse modelview
	vec3f_scalen(eyepos, inverse + C3, inverse[R3 + C3]);

	// calculate modelview-projection transform
	mat4f_multiplyn(transform, r->window->projection, modelview);

	switch (obj->type)
	{
	case RENDER_TYPE_WIREF:
		// MVP matrix
		glUniformMatrix4fv(r->uniforms_wiref.transform, 1, GL_FALSE, transform);
		break;

	// bump mapping uses all of the uniforms below as well
	case RENDER_TYPE_BUMPM:
		int zero = 0;
		glUniform1iv(r->uniforms_bumpm.tex_normal, 1, &zero);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obj->id_gl_textures[RENDER_TEXTURE_NORMAL]);

	case RENDER_TYPE_SOLID:
		// MVP matrix and eye position uniforms
		glUniformMatrix4fv(r->uniforms_solid.transform, 1, GL_FALSE, transform);
		glUniform3fv(r->uniforms_solid.eyepos, 1, eyepos);

		// light property uniforms
		for (i = 0; i < RENDER_MAX_LIGHTS; i++)
		{
			if (!obj->lights[i])
			{
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_POS);
				glUniform3fv(r->uniforms_solid.lights[i][RENDER_LIGHT_POS], 1, temp);
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_DIF);
				glUniform3fv(r->uniforms_solid.lights[i][RENDER_LIGHT_DIF], 1, temp);
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_SPC);
				glUniform3fv(r->uniforms_solid.lights[i][RENDER_LIGHT_SPC], 1, temp);
			}
			else
			{
				glUniform3fv(r->uniforms_solid.lights[i][RENDER_LIGHT_POS], 1, obj->lights[i]->pos);
				glUniform3fv(r->uniforms_solid.lights[i][RENDER_LIGHT_DIF], 1, obj->lights[i]->dif);
				glUniform3fv(r->uniforms_solid.lights[i][RENDER_LIGHT_SPC], 1, obj->lights[i]->spc);
			}
		}

		// ambient uniform
		if (!obj->ambient)
		{
			vec3f_set(temp, RENDER_DEFAULT_AMBIENT);
			glUniform3fv(r->uniforms_solid.ambient, 1, temp);
		}
		else
			glUniform3fv(r->uniforms_solid.ambient, 1, obj->ambient);

		// material uniforms
		glUniform3fv(r->uniforms_solid.material[RENDER_MATERIAL_AMB], 1, obj->material.amb);
		glUniform3fv(r->uniforms_solid.material[RENDER_MATERIAL_DIF], 1, obj->material.dif);
		glUniform3fv(r->uniforms_solid.material[RENDER_MATERIAL_SPC], 1, obj->material.spc);
		glUniform1fv(r->uniforms_solid.material[RENDER_MATERIAL_SHN], 1, &obj->material.shn);
		break;

	default:
		break;
	}

	if (num_draw)
		glDrawArrays(obj->id_gl_mode, 0, (GLsizei)num_draw);
	else
		glDrawArrays(obj->id_gl_mode, 0, (GLsizei)obj->num_verts);
}


/*	initialize renderer, including shader objects
	param:	r				renderer struct to initialize (modified)
	param:	window			window to render to
*/
unsigned renderer_init(struct renderer* r, struct window* window)
{
	unsigned wirefvert, wireffrag;
	unsigned solidvert, solidfrag;
	unsigned bumpmvert, bumpmfrag;
	char uniform[16];
	unsigned i, len;

	// initialize shaders
	if (!(wirefvert = shader_create(RENDER_SHADER_WIREFVERT, SHADER_VERTEX)))
		return 0;
	if (!(wireffrag = shader_create(RENDER_SHADER_WIREFFRAG, SHADER_FRAGMENT)))
		return 0;
	if (!(solidvert = shader_create(RENDER_SHADER_SOLIDVERT, SHADER_VERTEX)))
		return 0;
	if (!(solidfrag = shader_create(RENDER_SHADER_SOLIDFRAG, SHADER_FRAGMENT)))
		return 0;
	if (!(bumpmvert = shader_create(RENDER_SHADER_BUMPMVERT, SHADER_VERTEX)))
		return 0;
	if (!(bumpmfrag = shader_create(RENDER_SHADER_BUMPMFRAG, SHADER_FRAGMENT)))
		return 0;

	// create shader programs
	r->id_gl_wiref = shader_program(wirefvert, wireffrag);
	r->id_gl_solid = shader_program(solidvert, solidfrag);
	r->id_gl_bumpm = shader_program(bumpmvert, bumpmfrag);

	// bind attribute locations
	glBindAttribLocation(r->id_gl_wiref, RENDER_ATTRIB_POS, "vertpos");
	glBindAttribLocation(r->id_gl_wiref, RENDER_ATTRIB_COL, "vertcol");

	glBindAttribLocation(r->id_gl_solid, RENDER_ATTRIB_POS, "vertpos");
	glBindAttribLocation(r->id_gl_solid, RENDER_ATTRIB_NOR, "vertnor");

	glBindAttribLocation(r->id_gl_bumpm, RENDER_ATTRIB_POS, "vertpos");
	glBindAttribLocation(r->id_gl_bumpm, RENDER_ATTRIB_NOR, "vertnor");
	glBindAttribLocation(r->id_gl_bumpm, RENDER_ATTRIB_TAN, "verttan");
	glBindAttribLocation(r->id_gl_bumpm, RENDER_ATTRIB_TEX, "verttex");

	// link programs
	if (!shader_link(r->id_gl_wiref))
		return 0;
	if (!shader_link(r->id_gl_solid))
		return 0;
	if (!shader_link(r->id_gl_bumpm))
		return 0;

	// assign vertex sizes to each render type
	r->vertsize[RENDER_TYPE_WIREF] = RENDER_VERTSIZE_WIREF;
	r->vertsize[RENDER_TYPE_SOLID] = RENDER_VERTSIZE_SOLID;
	r->vertsize[RENDER_TYPE_BUMPM] = RENDER_VERTSIZE_BUMPM;

	// assign program ID's to render types
	r->shader[RENDER_TYPE_WIREF] = r->id_gl_wiref;
	r->shader[RENDER_TYPE_SOLID] = r->id_gl_solid;
	r->shader[RENDER_TYPE_BUMPM] = r->id_gl_bumpm;

	// register window pointer
	r->window = window;


	// get wireframe uniform locations
	r->uniforms_wiref.transform = glGetUniformLocation(r->id_gl_wiref, "transform");


	// get solid uniform locations
	r->uniforms_solid.transform = glGetUniformLocation(r->id_gl_solid, "transform");
	r->uniforms_solid.eyepos = glGetUniformLocation(r->id_gl_solid, "eyepos");

	for (i = 0; i < RENDER_MAX_LIGHTS; i++)
	{
		sprintf_s(uniform, 16, "lights[%d].", i);
		len = strlen(uniform);

		strcpy_s(uniform + len, 4, "pos");
		r->uniforms_solid.lights[i][RENDER_LIGHT_POS] = glGetUniformLocation(r->id_gl_solid, uniform);
		strcpy_s(uniform + len, 4, "dif");
		r->uniforms_solid.lights[i][RENDER_LIGHT_DIF] = glGetUniformLocation(r->id_gl_solid, uniform);
		strcpy_s(uniform + len, 4, "spc");
		r->uniforms_solid.lights[i][RENDER_LIGHT_SPC] = glGetUniformLocation(r->id_gl_solid, uniform);
	}

	r->uniforms_solid.ambient = glGetUniformLocation(r->id_gl_solid, "ambient");

	r->uniforms_solid.material[RENDER_MATERIAL_AMB] = glGetUniformLocation(r->id_gl_solid, "material.amb");
	r->uniforms_solid.material[RENDER_MATERIAL_DIF] = glGetUniformLocation(r->id_gl_solid, "material.dif");
	r->uniforms_solid.material[RENDER_MATERIAL_SPC] = glGetUniformLocation(r->id_gl_solid, "material.spc");
	r->uniforms_solid.material[RENDER_MATERIAL_SHN] = glGetUniformLocation(r->id_gl_solid, "material.shn");


	// get bump mapped uniform locations
	r->uniforms_bumpm.transform = glGetUniformLocation(r->id_gl_bumpm, "transform");
	r->uniforms_bumpm.eyepos = glGetUniformLocation(r->id_gl_bumpm, "eyepos");

	for (i = 0; i < RENDER_MAX_LIGHTS; i++)
	{
		sprintf_s(uniform, 16, "lights[%d].", i);
		len = strlen(uniform);

		strcpy_s(uniform + len, 4, "pos");
		r->uniforms_bumpm.lights[i][RENDER_LIGHT_POS] = glGetUniformLocation(r->id_gl_bumpm, uniform);
		strcpy_s(uniform + len, 4, "dif");
		r->uniforms_bumpm.lights[i][RENDER_LIGHT_DIF] = glGetUniformLocation(r->id_gl_bumpm, uniform);
		strcpy_s(uniform + len, 4, "spc");
		r->uniforms_bumpm.lights[i][RENDER_LIGHT_SPC] = glGetUniformLocation(r->id_gl_bumpm, uniform);
	}

	r->uniforms_bumpm.ambient = glGetUniformLocation(r->id_gl_bumpm, "ambient");

	r->uniforms_bumpm.tex_normal = glGetUniformLocation(r->id_gl_bumpm, "tex_normal");

	r->uniforms_bumpm.material[RENDER_MATERIAL_AMB] = glGetUniformLocation(r->id_gl_bumpm, "material.amb");
	r->uniforms_bumpm.material[RENDER_MATERIAL_DIF] = glGetUniformLocation(r->id_gl_bumpm, "material.dif");
	r->uniforms_bumpm.material[RENDER_MATERIAL_SPC] = glGetUniformLocation(r->id_gl_bumpm, "material.spc");
	r->uniforms_bumpm.material[RENDER_MATERIAL_SHN] = glGetUniformLocation(r->id_gl_bumpm, "material.shn");


	// flag shaders for deletion
	shader_delete(wirefvert);
	shader_delete(wireffrag);
	shader_delete(solidvert);
	shader_delete(solidfrag);
	shader_delete(bumpmvert);
	shader_delete(bumpmfrag);

	return 1;
}