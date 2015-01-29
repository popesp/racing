#version	140


in		vec2		pass_texcoords;

out		vec4		frag_color;

uniform	sampler2D	diffuse;


void main()
{
	frag_color = texture2D(diffuse, pass_texcoords);
}