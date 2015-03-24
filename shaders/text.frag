#version	140


in		vec3		pass_vertcol;
in		vec2		pass_verttex;

out		vec4		frag_color;

uniform	sampler2D	tex_diffuse;


void main()
{
	float alpha;

	alpha = texture2D(tex_diffuse, pass_verttex).a;

	frag_color = vec4(pass_vertcol * alpha, alpha);
}