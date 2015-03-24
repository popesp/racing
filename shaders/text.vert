#version	140


in		vec3		vertpos;
in		vec3		vertcol;
in		vec2		verttex;

out		vec3		pass_vertcol;
out		vec2		pass_verttex;


void main()
{
	pass_vertcol = vertcol;
	pass_verttex = verttex;
	gl_Position = vec4(vertpos, 1.f);
}