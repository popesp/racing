#version	140


in		vec3		pass_vertcol;

out		vec3		fragcol;


void main()
{
	fragcol = pass_vertcol;
}