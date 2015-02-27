#include	"render.h"

#include	<GL/glew.h>			// GL
#include	<GLFW/glfw3.h>		// GL
#include	<stdio.h>			// sprintf
#include	<string.h>			// strlen, strcpy
#include	"../math/mat4f.h"	// invertn, multiplyn
#include	"../math/vec3f.h"	// set, scalen
#include	"../mem.h"			// calloc, free
#include	"shader.h"			// create, program, link, delete
#include	"texture.h"
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
	case RENDER_TYPE_WIRE_S:
		attrib(RENDER_ATTRIB_POS, RENDER_ATTRIBSIZE_POS, RENDER_VERTSIZE_WIRE_S, 0);
		attrib(RENDER_ATTRIB_COL, RENDER_ATTRIBSIZE_COL, RENDER_VERTSIZE_WIRE_S, RENDER_ATTRIBSIZE_POS);
		break;

	case RENDER_TYPE_TXTR_S:
		attrib(RENDER_ATTRIB_POS, RENDER_ATTRIBSIZE_POS, RENDER_VERTSIZE_TXTR_S, 0);
		attrib(RENDER_ATTRIB_TEX, RENDER_ATTRIBSIZE_TEX, RENDER_VERTSIZE_TXTR_S, RENDER_ATTRIBSIZE_POS);
		break;

	case RENDER_TYPE_MATS_L:
		attrib(RENDER_ATTRIB_POS, RENDER_ATTRIBSIZE_POS, RENDER_VERTSIZE_MATS_L, 0);
		attrib(RENDER_ATTRIB_NOR, RENDER_ATTRIBSIZE_NOR, RENDER_VERTSIZE_MATS_L, RENDER_ATTRIBSIZE_POS);
		break;

	case RENDER_TYPE_TXTR_L:
		attrib(RENDER_ATTRIB_POS, RENDER_ATTRIBSIZE_POS, RENDER_VERTSIZE_TXTR_L, 0);
		attrib(RENDER_ATTRIB_NOR, RENDER_ATTRIBSIZE_NOR, RENDER_VERTSIZE_TXTR_L, RENDER_ATTRIBSIZE_POS);
		attrib(RENDER_ATTRIB_TEX, RENDER_ATTRIBSIZE_TEX, RENDER_VERTSIZE_TXTR_L, RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR);
		break;

	case RENDER_TYPE_BUMP_L:
		attrib(RENDER_ATTRIB_POS, RENDER_ATTRIBSIZE_POS, RENDER_VERTSIZE_BUMP_L, 0);
		attrib(RENDER_ATTRIB_NOR, RENDER_ATTRIBSIZE_NOR, RENDER_VERTSIZE_BUMP_L, RENDER_ATTRIBSIZE_POS);
		attrib(RENDER_ATTRIB_TAN, RENDER_ATTRIBSIZE_TAN, RENDER_VERTSIZE_BUMP_L, RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR);
		attrib(RENDER_ATTRIB_TEX, RENDER_ATTRIBSIZE_TEX, RENDER_VERTSIZE_BUMP_L, RENDER_ATTRIBSIZE_POS + RENDER_ATTRIBSIZE_NOR + RENDER_ATTRIBSIZE_TAN);
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

	for (i = 0; i < RENDER_TEXTURE_TYPES; i++)
		obj->texture_ids[i] = 0;

	// set model transformation to the identity
	mat4f_identity(obj->matrix_model);

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
	param:	modelworld		transformation matrix for the object from model-space to world-space
	param:	worldview		transformation matrix for world-space to view-space
	param:	num_draw		number of vertices to draw from the buffer; if 0, draw them all
*/
void renderable_render(struct renderer* r, struct renderable* obj, mat4f modelworld, mat4f worldview, unsigned num_draw)
{
	mat4f modelview, inverse_mw, inverse_mv, mvp;
	vec3f eyepos, temp;
	int i, unit;

	glUseProgram(r->shader[obj->type]);
	glBindVertexArray(obj->id_gl_vao);

	// multiply by object model matrix
	mat4f_multiply(modelworld, obj->matrix_model);

	// calculate inverse model-world
	mat4f_invertn(inverse_mw, modelworld);

	// multiply world-view transform by model-world transform to get final model-view matrix
	mat4f_multiplyn(modelview, worldview, modelworld);

	// calculate inverse model-view
	mat4f_invertn(inverse_mv, modelview);

	// hack to extract eye position from inverse model-view
	vec3f_scalen(eyepos, inverse_mv + C3, inverse_mv[R3 + C3]);

	// calculate model-view-projection transform
	mat4f_multiplyn(mvp, r->window->projection, modelview);

	switch (obj->type)
	{
	case RENDER_TYPE_WIRE_S:
		// MVP matrix
		glUniformMatrix4fv(r->uniforms_wire_s.transform, 1, GL_FALSE, mvp);
		break;

	case RENDER_TYPE_TXTR_S:
		// MVP matrix
		glUniformMatrix4fv(r->uniforms_txtr_s.transform, 1, GL_FALSE, mvp);

		// texture uniforms
		unit = RENDER_TEXTURE_DIFFUSE;
		glUniform1iv(r->uniforms_txtr_s.tex_diffuse, 1, &unit);

		glActiveTexture(GL_TEXTURE0 + (unsigned)unit);
		glBindTexture(GL_TEXTURE_2D, r->tm->textures[obj->texture_ids[RENDER_TEXTURE_DIFFUSE]].gl_id);

		break;

	case RENDER_TYPE_MATS_L:
		// MVP matrix and eye position uniforms
		glUniformMatrix4fv(r->uniforms_mats_l.transform, 1, GL_FALSE, mvp);
		glUniform3fv(r->uniforms_mats_l.eyepos, 1, eyepos);

		// light property uniforms
		for (i = 0; i < RENDER_MAX_LIGHTS; i++)
		{
			if (!obj->lights[i])
			{
				// multiply light position by inverse model-world to get into model-space
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_POS);
				mat4f_fulltransformvec3f(temp, inverse_mw);
				glUniform3fv(r->uniforms_mats_l.lights[i][RENDER_LIGHT_POS], 1, temp);

				vec3f_set(temp, RENDER_DEFAULT_LIGHT_DIF);
				glUniform3fv(r->uniforms_mats_l.lights[i][RENDER_LIGHT_DIF], 1, temp);
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_SPC);
				glUniform3fv(r->uniforms_mats_l.lights[i][RENDER_LIGHT_SPC], 1, temp);
			}
			else
			{
				// multiply light position by inverse model-world to get into model-space
				vec3f_copy(temp, obj->lights[i]->pos);
				mat4f_fulltransformvec3f(temp, inverse_mw);
				glUniform3fv(r->uniforms_mats_l.lights[i][RENDER_LIGHT_POS], 1, temp);

				glUniform3fv(r->uniforms_mats_l.lights[i][RENDER_LIGHT_DIF], 1, obj->lights[i]->dif);
				glUniform3fv(r->uniforms_mats_l.lights[i][RENDER_LIGHT_SPC], 1, obj->lights[i]->spc);
			}
		}

		// ambient uniform
		if (!obj->ambient)
		{
			vec3f_set(temp, RENDER_DEFAULT_AMBIENT);
			glUniform3fv(r->uniforms_mats_l.ambient, 1, temp);
		}
		else
			glUniform3fv(r->uniforms_mats_l.ambient, 1, obj->ambient);

		// material uniforms
		glUniform3fv(r->uniforms_mats_l.material[RENDER_MATERIAL_AMB], 1, obj->material.amb);
		glUniform3fv(r->uniforms_mats_l.material[RENDER_MATERIAL_DIF], 1, obj->material.dif);
		glUniform3fv(r->uniforms_mats_l.material[RENDER_MATERIAL_SPC], 1, obj->material.spc);
		glUniform1fv(r->uniforms_mats_l.material[RENDER_MATERIAL_SHN], 1, &obj->material.shn);
		break;

	case RENDER_TYPE_TXTR_L:
		// MVP matrix and eye position uniforms
		glUniformMatrix4fv(r->uniforms_txtr_l.transform, 1, GL_FALSE, mvp);
		glUniform3fv(r->uniforms_txtr_l.eyepos, 1, eyepos);

		// light property uniforms
		for (i = 0; i < RENDER_MAX_LIGHTS; i++)
		{
			if (!obj->lights[i])
			{
				// multiply light position by inverse model-world to get into model-space
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_POS);
				mat4f_fulltransformvec3f(temp, inverse_mw);
				glUniform3fv(r->uniforms_txtr_l.lights[i][RENDER_LIGHT_POS], 1, temp);

				vec3f_set(temp, RENDER_DEFAULT_LIGHT_DIF);
				glUniform3fv(r->uniforms_txtr_l.lights[i][RENDER_LIGHT_DIF], 1, temp);
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_SPC);
				glUniform3fv(r->uniforms_txtr_l.lights[i][RENDER_LIGHT_SPC], 1, temp);
			}
			else
			{
				// multiply light position by inverse model-world to get into model-space
				vec3f_copy(temp, obj->lights[i]->pos);
				mat4f_fulltransformvec3f(temp, inverse_mw);
				glUniform3fv(r->uniforms_txtr_l.lights[i][RENDER_LIGHT_POS], 1, temp);

				glUniform3fv(r->uniforms_txtr_l.lights[i][RENDER_LIGHT_DIF], 1, obj->lights[i]->dif);
				glUniform3fv(r->uniforms_txtr_l.lights[i][RENDER_LIGHT_SPC], 1, obj->lights[i]->spc);
			}
		}

		// ambient uniform
		if (!obj->ambient)
		{
			vec3f_set(temp, RENDER_DEFAULT_AMBIENT);
			glUniform3fv(r->uniforms_txtr_l.ambient, 1, temp);
		}
		else
			glUniform3fv(r->uniforms_txtr_l.ambient, 1, obj->ambient);

		// texture uniforms
		unit = RENDER_TEXTURE_NORMAL;
		glUniform1iv(r->uniforms_txtr_l.tex_diffuse, 1, &unit);

		glActiveTexture(GL_TEXTURE0 + RENDER_TEXTURE_NORMAL);
		glBindTexture(GL_TEXTURE_2D, r->tm->textures[obj->texture_ids[RENDER_TEXTURE_DIFFUSE]].gl_id);

		// material uniforms
		glUniform3fv(r->uniforms_txtr_l.material[RENDER_MATERIAL_AMB], 1, obj->material.amb);
		glUniform3fv(r->uniforms_txtr_l.material[RENDER_MATERIAL_DIF], 1, obj->material.dif);
		glUniform3fv(r->uniforms_txtr_l.material[RENDER_MATERIAL_SPC], 1, obj->material.spc);
		glUniform1fv(r->uniforms_txtr_l.material[RENDER_MATERIAL_SHN], 1, &obj->material.shn);
		break;

	case RENDER_TYPE_BUMP_L:
		// MVP matrix and eye position uniforms
		glUniformMatrix4fv(r->uniforms_bump_l.transform, 1, GL_FALSE, mvp);
		glUniform3fv(r->uniforms_bump_l.eyepos, 1, eyepos);

		// light property uniforms
		for (i = 0; i < RENDER_MAX_LIGHTS; i++)
		{
			if (!obj->lights[i])
			{
				// multiply light position by inverse model-world to get into model-space
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_POS);
				mat4f_fulltransformvec3f(temp, inverse_mw);
				glUniform3fv(r->uniforms_bump_l.lights[i][RENDER_LIGHT_POS], 1, temp);

				vec3f_set(temp, RENDER_DEFAULT_LIGHT_DIF);
				glUniform3fv(r->uniforms_bump_l.lights[i][RENDER_LIGHT_DIF], 1, temp);
				vec3f_set(temp, RENDER_DEFAULT_LIGHT_SPC);
				glUniform3fv(r->uniforms_bump_l.lights[i][RENDER_LIGHT_SPC], 1, temp);
			}
			else
			{
				// multiply light position by inverse model-world to get into model-space
				vec3f_copy(temp, obj->lights[i]->pos);
				mat4f_fulltransformvec3f(temp, inverse_mw);
				glUniform3fv(r->uniforms_bump_l.lights[i][RENDER_LIGHT_POS], 1, temp);

				glUniform3fv(r->uniforms_bump_l.lights[i][RENDER_LIGHT_DIF], 1, obj->lights[i]->dif);
				glUniform3fv(r->uniforms_bump_l.lights[i][RENDER_LIGHT_SPC], 1, obj->lights[i]->spc);
			}
		}

		// ambient uniform
		if (!obj->ambient)
		{
			vec3f_set(temp, RENDER_DEFAULT_AMBIENT);
			glUniform3fv(r->uniforms_bump_l.ambient, 1, temp);
		}
		else
			glUniform3fv(r->uniforms_bump_l.ambient, 1, obj->ambient);

		// texture uniforms
		unit = RENDER_TEXTURE_NORMAL;
		glUniform1iv(r->uniforms_bump_l.tex_normal, 1, &unit);

		glActiveTexture(GL_TEXTURE0 + RENDER_TEXTURE_NORMAL);
		glBindTexture(GL_TEXTURE_2D, r->tm->textures[obj->texture_ids[RENDER_TEXTURE_NORMAL]].gl_id);

		// material uniforms
		glUniform3fv(r->uniforms_bump_l.material[RENDER_MATERIAL_AMB], 1, obj->material.amb);
		glUniform3fv(r->uniforms_bump_l.material[RENDER_MATERIAL_DIF], 1, obj->material.dif);
		glUniform3fv(r->uniforms_bump_l.material[RENDER_MATERIAL_SPC], 1, obj->material.spc);
		glUniform1fv(r->uniforms_bump_l.material[RENDER_MATERIAL_SHN], 1, &obj->material.shn);
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
	param:	tm				texture manager
	param:	window			window to render to
*/
unsigned renderer_init(struct renderer* r, struct texturemanager* tm, struct window* window)
{
	unsigned vert_wire_s, frag_wire_s;
	unsigned vert_txtr_s, frag_txtr_s;
	unsigned vert_mats_l, frag_mats_l;
	unsigned vert_txtr_l, frag_txtr_l;
	unsigned vert_bump_l, frag_bump_l;
	char uniform[16];
	unsigned i, len;

	// initialize shaders
	if (!(vert_wire_s = shader_create(RENDER_SHADER_VERT_WIRE_S, SHADER_VERTEX)))
		return 0;
	if (!(frag_wire_s = shader_create(RENDER_SHADER_FRAG_WIRE_S, SHADER_FRAGMENT)))
		return 0;
	if (!(vert_txtr_s = shader_create(RENDER_SHADER_VERT_TXTR_S, SHADER_VERTEX)))
		return 0;
	if (!(frag_txtr_s = shader_create(RENDER_SHADER_FRAG_TXTR_S, SHADER_FRAGMENT)))
		return 0;
	if (!(vert_mats_l = shader_create(RENDER_SHADER_VERT_MATS_L, SHADER_VERTEX)))
		return 0;
	if (!(frag_mats_l = shader_create(RENDER_SHADER_FRAG_MATS_L, SHADER_FRAGMENT)))
		return 0;
	if (!(vert_txtr_l = shader_create(RENDER_SHADER_VERT_TXTR_L, SHADER_VERTEX)))
		return 0;
	if (!(frag_txtr_l = shader_create(RENDER_SHADER_FRAG_TXTR_L, SHADER_FRAGMENT)))
		return 0;
	if (!(vert_bump_l = shader_create(RENDER_SHADER_VERT_BUMP_L, SHADER_VERTEX)))
		return 0;
	if (!(frag_bump_l = shader_create(RENDER_SHADER_FRAG_BUMP_L, SHADER_FRAGMENT)))
		return 0;

	// create shader programs
	r->id_gl_wire_s = shader_program(vert_wire_s, frag_wire_s);
	r->id_gl_txtr_s = shader_program(vert_txtr_s, frag_txtr_s);
	r->id_gl_mats_l = shader_program(vert_mats_l, frag_mats_l);
	r->id_gl_txtr_l = shader_program(vert_txtr_l, frag_txtr_l);
	r->id_gl_bump_l = shader_program(vert_bump_l, frag_bump_l);

	// bind attribute locations
	glBindAttribLocation(r->id_gl_wire_s, RENDER_ATTRIB_POS, "vertpos");
	glBindAttribLocation(r->id_gl_wire_s, RENDER_ATTRIB_COL, "vertcol");

	glBindAttribLocation(r->id_gl_txtr_s, RENDER_ATTRIB_POS, "vertpos");
	glBindAttribLocation(r->id_gl_txtr_s, RENDER_ATTRIB_TEX, "verttex");

	glBindAttribLocation(r->id_gl_mats_l, RENDER_ATTRIB_POS, "vertpos");
	glBindAttribLocation(r->id_gl_mats_l, RENDER_ATTRIB_NOR, "vertnor");

	glBindAttribLocation(r->id_gl_txtr_l, RENDER_ATTRIB_POS, "vertpos");
	glBindAttribLocation(r->id_gl_txtr_l, RENDER_ATTRIB_NOR, "vertnor");
	glBindAttribLocation(r->id_gl_txtr_l, RENDER_ATTRIB_TEX, "verttex");

	glBindAttribLocation(r->id_gl_bump_l, RENDER_ATTRIB_POS, "vertpos");
	glBindAttribLocation(r->id_gl_bump_l, RENDER_ATTRIB_NOR, "vertnor");
	glBindAttribLocation(r->id_gl_bump_l, RENDER_ATTRIB_TAN, "verttan");
	glBindAttribLocation(r->id_gl_bump_l, RENDER_ATTRIB_TEX, "verttex");

	// link programs
	if (!shader_link(r->id_gl_wire_s))
		return 0;
	if (!shader_link(r->id_gl_txtr_s))
		return 0;
	if (!shader_link(r->id_gl_mats_l))
		return 0;
	if (!shader_link(r->id_gl_txtr_l))
		return 0;
	if (!shader_link(r->id_gl_bump_l))
		return 0;

	// assign vertex sizes to each render type
	r->vertsize[RENDER_TYPE_WIRE_S] = RENDER_VERTSIZE_WIRE_S;
	r->vertsize[RENDER_TYPE_TXTR_S] = RENDER_VERTSIZE_TXTR_S;
	r->vertsize[RENDER_TYPE_MATS_L] = RENDER_VERTSIZE_MATS_L;
	r->vertsize[RENDER_TYPE_TXTR_L] = RENDER_VERTSIZE_TXTR_L;
	r->vertsize[RENDER_TYPE_BUMP_L] = RENDER_VERTSIZE_BUMP_L;

	// assign program ID's to render types
	r->shader[RENDER_TYPE_WIRE_S] = r->id_gl_wire_s;
	r->shader[RENDER_TYPE_TXTR_S] = r->id_gl_txtr_s;
	r->shader[RENDER_TYPE_MATS_L] = r->id_gl_mats_l;
	r->shader[RENDER_TYPE_TXTR_L] = r->id_gl_txtr_l;
	r->shader[RENDER_TYPE_BUMP_L] = r->id_gl_bump_l;

	// register window pointer
	r->window = window;
	r->tm = tm;


	// get wireframe uniform locations
	r->uniforms_wire_s.transform = glGetUniformLocation(r->id_gl_wire_s, "transform");


	// get textured uniform locations
	r->uniforms_txtr_s.transform = glGetUniformLocation(r->id_gl_txtr_s, "transform");

	r->uniforms_txtr_s.tex_diffuse = glGetUniformLocation(r->id_gl_txtr_s, "tex_diffuse");


	// get solid uniform locations
	r->uniforms_mats_l.transform = glGetUniformLocation(r->id_gl_mats_l, "transform");
	r->uniforms_mats_l.eyepos = glGetUniformLocation(r->id_gl_mats_l, "eyepos");

	for (i = 0; i < RENDER_MAX_LIGHTS; i++)
	{
		sprintf_s(uniform, 16, "lights[%d].", i);
		len = strlen(uniform);

		strcpy_s(uniform + len, 4, "pos");
		r->uniforms_mats_l.lights[i][RENDER_LIGHT_POS] = glGetUniformLocation(r->id_gl_mats_l, uniform);
		strcpy_s(uniform + len, 4, "dif");
		r->uniforms_mats_l.lights[i][RENDER_LIGHT_DIF] = glGetUniformLocation(r->id_gl_mats_l, uniform);
		strcpy_s(uniform + len, 4, "spc");
		r->uniforms_mats_l.lights[i][RENDER_LIGHT_SPC] = glGetUniformLocation(r->id_gl_mats_l, uniform);
	}

	r->uniforms_mats_l.ambient = glGetUniformLocation(r->id_gl_mats_l, "ambient");

	r->uniforms_mats_l.material[RENDER_MATERIAL_AMB] = glGetUniformLocation(r->id_gl_mats_l, "material.amb");
	r->uniforms_mats_l.material[RENDER_MATERIAL_DIF] = glGetUniformLocation(r->id_gl_mats_l, "material.dif");
	r->uniforms_mats_l.material[RENDER_MATERIAL_SPC] = glGetUniformLocation(r->id_gl_mats_l, "material.spc");
	r->uniforms_mats_l.material[RENDER_MATERIAL_SHN] = glGetUniformLocation(r->id_gl_mats_l, "material.shn");


	// get diffuse mapped uniform locations
	r->uniforms_txtr_l.transform = glGetUniformLocation(r->id_gl_txtr_l, "transform");
	r->uniforms_txtr_l.eyepos = glGetUniformLocation(r->id_gl_txtr_l, "eyepos");

	for (i = 0; i < RENDER_MAX_LIGHTS; i++)
	{
		sprintf_s(uniform, 16, "lights[%d].", i);
		len = strlen(uniform);

		strcpy_s(uniform + len, 4, "pos");
		r->uniforms_txtr_l.lights[i][RENDER_LIGHT_POS] = glGetUniformLocation(r->id_gl_txtr_l, uniform);
		strcpy_s(uniform + len, 4, "dif");
		r->uniforms_txtr_l.lights[i][RENDER_LIGHT_DIF] = glGetUniformLocation(r->id_gl_txtr_l, uniform);
		strcpy_s(uniform + len, 4, "spc");
		r->uniforms_txtr_l.lights[i][RENDER_LIGHT_SPC] = glGetUniformLocation(r->id_gl_txtr_l, uniform);
	}

	r->uniforms_txtr_l.ambient = glGetUniformLocation(r->id_gl_txtr_l, "ambient");

	r->uniforms_txtr_l.tex_diffuse = glGetUniformLocation(r->id_gl_txtr_l, "tex_diffuse");

	r->uniforms_txtr_l.material[RENDER_MATERIAL_AMB] = glGetUniformLocation(r->id_gl_txtr_l, "material.amb");
	r->uniforms_txtr_l.material[RENDER_MATERIAL_DIF] = glGetUniformLocation(r->id_gl_txtr_l, "material.dif");
	r->uniforms_txtr_l.material[RENDER_MATERIAL_SPC] = glGetUniformLocation(r->id_gl_txtr_l, "material.spc");
	r->uniforms_txtr_l.material[RENDER_MATERIAL_SHN] = glGetUniformLocation(r->id_gl_txtr_l, "material.shn");


	// get bump mapped uniform locations
	r->uniforms_bump_l.transform = glGetUniformLocation(r->id_gl_bump_l, "transform");
	r->uniforms_bump_l.eyepos = glGetUniformLocation(r->id_gl_bump_l, "eyepos");

	for (i = 0; i < RENDER_MAX_LIGHTS; i++)
	{
		sprintf_s(uniform, 16, "lights[%d].", i);
		len = strlen(uniform);

		strcpy_s(uniform + len, 4, "pos");
		r->uniforms_bump_l.lights[i][RENDER_LIGHT_POS] = glGetUniformLocation(r->id_gl_bump_l, uniform);
		strcpy_s(uniform + len, 4, "dif");
		r->uniforms_bump_l.lights[i][RENDER_LIGHT_DIF] = glGetUniformLocation(r->id_gl_bump_l, uniform);
		strcpy_s(uniform + len, 4, "spc");
		r->uniforms_bump_l.lights[i][RENDER_LIGHT_SPC] = glGetUniformLocation(r->id_gl_bump_l, uniform);
	}

	r->uniforms_bump_l.ambient = glGetUniformLocation(r->id_gl_bump_l, "ambient");

	r->uniforms_bump_l.tex_normal = glGetUniformLocation(r->id_gl_bump_l, "tex_normal");

	r->uniforms_bump_l.material[RENDER_MATERIAL_AMB] = glGetUniformLocation(r->id_gl_bump_l, "material.amb");
	r->uniforms_bump_l.material[RENDER_MATERIAL_DIF] = glGetUniformLocation(r->id_gl_bump_l, "material.dif");
	r->uniforms_bump_l.material[RENDER_MATERIAL_SPC] = glGetUniformLocation(r->id_gl_bump_l, "material.spc");
	r->uniforms_bump_l.material[RENDER_MATERIAL_SHN] = glGetUniformLocation(r->id_gl_bump_l, "material.shn");


	// flag shaders for deletion
	shader_delete(vert_wire_s);
	shader_delete(frag_wire_s);
	shader_delete(vert_txtr_s);
	shader_delete(frag_txtr_s);
	shader_delete(vert_mats_l);
	shader_delete(frag_mats_l);
	shader_delete(vert_txtr_l);
	shader_delete(frag_txtr_l);
	shader_delete(vert_bump_l);
	shader_delete(frag_bump_l);

	return 1;
}