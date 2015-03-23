#version	140


in		vec3		pass_vertcol;

out		vec4		fragcol;


void main()
{
	fragcol = vec4(pass_vertcol, 1.f);
}