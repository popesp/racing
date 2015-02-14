#version	140


in		vec3		vertpos;
in		vec3		vertcol;

out		vec3		pass_vertcol;

uniform	mat4		transform;


void main()
{
	pass_vertcol = vertcol;
	gl_Position = transform*vec4(vertpos, 1.f);
}