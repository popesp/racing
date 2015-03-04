#include	"shader.h"

#include	<GL/glew.h>		// GL
#include	<GLFW/glfw3.h>	// GL
#include	<stdio.h>		// FILE, fopen, fseek, ftell, fread, fclose
#include	"../error.h"	// PRINT_ERROR
#include	"../mem.h"		// calloc, free


/*	create an OpenGL shader object, load source and compile
	param:	filename		file path to load shader source from
	param:	type			type of shader; either SHADER_VERTEX or SHADER_FRAGMENT
	return:	unsigned		0 if shader creation failed, otherwise the OpenGL id of the shader object
*/
unsigned shader_create(const char* filename, unsigned char type)
{
	unsigned id, len;
	char* text;
	char* info;
	FILE* file;
	int result;

	if (type == SHADER_VERTEX)
		id = glCreateShader(GL_VERTEX_SHADER);
	else if (type == SHADER_FRAGMENT)
		id = glCreateShader(GL_FRAGMENT_SHADER);
	else
		return 0u;

	if (fopen_s(&file, filename, "rb"))
	{
		PRINT_ERROR("Couldn't open %s.\n", filename);
		return 0u;
	}

	fseek(file, 0, SEEK_END);
	len = (unsigned)ftell(file);
	fseek(file, 0, SEEK_SET);

	text = (char*)mem_calloc(len + 1, sizeof(char));
	if (fread(text, 1, len, file) != len)
	{
		mem_free(text);
		PRINT_ERROR("Couldn't read from %s.\n", filename);
		return 0u;
	}
	fclose(file);

	glShaderSource(id, 1, (const char**)&text, NULL);
	glCompileShader(id);
	mem_free(text);

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &result);
		info = (char*)mem_calloc((size_t)result, sizeof(char));
		glGetShaderInfoLog(id, result, NULL, info);

		PRINT_ERROR("%s failed to compile:\n%s\n", filename, info);
		mem_free(info);
		return 0u;
	}

	return id;
}

/*	create a shader program and attach shaders
	param:	vertex			OpenGL id of vertex shader object
	param:	fragment		OpenGL id of fragment shader object
	return:	unsigned		OpenGL id of shader program
*/
unsigned shader_program(unsigned vertex, unsigned fragment)
{
	unsigned id;

	id = glCreateProgram();
	glAttachShader(id, vertex);
	glAttachShader(id, fragment);

	return id;
}

/*	link a shader program
	param:	id				OpenGL id of shader program to link
	return:	unsigned		0 if linking failed; 1 otherwise
*/
unsigned shader_link(unsigned id)
{
	char* info;
	int result;

	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &result);
	if (result == GL_FALSE)
	{
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &result);
		info = (char*)mem_calloc((size_t)result, sizeof(char));
		glGetProgramInfoLog(id, result, NULL, info);

		PRINT_ERROR("Failed to link shader program (ID: %d):\n%s\n", id, info);
		mem_free(info);
		return 0u;
	}

	return 1u;
}

/*	delete a shader object
	param:	id				OpenGL id of shader object to delete
*/
void shader_delete(unsigned id)
{
	glDeleteShader(id);
}