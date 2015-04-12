#version	140


#define	MAX_LIGHTS		8


struct light
{
	vec3	pos;
	vec3	dif;
	vec3	spc;
};


in		vec3		vertpos;
in		vec3		vertnor;
in		vec3		verttan;
in		vec2		verttex;

out		vec3		pass_normal;
out		vec3		pass_tangent;
out		vec2		pass_uv;
out		vec3		pass_toeye;
out		vec3		pass_tolight[MAX_LIGHTS];

uniform	mat4		transform;
uniform	vec3		eyepos;
uniform	int			num_lights;
uniform	light		lights[MAX_LIGHTS];


void main()
{
	int i;
	
	pass_normal = vertnor;
	pass_tangent = verttan;
	pass_uv = verttex;

	pass_toeye = eyepos - vertpos;
	
	for (i = 0; i < num_lights; i++)
		pass_tolight[i] = lights[i].pos - vertpos;
	
	gl_Position = transform*vec4(vertpos, 1.f);
}